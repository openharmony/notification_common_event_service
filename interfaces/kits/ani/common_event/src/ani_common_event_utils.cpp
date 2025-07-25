/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ani_common_event_utils.h"

#include "event_log_wrapper.h"

namespace OHOS {
namespace EventManagerFwkAni {

using namespace OHOS::EventFwk;
using namespace OHOS::AppExecFwk;
#define SETTER_METHOD_NAME(property) "<set>" #property

void AniCommonEventUtils::GetStdString(ani_env* env, ani_string str, std::string& result)
{
    auto ret = ANI_ERROR;
    ani_size sz {};
    ret = env->String_GetUTF8Size(str, &sz);
    if (ret != ANI_OK) {
        EVENT_LOGE("GetStdString String_GetUTF8Size error. result: %{public}d.", ret);
        return;
    }
    result.resize(sz + 1);
    ret = env->String_GetUTF8SubString(str, 0, sz, result.data(), result.size(), &sz);
    if (ret != ANI_OK) {
        EVENT_LOGE("GetStdString String_GetUTF8Size error. result: %{public}d.", ret);
        return;
    }
    result.resize(sz);
}

void AniCommonEventUtils::GetStdStringArrayClass(ani_env* env, ani_object arrayObj, std::vector<std::string>& strings)
{
    ani_double length;
    auto ret = ANI_ERROR;
    ret = env->Object_GetPropertyByName_Double(arrayObj, "length", &length);
    if (ret != ANI_OK) {
        EVENT_LOGE("GetStdStringArrayClass Object_GetPropertyByName_Double error. result: %{public}d.", ret);
        return;
    }

    for (ani_int i = 0; i < static_cast<ani_int>(length); i++) {
        ani_ref stringEntryRef;
        ret = env->Object_CallMethodByName_Ref(arrayObj, "$_get", "I:Lstd/core/Object;", &stringEntryRef, i);
        if (ret != ANI_OK) {
            EVENT_LOGE("GetStdStringArrayClass Object_CallMethodByName_Ref error. result: %{public}d.", ret);
            return;
        }

        std::string itemStr;
        GetStdString(env, static_cast<ani_string>(stringEntryRef), itemStr);
        strings.emplace_back(itemStr);
    }
}

bool AniCommonEventUtils::GetStringOrUndefined(ani_env* env, ani_object param, const char* name, std::string& res)
{
    ani_ref obj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;

    if ((status = env->Object_GetPropertyByName_Ref(param, name, &obj)) != ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }
    if (isUndefined) {
        EVENT_LOGW("%{public}s : undefined", name);
        return false;
    }

    ani_ref str = nullptr;
    if ((status = env->Object_CallMethodByName_Ref(reinterpret_cast<ani_object>(obj), "toString", nullptr, &str)) !=
        ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }

    GetStdString(env, reinterpret_cast<ani_string>(str), res);
    return true;
}

bool AniCommonEventUtils::GetIntOrUndefined(ani_env* env, ani_object param, const char* name, int32_t& res)
{
    ani_ref obj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;

    if ((status = env->Object_GetPropertyByName_Ref(param, name, &obj)) != ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }
    if (isUndefined) {
        EVENT_LOGW("%{public}s : undefined", name);
        return false;
    }

    ani_int result = 0;
    if ((status = env->Object_CallMethodByName_Int(reinterpret_cast<ani_object>(obj), "intValue", nullptr, &result)) !=
        ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }

    res = static_cast<int32_t>(result);
    return true;
}

bool AniCommonEventUtils::GetBooleanOrUndefined(ani_env* env, ani_object param, const char* name, bool& res)
{
    ani_ref obj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;

    if ((status = env->Object_GetPropertyByName_Ref(param, name, &obj)) != ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }
    if (isUndefined) {
        EVENT_LOGW("%{public}s : undefined", name);
        return false;
    }

    ani_boolean result = 0;
    if ((status = env->Object_CallMethodByName_Boolean(
        reinterpret_cast<ani_object>(obj), "unboxed", nullptr, &result)) != ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }

    res = static_cast<bool>(result);
    return true;
}

bool AniCommonEventUtils::GetStringArrayOrUndefined(
    ani_env* env, ani_object param, const char* name, std::vector<std::string>& res)
{
    ani_ref obj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;

    if ((status = env->Object_GetPropertyByName_Ref(param, name, &obj)) != ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        EVENT_LOGE("status : %{public}d", status);
        return false;
    }
    if (isUndefined) {
        EVENT_LOGW("%{public}s : undefined", name);
        return false;
    }

    GetStdStringArrayClass(env, static_cast<ani_object>(obj), res);
    return true;
}

void AniCommonEventUtils::ConvertCommonEventPublishData(ani_env* env, ani_object optionsObject, Want& want,
    CommonEventData &commonEventData, CommonEventPublishInfo &commonEventPublishInfo)
{
    // Get the code.
    int32_t code;
    if (GetIntOrUndefined(env, optionsObject, "code", code)) {
        commonEventData.SetCode(code);
    }

    // Get the data.
    std::string dataStr;
    if (GetStringOrUndefined(env, optionsObject, "data", dataStr)) {
        commonEventData.SetData(dataStr);
    }

    // Get the isOrdered.
    bool isOrdered;
    if (GetBooleanOrUndefined(env, optionsObject, "isOrdered", isOrdered)) {
        commonEventPublishInfo.SetOrdered(isOrdered);
    }

    // Get the isSticky.
    bool isSticky;
    if (GetBooleanOrUndefined(env, optionsObject, "isSticky", isSticky)) {
        commonEventPublishInfo.SetSticky(isSticky);
    }

    // Get the bundleName.
    std::string bundleNameStr;
    if (GetStringOrUndefined(env, optionsObject, "bundleName", bundleNameStr)) {
        commonEventPublishInfo.SetBundleName(bundleNameStr);
    }

    // Get the subscriberPermissions.
    std::vector<std::string> subscriberPermissionsStr;
    if (GetStringArrayOrUndefined(env, optionsObject, "subscriberPermissions", subscriberPermissionsStr)) {
        commonEventPublishInfo.SetSubscriberPermissions(subscriberPermissionsStr);
    }
}

void AniCommonEventUtils::ConvertCommonEventSubscribeInfo(
    ani_env* env, ani_object infoObject, CommonEventSubscribeInfo& subscribeInfo)
{
    // Get the events.
    std::vector<std::string> eventsStr;
    GetStringArrayOrUndefined(env, infoObject, "events", eventsStr);

    MatchingSkills matchingSkills;
    for (const auto &event : eventsStr) {
        matchingSkills.AddEvent(event);
    }
    CommonEventSubscribeInfo commonEventSubscribeInfo(matchingSkills);

    // Get the publisherPermission.
    std::string publisherPermissionStr;
    if (GetStringOrUndefined(env, infoObject, "publisherPermission", publisherPermissionStr)) {
        commonEventSubscribeInfo.SetPermission(publisherPermissionStr);
    }

    // Get the publisherDeviceId.
    std::string publisherDeviceIdStr;
    if (GetStringOrUndefined(env, infoObject, "publisherDeviceId", publisherDeviceIdStr)) {
        commonEventSubscribeInfo.SetDeviceId(publisherDeviceIdStr);
    }

    // Get the publisherBundleName.
    std::string publisherBundleNameStr;
    if (GetStringOrUndefined(env, infoObject, "publisherBundleName", publisherBundleNameStr)) {
        commonEventSubscribeInfo.SetPublisherBundleName(publisherBundleNameStr);
    }

    // Get the userId.
    int32_t userId;
    if (GetIntOrUndefined(env, infoObject, "userId", userId)) {
        commonEventSubscribeInfo.SetUserId(userId);
    }

    // Get the priority.
    int32_t priority;
    if (GetIntOrUndefined(env, infoObject, "priority", priority)) {
        commonEventSubscribeInfo.SetPriority(priority);
    }

    subscribeInfo = commonEventSubscribeInfo;
}

void AniCommonEventUtils::CreateNewObjectByClass(
    ani_env* env, const char* className, ani_class& cls, ani_object& ani_data)
{
    auto aniResult = ANI_ERROR;
    ani_method ctor = nullptr;
    aniResult = env->FindClass(className, &cls);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CreateNewObjectByClass findClass error. result: %{public}d.", aniResult);
        return;
    }
    if (cls == nullptr) {
        EVENT_LOGE("CreateNewObjectByClass cls is null.");
        return;
    }
    aniResult = env->Class_FindMethod(cls, "<ctor>", ":V", &ctor);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CreateNewObjectByClass Class_FindMethod error. result: %{public}d.", aniResult);
        return;
    }
    aniResult = env->Object_New(cls, ctor, &ani_data);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CreateNewObjectByClass Object_New error. result: %{public}d.", aniResult);
    }
    return;
}

