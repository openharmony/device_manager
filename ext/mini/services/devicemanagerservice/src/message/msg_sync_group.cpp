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

#include "msg_sync_group.h"
#include "device_manager_log.h"
#include "constants.h"
#include "device_manager_errno.h"

namespace OHOS {
namespace DistributedHardware {
MsgSyncGroup::MsgSyncGroup(std::vector<std::string> &groupList, std::string &deviceId)
{
    DMLOG(DM_LOG_INFO, "MsgSyncGroup construction started");
    mHead_ = std::make_shared<MsgHead>(DmMsgType::MSG_TYPE_SYNC_GROUP);
    for (auto str: groupList) {
        mGroupIdList_.push_back(str);
    }
    mDeviceId_ = deviceId;
    DMLOG(DM_LOG_INFO, "MsgSyncGroup construction completed");
}

void MsgSyncGroup::Encode(nlohmann::json &json)
{
    mHead_->Encode(json);
    json[TAG_DEVICE_ID] = mDeviceId_;
    json[TAG_GROUPIDS] = mGroupIdList_;
}

int32_t MsgSyncGroup::Decode(nlohmann::json &json)
{
    DMLOG(DM_LOG_INFO, "MsgSyncGroup decode started");
    if (!json.contains(TAG_DEVICE_ID) || !json.contains(TAG_GROUPIDS)) {
        DMLOG(DM_LOG_ERROR, "MsgSyncGroup::decode err ");
        return MSG_DECODE_PARA_FAILED;
    }

    MsgHead msgHead;
    mHead_ = msgHead.Decode(json);
    mDeviceId_ = json[TAG_DEVICE_ID];
    mGroupIdList_ = json.at(TAG_GROUPIDS).get<std::vector<std::string>>();
    DMLOG(DM_LOG_INFO, "MsgSyncGroup decode completed");
    return DEVICEMANAGER_OK;
}

std::string MsgSyncGroup::GetDeviceId()
{
    return mDeviceId_;
}

std::vector<std::string> MsgSyncGroup::GetGroupIdList()
{
    return mGroupIdList_;
}
}
}