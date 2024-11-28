/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common_event_manager_service.h"

#include "ability_manager_helper.h"
#include "access_token_helper.h"
#include "accesstoken_kit.h"
#include "bundle_manager_helper.h"
#include "common_event_constant.h"
#include "datetime_ex.h"
#include "event_log_wrapper.h"
#include "hitrace_meter_adapter.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include "publish_manager.h"
#include "refbase.h"
#include "system_ability_definition.h"
#include "os_account_manager_helper.h"
#include "xcollie/watchdog.h"
#include "ces_inner_error_code.h"
#include "system_time.h"
#include <mutex>
#include <new>

namespace OHOS {
namespace EventFwk {
namespace {
const std::string NOTIFICATION_CES_CHECK_SA_PERMISSION = "notification.ces.check.sa.permission";
}  // namespace

using namespace OHOS::Notification;

sptr<CommonEventManagerService> CommonEventManagerService::instance_;
std::mutex CommonEventManagerService::instanceMutex_;

sptr<CommonEventManagerService> CommonEventManagerService::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(instanceMutex_);
        if (instance_ == nullptr) {
            instance_ = new (std::nothrow) CommonEventManagerService();
        }
    }
    return instance_;
}

CommonEventManagerService::CommonEventManagerService()
    : serviceRunningState_(ServiceRunningState::STATE_NOT_START),
      runner_(nullptr),
      handler_(nullptr)
{
    supportCheckSaPermission_ = OHOS::system::GetParameter(NOTIFICATION_CES_CHECK_SA_PERMISSION, "false");
    EVENT_LOGD("instance created");
}

CommonEventManagerService::~CommonEventManagerService()
{
    EVENT_LOGD("instance destroyed");
}

ErrCode __attribute__((weak)) CommonEventManagerService::Init()
{
    EVENT_LOGD("ready to init");
    innerCommonEventManager_ = std::make_shared<InnerCommonEventManager>();
    if (!innerCommonEventManager_) {
        EVENT_LOGE("Failed to init without inner service");
        return ERR_INVALID_OPERATION;
    }

    commonEventSrvQueue_ = std::make_shared<ffrt::queue>("CesSrvMain");
    serviceRunningState_ = ServiceRunningState::STATE_RUNNING;

    return ERR_OK;
}

bool CommonEventManagerService::IsReady() const
{
    if (!innerCommonEventManager_) {
        EVENT_LOGE("innerCommonEventManager is null");
        return false;
    }

    if (!commonEventSrvQueue_) {
        EVENT_LOGE("queue object is null");
        return false;
    }

    return true;
}

int32_t CommonEventManagerService::PublishCommonEvent(const CommonEventData &event,
    const CommonEventPublishInfo &publishinfo, const sptr<IRemoteObject> &commonEventListener,
    const int32_t &userId)
{
    EVENT_LOGD("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return ERR_NOTIFICATION_CESM_ERROR;
    }

    if (userId != ALL_USER && userId != CURRENT_USER && userId != UNDEFINED_USER) {
        bool isSubsystem = AccessTokenHelper::VerifyNativeToken(IPCSkeleton::GetCallingTokenID());
        if (!isSubsystem && !AccessTokenHelper::IsSystemApp()) {
            EVENT_LOGE("publish to special user must be system application.");
            return ERR_NOTIFICATION_CES_COMMON_NOT_SYSTEM_APP;
        }
    }

    return PublishCommonEventDetailed(event,
        publishinfo,
        commonEventListener,
        IPCSkeleton::GetCallingPid(),
        IPCSkeleton::GetCallingUid(),
        IPCSkeleton::GetCallingTokenID(),
        userId);
}

bool CommonEventManagerService::PublishCommonEvent(const CommonEventData &event,
    const CommonEventPublishInfo &publishinfo, const sptr<IRemoteObject> &commonEventListener, const uid_t &uid,
    const int32_t &callerToken, const int32_t &userId)
{
    EVENT_LOGD("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != FOUNDATION_UID) {
        EVENT_LOGE("Only foundation can publish common event as proxy uid = %{public}d.", callingUid);
        return false;
    }

    return PublishCommonEventDetailed(
        event, publishinfo, commonEventListener, UNDEFINED_PID, uid, callerToken, userId) == ERR_OK ? true : false;
}

