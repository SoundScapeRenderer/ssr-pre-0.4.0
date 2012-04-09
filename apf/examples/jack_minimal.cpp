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

// Minimalistic example for the MimoProcessor with JACK.

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/posix_thread_policy.h"
#include "apf/posix_sync_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
      , apf::jack_policy, apf::posix_thread_policy, apf::posix_sync_policy>
{
  public:
    typedef MimoProcessorDefaultInput Input;
    class Output;

    MyProcessor();

    void process()
    {
      _process_list(_output_list);
    }

  private:
    rtlist_t _input_list, _output_list;
};

class MyProcessor::Output : public MimoProcessorOutput
{
  public:
    typedef MimoProcessorOutput::Params Params;

    explicit Output(const Params& p)
      : MimoProcessorOutput(p)
      , _combiner(_parent._input_list, _internal, _parent)
    {}

    virtual void process()
    {
      _combiner.copy(my_predicate());
    }

  private:
    struct my_predicate
    {
      // trivial, all inputs are used
      bool operator()(const Input&) { return true; }
    };

    combine_channels<rtlist_t, Input, InternalOutput> _combiner;
};

MyProcessor::MyProcessor()
  : MimoProcessorBase()
  , _input_list(_fifo)
  , _output_list(_fifo)
{
  Input::Params ip;
  ip.parent = this;
  _input_list.add(new Input(ip));
  _input_list.add(new Input(ip));

  Output::Params op;
  op.parent = this;
  _output_list.add(new Output(op));

  this->activate();
}

int main()
{
  MyProcessor processor;
  sleep(30);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
