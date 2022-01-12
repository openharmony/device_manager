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

#ifndef OHOS_DM_AUTH_RESPONSE_STATE_H
#define OHOS_DM_AUTH_RESPONSE_STATE_H

#include <memory>

namespace OHOS {
namespace DistributedHardware {

class DmAuthManager;
struct DmAuthResponseContext;
class AuthResponseState : public std::enable_shared_from_this<AuthResponseState> {
public:
    virtual ~AuthResponseState()
    {
        authManager_.reset();
    };
    virtual int32_t GetStateType() = 0;
    virtual void Enter() = 0;
    void Leave();
    void TransitionTo(std::shared_ptr<AuthResponseState> state);
    void SetAuthManager(std::shared_ptr<DmAuthManager> authManager);
    void SetAuthContext(std::shared_ptr<DmAuthResponseContext> context);
    std::shared_ptr<DmAuthResponseContext> GetAuthContext();

protected:
    std::weak_ptr<DmAuthManager> authManager_;
    std::shared_ptr<DmAuthResponseContext> context_;
};

class AuthResponseInitState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    void Enter() override;
};

class AuthResponseNegotiateState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    void Enter() override;
};

class AuthResponseConfirmState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    void Enter() override;
};

class AuthResponseGroupState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    void Enter() override;
};

class AuthResponseShowState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    void Enter() override;
};

class AuthResponseFinishState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    void Enter() override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_RESPONSE_STATE_H
