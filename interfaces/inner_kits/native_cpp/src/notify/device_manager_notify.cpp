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

#include "device_manager_notify.h"

#include "device_manager.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerNotify);

void DeviceManagerNotify::RegisterDeathRecipientCallback(const std::string &pkgName,
                                                         std::shared_ptr<DmInitCallback> dmInitCallback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmInitCallback_[pkgName] = dmInitCallback;
}

void DeviceManagerNotify::UnRegisterDeathRecipientCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmInitCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDeviceStateCallback(const std::string &pkgName,
                                                      std::shared_ptr<DeviceStateCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterDeviceStateCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDiscoveryCallback(const std::string &pkgName, uint16_t subscribeId,
                                                    std::shared_ptr<DiscoveryCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
        deviceDiscoveryCallbacks_[pkgName] = std::map<uint16_t, std::shared_ptr<DiscoveryCallback>>();
    }
    deviceDiscoveryCallbacks_[pkgName][subscribeId] = callback;
}

void DeviceManagerNotify::UnRegisterDiscoveryCallback(const std::string &pkgName, uint16_t subscribeId)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoveryCallbacks_.count(pkgName) > 0) {
        deviceDiscoveryCallbacks_[pkgName].erase(subscribeId);
        if (deviceDiscoveryCallbacks_[pkgName].empty()) {
            deviceDiscoveryCallbacks_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::RegisterAuthenticateCallback(const std::string &pkgName, const std::string &deviceId,
                                                       std::shared_ptr<AuthenticateCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (authenticateCallback_.count(pkgName) == 0) {
        authenticateCallback_[pkgName] = std::map<std::string, std::shared_ptr<AuthenticateCallback>>();
    }
    authenticateCallback_[pkgName][deviceId] = callback;
}

void DeviceManagerNotify::UnRegisterAuthenticateCallback(const std::string &pkgName, const std::string &deviceId)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (authenticateCallback_.count(pkgName) > 0) {
        authenticateCallback_[pkgName].erase(deviceId);
        if (authenticateCallback_[pkgName].empty()) {
            authenticateCallback_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::UnRegisterPackageCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_.erase(pkgName);
    deviceDiscoveryCallbacks_.erase(pkgName);
    authenticateCallback_.erase(pkgName);
    dmInitCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterVerifyAuthenticationCallback(const std::string &pkgName, const std::string &authPara,
                                                               std::shared_ptr<VerifyAuthCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    verifyAuthCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterVerifyAuthenticationCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    verifyAuthCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDeviceManagerFaCallback(const std::string &pkgName,
                                                          std::shared_ptr<DeviceManagerFaCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmFaCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterDeviceManagerFaCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (dmFaCallback_.count(pkgName) == 0) {
        LOGE("DeviceManager UnRegisterDeviceManagerFaCallback not register");
        return;
    }
    dmFaCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterCredentialCallback(const std::string &pkgName,
                                                     std::shared_ptr<CredentialCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    credentialCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterCredentialCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    credentialCallback_.erase(pkgName);
}

void DeviceManagerNotify::OnRemoteDied()
{
    LOGW("DeviceManager : OnRemoteDied");
    for (auto iter : dmInitCallback_) {
        iter.second->OnRemoteDied();
    }
}

void DeviceManagerNotify::OnDeviceOnline(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    LOGI("DeviceManager OnDeviceOnline pkgName:%s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceStateCallback_.count(pkgName) == 0) {
        LOGE("DeviceManager OnDeviceOnlinecallback not register");
        return;
    }
    deviceStateCallback_[pkgName]->OnDeviceOnline(deviceInfo);
}

void DeviceManagerNotify::OnDeviceOffline(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    LOGI("DeviceManager OnDeviceOffline pkgName:%s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceStateCallback_.count(pkgName) == 0) {
        LOGE("DeviceManager OnDeviceOfflinecallback not register");
        return;
    }
    deviceStateCallback_[pkgName]->OnDeviceOffline(deviceInfo);
}

void DeviceManagerNotify::OnDeviceChanged(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    LOGI("DeviceManager OnDeviceChanged pkgName:%s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceStateCallback_.count(pkgName) == 0) {
        LOGE("DeviceManager OnDeviceChangedcallback not register");
        return;
    }
    deviceStateCallback_[pkgName]->OnDeviceChanged(deviceInfo);
}

void DeviceManagerNotify::OnDeviceReady(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    LOGI("DeviceManager OnDeviceReady pkgName:%s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceStateCallback_.count(pkgName) == 0) {
        LOGE("DeviceManager OnDeviceReadycallback not register");
        return;
    }
    deviceStateCallback_[pkgName]->OnDeviceReady(deviceInfo);
}

void DeviceManagerNotify::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                        const DmDeviceInfo &deviceInfo)
{
    LOGI("DeviceManager OnDeviceFound pkgName:%s, subscribeId:%d.", pkgName.c_str(), (int32_t)subscribeId);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
        LOGE("DeviceManager OnDeviceFound: no register DiscoveryCallback for this package");
        return;
    }
    std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
    auto iter = discoverCallMap.find(subscribeId);
    if (iter == discoverCallMap.end()) {
        LOGE("DeviceManager OnDeviceFound: no register DiscoveryCallback for subscribeId %d", subscribeId);
        return;
    }
    iter->second->OnDeviceFound(subscribeId, deviceInfo);
}

void DeviceManagerNotify::OnDiscoveryFailed(const std::string &pkgName, uint16_t subscribeId, int32_t failedReason)
{
    LOGI("DeviceManager OnDiscoveryFailed pkgName:%s, subscribeId %d, reason %d", pkgName.c_str(), subscribeId,
         failedReason);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
        LOGE("DeviceManager OnDiscoveryFailed: no register DiscoveryCallback for this package");
        return;
    }
    std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
    auto iter = discoverCallMap.find(subscribeId);
    if (iter == discoverCallMap.end()) {
        LOGE("DeviceManager OnDiscoveryFailed: no register DiscoveryCallback for subscribeId %d", subscribeId);
        return;
    }
    iter->second->OnDiscoveryFailed(subscribeId, failedReason);
}

void DeviceManagerNotify::OnDiscoverySuccess(const std::string &pkgName, uint16_t subscribeId)
{
    LOGI("DeviceManager OnDiscoverySuccess pkgName:%s, subscribeId:%d.", pkgName.c_str(), subscribeId);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
        LOGE("DeviceManager OnDiscoverySuccess: no register DiscoveryCallback for this package");
        return;
    }
    std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
    auto iter = discoverCallMap.find(subscribeId);
    if (iter == discoverCallMap.end()) {
        LOGE("DeviceManager OnDiscoverySuccess: no register DiscoveryCallback for subscribeId %d", subscribeId);
        return;
    }
    iter->second->OnDiscoverySuccess(subscribeId);
}

void DeviceManagerNotify::OnAuthResult(const std::string &pkgName, const std::string &deviceId,
                                       const std::string &token, uint32_t status, uint32_t reason)
{
    LOGI("DeviceManagerNotify::OnAuthResult pkgName:%s, status:%d, reason:%u", pkgName.c_str(), status, reason);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (authenticateCallback_.count(pkgName) == 0) {
        LOGE("DeviceManager OnAuthResult: no register authCallback for this package");
        return;
    }
    std::map<std::string, std::shared_ptr<AuthenticateCallback>> &authCallMap = authenticateCallback_[pkgName];
    auto iter = authCallMap.find(deviceId);
    if (iter == authCallMap.end()) {
        LOGE("DeviceManager OnAuthResult: no register authCallback for deviceID ");
        return;
    }
    if (iter->second == nullptr) {
        LOGE("DeviceManager OnAuthResult: no register authCallback for OnAuthResult ");
        return;
    }
    iter->second->OnAuthResult(deviceId, token, status, (int32_t)reason);
    authenticateCallback_[pkgName].erase(deviceId);
    if (authenticateCallback_[pkgName].empty()) {
        authenticateCallback_.erase(pkgName);
    }
}

void DeviceManagerNotify::OnVerifyAuthResult(const std::string &pkgName, const std::string &deviceId,
                                             int32_t resultCode, int32_t flag)
{
    LOGI("DeviceManagerNotify::OnCheckAuthResult pkgName:%s, resultCode:%d, flag:%d", pkgName.c_str(), resultCode,
         flag);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (verifyAuthCallback_.count(pkgName) == 0) {
        LOGE("DeviceManager OnCheckAuthResult: no register authCallback for this package");
        return;
    }
    verifyAuthCallback_[pkgName]->OnVerifyAuthResult(deviceId, resultCode, flag);
    verifyAuthCallback_.erase(pkgName);
}

void DeviceManagerNotify::OnFaCall(std::string &pkgName, std::string &paramJson)
{
    LOGI("DeviceManager OnFaCallback pkgName:%s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (dmFaCallback_.count(pkgName) == 0) {
        LOGE("DeviceManager DmFaCallback not register");
        return;
    }
    dmFaCallback_[pkgName]->OnCall(paramJson);
}

void DeviceManagerNotify::OnCredentialResult(const std::string &pkgName, int32_t &action,
                                             const std::string &credentialResult)
{
    LOGI("DeviceManagerNotify::OnCredentialResult pkgName:%s, action:%d", pkgName.c_str(), action);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (credentialCallback_.count(pkgName) == 0) {
        LOGE("DeviceManager OnCredentialResult: no register credentialCallback_ for this package");
        return;
    }
    credentialCallback_[pkgName]->OnCredentialResult(action, credentialResult);
}
} // namespace DistributedHardware
} // namespace OHOS
