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

#ifndef OHOS_DM_SOFTBUS_SESSION_CALLBACK_H
#define OHOS_DM_SOFTBUS_SESSION_CALLBACK_H

namespace OHOS {
namespace DistributedHardware {
class ISoftbusSessionCallback {
public:
    virtual void OnSessionOpened(const std::string &pkgName, int32_t sessionId, int32_t sessionSide,
                                 int32_t result) = 0;
    virtual void OnSessionClosed(const std::string &pkgName, int32_t sessionId) = 0;
    virtual void OnDataReceived(const std::string &pkgName, int32_t sessionId, std::string message) = 0;
    virtual void GetIsCryptoSupport(bool &isCryptoSupport) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_SESSION_CALLBACK_H
