/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dm_hidumper.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(HidumpHelper);
int32_t HidumpHelper::HiDump(const std::vector<std::string>& args, std::string &result)
{
    return DM_OK;
}

int32_t HidumpHelper::GetArgsType(const std::vector<std::string>& args, std::vector<HidumperFlag> &Flag)
{
    return DM_OK;
}

void HidumpHelper::SetNodeInfo(const DmDeviceInfo& deviceInfo, const bool deviceStates)
{
    return;
}

} // namespace DistributedHardware
} // namespace OHOS
