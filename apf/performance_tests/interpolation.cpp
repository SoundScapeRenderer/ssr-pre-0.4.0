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

// Performance tests for the interpolation.

#include <cstdlib>  // for random()

#include "apf/pointer_policy.h"
#include "apf/posix_thread_policy.h"
#include "apf/mimoprocessor.h"
#include "apf/container.h"  // for fixed_matrix
#include "apf/stopwatch.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::pointer_policy<float*>
                    , apf::posix_thread_policy>
{
  public:
    typedef DefaultInput Input;
    class Output;
    class CombineFunction;

    MyProcessor(const apf::parameter_map& p);

    void process()
    {
      _process_list(_output_list);
    }

  private:
    rtlist_t _input_list, _output_list;
};

class MyProcessor::CombineFunction
{
  public:
    CombineFunction(size_t block_size)
      : _block_size(float(block_size))
    {}

    int select(const Input&)
    {
      _interpolator.set(3.14f, 666.666f, _block_size);
      return 2;  // Always force interpolation
    }

    float operator()(float)
    {
      throw std::logic_error("This is never used!");
      return 0.0f;
    }

    float operator()(float in, float index)
    {
      return in * _interpolator(index);
    }

  private:
    // float is much faster here than int because less casts are necessary
    float _block_size;
    apf::math::linear_interpolator<float> _interpolator;
};

class MyProcessor::Output : public MimoProcessorBase::Output
{
  public:
    explicit Output(const Params& p)
      : MimoProcessorBase::Output(p)
      , _combine_and_interpolate(this->parent._input_list, _internal)
    {}

    virtual void process()
    {
      _combine_and_interpolate.process(CombineFunction(this->parent.block_size()));
    }

  private:
    apf::CombineChannelsInterpolation<rtlist_proxy<Input>, InternalOutput>
      _combine_and_interpolate;
};

MyProcessor::MyProcessor(const apf::parameter_map& p)
  : MimoProcessorBase(p)
  , _input_list(_fifo)
  , _output_list(_fifo)
{
  for (int i = 0; i < p.get<int>("in_channels"); ++i)
  {
    _input_list.add(new Input(MimoProcessorBase::Input::Params(this)));
  }

  for (int i = 0; i < p.get<int>("out_channels"); ++i)
  {
    _output_list.add(new Output(MimoProcessorBase::Output::Params(this)));
  }
}

int main()
{
  // TODO: check for input arguments

  int in_channels = 10;
  int out_channels = 70;
  int block_size = 512;
  int repetitions = 1000;
  int threads = 1;

  apf::fixed_matrix<float> m_in(in_channels, block_size);
  apf::fixed_matrix<float> m_out(out_channels, block_size);

  // WARNING: this is not really a meaningful audio signal:
  std::generate(m_in.begin(), m_in.end(), random);

  apf::parameter_map p;
  p.set("in_channels", in_channels);
  p.set("out_channels", out_channels);
  p.set("block_size", block_size);
  p.set("sample_rate", 44100);  // Not really relevant in this case
  p.set("threads", threads);

  MyProcessor processor(p);

  processor.activate();

  {
    apf::StopWatch watch("processing");
    for (int i = 0; i < repetitions; ++i)
    {
      processor.audio_callback(block_size
          , m_in.get_channel_ptrs(), m_out.get_channel_ptrs());
    }
  }

  processor.deactivate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
