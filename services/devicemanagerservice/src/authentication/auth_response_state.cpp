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

#include "auth_response_state.h"

#include "dm_auth_manager.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
void AuthResponseState::Leave()
{
}

void AuthResponseState::SetAuthContext(std::shared_ptr<DmAuthResponseContext> context)
{
    context_ = std::move(context);
}

std::shared_ptr<DmAuthResponseContext> AuthResponseState::GetAuthContext()
{
    return context_;
}

void AuthResponseState::SetAuthManager(std::shared_ptr<DmAuthManager> authManager)
{
    authManager_ = std::move(authManager);
}

void AuthResponseState::TransitionTo(std::shared_ptr<AuthResponseState> state)
{
    LOGI("AuthRequestState:: TransitionTo");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    state->SetAuthManager(stateAuthManager);
    stateAuthManager->SetAuthResponseState(state);
    state->SetAuthContext(context_);
    this->Leave();
    state->Enter();
}

int32_t AuthResponseInitState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_INIT;
}

void AuthResponseInitState::Enter()
{
    LOGI("AuthResponse:: AuthResponseInitState  Enter");
}

int32_t AuthResponseNegotiateState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_NEGOTIATE;
}

void AuthResponseNegotiateState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->RespNegotiate(context_->sessionId);
}

int32_t AuthResponseConfirmState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_CONFIRM;
}

void AuthResponseConfirmState::Enter()
{
    LOGI("AuthResponse:: AuthResponseConfirmState  Enter");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->ShowConfigDialog();
}

int32_t AuthResponseGroupState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_GROUP;
}

void AuthResponseGroupState::Enter()
{
    LOGI("AuthResponse:: AuthResponseGroupState  Enter");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->CreateGroup();
}

int32_t AuthResponseShowState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_SHOW;
}

void AuthResponseShowState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->ShowAuthInfoDialog();
}

int32_t AuthResponseFinishState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_FINISH;
}

void AuthResponseFinishState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->AuthenticateFinish();
}
} // namespace DistributedHardware
} // namespace OHOS