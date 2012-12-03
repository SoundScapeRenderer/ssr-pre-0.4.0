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

// Example for the Convolver.

#include <iostream>
#include <sndfile.hh>

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/posix_thread_policy.h"
#include "apf/shareddata.h"
#include "apf/convolver.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::jack_policy
                    , apf::posix_thread_policy>
{
  public:
    typedef MimoProcessorBase::DefaultInput Input;
    typedef MimoProcessorBase::DefaultOutput Output;

    template<typename In>
    MyProcessor(In first, In last);

    ~MyProcessor() { this->deactivate(); }

    void process()
    {
      _convolver.add_input_block(_input->begin());
      _convolver.rotate_queues();  // TODO: check if necessary?
      if (reverb() != _old_reverb)
      {
        if (reverb())
        {
          _convolver.set_filter(_filter_partitions);
        }
        else
        {
          // Load Dirac
          float one = 1.0f;
          _convolver.set_filter(&one, (&one)+1);
          // One could prepare a frequency domain version to avoid repeated FFTs
        }
        _old_reverb = reverb();
      }
      float* result = _convolver.convolve_signal();
      std::copy(result, result + this->block_size(), _output->begin());
    }

    apf::SharedData<bool> reverb;

  private:
    bool _old_reverb;

    rtlist_t _input_list, _output_list;

    Input* _input;
    Output* _output;

    size_t _partitions;

    apf::Convolver _convolver;

    apf::Convolver::filter_t _filter_partitions;
};

template<typename In>
MyProcessor::MyProcessor(In first, In last)
  : MimoProcessorBase()
  , reverb(_fifo, true)
  , _old_reverb(false)
  , _input_list(_fifo)
  , _output_list(_fifo)
  , _partitions((std::distance(first, last)/this->block_size())+1)
  , _convolver(this->block_size(), _partitions)
  , _filter_partitions(std::make_pair(_partitions, this->block_size() * 2))
{
  _convolver.prepare_filter(first, last, _filter_partitions);

  _input = _input_list.add(new Input(Input::Params(this)));

  _output = _output_list.add(new Output(Output::Params(this)));

  std::cout << "Press <enter> to switch and q to quit" << std::endl;
  this->activate();
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " <IR filename>" << std::endl;
    return 1;
  }

  SndfileHandle in(argv[1], SFM_READ);

  if (in.error()) throw std::runtime_error(in.strError());

  if (in.channels() != 1)
  {
    throw std::runtime_error("Only mono files are supported!");
  }

  std::vector<float> ir(in.frames());

  if (in.readf(&ir[0], in.frames()) != in.frames())
  {
    throw std::runtime_error("Couldn't load audio file!");
  }

  MyProcessor processor(ir.begin(), ir.end());

  if (in.samplerate() != int(processor.sample_rate()))
  {
    throw std::runtime_error("Samplerate mismatch!");
  }

  std::string input;
  bool reverb = true;

  for (;;)
  {
    std::getline(std::cin, input);
    if (input == "")
    {
      if (reverb)
      {
        processor.reverb(false);
        reverb = false;
        std::cout << "filter off" << std::endl;
      }
      else
      {
        processor.reverb(true);
        reverb = true;
        std::cout << "filter on" << std::endl;
      }
    }
    else if (input == "q")
    {
      break;
    }
    else
    {
      std::cout << "What? Type q to quit!" << std::endl;
    }
  }
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
