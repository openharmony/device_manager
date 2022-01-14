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

#include "device_manager_service.h"

#include "device_manager_service_listener.h"
#include "dm_constants.h"
#include "dm_device_info_manager.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);

int32_t DeviceManagerService::Init()
{
    if (intFlag_) {
        LOGE("Init failed, singleton cannot be initialized multiple times");
        return DM_INT_MULTIPLE;
    }
    if (softbusConnector_ == nullptr) {
        softbusConnector_ = std::make_shared<SoftbusConnector>();
        if (softbusConnector_ == nullptr) {
            LOGE("Init failed, softbusConnector_ apply for failure");
            return DM_MAKE_SHARED_FAIL;
        }
    }
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
        if (softbusConnector_ == nullptr) {
            LOGE("Init failed, listener_ apply for failure");
            return DM_MAKE_SHARED_FAIL;
        }
    }
    if (deviceInfoMgr_ == nullptr) {
        deviceInfoMgr_ = std::make_shared<DmDeviceInfoManager>(softbusConnector_);
        if (deviceInfoMgr_ == nullptr) {
            LOGE("Init failed, deviceInfoMgr_ apply for failure");
            return DM_MAKE_SHARED_FAIL;
        }
    }
    if (deviceStateMgr_ == nullptr) {
        deviceStateMgr_ = std::make_shared<DmDeviceStateManager>(softbusConnector_, listener_);
        if (deviceStateMgr_ == nullptr) {
            LOGE("Init failed, deviceStateMgr_ apply for failure");
            return DM_MAKE_SHARED_FAIL;
        }
        deviceStateMgr_->RegisterSoftbusStateCallback();
    }
    if (discoveryMgr_ == nullptr) {
        discoveryMgr_ = std::make_shared<DmDiscoveryManager>(softbusConnector_, listener_);
        if (discoveryMgr_ == nullptr) {
            LOGE("Init failed, discoveryMgr_ apply for failure");
            return DM_MAKE_SHARED_FAIL;
        }
    }
    if (authMgr_ == nullptr) {
        authMgr_ = std::make_shared<DmAuthManager>(softbusConnector_, listener_);
        if (authMgr_ == nullptr) {
            LOGE("Init failed, authMgr_ apply for failure");
            return DM_MAKE_SHARED_FAIL;
        }
        authMgr_->RegisterSessionCallback();
    }
    LOGI("Init success, singleton initialized");
    intFlag_ = true;
    return DM_OK;
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                   std::vector<DmDeviceInfo> &deviceList)
{
    if (!intFlag_) {
        LOGE("GetTrustedDeviceList failed, singleton not init or init fail");
        return DM_NOT_INIT;
    }
    if (pkgName.empty()) {
        LOGE("GetTrustedDeviceList failed, pkgName is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    return deviceInfoMgr_->GetTrustedDeviceList(pkgName, extra, deviceList);
}

int32_t DeviceManagerService::GetLocalDeviceInfo(DmDeviceInfo &info)
{
    if (!intFlag_) {
        LOGE("GetLocalDeviceInfo failed, singleton not init or init fail");
        return DM_NOT_INIT;
    }
    return deviceInfoMgr_->GetLocalDeviceInfo(info);
}

int32_t DeviceManagerService::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &udid)
{
    if (!intFlag_) {
        LOGE("GetLocalDeviceInfo failed, singleton not init or init fail");
        return DM_NOT_INIT;
    }

    if (pkgName.empty()) {
        LOGE("StartDeviceDiscovery failed, pkgName is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    SoftbusConnector::GetUdidByNetworkId(netWorkId.c_str(), udid);
    return DM_OK;
}

int32_t DeviceManagerService::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &uuid)
{
    if (!intFlag_) {
        LOGE("GetLocalDeviceInfo failed, singleton not init or init fail");
        return DM_NOT_INIT;
    }

    if (pkgName.empty()) {
        LOGE("StartDeviceDiscovery failed, pkgName is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    SoftbusConnector::GetUuidByNetworkId(netWorkId.c_str(), uuid);
    return DM_OK;
}

int32_t DeviceManagerService::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                                   const std::string &extra)
{
    if (!intFlag_) {
        LOGE("StartDeviceDiscovery failed, singleton not init or init fail");
        return DM_NOT_INIT;
    }
    if (pkgName.empty()) {
        LOGE("StartDeviceDiscovery failed, pkgName is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    return discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
}

int32_t DeviceManagerService::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    if (!intFlag_) {
        LOGE("StopDeviceDiscovery failed, singleton not init or init fail");
        return DM_NOT_INIT;
    }
    if (pkgName.empty()) {
        LOGE("StopDeviceDiscovery failed, pkgName is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    return discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeId);
}

int32_t DeviceManagerService::AuthenticateDevice(const std::string &pkgName, int32_t authType,
                                                 const std::string &deviceId, const std::string &extra)
{
    if (!intFlag_) {
        LOGE("AuthenticateDevice failed, singleton not init or init fail");
        return DM_NOT_INIT;
    }
    if (pkgName.empty()) {
        LOGE("AuthenticateDevice failed, pkgName is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    if (deviceId.empty()) {
        LOGE("AuthenticateDevice failed, deviceId is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    return authMgr_->AuthenticateDevice(pkgName, authType, deviceId, extra);
}

int32_t DeviceManagerService::UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId)
{
    if (!intFlag_) {
        LOGE("UnAuthenticateDevice failed, singleton not init or init fail");
        return DM_NOT_INIT;
    }
    if (pkgName.empty()) {
        LOGE("UnAuthenticateDevice failed, pkgName is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    if (deviceId.empty()) {
        LOGE("UnAuthenticateDevice failed, deviceId is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    return authMgr_->UnAuthenticateDevice(pkgName, deviceId);
}

int32_t DeviceManagerService::VerifyAuthentication(const std::string &authParam)
{
    if (!intFlag_) {
        LOGE("VerifyAuthentication failed, singleton not init or init fail");
        return DM_NOT_INIT;
    }
    return authMgr_->VerifyAuthentication(authParam);
}

int32_t DeviceManagerService::GetFaParam(std::string &pkgName, DmAuthParam &authParam)
{
    if (pkgName.empty()) {
        LOGE("GetFaParam failed, pkgName is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    authMgr_->GetAuthenticationParam(authParam);
    return DM_OK;
}

int32_t DeviceManagerService::SetUserOperation(std::string &pkgName, int32_t action)
{
    if (pkgName.empty()) {
        LOGE("SetUserOperation failed, pkgName is empty");
        return DM_INPUT_PARA_EMPTY;
    }
    authMgr_->OnUserOperation(action);
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS