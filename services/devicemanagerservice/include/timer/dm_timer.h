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

#ifndef TIMER_H
#define TIMER_H
#include <string>
#include <errno.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <cstdio>

#include "device_manager_log.h"


namespace OHOS {
namespace DistributedHardware {

typedef void (*TimeoutHandle)(void *data);

#define MAXEVENTS 255

enum DmTimerStatus : int32_t {
    DM_STATUS_INIT = 0,
    DM_STATUS_RUNNING = 1,
    DM_STATUS_BUSY = 2,
    DM_STATUS_CREATE_ERROR = 3,
    DM_STATUS_FINISH = 6,
};

class DmTimer {
public:
    DmTimer();
    DmTimerStatus Start(uint32_t timeOut, TimeoutHandle handle, void *data);
    void Stop(int32_t code);
    void WiteforTimeout();
    void Release();

private:
    int32_t CreateTimeFd();

private:
    DmTimerStatus mStatus;
    uint32_t mTimeOutS;
    TimeoutHandle mHandle;
    void* mHandleData;
    int32_t mTimeFd[2];
    struct epoll_event mEv;
    struct epoll_event mEvents[MAXEVENTS];
    int32_t mEpFd;
    std::thread mThread;
};
}
}
#endif

