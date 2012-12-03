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

// Minimalistic example for the MimoProcessor with JACK.

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/posix_thread_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
      , apf::jack_policy, apf::posix_thread_policy>
{
  public:
    typedef MimoProcessorBase::DefaultInput Input;
    class Output;

    MyProcessor();

    void process()
    {
      _process_list(_output_list);
    }

  private:
    rtlist_t _input_list, _output_list;
};

class MyProcessor::Output : public MimoProcessorBase::Output
{
  public:
    explicit Output(const Params& p)
      : MimoProcessorBase::Output(p)
      , _combiner(this->parent._input_list, _internal)
    {}

    virtual void process()
    {
      _combiner.process(my_predicate());
    }

  private:
    struct my_predicate
    {
      // trivial, all inputs are used
      int select(const Input&) { return 1; }
    };

    apf::CombineChannelsCopy<rtlist_proxy<Input>, InternalOutput> _combiner;
};

MyProcessor::MyProcessor()
  : MimoProcessorBase()
  , _input_list(_fifo)
  , _output_list(_fifo)
{
  _input_list.add(new Input(Input::Params(this)));
  _input_list.add(new Input(Input::Params(this)));

  _output_list.add(new Output(Output::Params(this)));
}

int main()
{
  MyProcessor processor;
  processor.activate();
  sleep(30);
  processor.deactivate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
