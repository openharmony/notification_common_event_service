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

#include "os_account_manager_helper.h"

namespace {
int32_t g_MockId = 100; // default id when there is no os_account part
bool g_MockQueryActiveOsAccountIdsRet = true;
bool g_MockGetOsAccountLocalIdFromUidRet = true;
int32_t g_MockIdForGetOsAccountLocalIdFromUid = 100;
}

void MockQueryActiveOsAccountIds(bool mockRet, uint8_t mockCase)
{
    g_MockQueryActiveOsAccountIdsRet = mockRet;
    switch (mockCase) {
        case 1: {
            g_MockId = 101;
            break;
        }
        default: {
            g_MockId = 100;
            break;
        }
    }
}

void ResetAccountMock()
{
    g_MockId = 100;
    g_MockQueryActiveOsAccountIdsRet = true;
    g_MockGetOsAccountLocalIdFromUidRet = true;
    g_MockIdForGetOsAccountLocalIdFromUid = 100;
}

void MockGetOsAccountLocalIdFromUid(bool mockRet, uint8_t mockCase = 0)
{
    g_MockGetOsAccountLocalIdFromUidRet = mockRet;
    switch (mockCase) {
        case 1: { // mock for invalid id
            g_MockIdForGetOsAccountLocalIdFromUid = -2;
            break;
        }
        case 2: { // mock for system id
            g_MockIdForGetOsAccountLocalIdFromUid = 88;
            break;
        }
        default: {
            g_MockIdForGetOsAccountLocalIdFromUid = 100;
            break;
        }
    }
}

namespace OHOS {
namespace EventFwk {
ErrCode OsAccountManagerHelper::QueryActiveOsAccountIds(std::vector<int32_t>& ids)
{
    ids.emplace_back(g_MockId);
    return g_MockQueryActiveOsAccountIdsRet ? ERR_OK : ERR_INVALID_OPERATION;
}

ErrCode OsAccountManagerHelper::GetOsAccountLocalIdFromUid(const int32_t uid, int32_t &id)
{
    id = g_MockIdForGetOsAccountLocalIdFromUid;
    return g_MockGetOsAccountLocalIdFromUidRet ? ERR_OK : ERR_INVALID_OPERATION;
}
}  // namespace EventFwk
}  // namespace OHOS