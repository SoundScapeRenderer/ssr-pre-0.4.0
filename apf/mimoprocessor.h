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
/// Multi-threaded MIMO (multiple input, multiple output) processor.

#ifndef APF_MIMOPROCESSOR_H
#define APF_MIMOPROCESSOR_H

#include <vector>    // for _thread_data
#include <cassert>   // for assert()
#include <stdexcept> // for std::logic_error
#include <algorithm> // for for_each

#include "apf/rtlist.h"
#include "apf/parameter_map.h"
#include "apf/math.h"  // for raised_cosine
#include "apf/misc.h"  // for NonCopyable
#include "apf/accumulating_iterator.h"  // for make_accumulating_iterator()
#include "apf/cast_iterator.h"  // for make_cast_iterator()
#include "apf/index_iterator.h"
#include "apf/function_input_iterator.h"

#define APF_MIMOPROCESSOR_TEMPLATES template<typename Derived, typename interface_policy, typename thread_policy, typename sync_policy, template<typename> class xfade_policy>
#define APF_MIMOPROCESSOR_BASE MimoProcessor<Derived, interface_policy, thread_policy, sync_policy, xfade_policy>

#ifndef MIMOPROCESSOR_DEFAULT_THREADS
#define MIMOPROCESSOR_DEFAULT_THREADS 1
#endif

namespace apf
{

template<typename T>
class raised_cosine_policy
{
  private:
    typedef function_input_iterator<index_iterator<T>, math::raised_cosine<T> >
      iterator_type;

  public:
    typedef typename std::vector<T>::const_iterator iterator;
    typedef typename std::vector<T>::const_reverse_iterator reverse_iterator;

    raised_cosine_policy(size_t block_size)
      : _xfade_data(
          iterator_type(index_iterator<T>()
            , math::raised_cosine<T>(static_cast<T>(2*block_size))),
          // block_size + 1 because we also use it in reverse order
          iterator_type(index_iterator<T>(static_cast<T>(block_size + 1))))
    {}

    iterator fade_out_begin() const { return _xfade_data.begin(); }
    reverse_iterator fade_in_begin() const { return _xfade_data.rbegin(); }

  private:
    const std::vector<T> _xfade_data;
};

template<typename T>
struct disable_xfade
{
  disable_xfade(size_t) {}

