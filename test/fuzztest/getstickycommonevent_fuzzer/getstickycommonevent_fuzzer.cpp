/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "getstickycommonevent_fuzzer.h"
#include "common_event_manager.h"
#include "fuzz_common_base.h"

namespace OHOS {
    bool DoSomethingInterestingWithMyAPI(FuzzData fuzzData)
    {
        std::string event = fuzzData.GenerateRandomString();
        EventFwk::CommonEventData commonEventData;

        return EventFwk::CommonEventManager::GetStickyCommonEvent(event, commonEventData);
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }
    OHOS::FuzzData fuzzData(data, size);
    OHOS::DoSomethingInterestingWithMyAPI(fuzzData);
    return 0;
}
