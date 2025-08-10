#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-08-10 17:14:37
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HX_HXAV_H_
#define _HX_HXAV_H_

#include <HXLibs/macro/log.hpp>

#define __HX_AV_INFO__ "本视频为赤石C++, 请理性看待; 不建议使用在开源代码中 (公司代码无所谓)"

/**
 * @brief 编译日志宏
 */
#define HX_AV_INFO(x) _Pragma(__HX_MACRO_TO_STR__(message("前排提醒: " __HX_MACRO_TO_STR__(x))))

#endif // !_HX_HXAV_H_