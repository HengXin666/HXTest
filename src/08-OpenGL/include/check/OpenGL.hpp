#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-05 22:49:40
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
#ifndef _HX_OPENGL_H_
#define _HX_OPENGL_H_

#include <glad/glad.h>

#include <HXprint/print.h>

namespace HX {

inline void checkOpenGLError(const char* fileName, std::size_t line, const char* code) {
    auto err = glGetError();
    if (err != GL_NO_ERROR) {
        HX::print::println(fileName, ": ", line, ": ", 
            code, " failed: ", [&]{
            switch (err) {
#define PER_GL_ERROR(x) case GL_##x: return #x;
            PER_GL_ERROR(NO_ERROR)
            PER_GL_ERROR(INVALID_ENUM)
            PER_GL_ERROR(INVALID_VALUE)
            PER_GL_ERROR(INVALID_OPERATION)
            PER_GL_ERROR(STACK_OVERFLOW)
            PER_GL_ERROR(STACK_UNDERFLOW)
            PER_GL_ERROR(OUT_OF_MEMORY)
#undef PER_GL_ERROR
            }
            return "unknown error";
        }());
    }
}

} // namespace HX

#define CHECK_GL(CODE) do {                             \
    (CODE);                                             \
    HX::checkOpenGLError(__FILE__,  __LINE__, #CODE);   \
} while (0)

#endif // !_HX_OPENGL_H_