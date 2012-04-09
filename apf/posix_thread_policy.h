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
/// POSIX thread policy class.

#ifndef APF_POSIX_THREAD_POLICY_H
#define APF_POSIX_THREAD_POLICY_H

#ifndef _REENTRANT
#error You need to compile with _REENTRANT defined since this uses threads!
#endif

#include <stdexcept> // for std::runtime_error
#include <pthread.h>

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
    class Thread;

  protected:
     posix_thread_policy() {} ///< Protected ctor.
    ~posix_thread_policy() {} ///< Protected dtor.
};

class posix_thread_policy::Thread
{
  public:
    // TODO: more general thread creation API, maybe compatible to C++0x threads
    // for now, we stick to the pthread void* stuff
    void create(void* (*f)(void*), void* data, int sched_priority)
    {
      if (pthread_create(&_thread_id, 0, f, data))
      {
        throw std::runtime_error("Can't create thread!");
      }

      struct sched_param param;
      param.sched_priority = sched_priority;
      if (param.sched_priority != -1)
      {
        if (pthread_setschedparam(_thread_id, SCHED_FIFO, &param))
        {
          throw std::runtime_error("Can't set scheduling priority for thread!");
        }
      }
    }

  private:
    pthread_t _thread_id;
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
