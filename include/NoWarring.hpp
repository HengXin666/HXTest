#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-26 18:45:38
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
#ifndef _HX_NOWARRING_H_
#define _HX_NOWARRING_H_

#if defined(_MSC_VER)
    #define DISABLE_ALL_WARNINGS            \
        __pragma(warning(push, 0))          \
        __pragma(warning(disable: 4100 4189 4996)) // 可选: MSVC的一些常见噪声
    #define ENABLE_ALL_WARNINGS             \
        __pragma(warning(pop))

#elif defined(__clang__)
    #define DISABLE_ALL_WARNINGS            \
        _Pragma("clang diagnostic push")    \
        _Pragma("clang diagnostic ignored \"-Wall\"")          \
        _Pragma("clang diagnostic ignored \"-Wextra\"")        \
        _Pragma("clang diagnostic ignored \"-Wpedantic\"")     \
        _Pragma("clang diagnostic ignored \"-Weverything\"")   // 全关闭
    #define ENABLE_ALL_WARNINGS             \
        _Pragma("clang diagnostic pop")

#elif defined(__GNUC__)
    #define DISABLE_ALL_WARNINGS            \
        _Pragma("GCC diagnostic push")      \
        _Pragma("GCC diagnostic ignored \"-Wall\"")            \
        _Pragma("GCC diagnostic ignored \"-Wextra\"")          \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"")
    #define ENABLE_ALL_WARNINGS             \
        _Pragma("GCC diagnostic pop")

#else
    #define DISABLE_ALL_WARNINGS
    #define ENABLE_ALL_WARNINGS
#endif

#endif // !_HX_NOWARRING_H_