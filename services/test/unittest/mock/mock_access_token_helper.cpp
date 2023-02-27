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

#include "access_token_helper.h"

namespace {
    bool g_mockVerifyNativeTokenRet = true;
    bool g_mockVerifyAccessTokenRet = true;
}

void mockVerifyNativeToken(bool mockRet)
{
    g_mockVerifyNativeTokenRet = mockRet;
}

void mockVerifyAccessToken(bool mockRet)
{
    g_mockVerifyAccessTokenRet = mockRet;
}

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace EventFwk {
bool AccessTokenHelper::VerifyNativeToken(const AccessTokenID &callerToken)
{
    return g_mockVerifyNativeTokenRet;
}

bool AccessTokenHelper::VerifyAccessToken(const AccessTokenID &callerToken,
    const std::string &permission)
{
    return g_mockVerifyAccessTokenRet;
}

bool AccessTokenHelper::IsSystemApp()
{
    return true;
}
}  // namespace EventFwk
}  // namespace OHOS
