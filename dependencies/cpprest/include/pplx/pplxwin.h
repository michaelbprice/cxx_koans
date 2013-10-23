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
* pplxwin.h
*
* Windows specific pplx implementations
*
* For the latest on this and related APIs, please see http://casablanca.codeplex.com.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#ifndef _PPLXWIN_H
#define _PPLXWIN_H

#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) 
#error This file must not be included for Visual Studio 12 or later
#endif

#ifdef _MS_WINDOWS

#include "compat/windows_compat.h"
#include "pplx/pplxinterface.h"

namespace pplx
{

namespace details
{
    namespace platform
    {
        /// <summary>
        /// Returns a unique identifier for the execution thread where this routine in invoked
        /// </summary>
        _PPLXIMP long __cdecl GetCurrentThreadId();

        /// <summary>
        /// Yields the execution of the current execution thread - typically when spin-waiting
        /// </summary>
        _PPLXIMP void __cdecl YieldExecution();

        /// <summary>
        /// Caputeres the callstack
        /// </summary>
        __declspec(noinline) _PPLXIMP size_t __cdecl CaptureCallstack(void **, size_t, size_t);

#if defined(__cplusplus_winrt)
        /// <summary>
        // Internal API which retrieves the next async id.
        /// </summary>
        _PPLXIMP unsigned int __cdecl GetNextAsyncId();
#endif
    }

    /// <summary>
    /// Manual reset event
    /// </summary>
    class event_impl
    {
    public:

        static const unsigned int timeout_infinite = 0xFFFFFFFF;

        _PPLXIMP event_impl();

        _PPLXIMP ~event_impl();

        _PPLXIMP void set();

        _PPLXIMP void reset();

        _PPLXIMP unsigned int wait(unsigned int timeout);

        unsigned int wait()
        {
            return wait(event_impl::timeout_infinite);
        }

    private:
        // Windows events
        void * _M_impl;

        event_impl(const event_impl&);                  // no copy constructor
        event_impl const & operator=(const event_impl&); // no assignment operator
    };

    /// <summary>
    /// Mutex - lock for mutual exclusion
    /// </summary>
    class critical_section_impl
    {
    public:

        _PPLXIMP critical_section_impl();

        _PPLXIMP ~critical_section_impl();

        _PPLXIMP void lock();

        _PPLXIMP void unlock();

    private:

        typedef void * _PPLX_BUFFER;

        // Windows critical section
        _PPLX_BUFFER _M_impl[8];

        critical_section_impl(const critical_section_impl&);                  // no copy constructor
        critical_section_impl const & operator=(const critical_section_impl&); // no assignment operator
    };

#if _WIN32_WINNT >= _WIN32_WINNT_VISTA 
    /// <summary>
    /// Reader writer lock
    /// </summary>
    class reader_writer_lock_impl
    {
    public:

        class scoped_lock_read
        {
        public:
            explicit scoped_lock_read(reader_writer_lock_impl &_Reader_writer_lock) : _M_reader_writer_lock(_Reader_writer_lock)
            {
                _M_reader_writer_lock.lock_read();
            }

            ~scoped_lock_read()
            {
                _M_reader_writer_lock.unlock();
            }

        private:
            reader_writer_lock_impl& _M_reader_writer_lock;
            scoped_lock_read(const scoped_lock_read&);                    // no copy constructor
            scoped_lock_read const & operator=(const scoped_lock_read&);  // no assignment operator
        };

        _PPLXIMP reader_writer_lock_impl();

        _PPLXIMP void lock();

        _PPLXIMP void lock_read();

        _PPLXIMP void unlock();

    private:

        // Windows slim reader writer lock
        void * _M_impl;

        // Slim reader writer lock doesn't have a general 'unlock' method.
        // We need to track how it was acquired and release accordingly.
        // true - lock exclusive
        // false - lock shared
        bool m_locked_exclusive;
    };  
#endif // _WIN32_WINNT >= _WIN32_WINNT_VISTA 

    /// <summary>
    /// Recursive mutex
    /// </summary>
    class recursive_lock_impl
    {
    public:

        recursive_lock_impl()
            : _M_owner(-1), _M_recursionCount(0)
        {
        }

        ~recursive_lock_impl()
        {
            _ASSERTE(_M_owner == -1);
            _ASSERTE(_M_recursionCount == 0);
        }

        void recursive_lock_impl::lock()
        {
            auto id = ::pplx::details::platform::GetCurrentThreadId();

            if ( _M_owner == id )
            {
                _M_recursionCount++;
            }
            else
            {
                _M_cs.lock();
                _M_owner = id;
                _M_recursionCount = 1;
            }            
        }

        void recursive_lock_impl::unlock()
        {
            _ASSERTE(_M_owner == ::pplx::details::platform::GetCurrentThreadId());
            _ASSERTE(_M_recursionCount >= 1);

            _M_recursionCount--;

            if ( _M_recursionCount == 0 )
            {
                _M_owner = -1;
                _M_cs.unlock();
            }           
        }

    private:
        pplx::details::critical_section_impl _M_cs;
        long _M_recursionCount;
        volatile long _M_owner;
    };

    class timer_impl
    {
    public:
        timer_impl()
            : m_timerImpl(nullptr)
        {
        }

        _PPLXIMP void start(unsigned int ms, bool repeat, TaskProc_t userFunc, _In_ void * context);
        _PPLXIMP void stop(bool waitForCallbacks);

        class _Timer_interface
        {
        public:
            virtual ~_Timer_interface()
            {
            }

            virtual void start(unsigned int ms, bool repeat) = 0;
            virtual void stop(bool waitForCallbacks) = 0;
        };

    private:
        _Timer_interface * m_timerImpl;
    };

    class windows_scheduler : public pplx::scheduler_interface
    {
    public:
        _PPLXIMP virtual void schedule( TaskProc_t proc, _In_ void* param);
    };

    /// <summary>
    /// Timer
    /// </summary>
    typedef details::timer_impl timer_t;

} // namespace details

/// <summary>
///  A generic RAII wrapper for locks that implement the critical_section interface
///  std::lock_guard
/// </summary>
template<class _Lock>
class scoped_lock
{
public:
    explicit scoped_lock(_Lock& _Critical_section) : _M_critical_section(_Critical_section)
    {
        _M_critical_section.lock();
    }

    ~scoped_lock()
    {
        _M_critical_section.unlock();
    }

private:
    _Lock& _M_critical_section;

    scoped_lock(const scoped_lock&);                    // no copy constructor
    scoped_lock const & operator=(const scoped_lock&);  // no assignment operator
};

// The extensibility namespace contains the type definitions that are used internally
namespace extensibility
{
    typedef ::pplx::details::event_impl event_t;

    typedef ::pplx::details::critical_section_impl critical_section_t;
    typedef scoped_lock<critical_section_t> scoped_critical_section_t;

#if _WIN32_WINNT >= _WIN32_WINNT_VISTA 
    typedef ::pplx::details::reader_writer_lock_impl reader_writer_lock_t;
    typedef scoped_lock<reader_writer_lock_t> scoped_rw_lock_t;
    typedef reader_writer_lock_t::scoped_lock_read scoped_read_lock_t;  
#endif // _WIN32_WINNT >= _WIN32_WINNT_VISTA 


    typedef ::pplx::details::recursive_lock_impl recursive_lock_t;
    typedef scoped_lock<recursive_lock_t> scoped_recursive_lock_t;
}

/// <summary>
/// Default scheduler type
/// </summary>
typedef details::windows_scheduler default_scheduler_t;

namespace details
{
    /// <summary>
    /// Terminate the process due to unhandled exception
    /// </summary>

    #ifndef _REPORT_PPLTASK_UNOBSERVED_EXCEPTION
    #define _REPORT_PPLTASK_UNOBSERVED_EXCEPTION() do { \
        __debugbreak(); \
        std::terminate(); \
    } while(false)
    #endif // _REPORT_PPLTASK_UNOBSERVED_EXCEPTION

} // namespace details

} // namespace pplx

#endif // _MS_WINDOWS
#endif // _PPLXWIN_H