int32_t CommonEventManagerService::PublishCommonEventDetailed(const CommonEventData &event,
    const CommonEventPublishInfo &publishinfo, const sptr<IRemoteObject> &commonEventListener, const pid_t &pid,
    const uid_t &uid, const int32_t &clientToken, const int32_t &userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);

    if (AccessTokenHelper::IsDlpHap(clientToken)) {
        EVENT_LOGE("DLP hap not allowed to send common event");
        return ERR_NOTIFICATION_CES_NOT_SA_SYSTEM_APP;
    }
    struct tm recordTime = {0};
    if (!GetSystemCurrentTime(&recordTime)) {
        EVENT_LOGE("Failed to GetSystemCurrentTime");
        return ERR_NOTIFICATION_SYS_ERROR;
    }
    int32_t errCode = CheckUserIdParams(userId);
    if (errCode != ERR_OK) {
        return errCode;
    }

    if (DelayedSingleton<PublishManager>::GetInstance()->CheckIsFloodAttack(uid)) {
        EVENT_LOGE("Too many common events have been sent in a short period from (pid = %{public}d, uid = "
                   "%{public}d, userId = %{public}d)", pid, uid, userId);
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    wptr<CommonEventManagerService> weakThis = this;
    std::function<void()> publishCommonEventFunc = [wp,
        event,
        publishinfo,
        commonEventListener,
        recordTime,
        pid,
        uid,
        clientToken,
        userId,
        weakThis] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        std::string bundleName = "";
        if (!AccessTokenHelper::VerifyNativeToken(clientToken)) {
            bundleName = DelayedSingleton<BundleManagerHelper>::GetInstance()->GetBundleName(uid);
        }
        sptr<CommonEventManagerService> commonEventManagerService = weakThis.promote();
        if (commonEventManagerService == nullptr) {
            EVENT_LOGE("CommonEventManager not exist");
            return;
        }
        bool ret = innerCommonEventManager->PublishCommonEvent(event,
            publishinfo,
            commonEventListener,
            recordTime,
            pid,
            uid,
            clientToken,
            userId,
            bundleName,
            commonEventManagerService);
        if (!ret) {
            EVENT_LOGE("failed to publish event %{public}s", event.GetWant().GetAction().c_str());
        }
    };
    EVENT_LOGI("Start to submit publish commonEvent <%{public}d>", uid);
    commonEventSrvQueue_->submit(publishCommonEventFunc);
    return ERR_OK;
}

int32_t CommonEventManagerService::SubscribeCommonEvent(const CommonEventSubscribeInfo &subscribeInfo,
    const sptr<IRemoteObject> &commonEventListener, const int32_t instanceKey)
{
    HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGD("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    struct tm recordTime = {0};
    if (!GetSystemCurrentTime(&recordTime)) {
        EVENT_LOGE("Failed to GetSystemCurrentTime");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    int32_t errCode = CheckUserIdParams(subscribeInfo.GetUserId());
    if (errCode != ERR_OK) {
        return errCode;
    }

    auto callingUid = IPCSkeleton::GetCallingUid();
    auto callingPid = IPCSkeleton::GetCallingPid();
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    std::string bundleName = "";
    if (!AccessTokenHelper::VerifyNativeToken(callerToken)) {
        bundleName = DelayedSingleton<BundleManagerHelper>::GetInstance()->GetBundleName(callingUid);
    }
    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    int64_t startTime = SystemTime::GetNowSysTime();
    std::function<void()> subscribeCommonEventFunc = [wp,
        subscribeInfo,
        commonEventListener,
        recordTime,
        callingPid,
        callingUid,
        callerToken,
        bundleName,
        instanceKey,
        startTime] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        bool ret = innerCommonEventManager->SubscribeCommonEvent(subscribeInfo,
            commonEventListener,
            recordTime,
            callingPid,
            callingUid,
            callerToken,
            bundleName,
            instanceKey,
            startTime);
        if (!ret) {
            EVENT_LOGE("failed to subscribe event");
        }
    };

    EVENT_LOGI("Start to submit subscribe commonEvent <%{public}d>", callingUid);
    commonEventSrvQueue_->submit(subscribeCommonEventFunc);
    return ERR_OK;
}

int32_t CommonEventManagerService::UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener)
{
    HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGD("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> unsubscribeCommonEventFunc = [wp, commonEventListener] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        bool ret = innerCommonEventManager->UnsubscribeCommonEvent(commonEventListener);
        if (!ret) {
            EVENT_LOGE("failed to unsubscribe event");
        }
    };

    commonEventSrvQueue_->submit(unsubscribeCommonEventFunc);
    return ERR_OK;
}

