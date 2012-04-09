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

// A small example of the MimoProcessor with varying JACK input ports.
// This is a stand-alone program.

#include <algorithm>  // for std::transform()

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

    MyProcessor()
      : _input_list(_fifo)
      , _output_list(_fifo)
    {
      Output::Params p;
      p.parent = this;
      _output_list.add(new Output(p));

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
    p.dict.set("id", i * 10);
    inputs.push_back(engine.add_input(p));
    sleep(1);
  }

  sleep(2);

  // remove the inputs one by one ...
  while (inputs.begin() != inputs.end())
  {
    engine.rem_input(inputs.front());
    inputs.erase(inputs.begin());
    sleep(1);
  }

  sleep(2);

  //engine.deactivate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
