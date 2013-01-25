/******************************************************************************
 * Copyright © 2012      Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
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
 * http://SoundScapeRenderer.github.com                  ssr@spatialaudio.net *
 ******************************************************************************/

/// @file
/// Renderer base class.

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

    APF_PROCESS(RendererBase, Base)
    {
      this->_process_list(_source_list);
    }

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

    Source* get_source(int id);

    // May only be used in realtime thread!
    const rtlist_t& get_source_list() const { return _source_list; }

    const std::map<int, Source*>& get_source_map() const { return _source_map; }

    sample_type get_master_level() const { return _master_level; }

    bool show_head() const { return _show_head; }

    rtlist_t& get_non_const_output_list()
    {
      return const_cast<rtlist_t&>(this->get_output_list());
    }

    // TODO: proper solution for getting the reproduction setup
    template<typename SomeListType>
    void get_loudspeakers(SomeListType&) {}

    const sample_type master_volume_correction;  // linear

  protected:
    /// %Renderer exception  
    struct renderer_error : public std::runtime_error
    {
      renderer_error(const std::string& s): std::runtime_error(s) {}
    };

    RendererBase(const apf::parameter_map& p);

    // TODO: make private?
    sample_type _master_level;

    rtlist_t _source_list;

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

    apf::parameter_map _add_params(const apf::parameter_map& params)
    {
      apf::parameter_map temp(params);
      temp.set("name", params.get("name", Derived::name()));
      return temp;
    }

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
  : Base(_add_params(p))
  , state(_fifo)
  , master_volume_correction(apf::math::dB2linear(
        this->params.get("master_volume_correction", 0.0)))
  , _master_level()
  , _source_list(_fifo)
  , _show_head(true)
  , _highest_id(0)
{}

/** Create a new source.
 * @return ID of new source
 * @throw unknown whatever the Derived::Source constructor throws
 **/
