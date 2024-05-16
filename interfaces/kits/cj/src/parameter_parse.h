/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PARAMETER_PARSE_H
#define PARAMETER_PARSE_H

#include "common_event_defination.h"
#include "common_event_data.h"

namespace OHOS::CommonEventManager {
    using CommonEventData = OHOS::EventFwk::CommonEventData;
    using WantParams = OHOS::AAFwk::WantParams;
    void GetCommonEventData(const CommonEventData &data, CCommonEventData &cData);
    void charPtrToVector(char **charPtr, int size, std::vector<std::string> &result);
    void SetFdData(CParameters *head, WantParams &wantP);
    void SetDataParameters(CArrParameters parameters, WantParams &wantP);
}

#endif