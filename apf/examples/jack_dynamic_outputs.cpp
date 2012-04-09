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

// A small example of the MimoProcessor with varying JACK output ports.
// This is a stand-alone program.

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/posix_thread_policy.h"
#include "apf/posix_sync_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::jack_policy
                    , apf::posix_thread_policy
                    , apf::posix_sync_policy>
{
  public:
    typedef MimoProcessorDefaultInput Input;

    class Output : public MimoProcessorOutput
    {
      public:
        explicit Output(const Params& p)
          : MimoProcessorOutput(p)
          , _combiner(_parent._input_list, _internal, _parent)
        {}

        virtual void process();

      private:
        struct select_all_inputs
        {
          bool operator()(const Input&) { return true; }
        };

        combine_channels<rtlist_t, Input, InternalOutput> _combiner;
    };

    MyProcessor()
      : _input_list(_fifo)
      , _output_list(_fifo)
    {
      Input::Params p;
      p.parent = this;
      _input_list.add(new Input(p));

      // TODO: remove input in destructor?
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

void MyProcessor::Output::process()
{
  _combiner.copy(select_all_inputs());
}

int main()
{
  int out_channels = 20;

  MyProcessor engine;
  engine.activate();

  sleep(2);

  std::vector<MyProcessor::Output*> outputs;

  for (int i = 1; i <= out_channels; ++i)
  {
    MyProcessor::Output::Params p;
    p.dict.set("id", i * 10);
    outputs.push_back(engine.add_output(p));
    sleep(1);
  }

  sleep(2);

  // remove the outputs one by one ...
  while (outputs.begin() != outputs.end())
  {
    engine.rem_output(outputs.front());
    outputs.erase(outputs.begin());
    sleep(1);
  }

  sleep(2);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
