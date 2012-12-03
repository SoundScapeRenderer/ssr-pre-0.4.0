/******************************************************************************
 * Copyright © 2012      Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the Audio Processing Framework (APF).                 *
 *                                                                            *
 * The APF is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The APF is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 *                                 http://AudioProcessingFramework.github.com *
 ******************************************************************************/

/// @file
/// POSIX thread policy class.

#ifndef APF_POSIX_THREAD_POLICY_H
#define APF_POSIX_THREAD_POLICY_H

#ifndef _REENTRANT
#error You need to compile with _REENTRANT defined since this uses threads!
#endif

#include <stdexcept>  // for std::runtime_error
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>  // for usleep()

#include "apf/misc.h"  // for NonCopyable

#ifndef APF_MIMOPROCESSOR_THREAD_POLICY
#define APF_MIMOPROCESSOR_THREAD_POLICY apf::posix_thread_policy
#endif

namespace apf
{

/// @c thread_policy using the POSIX thread library.
/// @see MimoProcessor
/// @ingroup apf_policies
class posix_thread_policy
{
  public:
    typedef useconds_t useconds_type;

    template<typename F> class ScopedThread;
    template<typename F> class DetachedThread;
    class Lock;  // TODO: read-write lock?
    class Semaphore;

  protected:
     posix_thread_policy() {}  ///< Protected ctor.
    ~posix_thread_policy() {}  ///< Protected dtor.

  private:
    class ThreadBase;
};

class posix_thread_policy::ThreadBase
{
  public:
    typedef pthread_t native_handle_type;

    void create(void* (*f)(void*), void* data)
    {
      if (pthread_create(&_thread_id, 0, f, data))
      {
        throw std::runtime_error("Can't create thread!");
      }
    }

    bool join() { return !pthread_join(_thread_id, 0); }

    native_handle_type native_handle() const { return _thread_id; }

  protected:
    ThreadBase() {}
    ~ThreadBase() {}

  private:
    native_handle_type _thread_id;
};

template<typename F>
class posix_thread_policy::ScopedThread : public ThreadBase, NonCopyable
{
  public:
    ScopedThread(F f, useconds_type usleeptime)
      : _kill_thread(false)
      , _function(f)
      , _usleeptime(usleeptime)
    {
      this->create(&_thread_aux, this);
    }

    ~ScopedThread()
    {
      _kill_thread = true;
      this->join();
    }

  private:
    static void* _thread_aux(void *arg)
    {
      static_cast<ScopedThread*>(arg)->_thread();
      return 0;
    }

    void _thread()
    {
      while (!_kill_thread)
      {
        _function();
        usleep(_usleeptime);
      }
    }

    volatile bool _kill_thread;
    F _function;
    useconds_type _usleeptime;
};

template<typename F>
class posix_thread_policy::DetachedThread : public ThreadBase
{
  public:
    DetachedThread(F f)
      : _function(f)
    {
      this->create(&_thread_aux, this);
      pthread_detach(this->native_handle());  // return value is ignored!
    }

  private:
    static void* _thread_aux(void* arg)
    {
      static_cast<DetachedThread*>(arg)->_thread();
      return 0;
    }

    void _thread()
    {
      for (;;)
      {
        _function();
      }
    }

    F _function;
};

/** Inner type Lock.
 * Wrapper class for a mutex.
 **/
class posix_thread_policy::Lock : NonCopyable
{
  public:
    // TODO: parameter: initial lock state?
    Lock()
    {
      if (pthread_mutex_init(&_lock, NULL))
      {
        throw std::runtime_error("Can't init mutex. (impossible !!!)");
      }
    }

    // TODO: change return type to bool?
    int   lock() { return pthread_mutex_lock(  &_lock); }
    int unlock() { return pthread_mutex_unlock(&_lock); }

    // TODO: trylock?

  private:
    pthread_mutex_t _lock;
};

class posix_thread_policy::Semaphore : NonCopyable
{
  public:
    typedef unsigned int value_type;

    explicit Semaphore(value_type value = 0)
    {
      if (sem_init(&_semaphore, 0, value))
      {
        throw std::runtime_error("Error initializing Semaphore!");
      }
    }

    ~Semaphore() { sem_destroy(&_semaphore); }

    bool post() { return sem_post(&_semaphore) == 0; }
    bool wait() { return sem_wait(&_semaphore) == 0; }

  private:
    sem_t _semaphore;
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