  // These functions are just there to fool the compiler ...
  int* _fade_out_begin() { throw std::logic_error("This may not be called!"); }
  int* _fade_in_begin()  { throw std::logic_error("This may not be called!"); }
};

/** Multi-threaded multiple-input-multiple-output (MIMO) processor.
 * Derive your own class from MimoProcessor and also use it as first template
 * argument. This is called the "Curiously Recurring Template Pattern" (CRTP).
 * The rest of the template arguments are \ref apf_policies ("Policy-based
 * Design").
 *
 * @tparam Derived Your derived class -> CRTP!
 * @tparam interface_policy Policy class. You can use existing policies (e.g.
 *   jack_policy, pointer_policy<T*>) or write your own policy class.
 * @tparam thread_policy Policy for threads.
 * @tparam sync_policy Policy for locks and semaphores.
 *
 * Example:
 *                                                                         @code
 * class MyProcessor : public MimoProcessor<MyProcessor
 *                                          , my_interface_policy
 *                                          , my_thread_policy
 *                                          , my_sync_policy>
 * {
 *   public:
 *     class Input : public MimoProcessorInput
 *     {
 *       explicit Input(const Params& p) : MimoProcessorInput(p) {}
 *       virtual void process()
 *       {
 *         // ...
 *         // _internal.begin() and _internal.end() gives access to audio data
 *       }
 *     };
 *     class Output : public MimoProcessorOutput
 *     {
 *       explicit Output(const Params& p) : MimoProcessorOutput(p) {}
 *       virtual void process()
 *       {
 *         // ...
 *         // _internal.begin() and _internal.end() gives access to audio data
 *       }
 *     };
 *     class MyIntermediateThing : public Item
 *     {
 *       // you can create other classes and use them in their own RtList, as
 *       // long as they are derived from Item and have a process() function.
 *       virtual void process() { ... }
 *     };
 *
 *     MyProcessor(const parameter_map& params, ...)
 *       : MimoProcessorBase(params)
 *       , _input_list(_fifo)
 *       , _intermediate_list(_fifo)
 *       , _output_list(_fifo)
 *     {
 *       ...
 *       _input_list.add(new Input(...));
 *       _intermediate_list.add(new MyIntermediateThing(...));
 *       ...
 *       this->activate();
 *     }
 *
 *     void process()
 *     {
 *       _process_list(_input_list);
 *       _process_list(_intermediate_list);
 *       _process_list(_output_list);
 *     }
 *
 *   private:
 *     rtlist_t _input_list, _intermediate_list, _output_list;
 * };
 *                                                                      @endcode
 **/
template<typename Derived
  , typename interface_policy, typename thread_policy, typename sync_policy
  , template<typename> class xfade_policy = raised_cosine_policy>
class MimoProcessor : public interface_policy
                    , public thread_policy
                    , public sync_policy
                    , public xfade_policy<typename interface_policy::sample_type>
{
  public:
    typedef typename interface_policy::sample_type sample_type;
    typedef typename thread_policy::Thread Thread;
    typedef typename sync_policy::Lock Lock;
    typedef typename sync_policy::Semaphore Semaphore;

    typedef apf::parameter_map parameter_map;
    typedef apf::CommandQueue CommandQueue;

    class Input;
    class Output;
    class InternalInput;
    class InternalOutput;
    class DefaultInput;
    class DefaultOutput;

    typedef typename  Input::Params  InputParams;
    typedef typename Output::Params OutputParams;

    template<typename L, typename item_type, typename Out>
    class combine_channels;

    /// Abstract base class for list items.
    struct Item : NonCopyable
    {
      virtual ~Item() {}

      /// to be overwritten in the derived class
      virtual void process() = 0;
    };

    /// Lock is released when it goes out of scope.
    class ScopedLock : NonCopyable
    {
      public:
        explicit ScopedLock(Lock& obj) : _obj(obj) { _obj.lock(); }
        ~ScopedLock() { _obj.unlock(); }

      private:
        Lock& _obj;
    };

    bool activate()
    {
      _fifo.reactivate();  // no return value
      return interface_policy::activate();
    }

    bool deactivate()
    {
      // TODO: clear queue here?
      if (!interface_policy::deactivate()) return false;

      // exceptionally, this is called from the non-realtime thread:
      _fifo.process_commands();  // TODO: avoid this?
      if (!_fifo.deactivate()) throw std::logic_error("Bug: FIFO not empty!");
      return true;
    }

    void wait_for_rt_thread() { _fifo.wait(); }

    const parameter_map params;

  protected:
    typedef  Input MimoProcessorInput;
    typedef Output MimoProcessorOutput;
    typedef  DefaultInput MimoProcessorDefaultInput;
    typedef DefaultOutput MimoProcessorDefaultOutput;
    typedef APF_MIMOPROCESSOR_BASE MimoProcessorBase;

    typedef RtList<Item*> rtlist_t;
    typedef typename rtlist_t::size_type      size_type;
    typedef typename rtlist_t::iterator       rtlist_iterator;
    typedef typename rtlist_t::const_iterator rtlist_const_iterator;

    /// Proxy class for accessing an RtList.
    /// @note This is for read-only access. Write access is only allowed in the
    ///   process() member function from within the object itself.
    template<typename T>
    struct rtlist_proxy : apf::cast_proxy_const<T, rtlist_t>
    {
      rtlist_proxy(const rtlist_t& l) : apf::cast_proxy_const<T, rtlist_t>(l) {}
    };

    enum { no_xfade = 0, fade_out, fade_in } _fade_type;

    explicit MimoProcessor(const parameter_map& params = parameter_map());

    /// Protected non-virtual destructor
    ~MimoProcessor()
    {
      // TODO: find better place for deactivate()?
      this->deactivate();
    }

    void _process_list(rtlist_t& l);
    void _process_list(rtlist_t& l1, rtlist_t& l2);

    CommandQueue _fifo;

  private:
    struct thread_data_t /// unique data for 1 thread
    {
      thread_data_t()
        : wait_semaphore(0)
        , cont_semaphore(0)
      {}

      int thread_number;        ///< number of thread (starting with 1)
      MimoProcessor* obj;       ///< pointer to parent object
      Semaphore wait_semaphore; ///< "wait" semaphore
      Semaphore cont_semaphore; ///< "continue" semaphore
      Thread thread;            ///< thread object
    };

    template<typename X> class Xput;
    template<typename X> class InternalXput;

    // the second template argument is a dummy argument because only partial
    // specialization is allowed in class templates.
    // see http://stackoverflow.com/q/2537716/500098
    template<template<typename> class, int = 0> class XFadeSwitch;

    virtual void _process();

    void _process_current_list(int thread_number = 0);

    void _thread(int thread_number);
    static void* _thread_aux(void* data);

    rtlist_t _internal_list; ///< internal Input/Output objects are stored here

    // TODO: make "volatile"?
    rtlist_t* _current_list;

    /// Number of threads (main thread plus worker threads)
    const int _num_threads;

    std::vector<thread_data_t> _thread_data;

    XFadeSwitch<xfade_policy> _xfade_switch;
};

/** Combine and accumulate channels, including crossfade.
 * @tparam L Input list type
 * @tparam item_type pointers in the input list are casted to this type
 * @tparam Target output type
 **/
APF_MIMOPROCESSOR_TEMPLATES
template<typename L, typename item_type, typename Target>
class APF_MIMOPROCESSOR_BASE::combine_channels
{
  private:
    typedef typename std::iterator_traits<typename Target::iterator>::value_type
      out_type;
    typedef cast_proxy_const<item_type, L> list_proxy;

