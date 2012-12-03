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

// A simple example for the usage of the MimoProcessor.
// The used policies can be specified with the preprocessor macros
// APF_MIMOPROCESSOR_*_POLICY.

#include <algorithm>  // for std::transform()
#include <functional> // for std::multiplies(), std::bind2nd()

#include "apf/mimoprocessor.h"
#include "apf/stringtools.h"
#include "apf/misc.h"

// Make sure that APF_MIMOPROCESSOR_INTERFACE_POLICY and
// APF_MIMOPROCESSOR_THREAD_POLICY are
// #define'd before #include'ing this header file!

class SimpleProcessor : public apf::MimoProcessor<SimpleProcessor
                      , APF_MIMOPROCESSOR_INTERFACE_POLICY
                      , APF_MIMOPROCESSOR_THREAD_POLICY>
{
  public:
    class Input : public MimoProcessorBase::Input
       , public apf::has_begin_and_end<std::vector<sample_type>::const_iterator>
    {
      public:
        explicit Input(const Params& p)
          : MimoProcessorBase::Input(p)
          , _buffer(this->parent.block_size())
        {
          // initialize protected members from has_begin_and_end
          _begin = _buffer.begin();
          _end = _buffer.end();
        }

      private:
        virtual void process()
        {
          // Copying the input buffers is only needed for the Pd external
          // because input buffers are re-used as output buffers!
          // In non-trivial applications there will be some intermediate buffer
          // anyway and copying the input buffers will not be necessary.

          std::copy(_internal.begin(), _internal.end(), _buffer.begin());
        }

        std::vector<sample_type> _buffer;
    };

    class Output;

    explicit SimpleProcessor(const apf::parameter_map& p=apf::parameter_map());

    ~SimpleProcessor()
    {
      this->deactivate();
      _input_list.clear();
      _output_list.clear();
    }

    void process()
    {
      _process_list(_input_list);
      _process_list(_output_list);
    }

    Input* add_input(const Input::Params& p)
    {
      Input::Params temp = p;
      temp.parent = this;
      return _input_list.add(new Input(temp));
    }

  private:
    rtlist_t _input_list, _output_list;
};

class SimpleProcessor::Output : public MimoProcessorBase::Output
{
  public:
    typedef MimoProcessorBase::Output::Params Params;

    explicit Output(const Params& p)
      : MimoProcessorBase::Output(p)
      , _combiner(this->parent._input_list, _internal)
    {}

    virtual void process()
    {
      float weight = 1.0f / float(this->parent._input_list.size());

      _combiner.process(simple_predicate(weight));
    }

  private:
    class simple_predicate
    {
      public:
        explicit simple_predicate(float weight)
          : _weight(weight)
        {}

        int select(const Input&)
        {
          // trivial, all inputs are used; no crossfade/interpolation
          return 1;
        }

        float operator()(float in)
        {
          return in * _weight;
        }

      private:
        float _weight;
    };

    apf::CombineChannels<rtlist_proxy<Input>, InternalOutput> _combiner;
};

SimpleProcessor::SimpleProcessor(const apf::parameter_map& p)
  : MimoProcessorBase(p)
  , _input_list(_fifo)
  , _output_list(_fifo)
{
  Input::Params ip;
  ip.parent = this;
  std::string in_port_prefix = p.get("in_port_prefix", "");
  int in_ch = p.get<int>("in_channels");
  for (int i = 1; i <= in_ch; ++i)
  {
    ip.set("id", i);
    if (in_port_prefix != "")
    {
      ip.set("connect_to", in_port_prefix + apf::str::A2S(i));
    }
    _input_list.add(new Input(ip));  // ignore return value
  }

  Output::Params op;
  op.parent = this;
  std::string out_port_prefix = p.get("out_port_prefix", "");
  int out_ch = p.get<int>("out_channels");
  for (int i = 1; i <= out_ch; ++i)
  {
    op.set("id", i);
    if (out_port_prefix != "")
    {
      op.set("connect_to", out_port_prefix + apf::str::A2S(i));
    }
    _output_list.add(new Output(op));  // ignore return value
  }

  this->activate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
