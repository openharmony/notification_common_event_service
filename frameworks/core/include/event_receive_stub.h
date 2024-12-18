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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_EVENT_RECEIVE_STUB_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_EVENT_RECEIVE_STUB_H

#include <mutex>

#include "common_event_service_ipc_interface_code.h"
#include "ievent_receive.h"
#include "iremote_stub.h"

namespace OHOS {
namespace EventFwk {
class EventReceiveStub : public IRemoteStub<IEventReceive> {
public:
    EventReceiveStub();

    virtual ~EventReceiveStub() override;

    /**
     * Processes the remote Request.
     *
     * @param code Indicates the code to send.
     * @param data Indicates the message to send.
     * @param reply Indicates the message to reply.
     * @param option Indicates the message option.
     * @return Returns ERR_NONE if success, otherwise ERR_CODE.
     */
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static std::mutex mutex_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_EVENT_RECEIVE_STUB_H