  public:
    combine_channels(const L& in, Target& out
        , const MimoProcessorBase& parent)
      : _in(in)
      , _out(out)
      , _parent(parent)
      , _xfade_buffer(_parent.block_size())
      , _no_fade_out(true)
    {}

    // partial template specialization doesn't work with function templates,
    // therefore we use a dummy argument to switch behaviour.

    template<typename Predicate>
    void transform(Predicate pred) { _impl(pred, _transform_tag()); }

    template<typename Predicate>
    void copy(Predicate pred) { _impl(pred, _copy_tag()); }

  private:
    struct _transform_tag {};
    struct _copy_tag {};

    // overloaded function with unused predicate
    template<typename In, typename Out, typename Predicate>
    void
    _process_helper(bool target_is_empty, In first, In last, Out out
        , Predicate, _copy_tag)
    {
      if (target_is_empty)
      {
        // overwrite
        std::copy(first, last, out);
      }
      else
      {
        // accumulate
        std::copy(first, last, apf::make_accumulating_iterator(out));
      }
    }

    template<typename In, typename Out, typename Predicate>
    void
    _process_helper(bool target_is_empty, In first, In last, Out out
        , Predicate pred, _transform_tag)
    {
      if (target_is_empty)
      {
        // overwrite
        std::transform(first, last, out, pred);
      }
      else
      {
        // accumulate
        std::transform(first, last, apf::make_accumulating_iterator(out), pred);
      }
    }

