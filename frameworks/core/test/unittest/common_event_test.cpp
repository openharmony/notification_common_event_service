/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "ces_inner_error_code.h"
#define private public
#include "common_event.h"
#include "common_event_death_recipient.h"
#undef private
#include "common_event_publish_info.h"
#include "mock_common_event_stub.h"
#include "common_event_subscriber.h"
#include "event_receive_proxy.h"
#include "matching_skills.h"

using namespace testing::ext;
using namespace OHOS::EventFwk;

namespace {
    const std::string EVENT = "com.ces.test.event";
    const std::string PERMISSION = "com.ces.test.permission";
    constexpr uint16_t SYSTEM_UID = 1000;
    constexpr int32_t ERR_OK = 0;
}

class CommonEventTest : public CommonEventSubscriber, public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

void CommonEventTest::SetUpTestCase()
{}

void CommonEventTest::TearDownTestCase()
{}

void CommonEventTest::SetUp()
{}

void CommonEventTest::TearDown()
{}

/*
 * tc.number: CommonEvent_001
 * tc.name: test PublishCommonEvent
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke PublishCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_001, TestSize.Level0)
{
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    std::shared_ptr<CommonEventSubscriber> subscriber = nullptr;

    CommonEvent commonEvent;
    bool publishCommonEvent = commonEvent.PublishCommonEvent(data, publishInfo, subscriber);
    EXPECT_EQ(true, publishCommonEvent);
}

/*
 * tc.number: CommonEvent_002
 * tc.name: test PublishCommonEvent
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke PublishCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_002, TestSize.Level0)
{
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    std::shared_ptr<CommonEventSubscriber> subscriber = nullptr;

    // only sa can publish event as proxy, so return false.
    CommonEvent commonEvent;
    bool publishCommonEvent = commonEvent.PublishCommonEvent(data, publishInfo, subscriber, SYSTEM_UID, 0);
    EXPECT_EQ(false, publishCommonEvent);
}

/*
 * tc.number: CommonEvent_003
 * tc.name: test Freeze
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke Freeze interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_003, TestSize.Level0)
{
    CommonEvent commonEvent;
    bool freeze = commonEvent.Freeze(SYSTEM_UID);
    EXPECT_EQ(false, freeze);
}

/*
 * tc.number: CommonEvent_004
 * tc.name: test Unfreeze
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke Unfreeze interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_004, TestSize.Level0)
{
    CommonEvent commonEvent;
    bool unfreeze = commonEvent.Unfreeze(SYSTEM_UID);
    EXPECT_EQ(false, unfreeze);
}

/*
 * tc.number: CommonEvent_005
 * tc.name: test Unfreeze
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke Unfreeze interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_005, TestSize.Level0)
{
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    std::shared_ptr<EventReceiveProxy> result = std::make_shared<EventReceiveProxy>(nullptr);
    if (result != nullptr) {
        result->NotifyEvent(data, true, true);
    }

    CommonEvent commonEvent;
    bool unfreezeAll = commonEvent.UnfreezeAll();
    EXPECT_EQ(false, unfreezeAll);
}

/*
 * tc.number: CommonEvent_006
 * tc.name: test SetFreezeStatus
 * tc.type: FUNC
 * tc.require: issueI5NGO7
 * tc.desc: Invoke SetFreezeStatus interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEvent_006, TestSize.Level0)
{
    CommonEvent commonEvent;
    std::set<int> pidList = {1000};
    bool freeze = commonEvent.SetFreezeStatus(pidList, true);
    EXPECT_EQ(false, freeze);
}

/*
 * tc.number: CommonEventStub_001
 * tc.name: test PublishCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke PublishCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_001, TestSize.Level0)
{
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    const int32_t userId = 1;

    MockCommonEventStub commonEventStub;
    int32_t publishCommonEvent = -1;
    commonEventStub.PublishCommonEvent(data, publishInfo, nullptr, userId, publishCommonEvent);
    EXPECT_EQ(ERR_OK, publishCommonEvent);
}

/*
 * tc.number: CommonEventStub_002
 * tc.name: test PublishCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke PublishCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_002, TestSize.Level0)
{
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    const int32_t userId = 1;

    MockCommonEventStub commonEventStub;
    bool publishCommonEvent = false;
    commonEventStub.PublishCommonEvent(data, publishInfo, nullptr, SYSTEM_UID, 0, userId, publishCommonEvent);
    EXPECT_EQ(true, publishCommonEvent);
}

/*
 * tc.number: CommonEventStub_003
 * tc.name: test SubscribeCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke SubscribeCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_003, TestSize.Level0)
{
    CommonEventSubscribeInfo subscribeInfo;

    MockCommonEventStub commonEventStub;
    int32_t subscribeCommonEvent = -1;
    commonEventStub.SubscribeCommonEvent(subscribeInfo, nullptr, 0, subscribeCommonEvent);
    EXPECT_EQ(ERR_OK, subscribeCommonEvent);
}

/*
 * tc.number: CommonEventStub_004
 * tc.name: test UnsubscribeCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke UnsubscribeCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_004, TestSize.Level0)
{
    MockCommonEventStub commonEventStub;
    int32_t unsubscribeCommonEvent = -1;
    commonEventStub.UnsubscribeCommonEvent(nullptr, unsubscribeCommonEvent);
    EXPECT_EQ(true, static_cast<bool>(unsubscribeCommonEvent));
}

/*
 * tc.number: CommonEventStub_005
 * tc.name: test GetStickyCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke GetStickyCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_005, TestSize.Level0)
{
    const std::string event = "aa";

    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    MockCommonEventStub commonEventStub;
    bool getStickyCommonEvent = false;
    commonEventStub.GetStickyCommonEvent(event, data, getStickyCommonEvent);
    EXPECT_EQ(true, getStickyCommonEvent);
}

/*
 * tc.number: CommonEventStub_006
 * tc.name: test DumpState
 * tc.type: FUNC
 * tc.desc: Invoke DumpState interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_006, TestSize.Level0)
{
    const uint8_t dumpType = 1;
    const int32_t userId = 2;
    std::vector<std::string> state;

    MockCommonEventStub commonEventStub;
    bool dumpState = false;
    commonEventStub.DumpState(dumpType, EVENT, userId, state, dumpState);
    EXPECT_EQ(true, dumpState);
}

/*
 * tc.number: CommonEventStub_007
 * tc.name: test FinishReceiver
 * tc.type: FUNC
 * tc.desc: Invoke FinishReceiver interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_007, TestSize.Level0)
{
    const int32_t code = 1;
    const std::string receiverData = "bb";

    MockCommonEventStub commonEventStub;
    bool finishReceiver = false;
    commonEventStub.FinishReceiver(nullptr, code, receiverData, true, finishReceiver);
    EXPECT_EQ(true, finishReceiver);
}

/*
 * tc.number: CommonEventStub_008
 * tc.name: test Freeze Unfreeze UnfreezeAll
 * tc.type: FUNC
 * tc.desc: Invoke Freeze Unfreeze UnfreezeAll interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_008, TestSize.Level0)
{
    MockCommonEventStub commonEventStub;
    bool freeze = false;
    commonEventStub.Freeze(SYSTEM_UID, freeze);
    EXPECT_EQ(true, freeze);
    bool unfreeze = false;
    commonEventStub.Unfreeze(SYSTEM_UID, unfreeze);
    EXPECT_EQ(true, unfreeze);
    bool unfreezeAll = false;
    commonEventStub.UnfreezeAll(unfreezeAll);
    EXPECT_EQ(true, unfreezeAll);
}

/*
 * tc.number: CommonEventStub_009
 * tc.name: test OnRemoteRequest
 * tc.type: FUNC
 * tc.desc: Invoke OnRemoteRequest interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_009, TestSize.Level0)
{
    const uint32_t code = -1;
    OHOS::MessageParcel dataParcel;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    MockCommonEventStub commonEventStub;
    int result = commonEventStub.OnRemoteRequest(code, dataParcel, reply, option);
    EXPECT_EQ(OHOS::ERR_TRANSACTION_FAILED, result);
}

/*
 * tc.number: CommonEventStub_0010
 * tc.name: test SetFreezeStatus
 * tc.type: FUNC
 * tc.desc: Invoke SetFreezeStatus interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_0010, TestSize.Level0)
{
    MockCommonEventStub commonEventStub;
    std::set<int32_t> pidList = {1000};
    bool result = false;
    commonEventStub.SetFreezeStatus(pidList, true, result);
    EXPECT_EQ(false, result);
}

/*
 * tc.number: CommonEventStub_004
 * tc.name: test UnsubscribeCommonEvent
 * tc.type: FUNC
 * tc.desc: Invoke UnsubscribeCommonEvent interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventStub_0011, TestSize.Level0)
{
    MockCommonEventStub commonEventStub;
    int32_t unsubscribeCommonEvent = false;
    commonEventStub.UnsubscribeCommonEventSync(nullptr, unsubscribeCommonEvent);
    EXPECT_EQ(true, static_cast<bool>(unsubscribeCommonEvent));
}

/*
 * tc.number: MatchingSkills_001
 * tc.name: test Unmarshalling
 * tc.type: FUNC
 * tc.desc: Invoke Unmarshalling interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, MatchingSkills_001, TestSize.Level0)
{
    bool unmarshalling = true;
    OHOS::Parcel parcel;
    std::shared_ptr<MatchingSkills> result = std::make_shared<MatchingSkills>();

    // write entity
    int32_t value = 1;
    parcel.WriteInt32(value);
    std::vector<std::u16string> actionU16Entity;
    for (std::vector<std::string>::size_type i = 0; i < 3; i++) {
        actionU16Entity.emplace_back(OHOS::Str8ToStr16("test"));
    }
    result->WriteVectorInfo(parcel, actionU16Entity);

    //write event
    parcel.WriteInt32(value);
    std::vector<std::u16string> actionU16Event;
    for (std::vector<std::string>::size_type i = 0; i < 3; i++) {
        actionU16Event.emplace_back(OHOS::Str8ToStr16("test"));
    }
    result->WriteVectorInfo(parcel, actionU16Event);

    // write scheme
    parcel.WriteInt32(value);
    std::vector<std::u16string> actionU16Scheme;
    for (std::vector<std::string>::size_type i = 0; i < 3; i++) {
        actionU16Scheme.emplace_back(OHOS::Str8ToStr16("test"));
    }
    result->WriteVectorInfo(parcel, actionU16Scheme);

    if (nullptr == result->Unmarshalling(parcel)) {
        unmarshalling = false;
    }
    EXPECT_EQ(true, unmarshalling);
}

/*
 * @tc.number: CommonEventPublishInfo_0100
 * @tc.name: verify SetBundleName
 * @tc.desc: Invoke SetBundleName interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventPublishInfo_0100, Function | MediumTest | Level0)
{
    const std::string bundleName = "aa";
    CommonEventPublishInfo publishInfo;
    publishInfo.SetBundleName(bundleName);
    std::string getBundleName = publishInfo.GetBundleName();
    EXPECT_EQ(bundleName, getBundleName);
}

/*
 * @tc.number: CommonEventSubscribeInfo_0100
 * @tc.name: verify Unmarshalling
 * @tc.desc: Invoke Unmarshalling interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventSubscribeInfo_0100, Function | MediumTest | Level0)
{
    bool result = false;
    OHOS::Parcel parcel;
    std::string eventName = "";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    if (nullptr == subscribeInfo.Unmarshalling(parcel)) {
        result = true;
    }
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventPublishInfo_0300
 * @tc.name: verify SetSubscriberType
 * @tc.desc: Invoke SetSubscriberType interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventPublishInfo_0300, Function | MediumTest | Level0)
{
    int32_t subscribeType = 1;
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSubscriberType(subscribeType);
    EXPECT_EQ(subscribeType, publishInfo.GetSubscriberType());
}

/*
 * @tc.number: CommonEventSubscriber_0102
 * @tc.name: verify IsStickyCommonEvent
 * @tc.desc: Invoke OnAddSystemAbility interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, CommonEventSubscriber_0102, Function | MediumTest | Level0)
{
    auto commonEventDeathRecipient = OHOS::DelayedSingleton<CommonEventDeathRecipient>::GetInstance();
    commonEventDeathRecipient->SubscribeSAManager();
    commonEventDeathRecipient->statusChangeListener_->OnRemoveSystemAbility(0, "");
    commonEventDeathRecipient->statusChangeListener_->OnAddSystemAbility(0, "");
    bool result = CommonEventTest::IsStickyCommonEvent();
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: SetStaticSubscriberState_0100
 * @tc.name: verify SetStaticSubscriberState
 * @tc.desc: Invoke SetStaticSubscriberState interface verify whether it is normal
 */
HWTEST_F(CommonEventTest, SetStaticSubscriberState_0100, Function | SmallTest | Level0)
{
    CommonEvent commonEvent;
    int32_t result = commonEvent.SetStaticSubscriberState(true);
    EXPECT_EQ(OHOS::Notification::ERR_NOTIFICATION_CES_COMMON_NOT_SYSTEM_APP, result);
}

/*
 * @tc.number: SetStaticSubscriberStateWithTwoParameters_0100
 * @tc.name: verify SetStaticSubscriberState
 * @tc.desc: Failed to call SetStaticSubscriberState API to obtain proxy
 */
HWTEST_F(CommonEventTest, SetStaticSubscriberStateWithTwoParameters_0100, Function | SmallTest | Level0)
{
    CommonEvent commonEvent;
    std::vector<std::string> events;
    events.push_back("StaticCommonEventA");
    int32_t result = commonEvent.SetStaticSubscriberState(events, true);
    EXPECT_EQ(result, OHOS::Notification::ERR_NOTIFICATION_CES_COMMON_NOT_SYSTEM_APP);
}