void AniCommonEventUtils::CreateBusinessErrorObject(
    ani_env* env, ani_object& object, int32_t code, const std::string& message)
{
    ani_status aniResult = ANI_ERROR;
    ani_class cls;
    ani_method ctor = nullptr;
    if (env == nullptr) {
        EVENT_LOGE("CreateBusinessErrorObject env is null.");
        return;
    }

    aniResult = env->FindClass("L@ohos/base/BusinessError;", &cls);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CreateBusinessErrorObject findClass error. result: %{public}d.", aniResult);
        return;
    }
    if (cls == nullptr) {
        EVENT_LOGE("CreateBusinessErrorObject cls is null.");
        return;
    }
    aniResult = env->Class_FindMethod(cls, "<ctor>", ":V", &ctor);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CreateBusinessErrorObject Class_FindMethod error. result: %{public}d.", aniResult);
        return;
    }
    aniResult = env->Object_New(cls, ctor, &object);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CreateBusinessErrorObject Object_New error. result: %{public}d.", aniResult);
    }

    ani_string string = nullptr;
    env->String_NewUTF8(message.c_str(), message.size(), &string);
    aniResult = env->Object_SetFieldByName_Double(object, "code", ani_double(code));
    aniResult = env->Object_SetFieldByName_Ref(object, "data", string);
}