    template<typename Predicate, typename Tag>
    void _impl(Predicate pred, Tag tag)
    {
      assert(std::distance(_out.begin(), _out.end()) == _parent.block_size());
      assert(_xfade_buffer.size() == static_cast<size_t>(_parent.block_size()));

      bool target_is_empty = true;

      for (typename list_proxy::iterator item = _in.begin()
          ; item != _in.end()
          ; ++item)
      {
        assert(std::distance(item->begin(), item->end())
            == _parent.block_size());

        if (pred(*item))
        {
          if (_parent._fade_type == no_xfade)
          {
            // write to _out
            _process_helper(target_is_empty, item->begin(), item->end()
                , _out.begin(), pred, tag);
          }
          else
          {
            // write to buffer
            _process_helper(target_is_empty, item->begin(), item->end()
                , _xfade_buffer.begin(), pred, tag);
          }

          target_is_empty = false;
        }
        else
        {
          // do nothing
        }
      }

      switch (_parent._fade_type)
      {
        case fade_out:
          if (!(_no_fade_out = target_is_empty))
          {
            std::transform(_xfade_buffer.begin(), _xfade_buffer.end()
                , _parent.fade_out_begin(), _out.begin()
                , std::multiplies<out_type>());
          }
          break;

        case fade_in:
          if (target_is_empty)
          {
            if (_no_fade_out)
            {
              std::fill(_out.begin(), _out.end(), out_type());
            }
            else
            {
              // do nothing
            }
          }
          else
          {
            if (_no_fade_out)
            {
              // overwrite
              std::transform(_xfade_buffer.begin(), _xfade_buffer.end()
                  , _parent.fade_in_begin(), _out.begin()
                  , std::multiplies<out_type>());
            }
            else
            {
              // accumulate
              std::transform(_xfade_buffer.begin(), _xfade_buffer.end()
                  , _parent.fade_in_begin()
                  , apf::make_accumulating_iterator(_out.begin())
                  , std::multiplies<out_type>());
            }
          }
          break;

        case no_xfade:
          if (target_is_empty)
          {
            std::fill(_out.begin(), _out.end(), out_type());
          }
          break;

        default:
          throw std::logic_error("Bug: unknown fade type!");
      }
    }

    const list_proxy _in;
    Target& _out;
    const MimoProcessorBase& _parent;
    std::vector<out_type> _xfade_buffer;

    bool _no_fade_out;
};

// Private helper class to avoid code duplication in internal Input/Output
// Template parameter X: Base class (Input/Output from interface_policy)
APF_MIMOPROCESSOR_TEMPLATES
template<typename X>
class APF_MIMOPROCESSOR_BASE::InternalXput : public X, public Item
{
  public:
    // Parameters for an Input or Output.
    // You can add your own parameters by deriving from it.
    struct Params
    {
      Params() : parent(0) {}
      Derived* parent;
      parameter_map dict;
    };

    virtual void process()
    {
      this->fetch_buffer();
    }

