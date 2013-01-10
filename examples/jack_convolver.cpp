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

    template<typename In>
    MyProcessor(In first, In last);

    ~MyProcessor() { this->deactivate(); }

    struct Output : MimoProcessorBase::DefaultOutput
    {
      Output(const Params& p) : MimoProcessorBase::DefaultOutput(p) {}

      // Deactivate process() function, fetch_buffer() is called earlier!
      virtual void process() {}
    };

    APF_PROCESS(MyProcessor, MimoProcessorBase)
    {
      _convolver_in.add_block(_input->begin());
      _convolver_out.rotate_queues();  // TODO: check if necessary?
      if (this->reverb() != _old_reverb)
      {
        if (this->reverb())
        {
          _convolver_out.set_filter(_filter_partitions);
        }
        else
        {
          // Load Dirac
          float one = 1.0f;
          _convolver_out.set_filter(&one, (&one)+1);
          // One could prepare frequency domain version to avoid repeated FFTs
        }
        _old_reverb = this->reverb();
      }
      float* result = _convolver_out.convolve();

      // This is necessary because _output is used before _output_list is
      // processed:
      _output->fetch_buffer();

      std::copy(result, result + this->block_size(), _output->begin());
    }

    apf::SharedData<bool> reverb;

  private:
    bool _old_reverb;

    Input* _input;
    Output* _output;

    size_t _partitions;

    apf::conv::Input _convolver_in;
    apf::conv::Output _convolver_out;

    apf::conv::filter_t _filter_partitions;
};

template<typename In>
MyProcessor::MyProcessor(In first, In last)
  : MimoProcessorBase()
  , reverb(_fifo, true)
  , _old_reverb(false)
  , _partitions((std::distance(first, last) + this->block_size() - 1)
      / this->block_size())
  , _convolver_in(this->block_size(), _partitions)
  , _convolver_out(_convolver_in)
  , _filter_partitions(std::make_pair(_partitions, this->block_size() * 2))
{
  _convolver_in.prepare_filter(first, last, _filter_partitions);

  _input = this->add<Input>();
  _output = this->add<Output>();

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
