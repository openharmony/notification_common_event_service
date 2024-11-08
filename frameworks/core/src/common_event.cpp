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

#include "common_event.h"
#include "common_event_constant.h"
#include "common_event_death_recipient.h"
#include "event_log_wrapper.h"
#include "hitrace_meter_adapter.h"
#include "icommon_event.h"
#include "iservice_registry.h"
#include "refbase.h"
#include "system_ability_definition.h"
#include "ces_inner_error_code.h"
#include <memory>
#include <mutex>

namespace OHOS {
namespace EventFwk {
namespace {
const int32_t MAX_RETRY_TIME = 30;
const int32_t SLEEP_TIME = 1000;
}

std::mutex CommonEvent::instanceMutex_;
std::shared_ptr<CommonEvent> CommonEvent::instance_;

using namespace OHOS::Notification;

std::shared_ptr<CommonEvent> CommonEvent::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(instanceMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<CommonEvent>();
        }
    }
    return instance_;
}

bool CommonEvent::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGD("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        EVENT_LOGE("failed to check publish parameter");
        return false;
    }
    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    return proxy->PublishCommonEvent(
        data, publishInfo, commonEventListener, UNDEFINED_USER) == ERR_OK ? true : false;
}

int32_t CommonEvent::PublishCommonEventAsUser(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, const int32_t &userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGD("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        EVENT_LOGE("failed to check publish parameter");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }
    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }
    return proxy->PublishCommonEvent(data, publishInfo, commonEventListener, userId);
}

bool CommonEvent::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid, const int32_t &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGD("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        EVENT_LOGE("failed to check publish parameter");
        return false;
    }
    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    return proxy->PublishCommonEvent(data, publishInfo, commonEventListener, uid, callerToken, UNDEFINED_USER);
}

bool CommonEvent::PublishCommonEventAsUser(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid,
    const int32_t &callerToken, const int32_t &userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGD("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        EVENT_LOGE("failed to check publish parameter");
        return false;
    }
    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    return proxy->PublishCommonEvent(data, publishInfo, commonEventListener, uid, callerToken, userId);
}

__attribute__((no_sanitize("cfi"))) bool CommonEvent::PublishParameterCheck(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo, const std::shared_ptr<CommonEventSubscriber> &subscriber,
    sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGD("enter");
    if (data.GetWant().GetAction() == std::string()) {
        EVENT_LOGE("the commonEventdata action is null");
        return false;
    }

    if (!publishInfo.IsOrdered() && (subscriber != nullptr)) {
        EVENT_LOGE("When publishing unordered events, the subscriber object is not required.");
        return false;
    }

    if (subscriber) {
        if (CreateCommonEventListener(subscriber, commonEventListener) == SUBSCRIBE_FAILED) {
            EVENT_LOGE("failed to CreateCommonEventListener");
            return false;
        }
    }

    return true;
}

__attribute__((no_sanitize("cfi"))) int32_t CommonEvent::SubscribeCommonEvent(
    const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGD("enter");

    if (subscriber == nullptr) {
        EVENT_LOGE("the subscriber is null");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }

    if (subscriber->GetSubscribeInfo().GetMatchingSkills().CountEvent() == 0) {
        EVENT_LOGE("the subscriber has no event");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }
    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return ERR_NOTIFICATION_CESM_ERROR;
    }
    DelayedSingleton<CommonEventDeathRecipient>::GetInstance()->SubscribeSAManager();
    sptr<IRemoteObject> commonEventListener = nullptr;
    uint8_t subscribeState = CreateCommonEventListener(subscriber, commonEventListener);
    if (subscribeState == INITIAL_SUBSCRIPTION) {
        auto res = proxy->SubscribeCommonEvent(subscriber->GetSubscribeInfo(),
        commonEventListener, UNDEFINED_INSTANCE_KEY);
        if (res != ERR_OK) {
            EVENT_LOGD("subscribe common event failed, remove event listener");
            std::lock_guard<std::mutex> lock(eventListenersMutex_);
            auto eventListener = eventListeners_.find(subscriber);
            if (eventListener != eventListeners_.end()) {
                eventListeners_.erase(eventListener);
            }
        }
        return res;
    } else if (subscribeState == ALREADY_SUBSCRIBED) {
        return ERR_OK;
    } else {
        return ERR_NOTIFICATION_CES_COMMON_SYSTEMCAP_NOT_SUPPORT;
    }
}

__attribute__((no_sanitize("cfi"))) int32_t CommonEvent::UnSubscribeCommonEvent(
    const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    HITRACE_METER_NAME(HITRACE_TAG_NOTIFICATION, __PRETTY_FUNCTION__);
    EVENT_LOGD("enter");

    if (subscriber == nullptr) {
        EVENT_LOGE("the subscriber is null");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }
    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return ERR_NOTIFICATION_CESM_ERROR;
    }
    std::lock_guard<std::mutex> lock(eventListenersMutex_);
    auto eventListener = eventListeners_.find(subscriber);
    if (eventListener != eventListeners_.end()) {
        EVENT_LOGD("before UnsubscribeCommonEvent listeners size is %{public}zu", eventListeners_.size());
        if (proxy->UnsubscribeCommonEvent(eventListener->second->AsObject()) == ERR_OK) {
            eventListener->second->Stop();
            eventListeners_.erase(eventListener);
            return ERR_OK;
        }
        return ERR_NOTIFICATION_SEND_ERROR;
    } else {
        EVENT_LOGW("No specified subscriber has been registered");
    }

    return ERR_OK;
}

bool CommonEvent::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGD("enter");

    if (event.empty()) {
        EVENT_LOGE("event is empty");
        return false;
    }
    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    return proxy->GetStickyCommonEvent(event, eventData);
}

