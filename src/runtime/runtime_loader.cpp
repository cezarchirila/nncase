/* Copyright 2020 Canaan Inc.
 *
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
#ifdef WIN32
#include <Windows.h>
#endif

#include "runtime_loader.h"
#include <fmt/format.h>
#include <nncase/runtime/runtime_module.h>

using namespace nncase;
using namespace nncase::runtime;

#define STR_(x) #x
#define STR(x) STR_(x)

#ifndef NNCASE_SIMULATOR
// builtin runtime
//#include <builtin_runtimes.inl>
#endif

namespace
{
#ifdef WIN32
#define TRY_WIN32_IF_NOT(x)                                                       \
    if (!(x))                                                                     \
    {                                                                             \
        return err(std::error_condition(GetLastError(), std::system_category())); \
    }

result<rt_module_activator_t> find_runtime_activator(const module_type_t &type)
{
#ifdef NNCASE_SIMULATOR
    auto module_name = fmt::format("nncase.module.{}.dll", type.data());
#else
    auto module_name = fmt::format("nncase.rt_module.{}.dll", type.data());
#endif
    auto mod = LoadLibraryA(module_name.c_str());
    TRY_WIN32_IF_NOT(mod);
    auto proc = GetProcAddress(mod, STR(RUNTIME_MODULE_ACTIVATOR_NAME));
    TRY_WIN32_IF_NOT(proc);
    return ok(reinterpret_cast<rt_module_activator_t>(proc));
}
#else
#define NNCASE_NO_LOADABLE_RUNTIME
#endif
}

result<std::unique_ptr<runtime_module>> runtime_module::create(const module_header &header)
{
    result<std::unique_ptr<runtime_module>> rt_module(nncase_errc::runtime_not_found);
//#ifndef NNCASE_SIMULATOR
//    for (auto &reg : builtin_runtimes)
//    {
//        if (!strcmp(target_id.data(), reg.id.data()))
//        {
//            return reg.activator();
//        }
//    }
//#endif

#ifndef NNCASE_NO_LOADABLE_RUNTIME
    try_var(activator, find_runtime_activator(header.type));
    activator(header, rt_module);
#else
    result = err(nncase_errc::runtime_not_found);
#endif
    return rt_module;
}