template<typename Derived>
int RendererBase<Derived>::add_source(const apf::parameter_map& p)
{
  ScopedLock guard(_lock);

  int id = _get_new_id();

  typename Derived::Input::Params in_params;
  in_params = p;
  in_params.set("id", in_params.get("id", id));
  typename Derived::Input* in = this->add(in_params);

  // WARNING: if Derived::Input throws an exception, the SSR crashes!

  typename Derived::Source::Params src_params;
  src_params = p;
  assert(dynamic_cast<Derived*>(this));
  src_params.parent = static_cast<Derived*>(this);
  src_params.fifo = &_fifo;
  src_params.input = in;

  typename Derived::Source* src;

  try
  {
    src = _source_list.add(new typename Derived::Source(src_params));
  }
  catch (...)
  {
    // TODO: really remove the corresponding Input?
    this->rem(in);
    throw;
  }

  // This cannot be done in the Derived::Source constructor because then the
  // connections to the Outputs are active before the Source is properly added
  // to the source list:
  src->connect(*static_cast<Derived*>(this));

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

  assert(dynamic_cast<typename Derived::Source*>(source));
  static_cast<typename Derived::Source*>(source)->disconnect(
      *static_cast<Derived*>(this));

  Input* input = const_cast<Input*>(&source->_input);
  _source_list.rem(source);

  // TODO: really remove the corresponding Input?
  // ATTENTION: there may be several sources using the input! (or not?)

  this->rem(input);
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
class RendererBase<Derived>::Source
                   : public Base::template ProcessItem<typename Derived::Source>
                   , public apf::has_begin_and_end<typename Input::iterator>
{
  private:
    typedef typename Base::template ProcessItem<typename Derived::Source>
      SourceBase;

  public:
    typedef typename std::iterator_traits<typename Input::iterator>::value_type
      sample_type;

    friend class RendererBase<Derived>;  // rem_source() needs access to _input

    struct Params : apf::parameter_map
    {
      Params() : parent(0) {}
      Derived* parent;
      const typename Derived::Input* input;
      apf::CommandQueue* fifo;

      Params& operator=(const apf::parameter_map& p)
      {
        this->apf::parameter_map::operator=(p);
        return *this;
      }
    };

    explicit Source(const Params& p)
      : parent(*(p.parent ? p.parent : throw std::logic_error(
              "Bug (RendererBase::Source): parent == NULL!")))
      , position(*(p.fifo ? p.fifo : throw std::logic_error(
              "Bug (RendererBase::Source): fifo == NULL!")))
      , orientation(*p.fifo)
      , gain(*p.fifo, sample_type(1.0))
      , mute(*p.fifo, false)
      , model(*p.fifo, ::Source::point)
      , weighting_factor()
      , old_weighting_factor()
      , _input(*(p.input ? p.input : throw std::logic_error(
              "Bug (RendererBase::Source): input == NULL!")))
      , _pre_fader_level()
      , _level()
    {}

    APF_PROCESS(Source, SourceBase)
    {
      this->_begin = _input.begin();
      this->_end = _input.end();

      old_weighting_factor = weighting_factor;

      if (!_input.parent.state.processing() || mute())
      {
        weighting_factor = 0.0;
      }
      else
      {
        weighting_factor = gain();
        // If the renderer does something nonlinear, the master volume should
        // be applied to the output signal ... TODO: shall we care?
        weighting_factor *= _input.parent.state.master_volume();
        weighting_factor *= _input.parent.master_volume_correction;
      }

      _level_helper(_input.parent);
    }

    sample_type get_level() const { return _level; }

    // In the default case, the output level are ignored
    bool get_output_levels(sample_type*, sample_type*) const { return false; }

#if 0
    // TODO: check if all renderers implement this. If not, provide default.
    void connect(RendererBase&) {}
    void disconnect(RendererBase&) {}
#endif

    Derived& parent;

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
};

template<typename Derived>
class RendererBase<Derived>::Output : public Base::Output
{
  public:
    Output(const typename Base::Output::Params& p)
      : Base::Output(p)
      , _level()
    {}

    struct Process : Base::Output::Process
    {
      explicit Process(Output& o) : Base::Output::Process(o) , _out(o) {}

      ~Process()
      {
        _out._level_helper(_out.parent);
      }

      private:
        Output& _out;
    };

    sample_type get_level() const { return _level; }

  protected:
    void _level_helper(apf::enable_queries&)
    {
      _level = apf::math::max_amplitude(this->buffer.begin()
          , this->buffer.end());
    }

    void _level_helper(apf::disable_queries&) {}

  private:
    sample_type _level;
};

// This is a kind of C++ mixin class, but it also includes the CRTP
template<typename Derived, template<typename> class Base>
struct SourceToOutput : Base<Derived>
{
  typedef typename Base<Derived>::Input Input;

  struct Source : Base<Derived>::Source
  {
    typedef typename Base<Derived>::Source::Params Params;
    typedef apf::fixed_vector<typename Derived::SourceChannel> sourcechannels_t;

    Source(const Params& p)
      : Base<Derived>::Source(p)
      , sourcechannels()
    {}

    template<typename Arg>
    Source(const Params& p, const Arg& arg)
      : Base<Derived>::Source(p)
      , sourcechannels(arg)
    {}

    // ctor from sequence
    template<typename In>
    Source(const Params& p, In first, In last)
      : Base<Derived>::Source(p)
      , sourcechannels(first, last)
    {}

    void connect(SourceToOutput& parent)
    {
      std::list<typename Derived::SourceChannel*> temp;
      apf::append_pointers(this->sourcechannels, temp);
      parent.add_to_sublist(temp
         , apf::make_cast_proxy<Output>(parent.get_non_const_output_list())
         , &Output::sourcechannels);
    }

    void disconnect(SourceToOutput& parent)
    {
      std::list<typename Derived::SourceChannel*> temp;
      apf::append_pointers(this->sourcechannels, temp);
      parent.rem_from_sublist(temp
          , apf::make_cast_proxy<Output>(parent.get_non_const_output_list())
          , &Output::sourcechannels);
    }

    sourcechannels_t sourcechannels;
  };

  struct Output : Base<Derived>::Output
  {
    typedef typename Base<Derived>::Output::Params Params;
    typedef std::list<typename Derived::SourceChannel*> sourcechannels_t;

    Output(const Params& p) : Base<Derived>::Output(p) {}

    sourcechannels_t sourcechannels;
  };

  explicit SourceToOutput(const apf::parameter_map& params)
    : Base<Derived>(params)
  {}
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
