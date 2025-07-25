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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_H

#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "common_event_listener.h"
#include "icommon_event.h"

namespace OHOS {
namespace EventFwk {
class CommonEvent {
public:
    static std::shared_ptr<CommonEvent> GetInstance();
    /**
     * Publishes a common event.
     *
     * @param data Indicates the common event data.
     * @param publishInfo Indicates the publish info.
     * @param subscriber Indicates the common event subscriber.
     * @return Returns true if successful; false otherwise.
     */
    bool PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber);

    /**
     * Publishes a common event.
     *
     * @param data Indicates the common event data.
     * @param publishInfo Indicates the publish info.
     * @param subscriber Indicates the common event subscriber.
     * @param userId Indicates the user ID.
     * @return Returns ERR_OK if successful; otherwise failed.
     */
    int32_t PublishCommonEventAsUser(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber, const int32_t &userId);

    /**
     * Publishes a common event.
     *
     * @param data Indicates the common event data.
     * @param publishInfo Indicates the publish info.
     * @param subscriber Indicates the common event subscriber.
     * @param uid Indicates the uid of application.
     * @param callerToken Indicates the caller token.
     * @return Returns true if successful; false otherwise.
     */
    bool PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid, const int32_t &callerToken);

    /**
     * Publishes a common event.
     *
     * @param data Indicates the common event data.
     * @param publishInfo Indicates the publish info.
     * @param subscriber Indicates the common event subscriber.
     * @param uid Indicates the uid of application.
     * @param callerToken Indicates the caller token.
     * @param userId Indicates the user ID.
     * @return Returns true if successful; false otherwise.
     */
    bool PublishCommonEventAsUser(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid, const int32_t &callerToken,
        const int32_t &userId);

    /**
     * Subscribes to common events.
     *
     * @param subscriber Indicates the common event subscriber.
     * @return Returns ERR_OK if successful; otherwise failed.
     */
    int32_t SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber);

    /**
     * Unsubscribes from common events.
     *
     * @param subscriber Indicates the common event subscriber.
     * @return Returns true if successful; false otherwise.
     */
    int32_t UnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber);

    /**
     * Synchronized, unsubscribes from common events.
     *
     * @param subscriber Indicates the common event subscriber.
     * @return Returns true if successful; false otherwise.
     */
    int32_t UnSubscribeCommonEventSync(const std::shared_ptr<CommonEventSubscriber> &subscriber);

    /**
     * Gets the current sticky common event.
     *
     * @param event Indicates the common event.
     * @param eventData Indicates the common event data.
     * @return Returns true if successful; false otherwise.
     */
    bool GetStickyCommonEvent(const std::string &event, CommonEventData &eventData);

    /**
     * Finishes Receiver.
     *
     * @param proxy Indicates the receiver proxy.
     * @param code Indicates the code of a common event.
     * @param data Indicates the data of a common event.
     * @param abortEvent Indicates whether to cancel the current common event.
     * @return Returns true if successful; false otherwise.
     */
    bool FinishReceiver(
        const sptr<IRemoteObject> &proxy, const int32_t &code, const std::string &data, const bool &abortEvent);

    /**
     * Dumps state of common event service.
     *
     * @param dumpType Indicates the dump type.
     * @param event Specifies the information for the common event. Set null string ("") if you want to dump all.
     * @param userId Indicates the user ID.
     * @param state Indicates the state of common event service.
     * @return Returns true if successful; false otherwise.
     */
    bool DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
        std::vector<std::string> &state);

    /**
     * Freezes application.
     *
     * @param uid Indicates the uid of application.
     * @return Returns true if successful; false otherwise.
     */
    bool Freeze(const uid_t &uid);

    /**
     * Unfreezes application.
     *
     * @param uid Indicates the uid of application.
     * @return Returns true if successful; false otherwise.
     */
    bool Unfreeze(const uid_t &uid);

    /**
     * Unfreezes all frozen applications.
     *
     * @return Returns true if successful; false otherwise.
     */
    bool UnfreezeAll();

    /**
     * Remove sticky common event.
     *
     * @param event Name of the common event.
     * @return Returns ERR_OK if success; otherwise failed.
     */
    int32_t RemoveStickyCommonEvent(const std::string &event);

    /**
     * Set Static Subscriber State.
     *
     * @param enable static subscriber state.
     * @return Returns ERR_OK if success; otherwise failed.
     */
    int32_t SetStaticSubscriberState(bool enable);

    /**
     * Reconnect common event manager service once per 1000 milliseconds,
     * until the connection succeeds or reaching the max retry times.
     *
     * @return Returns true if successful; false otherwise.
     */
    bool Reconnect();

    /**
     * Resubscribe after common event manager service restarts.
     *
     */
    bool Resubscribe();

    /**
     * Set static subscriber state.
     *
     * @param events Static subscriber event name.
     * @param enable Static subscriber state.
     * @return Returns ERR_OK if success; otherwise failed.
     */
    int32_t SetStaticSubscriberState(const std::vector<std::string> &events, bool enable);

    /**
    * Set freeze status of process.
    *
    * @param pidList Indicates the list of process id.
    * @param isFreeze Indicates wheather the process is freezed.
    * @return Returns true if successful; false otherwise.
    */
    bool SetFreezeStatus(std::set<int> pidList, bool isFreeze);

private:
    /**
     * Gets common event proxy.
     *
     * @return Returns get common event proxy success or not.
     */
    sptr<ICommonEvent> GetCommonEventProxy();

    /**
     * Gets common evenet listener.
     *
     * @param  subscriber Indicates the subscriber.
     * @param  commonEventListener Indicates the common event listener.
     * @return Returns get common event listener success or not.
     */
    uint8_t CreateCommonEventListener(
        const std::shared_ptr<CommonEventSubscriber> &subscriber, sptr<IRemoteObject> &commonEventListener);

    /**
     * Parameter checking for publishing public events.
     *
     * @param data Indicates the common event data.
     * @param publishInfo Indicates the publish info.
     * @param subscriber Indicates the common event subscriber.
     * @return Returns true if successful; false otherwise.
     */
    bool PublishParameterCheck(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber, sptr<IRemoteObject> &commonEventListener);

    void LogCachedSubscriber();

private:
    static std::mutex instanceMutex_;
    static std::shared_ptr<CommonEvent> instance_;
    std::mutex eventListenersMutex_;
    std::map<std::shared_ptr<CommonEventSubscriber>, sptr<CommonEventListener>> eventListeners_;
    const size_t SUBSCRIBER_MAX_SIZE = 200;
    static const uint8_t ALREADY_SUBSCRIBED = 0;
    static const uint8_t INITIAL_SUBSCRIPTION = 1;
    static const uint8_t SUBSCRIBE_FAILED = 2;
    static const uint8_t SUBSCRIBE_EXCEED_LIMIT = 3;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_H
