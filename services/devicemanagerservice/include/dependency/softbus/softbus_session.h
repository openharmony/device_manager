/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DM_SOFTBUS_SESSION_H
#define OHOS_DM_SOFTBUS_SESSION_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "inner_session.h"
#include "session.h"
#include "softbus_session_callback.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusSession {
public:
    static int32_t OnSessionOpened(int32_t sessionId, int32_t result);
    static void OnSessionClosed(int32_t sessionId);
    static void OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen);

public:
    SoftbusSession();
    ~SoftbusSession();
    int32_t RegisterSessionCallback(const std::string &pkgName, std::shared_ptr<ISoftbusSessionCallback> callback);
    int32_t UnRegisterSessionCallback(const std::string &pkgName);
    int32_t OpenAuthSession(const std::string &deviceId);
    void CloseAuthSession(int32_t sessionId);
    int32_t SendData(int32_t sessionId, std::string &message);
    void GetPeerDeviceId(int32_t sessionId, std::string &peerDevId);

private:
    static std::map<std::string, std::shared_ptr<ISoftbusSessionCallback>> sessionCallbackMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_SESSION_H
