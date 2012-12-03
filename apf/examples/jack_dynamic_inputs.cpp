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

// A small example of the MimoProcessor with varying JACK input ports.
// This is a stand-alone program.

#include <algorithm>  // for std::transform()

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/posix_thread_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::jack_policy
                    , apf::posix_thread_policy>
{
  public:
    typedef MimoProcessorBase::DefaultInput Input;

    class Output : public MimoProcessorBase::Output
    {
      public:
        explicit Output(const Params& p)
          : MimoProcessorBase::Output(p)
          , _combiner(this->parent._input_list, _internal)
        {}

        virtual void process()
        {
          float weight = 1.0f/static_cast<float>(this->parent._input_list.size());
          _combiner.process(simple_predicate(weight));
        }

      private:
        class simple_predicate
        {
          public:
            explicit simple_predicate(float weight) : _weight(weight) {}

            // trivial, all inputs are used; no crossfade/interpolation
            int select(const Input&) { return 1; }

            float operator()(float in) { return in * _weight; }

          private:
            float _weight;
        };

        apf::CombineChannels<rtlist_proxy<Input>, InternalOutput> _combiner;
    };

    MyProcessor()
      : _input_list(_fifo)
      , _output_list(_fifo)
    {
      _output_list.add(new Output(Output::Params(this)));

      // TODO: remove output in destructor?
    }

    void process()
    {
      // _input_list doesn't need to be processed, Input doesn't have .process()
      _process_list(_output_list);
    }

    Input* add_input(const Input::Params& p)
    {
      Input::Params temp = p;
      temp.parent = this;
      return _input_list.add(new Input(temp));
    }

    void rem_input(Input* in) { _input_list.rem(in); }
    void rem_output(Output* out) { _output_list.rem(out); }

  private:
    rtlist_t _input_list, _output_list;
};

int main()
{
  int in_channels = 20;

  MyProcessor engine;
  engine.activate();

  sleep(2);

  std::vector<MyProcessor::Input*> inputs;

  for (int i = 1; i <= in_channels; ++i)
  {
    MyProcessor::Input::Params p;
    p.set("id", i * 10);
    inputs.push_back(engine.add_input(p));
    sleep(1);
  }

  sleep(2);

  // remove the inputs one by one ...
  while (inputs.begin() != inputs.end())
  {
    engine.rem_input(inputs.front());
    // wait two times because InternalInput is removed in ~Input
    engine.wait_for_rt_thread();
    engine.wait_for_rt_thread();
    inputs.erase(inputs.begin());
    sleep(1);
  }

  sleep(2);

  engine.deactivate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