  protected:
    // Protected Constructor.
    explicit InternalXput(const Params& p)
      : X(*(p.parent
            ? p.parent
            : throw std::logic_error("Bug: Internal In/Output: parent == 0!"))
          , p.dict)
    {}
};

/// @throw std::logic_error if CommandQueue cannot be deactivated.
APF_MIMOPROCESSOR_TEMPLATES
APF_MIMOPROCESSOR_BASE::MimoProcessor(const parameter_map& params_)
  : interface_policy(params_)
  , xfade_policy<typename interface_policy::sample_type>(this->block_size())
  , params(params_)
  , _fifo(params.get("fifo_size", 128))
  , _internal_list(_fifo)
  , _current_list(0)
  , _num_threads(params.get("threads", MIMOPROCESSOR_DEFAULT_THREADS))
  , _thread_data(_num_threads-1) // Semaphores are initialized (to zero) here.
  , _xfade_switch(*this)
{
  assert(_num_threads > 0);

  // Create slave audio threads. Skip all this if we are running single-threaded
  for (int i = 1; i < _num_threads; i++)
  {
    thread_data_t* data = &_thread_data[i-1];

    data->thread_number = i; // NOTE: number 0 is reserved for the main thread
    data->obj = this;

    data->thread.create(_thread_aux, data, this->get_real_time_priority());
  }

  // deactivate FIFO for non-realtime initializations
  if (!_fifo.deactivate()) throw std::logic_error("Bug: FIFO not empty!");
}

APF_MIMOPROCESSOR_TEMPLATES
void
APF_MIMOPROCESSOR_BASE::_process_list(rtlist_t& l)
{
  _current_list = &l;
  _process_current_list();
}

APF_MIMOPROCESSOR_TEMPLATES
void
APF_MIMOPROCESSOR_BASE::_process_list(rtlist_t& l1, rtlist_t& l2)
{
  // TODO: extend for more than two lists?

  // WARNING: this is NOT conforming to the current C++ standard!
  // According to C++03 iterators to the spliced elements are invalidated!
  // In C++1x this will be fixed.
  // see http://stackoverflow.com/q/143156

  // see also http://stackoverflow.com/q/7681376

  rtlist_iterator temp = l2.begin();
  l2.splice(temp, l1);  // join lists: "L2 = L1 + L2"
  _process_list(l2);
  l1.splice(l1.end(), l2, l2.begin(), temp);  // restore original lists

  // not exception-safe (original lists are not restored), but who cares?
}

APF_MIMOPROCESSOR_TEMPLATES
void
APF_MIMOPROCESSOR_BASE::_process_current_list(int thread_number)
{
  if (thread_number == 0) // main thread triggers ...
  {
    assert(_current_list); // _current_list must not be NULL!

    if (_current_list->empty())
    {
      return;
    }
    else
    {
      // wake all threads (only if _num_threads > 1)
      for (int i = 1; i < _num_threads; ++i)
      {
        _thread_data[i-1].cont_semaphore.post();
      }
    }
  }
  else // ... the other threads wait.
  {
    _thread_data[thread_number-1].cont_semaphore.wait();
    // TODO: check if the thread should terminate (by _exiting?)

    assert(_current_list); // _current_list must not be NULL!
  }

  int n = 0;
  for (rtlist_iterator i = _current_list->begin()
      ; i != _current_list->end()
      ; ++i, ++n)
  {
    if (thread_number == n % _num_threads)
    {
      (*i)->process();
    }
  }

  if (thread_number == 0) // main thread waits (only if _num_threads > 1) ...
  {
    for (int i = 1; i < _num_threads; ++i)
    {
      _thread_data[i-1].wait_semaphore.wait();
    }
  }
  else // ... the others report that they're done.
  {
    _thread_data[thread_number-1].wait_semaphore.post();
  }
}

APF_MIMOPROCESSOR_TEMPLATES
template<template<typename> class, int>
class APF_MIMOPROCESSOR_BASE::XFadeSwitch
{
  private:
    typedef MimoProcessorBase P;

  public:
    XFadeSwitch(P& parent) : _p(parent) {}

    void operator()()
    {
      if (_p._fifo.commands_available())
      {
        _p._fade_type = fade_out;

        _p._process_list(_p._internal_list);

        static_cast<Derived*>(&_p)->process();

        _p._fifo.process_commands();

        _p._fade_type = fade_in;
      }
      else // static situation
      {
        // no crossfade
      }
    }

  private:
    P& _p;
};

APF_MIMOPROCESSOR_TEMPLATES
template<int dummy>
class APF_MIMOPROCESSOR_BASE::XFadeSwitch<disable_xfade, dummy>
{
  private:
    typedef APF_MIMOPROCESSOR_BASE P;

  public:
    XFadeSwitch(P& parent) : _p(parent) {}

    void operator()()
    {
      _p._fifo.process_commands();
    }