template<typename valueType>
void AniCommonEventUtils::CallSetter(
    ani_env* env, ani_class cls, ani_object object, const char* setterName, valueType value)
{
    ani_status aniResult = ANI_ERROR;
    ani_method setter;
    aniResult = env->Class_FindMethod(cls, setterName, nullptr, &setter);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CallSetter Class_FindMethod error. result: %{public}d.", aniResult);
        return;
    }
    aniResult = env->Object_CallMethod_Void(object, setter, value);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CallSetter Object_CallMethod_Void error. result: %{public}d.", aniResult);
    }
    return;
}

void AniCommonEventUtils::CreateAniDoubleObject(ani_env* env, ani_object &object, ani_double value)
{
    ani_status aniResult = ANI_ERROR;
    ani_class clsDouble = nullptr;
    ani_method ctor;
    aniResult = env->FindClass("Lstd/core/Double;", &clsDouble);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CreateAniDoubleObject FindClass error. result: %{public}d.", aniResult);
        return;
    }
    aniResult = env->Class_FindMethod(clsDouble, "<ctor>", "D:V", &ctor);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CreateAniDoubleObject Class_FindMethod error. result: %{public}d.", aniResult);
        return;
    }
    aniResult = env->Object_New(clsDouble, ctor, &object, value);
    if (aniResult != ANI_OK) {
        EVENT_LOGE("CreateAniDoubleObject Object_New error. result: %{public}d.", aniResult);
        return;
    }
}

void AniCommonEventUtils::ConvertCommonEventDataToEts(
    ani_env* env, ani_object& ani_data, const CommonEventData& commonEventData)
{
    ani_class cls = nullptr;
    CreateNewObjectByClass(env, "LcommonEvent/commonEventData/CommonEventDataImpl;", cls, ani_data);
    if ((ani_data == nullptr) || (cls == nullptr)) {
        EVENT_LOGE("ConvertCommonEventDataToEts ani_data or cls is null.");
        return;
    }

    ani_string string = nullptr;
    // set event [string]
    env->String_NewUTF8(
        commonEventData.GetWant().GetAction().c_str(), commonEventData.GetWant().GetAction().size(), &string);
    CallSetter(env, cls, ani_data, SETTER_METHOD_NAME(event), string);

    // set bundleName [string]
    env->String_NewUTF8(
        commonEventData.GetWant().GetBundle().c_str(), commonEventData.GetWant().GetBundle().size(), &string);
    CallSetter(env, cls, ani_data, SETTER_METHOD_NAME(bundleName), string);

    // set data [string]
    env->String_NewUTF8(commonEventData.GetData().c_str(), commonEventData.GetData().size(), &string);
    CallSetter(env, cls, ani_data, SETTER_METHOD_NAME(data), string);

    // set code [number]
    ani_object codeObject;
    CreateAniDoubleObject(env, codeObject, static_cast<ani_double>(commonEventData.GetCode()));
    CallSetter(env, cls, ani_data, SETTER_METHOD_NAME(code), codeObject);

    // set parameters [Record]
}

} // namespace EventManagerFwkAni
} // namespace OHOS