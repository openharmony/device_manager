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

#include "device_manager_impl.h"
#include <unistd.h>
#include "device_manager_notify.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_dfx_constants.h"
#include "dm_hisysevent.h"
#include "dm_hitrace.h"
#include "dm_log.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_get_dmfaparam_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_req.h"
#include "ipc_rsp.h"
#include "ipc_set_credential_req.h"
#include "ipc_set_credential_rsp.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_start_discovery_req.h"
#include "ipc_stop_discovery_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_verify_authenticate_req.h"
#include "ipc_register_dev_state_callback_req.h"
#include "securec.h"

namespace OHOS {
namespace DistributedHardware {
DeviceManagerImpl &DeviceManagerImpl::GetInstance()
{
    static DeviceManagerImpl instance;
    return instance;
}

int32_t DeviceManagerImpl::InitDeviceManager(const std::string &pkgName, std::shared_ptr<DmInitCallback> dmInitCallback)
{
    LOGI("DeviceManager::InitDeviceManager start, pkgName: %s", pkgName.c_str());
    DmTraceStart(DM_HITRACE_INIT);
    if (pkgName.empty() || dmInitCallback == nullptr) {
        LOGE("InitDeviceManager error: Invalid parameter");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    int32_t ret = DM_OK;
    int32_t retryNum = 0;
    while (retryNum < SERVICE_INIT_TRY_MAX_NUM) {
        ret = ipcClientProxy_->Init(pkgName);
        if (ret != ERR_DM_NOT_INIT) {
            break;
        }
        usleep(SLEEP_TIME_MS);
        retryNum++;
        if (retryNum == SERVICE_INIT_TRY_MAX_NUM) {
            LOGE("InitDeviceManager error: wait for device manager service starting timeout.");
            return ERR_DM_NOT_INIT;
        }
    }
    if (ret != DM_OK) {
        LOGE("InitDeviceManager error: proxy init failed ret: %d", ret);
        SysEventWrite(DM_INIT_DEVICE_MANAGER_FAILED, DM_HISYEVENT_FAULT, DM_INIT_DEVICE_MANAGER_FAILED_MSG);
        return ERR_DM_INIT_FAILED;
    }

    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    DmTraceEnd();
    LOGI("InitDeviceManager success");
    SysEventWrite(DM_INIT_DEVICE_MANAGER_SUCCESS, DM_HISYEVENT_BEHAVIOR, DM_INIT_DEVICE_MANAGER_SUCCESS_MSG);
    return DM_OK;
}

int32_t DeviceManagerImpl::UnInitDeviceManager(const std::string &pkgName)
{
    LOGI("DeviceManager::UnInitDeviceManager start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("UnInitDeviceManager error: Invalid parameter");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    int32_t ret = ipcClientProxy_->UnInit(pkgName);
    if (ret != DM_OK) {
        LOGE("UnInitDeviceManager error: proxy unInit failed ret: %d", ret);
        return ERR_DM_FAILED;
    }

    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(pkgName);
    LOGI("UnInitDeviceManager success");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("DeviceManager::GetTrustedDeviceList start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("GetTrustedDeviceList error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    std::shared_ptr<IpcGetTrustDeviceReq> req = std::make_shared<IpcGetTrustDeviceReq>();
    std::shared_ptr<IpcGetTrustDeviceRsp> rsp = std::make_shared<IpcGetTrustDeviceRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    int32_t ret = ipcClientProxy_->SendRequest(GET_TRUST_DEVICE_LIST, req, rsp);
    if (ret != DM_OK) {
        LOGE("GetTrustedDeviceList error: Send Request failed ret: %d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGI("GetTrustedDeviceList error: failed ret: %d", ret);
        return ERR_DM_IPC_RESPOND_FAILED;
    }

    deviceList = rsp->GetDeviceVec();
    LOGI("GetTrustedDeviceList completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &info)
{
    LOGI("DeviceManager::GetLocalDeviceInfo start, pkgName: %s", pkgName.c_str());
    DmTraceStart(DM_HITRACE_GET_LOCAL_DEVICE_INFO);
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcGetLocalDeviceInfoRsp> rsp = std::make_shared<IpcGetLocalDeviceInfoRsp>();
    req->SetPkgName(pkgName);
    int32_t ret = ipcClientProxy_->SendRequest(GET_LOCAL_DEVICE_INFO, req, rsp);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceInfo error: Send Request failed ret: %d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGI("GetLocalDeviceInfo error: failed ret: %d", ret);
        SysEventWrite(GET_LOCAL_DEVICE_INFO_FAILED, DM_HISYEVENT_BEHAVIOR, GET_LOCAL_DEVICE_INFO_FAILED_MSG);
        return ERR_DM_IPC_RESPOND_FAILED;
    }

    info = rsp->GetLocalDeviceInfo();
    DmTraceEnd();
    LOGI("GetLocalDeviceInfo completed,pkgname%s", req->GetPkgName().c_str());
    SysEventWrite(GET_LOCAL_DEVICE_INFO_SUCCESS, DM_HISYEVENT_BEHAVIOR, GET_LOCAL_DEVICE_INFO_SUCCESS_MSG);
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra,
                                                    std::shared_ptr<DeviceStateCallback> callback)
{
    LOGI("DeviceManager::RegisterDevStateCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty() || callback == nullptr) {
        LOGE("RegisterDevStateCallback error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    if (!extra.empty()) {
        RegisterDevStateCallback(pkgName, extra);
    }
    LOGI("RegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(const std::string &pkgName)
{
    LOGI("DeviceManager::UnRegisterDevStateCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("UnRegisterDevStateCallback error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(pkgName);
    std::string extra = "";
    UnRegisterDevStateCallback(pkgName, extra);
    LOGI("UnRegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                                const std::string &extra, std::shared_ptr<DiscoveryCallback> callback)
{
    LOGI("DeviceManager::StartDeviceDiscovery start, pkgName: %s", pkgName.c_str());
    DmTraceStart(DM_HITRACE_START_DEVICE);
    if (pkgName.empty() || callback == nullptr) {
        LOGE("StartDeviceDiscovery error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    LOGI("DeviceManager StartDeviceDiscovery in, pkgName %s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeInfo.subscribeId, callback);

    std::shared_ptr<IpcStartDiscoveryReq> req = std::make_shared<IpcStartDiscoveryReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    req->SetSubscribeInfo(subscribeInfo);
    int32_t ret = ipcClientProxy_->SendRequest(START_DEVICE_DISCOVER, req, rsp);
    if (ret != DM_OK) {
        LOGE("StartDeviceDiscovery error: Send Request failed ret: %d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("StartDeviceDiscovery error: Failed with ret %d", ret);
        SysEventWrite(START_DEVICE_DISCOVERY_FAILED, DM_HISYEVENT_BEHAVIOR, START_DEVICE_DISCOVERY_FAILED_MSG);
        return ret;
    }

    DmTraceEnd();
    LOGI("StartDeviceDiscovery completed, pkgName: %s", pkgName.c_str());
    SysEventWrite(START_DEVICE_DISCOVERY_SUCCESS, DM_HISYEVENT_BEHAVIOR, START_DEVICE_DISCOVERY_SUCCESS_MSG);
    return DM_OK;
}

int32_t DeviceManagerImpl::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    LOGI("DeviceManager::StopDeviceDiscovery start , pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("StopDeviceDiscovery error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    LOGI("StopDeviceDiscovery in, pkgName %s", pkgName.c_str());
    std::shared_ptr<IpcStopDiscoveryReq> req = std::make_shared<IpcStopDiscoveryReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetSubscribeId(subscribeId);
    int32_t ret = ipcClientProxy_->SendRequest(STOP_DEVICE_DISCOVER, req, rsp);
    if (ret != DM_OK) {
        LOGE("StopDeviceDiscovery error: Send Request failed ret: %d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("StopDeviceDiscovery error: Failed with ret %d", ret);
        return ret;
    }

    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(pkgName, subscribeId);
    LOGI("StopDeviceDiscovery completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::AuthenticateDevice(const std::string &pkgName, int32_t authType,
                                              const DmDeviceInfo &deviceInfo, const std::string &extra,
                                              std::shared_ptr<AuthenticateCallback> callback)
{
    LOGI("DeviceManager::AuthenticateDevice start , pkgName: %s", pkgName.c_str());
    DmTraceStart(DM_HITRACE_AUTH_TO_CONSULT);
    if (pkgName.empty()) {
        LOGE("AuthenticateDevice error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    std::string strDeviceId = deviceInfo.deviceId;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, strDeviceId, callback);
    std::shared_ptr<IpcAuthenticateDeviceReq> req = std::make_shared<IpcAuthenticateDeviceReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    req->SetAuthType(authType);
    req->SetDeviceInfo(deviceInfo);
    int32_t ret = ipcClientProxy_->SendRequest(AUTHENTICATE_DEVICE, req, rsp);
    if (ret != DM_OK) {
        LOGE("AuthenticateDevice error: Send Request failed ret: %d", ret);
        SysEventWrite(DM_SEND_REQUEST_FAILED, DM_HISYEVENT_BEHAVIOR, DM_SEND_REQUEST_FAILED_MSG);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    SysEventWrite(DM_SEND_REQUEST_SUCCESS, DM_HISYEVENT_BEHAVIOR, DM_SEND_REQUEST_SUCCESS_MSG);

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("AuthenticateDevice error: Failed with ret %d", ret);
        return ERR_DM_IPC_RESPOND_FAILED;
    }
    DmTraceEnd();
    LOGI("DeviceManager::AuthenticateDevice completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnAuthenticateDevice(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    LOGI("DeviceManager::UnAuthenticateDevice start , pkgName: %s, deviceId: %s", pkgName.c_str(),
        GetAnonyString(std::string(deviceInfo.deviceId)).c_str());
    if (pkgName.empty() || (deviceInfo.deviceId[0] == '\0')) {
        LOGE("UnAuthenticateDevice error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    std::shared_ptr<IpcUnAuthenticateDeviceReq> req = std::make_shared<IpcUnAuthenticateDeviceReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceInfo(deviceInfo);
    int32_t ret = ipcClientProxy_->SendRequest(UNAUTHENTICATE_DEVICE, req, rsp);
    if (ret != DM_OK) {
        LOGE("UnAuthenticateDevice error: Send Request failed ret: %d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("UnAuthenticateDevice error: Failed with ret %d", ret);
        SysEventWrite(UNAUTHENTICATE_DEVICE_FAILED, DM_HISYEVENT_BEHAVIOR, UNAUTHENTICATE_DEVICE_FAILED_MSG);
        return ERR_DM_IPC_RESPOND_FAILED;
    }
    SysEventWrite(UNAUTHENTICATE_DEVICE_SUCCESS, DM_HISYEVENT_BEHAVIOR, UNAUTHENTICATE_DEVICE_SUCCESS_MSG);

    LOGI("UnAuthenticateDevice completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDeviceManagerFaCallback(const std::string &pkgName,
                                                           std::shared_ptr<DeviceManagerFaCallback> callback)
{
    LOGI("DeviceManager::RegisterDeviceManagerFaCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty() || callback == nullptr) {
        LOGE("RegisterDeviceManagerFaCallback error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, callback);
    LOGI("DeviceManager::RegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDeviceManagerFaCallback(const std::string &pkgName)
{
    LOGI("DeviceManager::UnRegisterDeviceManagerFaCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("UnRegisterDeviceManagerFaCallback error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    LOGI("DeviceManager::UnRegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::VerifyAuthentication(const std::string &pkgName, const std::string &authPara,
                                                std::shared_ptr<VerifyAuthCallback> callback)
{
    LOGI("DeviceManager::VerifyAuthentication start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("VerifyAuthentication error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    
    DeviceManagerNotify::GetInstance().RegisterVerifyAuthenticationCallback(pkgName, authPara, callback);

    std::shared_ptr<IpcVerifyAuthenticateReq> req = std::make_shared<IpcVerifyAuthenticateReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetAuthPara(authPara);

    int32_t ret = ipcClientProxy_->SendRequest(VERIFY_AUTHENTICATION, req, rsp);
    if (ret != DM_OK) {
        LOGE("VerifyAuthentication error: Send Request failed ret: %d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("VerifyAuthentication error: Failed with ret %d", ret);
        return ret;
    }

    LOGI("VerifyAuthentication completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetFaParam(const std::string &pkgName, DmAuthParam &dmFaParam)
{
    LOGI("DeviceManager::GetFaParam start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("VerifyAuthentication failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcGetDmFaParamRsp> rsp = std::make_shared<IpcGetDmFaParamRsp>();
    req->SetPkgName(pkgName);

    if (ipcClientProxy_->SendRequest(SERVER_GET_DMFA_INFO, req, rsp) != DM_OK) {
        LOGI("DeviceManagerImpl::GetFaParam start 2");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    dmFaParam = rsp->GetDmAuthParam();
    LOGI("GetFaParam completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::SetUserOperation(const std::string &pkgName, int32_t action)
{
    LOGI("DeviceManager::SetUserOperation start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("VerifyAuthentication failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    std::shared_ptr<IpcGetOperationReq> req = std::make_shared<IpcGetOperationReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetOperation(action);

    if (ipcClientProxy_->SendRequest(SERVER_USER_AUTH_OPERATION, req, rsp) != DM_OK) {
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CheckAuthentication Failed with ret %d", ret);
        return ret;
    }
    LOGI("SetUserOperation completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                              std::string &udid)
{
    if (pkgName.empty()) {
        LOGE("VerifyAuthentication failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);

    if (ipcClientProxy_->SendRequest(GET_UDID_BY_NETWORK, req, rsp) != DM_OK) {
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CheckAuthentication Failed with ret %d", ret);
        return ret;
    }
    udid = rsp->GetUdid();
    return DM_OK;
}

int32_t DeviceManagerImpl::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                              std::string &uuid)
{
    if (pkgName.empty()) {
        LOGE("VerifyAuthentication failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);

    if (ipcClientProxy_->SendRequest(GET_UUID_BY_NETWORK, req, rsp) != DM_OK) {
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CheckAuthentication Failed with ret %d", ret);
        return ret;
    }
    uuid = rsp->GetUuid();
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("RegisterDevStateCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    std::shared_ptr<IpcRegisterDevStateCallbackReq> req = std::make_shared<IpcRegisterDevStateCallbackReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);

    if (ipcClientProxy_->SendRequest(REGISTER_DEV_STATE_CALLBACK, req, rsp) != DM_OK) {
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("RegisterDevStateCallback Failed with ret %d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("UnRegisterDevStateCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    std::shared_ptr<IpcRegisterDevStateCallbackReq> req = std::make_shared<IpcRegisterDevStateCallbackReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);

    if (ipcClientProxy_->SendRequest(UNREGISTER_DEV_STATE_CALLBACK, req, rsp) != DM_OK) {
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("UnRegisterDevStateCallback Failed with ret %d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerImpl::RequestCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    if (pkgName.empty()) {
        LOGE("request credential failed, pkgName is empty.");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    if (reqJsonStr.empty()) {
        LOGE("request credential reqJsonStr is empty.");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    LOGI("start to RequestCredential.");
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcSetCredentialRsp> rsp = std::make_shared<IpcSetCredentialRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(reqJsonStr);
    if (ipcClientProxy_->SendRequest(REQUEST_CREDENTIAL, req, rsp) != DM_OK) {
        LOGE("failed to send request while request credential.");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while request credential.");
        return ret;
    }
    returnJsonStr = rsp->GetCredentialResult();
    LOGI("request device credential completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    if (pkgName.empty() || credentialInfo.empty()) {
        LOGE("DeviceManagerImpl::ImportCredential failed, pkgName is %s, credentialInfo is %s",
            pkgName.c_str(), credentialInfo.c_str());
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    LOGI("start to ImportCredential.");
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(credentialInfo);
    if (ipcClientProxy_->SendRequest(IMPORT_CREDENTIAL, req, rsp) != DM_OK) {
        LOGE("failed to request while import credential.");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while import credential.");
        return ret;
    }
    LOGI("import credential to device completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    if (pkgName.empty() || deleteInfo.empty()) {
        LOGE("DeviceManagerImpl::DeleteCredential failed, pkgName is %s, deleteInfo is %s",
            pkgName.c_str(), deleteInfo.c_str());
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    LOGI("start to DeleteCredential.");
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(deleteInfo);
    if (ipcClientProxy_->SendRequest(DELETE_CREDENTIAL, req, rsp) != DM_OK) {
        LOGE("failed to request while delete credential.");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while import credential.");
        return ret;
    }
    LOGI("delete credential from device completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterCredentialCallback(const std::string &pkgName,
    std::shared_ptr<CredentialCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("RegisterCredentialCallback error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    LOGI("DeviceManager::RegisterCredentialCallback start, pkgName: %s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().RegisterCredentialCallback(pkgName, callback);
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    int32_t ret = ipcClientProxy_->SendRequest(REGISTER_CREDENTIAL_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        LOGE("RegisterCredentialCallback error: Send Request failed ret: %d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("RegisterCredentialCallback error: Failed with ret %d", ret);
        return ret;
    }
    LOGI("RegisterCredentialCallback completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("UnRegisterCredentialCallback error: Invalid para");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }

    LOGI("DeviceManager::UnRegisterCredentialCallback start, pkgName: %s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().UnRegisterCredentialCallback(pkgName);
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);

    int32_t ret = ipcClientProxy_->SendRequest(UNREGISTER_CREDENTIAL_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        LOGE("UnRegisterCredentialCallback error: Send Request failed ret: %d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("UnRegisterCredentialCallback Failed with ret %d", ret);
        return ret;
    }
    LOGI("UnRegisterCredentialCallback completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
