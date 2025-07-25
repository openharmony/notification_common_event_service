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

import hilog from '@ohos.hilog';
import{ AsyncCallback, BusinessError } from '@ohos.base';
import{ CommonEventPublishData } from 'commonEvent.commonEventPublishData';
import{ CommonEventData } from 'commonEvent.commonEventData';
import{ CommonEventSubscribeInfo } from 'commonEvent.commonEventSubscribeInfo';
import{ CommonEventSubscriber } from 'commonEvent.commonEventSubscriber';

const TAG: string = 'commonEventManager';
namespace commonEventManager {
    loadLibrary("ani_commoneventmanager.z");
    export native function publishExecute(event: string):int;
    export native function publishWithOptionsExecute(event: string, options: CommonEventPublishData):int;
    export native function createSubscriberExecute(subscribeInfo: CommonEventSubscribeInfo):CommonEventSubscriber;
    export native function subscribeExecute(subscriber: CommonEventSubscriber,
        callback: AsyncCallback<CommonEventData, void>):int;
    export native function unsubscribeExecute(subscriber: CommonEventSubscriber):int;

    export function publish(event: string, callback: AsyncCallback<void, void>): void {
        hilog.info(0x0000, TAG, 'publish called event: %{public}s', event);
        let task = taskpool.execute(commonEventManager.publishExecute, event);
        task.then((e: NullishType)=>{
            let resultCode: Int  = e as Int;
            hilog.info(0x0000, TAG, 'publish call result:%{public}d.', resultCode.toInt());
            let r: BusinessError = { code: resultCode.toInt(), data: undefined };
            callback(r, undefined);
        }, (err:Object): void => {
            hilog.error(0x0000, TAG, 'publish error.');
        });
    }

    export function publish(event: string, options: CommonEventPublishData, callback: AsyncCallback<void, void>): void {
        hilog.info(0x0000, TAG, 'publish with option call.');
        let task = taskpool.execute(commonEventManager.publishWithOptionsExecute, event, options);
        task.then((e: NullishType)=>{
            let resultCode: Int  = e as Int;
            hilog.info(0x0000, TAG, 'publish with option result:%{public}d.', resultCode.toInt());
            let r: BusinessError = { code: resultCode.toInt(), data: undefined };
            callback(r, undefined);
        }, (err:Object): void => {
            hilog.error(0x0000, TAG, 'publish with option error.');
        });
    }

    export function createSubscriber(subscribeInfo: CommonEventSubscribeInfo,
        callback: AsyncCallback<CommonEventSubscriber, void>): void {
        hilog.info(0x0000, TAG, 'createSubscriber callback call.');
        let task = taskpool.execute(commonEventManager.createSubscriberExecute, subscribeInfo);
        task.then((e: NullishType)=>{
            let result: CommonEventSubscriber = e as CommonEventSubscriber;
            hilog.info(0x0000, TAG, 'createSubscriber result.');
            let r: BusinessError = { code: 0, data: undefined };
            callback(r, result);
        }, (err:Object): void => {
            hilog.error(0x0000, TAG, 'createSubscriber error.');
        });
    }

    export function createSubscriber(subscribeInfo: CommonEventSubscribeInfo):Promise<CommonEventSubscriber> {
        hilog.info(0x0000, TAG, 'createSubscriber Promise call');
        let p = new Promise<CommonEventSubscriber>((resolve: (v: CommonEventSubscriber) => void,
            reject: (error: Error) => void) => {
            let p1 = taskpool.execute(commonEventManager.createSubscriberExecute, subscribeInfo);
            p1.then((e :NullishType)=>{
                hilog.info(0x0000, TAG, 'createSubscriber Promise result.');
                let result: CommonEventSubscriber  = e as CommonEventSubscriber;
                resolve(result);
            }, (err:Error): void => {
                hilog.error(0x0000, TAG, 'createSubscriber Promise error');
            });
        });
        return p;
    }

    export function createSubscriberSync(subscribeInfo: CommonEventSubscribeInfo): CommonEventSubscriber{
        hilog.info(0x0000, TAG, 'createSubscriberSync call');
        return commonEventManager.createSubscriberExecute(subscribeInfo);
    }

    export function subscribe(subscriber: CommonEventSubscriber, callback: AsyncCallback<CommonEventData, void>): void {
        hilog.info(0x0000, TAG, 'subscribe call.');
        let task = taskpool.execute(commonEventManager.subscribeExecute, subscriber, callback);
        task.then((e: NullishType)=>{
            let resultCode: Int  = e as Int;
            hilog.info(0x0000, TAG, 'subscribe result:%{public}d.', resultCode.toInt());
            if (resultCode != 0) {
                let a: CommonEventData;
                let r: BusinessError = { code: resultCode.toInt(), data: undefined };
                callback(r, a);
            }
        }, (err:Object): void => {
            hilog.error(0x0000, TAG, 'subscribe error');
        });
    }

    export function unsubscribe(subscriber: CommonEventSubscriber, callback?: AsyncCallback<void, void>): void {
        hilog.info(0x0000, TAG, 'unsubscribe call.');
        let task = taskpool.execute(commonEventManager.unsubscribeExecute, subscriber);
        task.then((e: NullishType)=>{
            let resultCode: Int  = e as Int;
            hilog.info(0x0000, TAG, 'unsubscribe result:%{public}d.', resultCode.toInt());
            let r: BusinessError = { code: resultCode.toInt(), data: undefined };
            if (callback != undefined) {
                callback(r, undefined);
            }
        }, (err:Object): void => {
            hilog.error(0x0000, TAG, 'unsubscribe error');
        });
    }

    export enum Support {
        COMMON_EVENT_SCREEN_OFF = 'usual.event.SCREEN_OFF',
        COMMON_EVENT_SCREEN_ON = 'usual.event.SCREEN_ON',
        COMMON_EVENT_USER_INFO_UPDATED = 'usual.event.USER_INFO_UPDATED',
        COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT = 'common.event.DISTRIBUTED_ACCOUNT_LOGOUT',
        COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN = 'common.event.DISTRIBUTED_ACCOUNT_LOGIN',
        COMMON_EVENT_HTTP_PROXY_CHANGE = 'usual.event.HTTP_PROXY_CHANGE',
        COMMON_EVENT_MINORSMODE_ON = 'usual.event.MINORSMODE_ON',
        COMMON_EVENT_MINORSMODE_OFF = 'usual.event.MINORSMODE_OFF',
        COMMON_EVENT_TIME_CHANGED = 'usual.event.TIME_CHANGED',
        COMMON_EVENT_TIMEZONE_CHANGED = 'usual.event.TIMEZONE_CHANGED'
    }

}

export default commonEventManager;
