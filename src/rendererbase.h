/******************************************************************************
 * Copyright (c) 2006-2012 Quality & Usability Lab                            *
 *                         Deutsche Telekom Laboratories, TU Berlin           *
 *                         Ernst-Reuter-Platz 7, 10587 Berlin, Germany        *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://tu-berlin.de/?id=ssr                  SoundScapeRenderer@telekom.de *
 ******************************************************************************/

/** @file
 * Renderer base class.
 *
 * $LastChangedDate: 2012-11-27 19:23:02 +0100 (Die, 27. Nov 2012) $
 * $LastChangedRevision: 2009 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef SSR_RENDERERBASE_H
#define SSR_RENDERERBASE_H

#include <string>

#include "apf/mimoprocessor.h"
#include "apf/shareddata.h"
#include "apf/parameter_map.h"
#include "apf/math.h"  // for dB2linear()

// TODO: avoid multiple ambiguous "Source" classes
#include "source.h"  // for ::Source::model_t

#include "maptools.h"

#ifndef SSR_QUERY_POLICY
#define SSR_QUERY_POLICY apf::disable_queries
#endif

namespace ssr
{

/** Renderer base class.
 * @todo more documentation!
 *
 * The parallel rendering engine uses the non-blocking datastructure RtList to
 * communicate between realtime and non-realtime threads.
 * All non-realtime accesses to RtList%s have to be locked with lock()/unlock()
 * or with ScopedLock to ensure single-reader/single-writer operation.
 **/
template<typename Derived>
class RendererBase : public apf::MimoProcessor<Derived
                     , APF_MIMOPROCESSOR_INTERFACE_POLICY
                     , APF_MIMOPROCESSOR_THREAD_POLICY
                     , SSR_QUERY_POLICY>
{
  private:
    typedef apf::MimoProcessor<Derived, APF_MIMOPROCESSOR_INTERFACE_POLICY
      , APF_MIMOPROCESSOR_THREAD_POLICY, SSR_QUERY_POLICY> Base;

  public:
    typedef typename Base::rtlist_t rtlist_t;
    typedef typename Base::DefaultInput Input;
    typedef typename Base::ScopedLock ScopedLock;
    typedef typename Base::sample_type sample_type;

    using Base::_fifo;

    class Source;
    class Output;

    struct State
    {
      State(apf::CommandQueue& fifo)
        : reference_position(fifo)
        , reference_orientation(fifo)
        , reference_offset_position(fifo)
        , reference_offset_orientation(fifo)
        , master_volume(fifo)
        , processing(fifo, true)
        , amplitude_reference_distance(fifo, 3)
      {}

      apf::SharedData<Position> reference_position;
      apf::SharedData<Orientation> reference_orientation;
      apf::SharedData<Position> reference_offset_position;
      apf::SharedData<Orientation> reference_offset_orientation;
      apf::SharedData<sample_type> master_volume;
      apf::SharedData<bool> processing;
      apf::SharedData<sample_type> amplitude_reference_distance;
    } state;

    // If you don't need a list proxy, just use a reference to the list
    template<typename L, typename ListProxy, typename DataMember>
    class AddToSublistCommand : public apf::CommandQueue::Command
    {
      public:
        AddToSublistCommand(L input, ListProxy output, DataMember member)
          : _input(input)
          , _output(output)
          , _member(member)
        {}

        virtual void execute()
        {
          apf::distribute_list(_input, _output, _member);
        }

        // Empty function, because no cleanup is necessary. 
        virtual void cleanup() {}

      private:
        L _input;
        ListProxy _output;
        DataMember _member;
    };

    template<typename L, typename ListProxy, typename DataMember>
    void add_to_sublist(const L& input, ListProxy output, DataMember member)
    {
      _fifo.push(new AddToSublistCommand<L, ListProxy, DataMember>(
            input, output, member));
    }

    template<typename L, typename ListProxy, typename DataMember>
    class RemFromSublistCommand : public apf::CommandQueue::Command
    {
      public:
        RemFromSublistCommand(L input, ListProxy output, DataMember member)
          : _input(input)
          , _output(output)
          , _member(member)
        {}

        virtual void execute()
        {
          apf::undistribute_list(_input, _output, _member, _garbage);
        }

        virtual void cleanup()
        {
          // Nothing to be done. _garbage is taken care of in the destructor.
        }

      private:
        L _input;
        ListProxy _output;
        DataMember _member;
        L _garbage;
    };

    template<typename L, typename ListProxy, typename DataMember>
    void rem_from_sublist(const L& input, ListProxy output, DataMember member)
    {
      _fifo.push(new RemFromSublistCommand<L, ListProxy, DataMember>(
            input, output, member));
    }

    int add_source(const apf::parameter_map& p = apf::parameter_map());
    void rem_source(Source* source);
    void rem_all_sources();

#if 0
    void process();
#endif

    Source* get_source(int id);

    // May only be used in realtime thread!
    const rtlist_t& get_source_list() const { return _source_list; }
    const rtlist_t& get_output_list() const { return _output_list; }

    const std::map<int, Source*>& get_source_map() const { return _source_map; }

    sample_type get_master_level() const { return _master_level; }

    bool show_head() const { return _show_head; }

    const sample_type master_volume_correction;  // linear

  protected:
    /// %Renderer exception  
    struct renderer_error : public std::runtime_error
    {
      renderer_error(const std::string& s): std::runtime_error(s) {}
    };

    RendererBase(const apf::parameter_map& p);

    template<typename P>
    Output* _add_output(const P& p)
    {
      P temp = p;
      temp.parent = static_cast<Derived*>(this);
      Output* out = new typename Derived::Output(temp);
      _output_list.add(out);
      return out;
    }

    // TODO: make private?
    sample_type _master_level;

    rtlist_t _input_list, _source_list, _output_list;

    // TODO: find a better solution to get loudspeaker vs. headphone renderer
    bool _show_head;

  private:
    class _find_source
    {
      public:
        _find_source(Source* source) : _source(source) {}

        bool operator()(const std::pair<int, Source*>& in)
        {
          return in.second == _source;
        }

      private:
        Source* _source;
    };

    int _get_new_id();

    std::map<int, Source*> _source_map;

    int _highest_id;

    typename Base::Lock _lock;
};