bool CommonEventManagerService::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGD("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    if (event.empty()) {
        EVENT_LOGE("event is empty");
        return false;
    }
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    bool isSubsystem = AccessTokenHelper::VerifyNativeToken(callerToken);
    if (!isSubsystem && !AccessTokenHelper::IsSystemApp()) {
        EVENT_LOGE("Not system application or subsystem request.");
        return ERR_NOTIFICATION_CES_COMMON_NOT_SYSTEM_APP;
    }
    auto callingUid = IPCSkeleton::GetCallingUid();
    std::string bundleName = DelayedSingleton<BundleManagerHelper>::GetInstance()->GetBundleName(callingUid);
    const std::string permission = "ohos.permission.COMMONEVENT_STICKY";
    bool ret = AccessTokenHelper::VerifyAccessToken(callerToken, permission);
    if (!ret) {
        EVENT_LOGE("No permission to get a sticky common event from %{public}s (uid = %{public}d)",
            bundleName.c_str(),
            callingUid);
        return false;
    }

    return innerCommonEventManager_->GetStickyCommonEvent(event, eventData);
}

bool CommonEventManagerService::DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
    std::vector<std::string> &state)
{
    EVENT_LOGD("enter");

    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (!AccessTokenHelper::VerifyShellToken(callerToken) && !AccessTokenHelper::VerifyNativeToken(callerToken)) {
        EVENT_LOGE("Not subsystem or shell request");
        return false;
    }
    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    innerCommonEventManager_->DumpState(dumpType, event, userId, state);

    return true;
}

bool CommonEventManagerService::FinishReceiver(
    const sptr<IRemoteObject> &proxy, const int32_t &code, const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGD("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }
    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> finishReceiverFunc = [wp, proxy, code, receiverData, abortEvent] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        innerCommonEventManager->FinishReceiver(proxy, code, receiverData, abortEvent);
    };

    commonEventSrvQueue_->submit(finishReceiverFunc);
    return true;
}

bool CommonEventManagerService::Freeze(const uid_t &uid)
{
    EVENT_LOGD("enter");

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!AccessTokenHelper::VerifyNativeToken(tokenId) ||
        AccessTokenHelper::GetCallingProcessName(tokenId) != RESOURCE_MANAGER_PROCESS_NAME) {
        EVENT_LOGE("Not subsystem request");
        return false;
    }

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }
    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> freezeFunc = [wp, uid] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        innerCommonEventManager->Freeze(uid);
    };

    commonEventSrvQueue_->submit(freezeFunc);
    return true;
}

bool CommonEventManagerService::Unfreeze(const uid_t &uid)
{
    EVENT_LOGD("enter");

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!AccessTokenHelper::VerifyNativeToken(tokenId) ||
        AccessTokenHelper::GetCallingProcessName(tokenId) != RESOURCE_MANAGER_PROCESS_NAME) {
        EVENT_LOGE("Not subsystem request");
        return false;
    }

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> unfreezeFunc = [wp, uid] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        innerCommonEventManager->Unfreeze(uid);
    };

    commonEventSrvQueue_->submit(unfreezeFunc);
    return true;
}

bool CommonEventManagerService::UnfreezeAll()
{
    EVENT_LOGD("enter");

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!AccessTokenHelper::VerifyNativeToken(tokenId) ||
        AccessTokenHelper::GetCallingProcessName(tokenId) != RESOURCE_MANAGER_PROCESS_NAME) {
        EVENT_LOGE("Not subsystem request");
        return false;
    }
    
    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> unfreezeAllFunc = [wp] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        innerCommonEventManager->UnfreezeAll();
    };

    commonEventSrvQueue_->submit(unfreezeAllFunc);
    return true;
}

