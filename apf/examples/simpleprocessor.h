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

// A simple example for the usage of the MimoProcessor.
// The used policies can be specified with the preprocessor macros
// MIMOPROCESSOR_*_POLICY.

#include <algorithm>  // for std::transform()
#include <functional> // for std::multiplies(), std::bind2nd()

#include "apf/mimoprocessor.h"
#include "apf/stringtools.h"
#include "apf/misc.h"

using apf::str::A2S;
using apf::has_begin_and_end;

// Make sure that APF_MIMOPROCESSOR_INTERFACE_POLICY,
// APF_MIMOPROCESSOR_THREAD_POLICY and APF_MIMOPROCESSOR_SYNC_POLICY are
// #define'd before #include'ing this header file!

class SimpleProcessor : public apf::MimoProcessor<SimpleProcessor
                      , APF_MIMOPROCESSOR_INTERFACE_POLICY
                      , APF_MIMOPROCESSOR_THREAD_POLICY
                      , APF_MIMOPROCESSOR_SYNC_POLICY>
{
  public:
    class Input : public MimoProcessorInput
            , public has_begin_and_end<std::vector<sample_type>::const_iterator>
    {
      public:
        explicit Input(const Params& p)
          : MimoProcessorInput(p)
          , _buffer(_parent.block_size())
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

    explicit SimpleProcessor(const parameter_map& p = apf::parameter_map());

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

    void clear()
    {
      _input_list.clear();
      _output_list.clear();
    }

  private:
    rtlist_t _input_list, _output_list;
};

class SimpleProcessor::Output : public MimoProcessorOutput
{
  public:
    typedef MimoProcessorOutput::Params Params;

    explicit Output(const Params& p)
      : MimoProcessorOutput(p)
      , _combiner(_parent._input_list, _internal, _parent)
    {}

    virtual void process()
    {
      float weight = 1.0f/static_cast<float>(_parent._input_list.size());

      _combiner.transform(simple_predicate(weight));
    }

  private:
    class simple_predicate
    {
      public:
        explicit simple_predicate(float weight)
          : _weight(weight)
        {}

        bool operator()(const Input&)
        {
          // trivial, all inputs are used
          return true;
        }

        float operator()(float in)
        {
          return in * _weight;
        }

      private:
        float _weight;
    };

    combine_channels<rtlist_t, Input, InternalOutput> _combiner;
};

SimpleProcessor::SimpleProcessor(const parameter_map& p)
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
    ip.dict.set("id", i);
    if (in_port_prefix != "")
    {
      ip.dict.set("connect_to", in_port_prefix + A2S(i));
    }
    _input_list.add(new Input(ip));  // ignore return value
  }

  Output::Params op;
  op.parent = this;
  std::string out_port_prefix = p.get("out_port_prefix", "");
  int out_ch = p.get<int>("out_channels");
  for (int i = 1; i <= out_ch; ++i)
  {
    op.dict.set("id", i);
    if (out_port_prefix != "")
    {
      op.dict.set("connect_to", out_port_prefix + A2S(i));
    }
    _output_list.add(new Output(op));  // ignore return value
  }

  this->activate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
