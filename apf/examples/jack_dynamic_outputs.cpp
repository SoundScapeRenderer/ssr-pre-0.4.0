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

// A small example of the MimoProcessor with varying JACK output ports.
// This is a stand-alone program.

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/dummy_thread_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::jack_policy
                    , apf::dummy_thread_policy>
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
          _combiner.process(select_all_inputs());
        }

      private:
        struct select_all_inputs
        {
          int select(const Input&) { return 1; }
        };

        apf::CombineChannelsCopy<rtlist_proxy<Input>, InternalOutput> _combiner;
    };

    MyProcessor(const apf::parameter_map& p)
      : MimoProcessorBase(p)
      , _input_list(_fifo)
      , _output_list(_fifo)
    {
      _input_list.add(new Input(Input::Params(this)));
    }

    void process()
    {
      // _input_list doesn't need to be processed, Input doesn't have .process()
      _process_list(_output_list);
    }

    Output* add_output(const Output::Params& p)
    {
      Output::Params temp = p;
      temp.parent = this;
      return _output_list.add(new Output(temp));
    }

    void rem_input(Input* in) { _input_list.rem(in); }
    void rem_output(Output* out) { _output_list.rem(out); }

  private:
    rtlist_t _input_list, _output_list;
};

int main()
{
  int out_channels = 20;

  apf::parameter_map p;
  p.set("threads", 1);
  //p.set("threads", 2);  // not allowed with dummy_thread_policy!
  MyProcessor engine(p);
  engine.activate();

  sleep(2);

  std::vector<MyProcessor::Output*> outputs;

  for (int i = 1; i <= out_channels; ++i)
  {
    MyProcessor::Output::Params op;
    op.set("id", i * 10);
    outputs.push_back(engine.add_output(op));
    sleep(1);
  }

  sleep(2);

  // remove the outputs one by one ...
  while (outputs.begin() != outputs.end())
  {
    engine.rem_output(outputs.front());
    // wait two times because InternalOutput is removed in ~Output
    engine.wait_for_rt_thread();
    engine.wait_for_rt_thread();
    outputs.erase(outputs.begin());
    sleep(1);
  }

  sleep(2);

  engine.deactivate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
