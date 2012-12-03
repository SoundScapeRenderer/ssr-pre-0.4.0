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
/// A (under certain circumstances) realtime-safe list.

#ifndef APF_RTLIST_H
#define APF_RTLIST_H

#include <list>

#include "apf/commandqueue.h"

namespace apf
{

template<typename T> class RtList; // no implementation, use <T*>!

/** A list for realtime access and non-realtime modification.
 * It is normally used by a realtime thread and a non-realtime thread at the
 * same time.
 *
 * The list is created and modified (using add(), rem(), ...) by the
 * non-realtime thread. These functions are not safe for multiple non-realtime
 * threads; access has to be locked in this case.
 *
 * Before the realtime thread can access the list elements, it has to call
 * CommandQueue::process_commands() to synchronize.
 *
 * TODO: more information about which functions are allowed in which thread.
 **/
template<typename T>
class RtList<T*> : NonCopyable
{
  public:
    typedef typename std::list<T*>          list_t;
    typedef typename list_t::value_type     value_type;
    typedef typename list_t::size_type      size_type;
    typedef typename list_t::iterator       iterator;
    typedef typename list_t::const_iterator const_iterator;

    class AddCommand;  // no implementation, use <T*>!
    class RemCommand;  // no implementation, use <T*>!
    class ClearCommand;  // no implementation, use <T*>!

    // Default constructor is not allowed!

    /// Constructor.
    /// @param fifo the CommandQueue
    explicit RtList(CommandQueue& fifo)
      : _fifo(fifo)
    {}

    /// Destructor.
    /// It is safer if all elements are removed - via rem() or clear() - before
    /// the destructor is called. But if not, they are deleted here.
    ~RtList()
    {
      for (iterator i = _the_actual_list.begin()
          ; i != _the_actual_list.end()
          ; ++i)
      {
        delete *i;
      }
      _the_actual_list.clear();
    }

    /// Add an element to the list.
    /// @param item Pointer to the list item.
    /// @return the same pointer
    /// @note Ownership is passed to the list!
    template<typename X>
    X* add(X* item)
    {
      _fifo.push(new AddCommand(&_the_actual_list, item));
      return item;
    }

    /// Add a range of elements to the list.
    /// @param first Begin of range to be added.
    /// @param last End of range to be added.
    /// @note Ownership is passed to the list!
    template<typename ForwardIterator>
    void add(ForwardIterator first, ForwardIterator last)
    {
      _fifo.push(new AddCommand(&_the_actual_list, first, last));
    }

    /// Remove an element from the list.
    void rem(T* to_rem)
    {
      _fifo.push(new RemCommand(&_the_actual_list, to_rem));
    }

    /// Remove all elements from the list.
    void clear()
    {
      _fifo.push(new ClearCommand(&_the_actual_list));
    }

    void splice(iterator position, RtList& x)
    {
      assert(&_fifo == &x._fifo);
      _the_actual_list.splice(position, x._the_actual_list);
    }

    void splice(iterator position, RtList& x, iterator first, iterator last)
    {
      assert(&_fifo == &x._fifo);
      _the_actual_list.splice(position, x._the_actual_list, first, last);
    }

    ///@{ @name Functions to be called from the realtime thread
    iterator       begin()       { return _the_actual_list.begin(); }
    const_iterator begin() const { return _the_actual_list.begin(); }
    iterator       end()         { return _the_actual_list.end(); }
    const_iterator end()   const { return _the_actual_list.end(); }
    bool           empty() const { return _the_actual_list.empty(); }
    size_type      size()  const { return _the_actual_list.size(); }
    ///@}

  private:
    /// Reference to the command queue
    CommandQueue& _fifo;
    list_t _the_actual_list;  ///< Well, the actual list.
};

/// Command to add an element to a list.
template<typename T>
class RtList<T*>::AddCommand : public CommandQueue::Command
{
  public:
    /// Constructor to add a single item.
    /// @param dst_list Ptr to the list to which the element will be added.
    /// @param element Ptr to the element that will be added.
    AddCommand(list_t* dst_list, T* element)
      : _splice_list(1, element) // make list with one element
      , _dst_list(dst_list)
    {
      assert(element != 0);
      assert(_dst_list != 0);
    }

    /// Constructor to add a bunch of items at once.
    /// @param dst_list Ptr to the list to which the elements will be added.
    /// @param first Begin of range to be added.
    /// @param last End of range to be added.
    template<typename InputIterator>
    AddCommand(list_t* dst_list, InputIterator first, InputIterator last)
      : _splice_list(first, last)
      , _dst_list(dst_list)
    {
      assert(_dst_list != 0);
    }

    virtual void execute()
    {
      _dst_list->splice(_dst_list->end(), _splice_list);
    }

    // Empty function, because no cleanup is necessary. 
    virtual void cleanup() {}

  private:
    /// List of elements (actually, it's only one element) to be added.  
    list_t _splice_list;

    /// Destination list.
    list_t* const _dst_list;
};

/// Command to remove an element from a list.
template<typename T>
class RtList<T*>::RemCommand : public CommandQueue::Command
{
  public:
    /// Constructor.
    /// @param dst_list List from which the element will be removed.
    /// @param element Element that will be removed.
    RemCommand(list_t* dst_list, T* element)
      : _dst_list(dst_list)
      , _element_to_remove(element)
    {
      assert(_dst_list != 0);
      assert(_element_to_remove != 0);
    }

    virtual void execute()
    {
      for (iterator i = _dst_list->begin(); i != _dst_list->end(); ++i)
      {
        if (*i == _element_to_remove)
        {
          _splice_list.splice(_splice_list.begin(), *_dst_list, i);
          break;
        }
      }
    }

    // this might be dangerous/unexpected.
    // but i would need a synchronized Command, which waited
    // for the operation to complete, otherwise.
    virtual void cleanup()
    {
      for (iterator i = _splice_list.begin(); i != _splice_list.end(); ++i)
      {
        delete *i;
      }
      _splice_list.clear();
    }

  private:
    /// List of elements (usually, it's only one element) to be removed.
    list_t _splice_list;
    list_t* const _dst_list;     ///< Destination list
    T* const _element_to_remove; ///< self-explanatory
};

/// Command to remove all elements from a list.
template<typename T>
class RtList<T*>::ClearCommand : public CommandQueue::Command
{
  public:
    /// Constructor.
    /// @param dst_list List from which all elements will be removed.
    ClearCommand(list_t* dst_list)
      : _dst_list(dst_list)
    {
      assert(_dst_list != 0);
    }

    virtual void execute()
    {
      _delinquent_list.swap(*_dst_list);
    }

    virtual void cleanup()
    {
      for (iterator i = _delinquent_list.begin()
          ; i != _delinquent_list.end()
          ; ++i)
      {
        delete *i;
      }
      _delinquent_list.clear();
    }

  private:
    list_t _delinquent_list;  ///< List of elements to be removed
    list_t* const _dst_list;  ///< Destination list
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
