/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_AUTHENTICATION_H
#define OHOS_DM_AUTHENTICATION_H

#include "dm_ability_manager.h"

namespace OHOS {
namespace DistributedHardware {
class DmAuthManager;
class IAuthentication {
public:
    virtual ~IAuthentication() = default;
    virtual int32_t ShowAuthInfo(std::string &authToken, std::shared_ptr<DmAuthManager> authManager) = 0;
    virtual int32_t StartAuth(std::string &authToken, std::shared_ptr<DmAuthManager> authManager) = 0;
    virtual int32_t VerifyAuthentication(std::string &authToken, const std::string &authParam) = 0;
    virtual int32_t CloseAuthInfo(const int32_t &pageId, std::shared_ptr<DmAuthManager> authManager) = 0;
    virtual int32_t UpdateAuthInfo(const int32_t &pageId) = 0;
};

using CreateIAuthAdapterFuncPtr = IAuthentication *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTHENTICATION_H
