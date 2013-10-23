/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved. 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* pplxconv.h
*
* Utilities to convert between PPL tasks and PPLX tasks
*
* For the latest on this and related APIs, please see http://casablanca.codeplex.com.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#ifndef _PPLXCONV_H
#define _PPLXCONV_H

#ifndef _MS_WINDOWS
#error This is only supported on Windows
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1700)

#if (_MSC_VER >= 1800)
#error This file must not be included for Visual Studio 12 or later
#endif

#include <ppltasks.h>
#include "pplx/pplxtasks.h"

namespace pplx
{
namespace _Ppl_conv_helpers
{
template<typename _Tc, typename _F>
auto _Set_value(_Tc _Tcp, const _F& _Func) -> decltype(_Tcp.set(_Func())) { return _Tcp.set(_Func()); }

template<typename _Tc, typename _F>
auto _Set_value(_Tc _Tcp, const _F& _Func, ...) -> decltype(_Tcp.set()) { _Func(); return _Tcp.set(); }

template<typename _TaskType, typename _OtherTaskType, typename _OtherTCEType>
_OtherTaskType _Convert_task(_TaskType _Task)
{
    _OtherTCEType _Tc;
    _Task.then([_Tc](_TaskType _Task2) {
        try
        {
            _Ppl_conv_helpers::_Set_value(_Tc, [=]{ return _Task2.get(); });
        }
        catch(...)
        {
            _Tc.set_exception(std::current_exception());
        }
    });
    _OtherTaskType _T_other(_Tc);
    return _T_other;
}
}

template<typename _TaskType>
concurrency::task<_TaskType> pplx_task_to_concurrency_task(pplx::task<_TaskType> _Task)
{
    return _Ppl_conv_helpers::_Convert_task<typename pplx::task<_TaskType>, concurrency::task<_TaskType>, concurrency::task_completion_event<_TaskType>>(_Task);
}

template<typename _TaskType>
pplx::task<_TaskType> concurrency_task_to_pplx_task(concurrency::task<_TaskType> _Task)
{
    return _Ppl_conv_helpers::_Convert_task<typename concurrency::task<_TaskType>, pplx::task<_TaskType>, pplx::task_completion_event<_TaskType>>(_Task);
}
} // namespace pplx

#endif

#endif // _PPLXCONV_H
