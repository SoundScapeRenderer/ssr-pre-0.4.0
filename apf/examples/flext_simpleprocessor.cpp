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

// Example for the MimoProcessor running as a Pd/MaxMSP external using flext.
//
// Compile/install Pd external with
//   CFLAGS=-I.. $FLEXTPATH/build.sh pd gcc
//   $FLEXTPATH/build.sh pd gcc install
//
// Clean up with
//   $FLEXTPATH/build.sh pd gcc clean

#include <flext.h>

#define APF_MIMOPROCESSOR_SAMPLE_TYPE t_sample

#include "apf/pointer_policy.h"
#include "apf/posix_thread_policy.h"
#include "apf/posix_sync_policy.h"

#include "simpleprocessor.h"

// check for appropriate flext version (CbSignal was introduced in 0.5.0)
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0!
#endif

namespace // anonymous
{
  // this function is only used in the constructor's initialization list
  apf::parameter_map engine_params(int inputs, int outputs, int threads
      , int block_size, int sample_rate)
  {
    apf::parameter_map temp;
    temp.set("in_channels", inputs);
    temp.set("out_channels", outputs);
    temp.set("threads", threads);
    temp.set("block_size", block_size);
    temp.set("sample_rate", sample_rate);
    return temp;
  }
}

class simpleprocessor: public flext_dsp
{
  FLEXT_HEADER(simpleprocessor, flext_dsp)

  public:
    simpleprocessor(int inputs, int outputs, int threads)
      : _engine(engine_params(inputs, outputs, threads
            , Blocksize(), Samplerate()))
    {
      AddInSignal(inputs);

      AddOutSignal(outputs);

      //FLEXT_ADDMETHOD(0, _left_float);

      FLEXT_ADDMETHOD_(0, "hello", _hello);
      FLEXT_ADDMETHOD_I(0, "hello", _hello_and_int);

      //FLEXT_ADDMETHOD(2, _sym); // register method for all other symbols?

      FLEXT_ADDMETHOD_(0, "help", _help);
    }

  private:
    //void _left_float(float input)
    //{
    //  post("Receiving %.2f from left inlet.", input);
    //  //ToOutFloat(1, input);
    //}

    // override signal function
    virtual void CbSignal()
    {
      _engine.audio_callback(Blocksize(), InSig(), OutSig());
    }

    void _hello()
    {
      post("hello yourself!");
    }

    void _hello_and_int(int input)
    {
      post("hello %i!", input);
    }

    //void _sym(t_symbol *s)
    //{
    //  post("symbol: %s", GetString(s));
    //}

    void _help()
    {
      post("%s - this is some useless help information.", thisName());
    }

    SimpleProcessor _engine;

  private:
    // FLEXT_CALLBACK_1(x, float) == FLEXT_CALLBACK_F(x)
    //FLEXT_CALLBACK_F(_left_float);

    FLEXT_CALLBACK(_hello)
    FLEXT_CALLBACK_I(_hello_and_int)
    //FLEXT_CALLBACK_S(_sym)

    FLEXT_CALLBACK(_help)
};

//FLEXT_NEW_DSP("simpleprocessor~", simpleprocessor) // no creation args
FLEXT_NEW_DSP_3("simpleprocessor~", simpleprocessor, int, int, int)

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
