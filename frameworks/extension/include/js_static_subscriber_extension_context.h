/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_COMMON_EVENT_SERVICE_JS_STATIC_SUBSCRIBER_EXTENSION_CONTEXT_H
#define OHOS_COMMON_EVENT_SERVICE_JS_STATIC_SUBSCRIBER_EXTENSION_CONTEXT_H

#include <memory>

#include "static_subscriber_extension_context.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace EventFwk {
napi_value CreateJsStaticSubscriberExtensionContext(napi_env env,
    std::shared_ptr<StaticSubscriberExtensionContext> context);
} // namespace EventFwk
} // namespace OHOS
#endif // OHOS_COMMON_EVENT_SERVICE_JS_STATIC_SUBSCRIBER_EXTENSION_CONTEXT_H
