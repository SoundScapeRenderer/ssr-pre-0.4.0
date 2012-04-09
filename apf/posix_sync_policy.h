/******************************************************************************
 * Copyright (c) 2006-2012 Quality & Usability Lab                            *
 *                         Deutsche Telekom Laboratories, TU Berlin           *
 *                         Ernst-Reuter-Platz 7, 10587 Berlin, Germany        *
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
 *                                                http://tu-berlin.de/?id=apf *
 ******************************************************************************/

/// @file
/// POSIX sync policy class.

#ifndef APF_POSIX_SYNC_POLICY_H
#define APF_POSIX_SYNC_POLICY_H

#include <stdexcept> // for std::runtime_error
#include <pthread.h>
#include <semaphore.h>

#include "apf/misc.h"  // for NonCopyable

#ifndef APF_MIMOPROCESSOR_SYNC_POLICY
#define APF_MIMOPROCESSOR_SYNC_POLICY apf::posix_sync_policy
#endif

namespace apf
{

/// @c sync_policy using the POSIX mutex and semaphore library.
/// @see MimoProcessor
/// @ingroup apf_policies
class posix_sync_policy
{
  public:
    class Lock;
    // TODO: read-write lock?

    class Semaphore;

  protected:
     posix_sync_policy() {} ///< Protected ctor.
    ~posix_sync_policy() {} ///< Protected dtor.
};

/** Inner type Lock.
 * Wrapper class for a mutex.
 **/
class posix_sync_policy::Lock : NonCopyable
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

class posix_sync_policy::Semaphore
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