int CommonEventManagerService::Dump(int fd, const std::vector<std::u16string> &args)
{
    EVENT_LOGD("enter");

    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (!AccessTokenHelper::VerifyShellToken(callerToken) && !AccessTokenHelper::VerifyNativeToken(callerToken)) {
        EVENT_LOGE("Not subsystem or shell request");
        return ERR_NOTIFICATION_CES_COMMON_PERMISSION_DENIED;
    }
    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return ERR_INVALID_VALUE;
    }
    std::string result;
    innerCommonEventManager_->HiDump(args, result);
    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        EVENT_LOGE("dprintf error");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t CommonEventManagerService::RemoveStickyCommonEvent(const std::string &event)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return ERR_NOTIFICATION_CESM_ERROR;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    bool isSubsystem = AccessTokenHelper::VerifyNativeToken(tokenId);
    if (!isSubsystem && !AccessTokenHelper::IsSystemApp()) {
        EVENT_LOGE("Not system application or subsystem request.");
        return ERR_NOTIFICATION_CES_COMMON_NOT_SYSTEM_APP;
    }

    const std::string permission = "ohos.permission.COMMONEVENT_STICKY";
    bool ret = AccessTokenHelper::VerifyAccessToken(tokenId, permission);
    if (!ret && (!isSubsystem || supportCheckSaPermission_.compare("true") == 0)) {
        EVENT_LOGE("No permission.");
        return ERR_NOTIFICATION_CES_COMMON_PERMISSION_DENIED;
    }

    return innerCommonEventManager_->RemoveStickyCommonEvent(event, IPCSkeleton::GetCallingUid());
}

int32_t CommonEventManagerService::SetStaticSubscriberState(bool enable)
{
    if (!AccessTokenHelper::IsSystemApp()) {
        EVENT_LOGE("Not system application");
        return ERR_NOTIFICATION_CES_COMMON_NOT_SYSTEM_APP;
    }

    return innerCommonEventManager_->SetStaticSubscriberState(enable);
}

int32_t CommonEventManagerService::SetStaticSubscriberState(const std::vector<std::string> &events, bool enable)
{
    if (!AccessTokenHelper::IsSystemApp()) {
        EVENT_LOGE("Not system application.");
        return ERR_NOTIFICATION_CES_COMMON_NOT_SYSTEM_APP;
    }
    return innerCommonEventManager_->SetStaticSubscriberState(events, enable);
}

bool CommonEventManagerService::SetFreezeStatus(std::set<int> pidList, bool isFreeze)
{
    EVENT_LOGD("enter");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!AccessTokenHelper::VerifyNativeToken(tokenId) ||
        AccessTokenHelper::GetCallingProcessName(tokenId) != RESOURCE_MANAGER_PROCESS_NAME) {
        EVENT_LOGE("Not subsystem request");
        return false;
    }
    
    if (!IsReady()) {
        EVENT_LOGE("CommonEventManagerService not ready");
        return false;
    }

    std::weak_ptr<InnerCommonEventManager> wp = innerCommonEventManager_;
    std::function<void()> setFreezeStatusFunc = [wp, pidList, isFreeze] () {
        std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = wp.lock();
        if (innerCommonEventManager == nullptr) {
            EVENT_LOGE("innerCommonEventManager not exist");
            return;
        }
        innerCommonEventManager->SetFreezeStatus(pidList, isFreeze);
    };

    commonEventSrvQueue_->submit(setFreezeStatusFunc);
    return true;
}

int32_t CommonEventManagerService::CheckUserIdParams(const int32_t &userId)
{
    if (userId != ALL_USER && userId != CURRENT_USER && userId != UNDEFINED_USER
        && !OsAccountManagerHelper::GetInstance()->CheckUserExists(userId)) {
        EVENT_LOGE("UserId %{public}d is not exists in OsAccount", userId);
        return ERR_NOTIFICATION_CES_USERID_INVALID;
    }
    return ERR_OK;
}
}  // namespace EventFwk
}  // namespace OHOS
