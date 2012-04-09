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
/// Command queue.

#ifndef APF_COMMANDQUEUE_H
#define APF_COMMANDQUEUE_H

#include <unistd.h> // for usleep()
#include <cassert>  // for assert()

#include "apf/lockfreefifo.h"

namespace apf
{

/** Manage command queue from non-realtime thread to realtime thread.
 * Commands can be added in the non-realtime thread (push() or push_and_wait()).
 *
 * Commands are executed when process_commands() is called from the realtime
 * thread.
 **/
class CommandQueue : NonCopyable
{
  public:
    /// Abstract base class for realtime commands.
    /// These commands are passed through queues into the realtime thread and
    /// after execution back to the non-realtime thread for cleanup.
    struct Command : NonCopyable
    {
      /// dtor.
      virtual ~Command() {}

      /// The actual command. This is called from the realtime thread.
      /// Overwritten in the derived class.
      virtual void execute() = 0;

      /// Cleanup of resources. This is called from the non-realtime thread.
      /// Overwritten in the derived class.
      virtual void cleanup() = 0;
    };

    /// Dummy command to synchronize with non-realtime thread.
    struct NoOpCommand : public Command
    {
      virtual void execute() {}
      virtual void cleanup() {}
    };

    /// Constructor.
    /// @param size maximum number of commands in queue.
    explicit CommandQueue(size_t size)
      : _in_fifo(size)
      , _out_fifo(size)
      , _active(true)
    {}

    /// Destructor.
    /// @attention Commands in the cleanup queue are cleaned up, but commands in
    /// the process queue are ignored and their memory is not freed!
    ~CommandQueue()
    {
      this->cleanup_commands();
      // TODO: warning if process queue is not empty?
      // TODO: if inactive -> process commands (if active -> ???)
    }

    /// @name Functions to be called from the non-realtime thread
    /// If there are multiple non-realtime threads, access has to be locked!
    //@{

    inline void push(Command* cmd);
    inline void wait(Command* cmd = 0);

    /// Trivial combination of push() and wait()
    void push_and_wait(Command* cmd) { this->push(cmd); this->wait(cmd); }

    /// Clean up all commands in the cleanup-queue.
    /// @note This function must be called from the non-realtime thread.
    void cleanup_commands()
    {
      Command* cmd;
      while ((cmd = _out_fifo.pop()) != 0) { _cleanup(cmd); }
    }

    // TODO: avoid return value?
    /// Deactivate queue; process following commands in the non-realtime thread.
    /// @return @b true on success
    /// @note The queue must be empty. If not, the queue is @em not deactivated
    ///   and @b false is returned.
    inline bool deactivate()
    {
      this->cleanup_commands();
      if (_in_fifo.empty()) _active = false;
      return !_active;
    }

    /// Re-activate queue. @see deactivate().
    inline void reactivate()
    {
      this->cleanup_commands();
      assert(_in_fifo.empty());
      _active = true;
    }

    //@}

    /// @name Functions to be called from the realtime thread
    //@{

    /// Execute all commands in the queue.
    /// After execution, the commands are queued for cleanup in the non-realtime
    /// thread.
    /// @note This function must be called from the realtime thread.
    void process_commands()
    {
      Command* cmd;
      while ((cmd = _in_fifo.pop()) != 0)
      {
        cmd->execute();
        _out_fifo.push(cmd);  // ignore return value
        // If _out_fifo is full, cmd is not cleaned up!
        // This is very unlikely to happen (if not impossible).
      }
    }

    /// Check if commands are available.
    /// @return @b true if commands are available.
    bool commands_available() const
    {
      return !_in_fifo.empty();
    }

    //@}

  private:
    /// Clean up and delete a command @p cmd
    void _cleanup(Command* cmd)
    {
      assert(cmd != 0);
      cmd->cleanup();
      delete cmd;
    }

    /// Queue of commands to execute in realtime thread
    LockFreeFifo<Command*> _in_fifo;
    /// Queue of executed commands to delete in non-realtime thread
    LockFreeFifo<Command*> _out_fifo;

    bool _active;  ///< default: true
};

/** Push a command to be executed in the realtime thread.
 * @note The command will be cleaned up when it comes back from the
 * realtime thread.
 * @param cmd The command to be executed. 
 **/
void CommandQueue::push(Command* cmd)
{
  // First remove all commands from _out_fifo.
  // This ensures that it's not going to be full which would block 
  // process_commands() and its calling realtime thread.  

  this->cleanup_commands();

  if (_active)
  {
    // Now push the command on _in_fifo; if the FIFO is full: retry, retry, ...
    while (!_in_fifo.push(cmd))
    {
      usleep(50);
    }
  }
  else
  {
    assert(cmd != 0);
    cmd->execute();
    _out_fifo.push(cmd);  // ignore return value, we know _out_fifo is empty
    // we don't clean up, because otherwise wait() wouldn't work!
  }
}

/** Wait for command execution.
 * Wait until @p cmd returns after execution by the realtime thread and
 * clean up its remains. All commands returning before @p cmd are also cleaned
 * up.
 * @param cmd The command to wait for. If no command is specified, wait until
 *   all commands are finished.
 **/
void CommandQueue::wait(Command* cmd)
{
  if (cmd == 0)
  {
    cmd = new NoOpCommand();
    this->push(cmd);
  }
  bool end_loop = false;
  while (!end_loop)
  {
    Command* out_cmd = _out_fifo.pop();
    if (out_cmd == 0)
    {
      usleep(50);
      continue;
    }
    if (out_cmd == cmd) end_loop = true;

    _cleanup(out_cmd);
  }
}

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