  private:
    P& _p;
};

APF_MIMOPROCESSOR_TEMPLATES
void
APF_MIMOPROCESSOR_BASE::_process()
{
  _fade_type = no_xfade;

  // if xfade is switched on, the fade-out-block is processed here
  _xfade_switch();

  _process_list(_internal_list);

  static_cast<Derived*>(this)->process();
}

/** Auxiliary function to start slave audio threads.  
 * @param data void pointer to thread data
 * @attention Thread numbers must be consecutive starting with 1 (thread number
 *   0 is reserved for the main thread).
 **/
APF_MIMOPROCESSOR_TEMPLATES
void*
APF_MIMOPROCESSOR_BASE::_thread_aux(void* data)
{
  thread_data_t* init_data = static_cast<thread_data_t*>(data);
  MimoProcessor* r = init_data->obj;
  r->_thread(init_data->thread_number);
  return 0;
}

/** Thread function for worker threads.
 * @param thread_number ID of thread 
 **/
APF_MIMOPROCESSOR_TEMPLATES
void
APF_MIMOPROCESSOR_BASE::_thread(int thread_number)
{
  while (1)
  {
    _process_current_list(thread_number);
  }
}

APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::InternalInput :
                           public InternalXput<typename interface_policy::Input>
{
  private:
    typedef InternalXput<typename interface_policy::Input> _base_type;

  public:
    typedef typename _base_type::iterator iterator;
    typedef typename _base_type::Params Params;

    iterator begin() const { return this->_begin; }
    iterator   end() const { return this->_end; }

  private:
    friend class APF_MIMOPROCESSOR_BASE;
    explicit InternalInput(const Params& p) : _base_type(p) {}
};

APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::InternalOutput :
                          public InternalXput<typename interface_policy::Output>
{
  private:
    typedef InternalXput<typename interface_policy::Output> _base_type;

  public:
    typedef typename _base_type::iterator iterator;
    typedef typename _base_type::Params Params;

    iterator begin() const { return this->_begin; }
    iterator   end() const { return this->_end; }

  private:
    friend class APF_MIMOPROCESSOR_BASE;
    explicit InternalOutput(const Params& p) : _base_type(p) {}
};

APF_MIMOPROCESSOR_TEMPLATES
template<typename X>
class APF_MIMOPROCESSOR_BASE::Xput : public Item
{
  public:
    typedef typename X::iterator iterator;
    typedef typename X::Params Params;

    virtual void process()
    {
      throw std::logic_error("Bug: Input/Output: process() not implemented!");
    }

    X& internal() const { return _internal; }

  protected:
    /// @throw std::logic_error if parent == NULL
    explicit Xput(const Params& p)
      : _parent(*(p.parent
          ? p.parent
          : throw std::logic_error("Bug: Input/Output: parent == 0!")))
      , _internal(*_parent._internal_list.add(new X(p)))
    {}

    ~Xput() { _parent._internal_list.rem(&_internal); }

    /// Parent object of the Input/Output (for use in derived class).
    Derived& _parent;

    X& _internal;  ///< Reference to InternalInput/InternalOutput
};

/// %Input class.
APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::Input : public Xput<InternalInput>
{
  private:
    typedef Xput<InternalInput> _base_type;

  public:
    typedef typename _base_type::Params Params;

    explicit Input(const Params& p) : _base_type(p) {}
};

/// %Input class with begin() and end().
APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::DefaultInput : public Input
{
  protected:
    using Input::_internal;

  public:
    typedef typename Input::Params Params;
    typedef typename Input::iterator iterator;

    DefaultInput(const Params& p) : Input(p) {}

    iterator begin() const { return _internal.begin(); }
    iterator   end() const { return _internal.end(); }
};

/// %Output class.
APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::Output : public Xput<InternalOutput>
{
  private:
    typedef Xput<InternalOutput> _base_type;

  public:
    typedef typename _base_type::Params Params;

    explicit Output(const Params& p) : _base_type(p) {}
};

/// %Output class with begin() and end().
APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::DefaultOutput : public Output
{
  protected:
    using Output::_internal;

  public:
    typedef typename Output::Params Params;
    typedef typename Output::iterator iterator;

    DefaultOutput(const Params& p) : Output(p) {}

    iterator begin() const { return _internal.begin(); }
    iterator   end() const { return _internal.end(); }
};

}  // namespace apf

#undef APF_MIMOPROCESSOR_TEMPLATES
#undef APF_MIMOPROCESSOR_BASE

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
