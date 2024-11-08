/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "mock_common_event_stub.h"
#include "refbase.h"
namespace OHOS {
namespace EventFwk {

sptr<MockCommonEventStub> MockCommonEventStub::instance_;
std::mutex MockCommonEventStub::instanceMutex_;

OHOS::sptr<MockCommonEventStub> MockCommonEventStub::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);

    if (instance_ == nullptr) {
        instance_ = new (std::nothrow) MockCommonEventStub();
        if (instance_ == nullptr) {
            return nullptr;
        }
    }

    return instance_;
}

int32_t MockCommonEventStub::PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
    const sptr<IRemoteObject> &commonEventListener, const int32_t &userId)
{
    EVENT_LOGI("enter");

    return ERR_OK;
}

int32_t MockCommonEventStub::SubscribeCommonEvent(const CommonEventSubscribeInfo &subscribeInfo,
    const sptr<IRemoteObject> &commonEventListener, const int32_t instanceKey)
{
    EVENT_LOGI("enter");

    subscribeInfoPtr = std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);

    return ERR_OK;
}

bool MockCommonEventStub::DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
    std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    if (subscribeInfoPtr) {
        // get matchingSkills
        auto matchingSkills = subscribeInfoPtr->GetMatchingSkills();
        // get events
        auto events = matchingSkills.GetEvents();
        EVENT_LOGI("event size %{public}zu", events.size());

        for (auto it : events) {
            state.emplace_back(it);
        }
    } else {
        // do nothing
    }

    subscribeInfoPtr = nullptr;

    return true;
}
} // namespace EventFwk
} // namespace OHOS