/** Constructor.
 * @param p Parameters for RendererBase and MimoProcessor
 **/
template<typename Derived>
RendererBase<Derived>::RendererBase(const apf::parameter_map& p)
  : Base(p)
  , state(_fifo)
  , master_volume_correction(apf::math::dB2linear(
        this->params.get("master_volume_correction", 0.0)))
  , _master_level()
  , _input_list(_fifo)
  , _source_list(_fifo)
  , _output_list(_fifo)
  , _show_head(true)
  , _highest_id(0)
{}

template<typename Derived>
int RendererBase<Derived>::add_source(const apf::parameter_map& p)
{
  ScopedLock guard(_lock);

  // TODO: re-use existing inputs?

  typename Derived::Input::Params temp;
  temp = p;
  temp.parent = static_cast<Derived*>(this);
  const typename Derived::Input* in = _input_list.add(new typename Derived::Input(temp));

  Source* src = _source_list.add(new typename Derived::Source(_fifo, *in));

  // This cannot be done in the Derived::Source constructor because then the
  // connections to the Outputs are active before the Source is properly added
  // to the source list:
  static_cast<typename Derived::Source*>(src)->connect_to_outputs(_output_list);

  int id = _get_new_id();
  // TODO: check if id already exists? e.g. if (_source_map.count(id) > 0) {...}
  // ... but this should never happen ...

  _source_map[id] = src;
  return id;
  // TODO: what happens on failure? can there be failure?
}

template<typename Derived>
void RendererBase<Derived>::rem_source(Source* source)
{
  // TODO: remove by ID instead of by pointer?
  ScopedLock guard(_lock);

  // work-around to delete source from _source_map
  typename std::map<int, Source*>::iterator delinquent
    = std::find_if(_source_map.begin(), _source_map.end()
        , _find_source(source));
  _source_map.erase(delinquent);

  static_cast<typename Derived::Source*>(source)->disconnect_from_outputs(
      _output_list);

  Input* input = const_cast<Input*>(&source->_input);
  _source_list.rem(source);

  // TODO: really remove the corresponding Input?
  // ATTENTION: there may be several sources using the input! (or not?)
  // ... and there may be the case that the number of inputs is constant.

  // TODO: this is temporary, doesn't work with a fixed number of inputs!
  _input_list.rem(input);
}

