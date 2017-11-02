/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <mutex>
#include <binder/ActivityManager.h>
#include <binder/Binder.h>
#include <binder/IServiceManager.h>

#include <utils/SystemClock.h>

namespace android {

ActivityManager::ActivityManager()
{
}

sp<IActivityManager> ActivityManager::getService()
{
    std::lock_guard<Mutex> scoped_lock(mLock);
    int64_t startTime = 0;
    sp<IActivityManager> service = mService;
    while (service == NULL || !IInterface::asBinder(service)->isBinderAlive()) {
        sp<IBinder> binder = defaultServiceManager()->checkService(String16("activity"));
        if (binder == NULL) {
            // Wait for the activity service to come back...
            if (startTime == 0) {
                startTime = uptimeMillis();
                ALOGI("Waiting for activity service");
            } else if ((uptimeMillis() - startTime) > 10000) {
                ALOGW("Waiting too long for activity service, giving up");
                service = NULL;
                break;
            }
            sleep(1);
        } else {
            service = interface_cast<IActivityManager>(binder);
            mService = service;
        }
    }
    return service;
}

int ActivityManager::openContentUri(const String16& stringUri)
{
    sp<IActivityManager> service = getService();
    return service != NULL ? service->openContentUri(stringUri) : -1;
}

void ActivityManager::registerUidObserver(const sp<IUidObserver>& observer,
                                          const int32_t event,
                                          const int32_t cutpoint,
                                          const String16& callingPackage)
{
    sp<IActivityManager> service = getService();
    if (service != NULL) {
        service->registerUidObserver(observer, event, cutpoint, callingPackage);
    }
}

void ActivityManager::unregisterUidObserver(const sp<IUidObserver>& observer)
{
    sp<IActivityManager> service = getService();
    if (service != NULL) {
        service->unregisterUidObserver(observer);
    }
}

}; // namespace android
