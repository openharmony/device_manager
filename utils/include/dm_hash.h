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

#ifndef OHOS_DM_HASH_H
#define OHOS_DM_HASH_H

#include <stdint.h>

namespace OHOS {
namespace DistributedHardware {
int32_t GetUdidHash(uint8_t *udid, int32_t udiddataLen, uint8_t outudiddData[32]);
int32_t GenerateStrHash(const unsigned char *strInput, uint32_t len, unsigned char *hash);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_HASH_H