template<typename Derived>
void RendererBase<Derived>::rem_all_sources()
{
  while (!_source_map.empty())
  {
    this->rem_source(_source_map.begin()->second);
  }
  _highest_id = 0;
}

#if 0
/** Main audio callback.
 * Is called by MimoProcessor in each audio cycle.
 * @see process_rt_commands()
 **/
template<typename Derived>
void
RendererBase<Derived>::process()
{
  this->_process_list(_source_list);
  this->_process_list(_output_list);
}
#endif

template<typename Derived>
typename RendererBase<Derived>::Source*
RendererBase<Derived>::get_source(int id)
{
  return maptools::get_item(_source_map, id);
}

template<typename Derived>
int
RendererBase<Derived>::_get_new_id()
{
  return ++_highest_id;
}

/// A sound source.
template<typename Derived>
class RendererBase<Derived>::Source : public Base::Item
                       , public apf::has_begin_and_end<typename Input::iterator>
{
  public:
    typedef typename std::iterator_traits<typename Input::iterator>::value_type
      sample_type;

    // TODO: remove. This is a temporary solution for rem_source
    friend class RendererBase<Derived>;

    Source(apf::CommandQueue& fifo, const Input& in)
      : position(fifo)
      , orientation(fifo)
      , gain(fifo, sample_type(1.0))
      , mute(fifo, false)
      , model(fifo, ::Source::point)
      , weighting_factor()
      , old_weighting_factor()
      , _input(in)
      , _pre_fader_level()
      , _level()
    {}

    virtual void process()
    {
      this->_begin = _input.begin();
      this->_end = _input.end();

      this->old_weighting_factor = this->weighting_factor;

      if (!_input.parent.state.processing() || this->mute())
      {
        this->weighting_factor = 0.0;
      }
      else
      {
        this->weighting_factor = this->gain();
        // If the renderer does something nonlinear, the master volume should be
        // applied to the output signal ... TODO: shall we care?
        this->weighting_factor *= _input.parent.state.master_volume();
        this->weighting_factor *= _input.parent.master_volume_correction;
      }

      _level_helper(_input.parent);

      // TODO: re-think (and improve!) this mechanism:
      static_cast<typename Derived::Source*>(this)->do_process();
    }

    sample_type get_level() const { return _level; }

    // In the default case, the output level are ignored
    bool get_output_levels(sample_type*, sample_type*) const { return false; }

    /// This is empty in the base class. Can be overwritten in derived class.
    void connect_to_outputs(rtlist_t&) {}
    /// This is empty in the base class. Can be overwritten in derived class.
    void disconnect_from_outputs(rtlist_t&) {}

    apf::SharedData<Position> position;
    apf::SharedData<Orientation> orientation;
    apf::SharedData<sample_type> gain;
    apf::SharedData<bool> mute;
    apf::SharedData< ::Source::model_t> model;

    sample_type weighting_factor, old_weighting_factor;

  protected:
    const Input& _input;

  private:
    void _level_helper(apf::enable_queries&)
    {
      _pre_fader_level = apf::math::max_amplitude(_input.begin(), _input.end());
      _level = _pre_fader_level * this->weighting_factor;
    }

    void _level_helper(apf::disable_queries&) {}

    sample_type _pre_fader_level;
    sample_type _level;

#if 0
  private:
    std::map<int,bool> _activity_map; ///< Under development  

  public:
    void set_output_activity(int output, bool active);
#endif
};

template<typename Derived>
class RendererBase<Derived>::Output : public Base::Output
{
  public:
    Output(const typename Base::Output::Params& p)
      : Base::Output(p)
      , _level()
    {}

    sample_type get_level() const { return _level; }

  protected:
    void _level_helper(apf::enable_queries&)
    {
      _level = apf::math::max_amplitude(this->_internal.begin()
          , this->_internal.end());
    }

    void _level_helper(apf::disable_queries&) {}

  private:
    sample_type _level;
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