bool CommonEvent::FinishReceiver(
    const sptr<IRemoteObject> &proxy, const int32_t &code, const std::string &data, const bool &abortEvent)
{
    EVENT_LOGD("enter");

    if (proxy == nullptr) {
        EVENT_LOGE("the proxy is null");
        return false;
    }
    sptr<ICommonEvent> cesProxy = GetCommonEventProxy();
    if (!cesProxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    return cesProxy->FinishReceiver(proxy, code, data, abortEvent);
}

bool CommonEvent::DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
    std::vector<std::string> &state)
{
    EVENT_LOGD("enter");

    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    return proxy->DumpState(dumpType, event, userId, state);
}

bool CommonEvent::Freeze(const uid_t &uid)
{
    EVENT_LOGD("enter");

    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    return proxy->Freeze(uid);
}

bool CommonEvent::Unfreeze(const uid_t &uid)
{
    EVENT_LOGD("enter");

    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    return proxy->Unfreeze(uid);
}

bool CommonEvent::UnfreezeAll()
{
    EVENT_LOGD("enter");

    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }
    return proxy->UnfreezeAll();
}

int32_t CommonEvent::RemoveStickyCommonEvent(const std::string &event)
{
    EVENT_LOGD("enter");

    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("the commonEventProxy is null");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }
    return proxy->RemoveStickyCommonEvent(event);
}

int32_t CommonEvent::SetStaticSubscriberState(bool enable)
{
    EVENT_LOGD("enter");
    sptr<ICommonEvent> proxy_ = GetCommonEventProxy();
    if (!proxy_) {
        EVENT_LOGE("failed to get commonEventProxy");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }
    return proxy_->SetStaticSubscriberState(enable);
}

int32_t CommonEvent::SetStaticSubscriberState(const std::vector<std::string> &events, bool enable)
{
    EVENT_LOGD("Called.");
    sptr<ICommonEvent> proxy_ = GetCommonEventProxy();
    if (!proxy_) {
        EVENT_LOGE("Failed to get common event proxy.");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }
    return proxy_->SetStaticSubscriberState(events, enable);
}

bool CommonEvent::SetFreezeStatus(std::set<int> pidList, bool isFreeze)
{
    EVENT_LOGD("enter");
    sptr<ICommonEvent> proxy_ = GetCommonEventProxy();
    if (!proxy_) {
        EVENT_LOGE("failed to get commonEventProxy");
        return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
    }
    return proxy_->SetFreezeStatus(pidList, isFreeze);
}

sptr<ICommonEvent> CommonEvent::GetCommonEventProxy()
{
    EVENT_LOGD("enter");
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        EVENT_LOGE("Failed to get system ability mgr.");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(COMMON_EVENT_SERVICE_ID);
    if (!remoteObject) {
        EVENT_LOGE("Failed to get COMMON Event Manager.");
        return nullptr;
    }

    auto proxy = iface_cast<ICommonEvent>(remoteObject);
    if (!proxy || !proxy->AsObject()) {
        EVENT_LOGE("Failed to get COMMON Event Manager's proxy");
        return nullptr;
    }
    return proxy;
}

uint8_t CommonEvent::CreateCommonEventListener(
    const std::shared_ptr<CommonEventSubscriber> &subscriber, sptr<IRemoteObject> &commonEventListener)
{
    if (subscriber == nullptr) {
        EVENT_LOGE("subscriber is null");
        return SUBSCRIBE_FAILED;
    }

    std::lock_guard<std::mutex> lock(eventListenersMutex_);

    auto eventListener = eventListeners_.find(subscriber);
    if (eventListener != eventListeners_.end()) {
        commonEventListener = eventListener->second->AsObject();
        EVENT_LOGW("subscriber has common event listener");
        return ALREADY_SUBSCRIBED;
    } else {
        if (eventListeners_.size() == SUBSCRIBER_MAX_SIZE) {
            EVENT_LOGE("the maximum number of subscriptions has been reached");
            return SUBSCRIBE_FAILED;
        }

        sptr<CommonEventListener> listener = new (std::nothrow) CommonEventListener(subscriber);
        if (!listener) {
            EVENT_LOGE("the common event listener is null");
            return SUBSCRIBE_FAILED;
        }
        commonEventListener = listener->AsObject();
        eventListeners_[subscriber] = listener;
    }

    return INITIAL_SUBSCRIPTION;
}

bool CommonEvent::Reconnect()
{
    EVENT_LOGD("enter");
    for (int32_t i = 0; i < MAX_RETRY_TIME; i++) {
        // try to connect ces
        if (!GetCommonEventProxy()) {
            // Sleep 1000 milliseconds before reconnect.
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
            EVENT_LOGE("Resubscribe failed, try again.");
            continue;
        }
        return true;
    }

    return false;
}

__attribute__((no_sanitize("cfi"))) bool CommonEvent::Resubscribe()
{
    EVENT_LOGD("enter");
    sptr<ICommonEvent> proxy = GetCommonEventProxy();
    if (!proxy) {
        EVENT_LOGE("failed to get commonEventProxy");
        return false;
    }

    std::lock_guard<std::mutex> lock(eventListenersMutex_);
    for (auto it = eventListeners_.begin(); it != eventListeners_.end();) {
        auto subscriber = it->first;
        auto listener = it->second;
        int32_t res = proxy->SubscribeCommonEvent(subscriber->GetSubscribeInfo(),
            listener, UNDEFINED_INSTANCE_KEY);
        if (res != ERR_OK) {
            EVENT_LOGW("subscribe common event failed, remove event listener");
            it = eventListeners_.erase(it);
        } else {
            it++;
        }
    }
    return true;
}
}  // namespace EventFwk
}  // namespace OHOS
