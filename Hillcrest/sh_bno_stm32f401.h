/****************************************************************************
* Copyright (C) 2016 Hillcrest Laboratories, Inc.
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License and 
* any applicable agreements you may have with Hillcrest Laboratories, Inc.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef BNO070_H
#define BNO070_H

#include "stm32f4xx_hal.h"

void bno_init(I2C_HandleTypeDef * _hi2c, TIM_HandleTypeDef * _htim);

#endif