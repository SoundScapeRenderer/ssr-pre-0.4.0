/******************************************************************************
 * Copyright © 2012      Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://SoundScapeRenderer.github.com                  ssr@spatialaudio.net *
 ******************************************************************************/

// NFC-HOA renderer as MEX file for GNU Octave and MATLAB.

#include <mex.h>
#include <memory>  // for std::auto_ptr

#ifdef SSR_MEX_USE_DOUBLE
#define APF_MIMOPROCESSOR_SAMPLE_TYPE double
#else
#define APF_MIMOPROCESSOR_SAMPLE_TYPE float
#endif

#include "apf/pointer_policy.h"
// TODO: make switch for Windows threads
#include "apf/posix_thread_policy.h"

#include "nfchoarenderer.h"

// The single entry-point for Matlab is the function mexFunction(), see below!

// global variables holding the state
std::auto_ptr<ssr::NfcHoaRenderer> engine;
size_t in_channels, out_channels, block_size, sample_rate, threads;
typedef ssr::NfcHoaRenderer::sample_type sample_type;
std::vector<sample_type*> inputs, outputs;

// TODO: separate file with generic helper functions (maybe apf::mex namespace?)

namespace mex
{

// TODO: check if (and how) user-specified overloads of convert() work
// TODO: use a traits class, if necessary

bool convert(const mxArray* in, std::string& out)
{
  if (!mxIsChar(in)) return false;
  if (mxGetM(in) != 1) return false;

  char* temp = mxArrayToString(in);
  out = temp;
  mxFree(temp);
  return true;
}

bool convert(const mxArray* in, double& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  out = mxGetScalar(in);
  return true;
}

bool convert(const mxArray* in, int& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  double temp = mxGetScalar(in);
  if (temp != floor(temp)) return false;
  out = temp;
  return true;
}

bool convert(const mxArray* in, size_t& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  double temp = mxGetScalar(in);
  if (temp < 0 || temp != floor(temp)) return false;
  out = temp;
  return true;
}

namespace internal
{

template<bool optional, typename T>
bool next_arg_helper(int& n, const mxArray**& p, T& data)
{
  return (n-- < 1) ? optional : convert(p++[0], data);
}

}  // namespace internal

template<typename T>
bool next_arg(int& n, const mxArray**& p, T& data)
{
  return internal::next_arg_helper<false>(n, p, data);
}

template<typename T>
bool next_optarg(int& n, const mxArray**& p, T& data)
{
  return internal::next_arg_helper<true>(n, p, data);
}

template<typename T>
void next_arg(int& n, const mxArray**& p, T& data, const std::string& error)
{
  if (!next_arg(n, p, data)) mexErrMsgTxt(error.c_str());
}

template<typename T>
void next_optarg(int& n, const mxArray**& p, T& data, const std::string& error)
{
  if (!next_optarg(n, p, data)) mexErrMsgTxt(error.c_str());
}

void error_no_further(int nrhs, const mxArray**, const std::string& name)
{
  if (nrhs > 0)
  {
    std::string msg("No further parameters are supported for '"
        + name + "'!");
    mexErrMsgTxt(msg.c_str());
  }
}

void error_no_output(int nlhs, mxArray**, const std::string& name)
{
  if (nlhs > 0)
  {
    std::string msg("No output parameters are supported for '" + name + "'!");
    mexErrMsgTxt(msg.c_str());
  }
}

void error_one_output(int nlhs, mxArray**, const std::string& name)
{
  if (nlhs > 1)
  {
    std::string msg("Only one output parameter is supported for '"
        + name + "'!");
    mexErrMsgTxt(msg.c_str());
  }
}

}  // namespace mex

void error_init()
{
  if (!engine.get())
  {
    mexErrMsgTxt("ssr_nfc_hoa isn't initialized, use 'init' first!");
  }
}

void help(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  mex::error_no_output(nlhs, plhs, "help");
  mex::error_no_further(nrhs, prhs, "help");

  mexPrintf("TODO: write help text!\n");
}

void init(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  mex::error_no_output(nlhs, plhs, "init");

  std::string reproduction_setup;
  mex::next_arg(nrhs, prhs, reproduction_setup, "'init': Second argument "
      "must be a string (the reproduction setup file name)!");

  mex::next_arg(nrhs, prhs, in_channels
      , "'init': Third argument must be the number of input channels!");

  mex::next_arg(nrhs, prhs, block_size
      , "'init': Fourth argument must be the block size!");

  mex::next_arg(nrhs, prhs, sample_rate
      , "'init': Fifth argument must be the sample rate!");

  threads = 1;  // TODO: get reasonable default value
  mex::next_optarg(nrhs, prhs, threads
      , "'init': Sixth argument: number of threads!");

  mex::error_no_further(nrhs, prhs, "init");

  mexPrintf("Starting ssr_nfc_hoa with following settings:\n"
      " * reproduction setup: %s\n"
      " * in channels: %d\n"
      " * block size: %d\n"
      " * sample rate: %d\n"
      " * threads: %d\n"
      , reproduction_setup.c_str(), in_channels
      , block_size, sample_rate, threads);

  apf::parameter_map params;
  params.set("reproduction_setup", reproduction_setup);
  // TODO: set XML Schema file?
  //params.set("xml_schema", xml_schema);
  params.set("block_size", block_size);
  params.set("sample_rate", sample_rate);
  params.set("threads", threads);
  engine.reset(new ssr::NfcHoaRenderer(params));

  engine->load_reproduction_setup();

  out_channels = engine->get_output_list().size();

  for (size_t i = 0; i < in_channels; ++i)
  {
    // TODO: specify ID?
    engine->add_source();
  }

  inputs.resize(in_channels);
  outputs.resize(out_channels);

  engine->activate();  // start parallel processing (if threads > 1)

  mexPrintf("Initialization completed, %d outputs available.\n", out_channels);
}

void process(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  error_init();

  if (nlhs != 1 || nrhs != 1)
  {
    mexErrMsgTxt("Exactly one input and one output is needed for 'process'!");
  }
  if (mxGetM(prhs[0]) != block_size)
  {
    mexErrMsgTxt("Number of rows must be the same as block size!");
  }
  if (mxGetN(prhs[0]) != in_channels)
  {
    mexErrMsgTxt("Number of columns must be the same as number of inputs!");
  }
  if (mxIsComplex(prhs[0]))
  {
    mexErrMsgTxt("Complex values are not allowed!");
  }
  if (!mxIsNumeric(prhs[0]))
  {
    mexErrMsgTxt("Input must be a numeric matrix!");
  }

#ifdef SSR_MEX_USE_DOUBLE
  if (!mxIsDouble(prhs[0]))
  {
    mexErrMsgTxt("This function only works with double precision data!");
  }
  plhs[0] = mxCreateDoubleMatrix(block_size, out_channels, mxREAL);
  sample_type* output = mxGetPr(plhs[0]);
  sample_type*  input = mxGetPr(prhs[0]);
#else
  if (mxGetClassID(prhs[0]) != mxSINGLE_CLASS)
  {
    mexErrMsgTxt("This function only works with single precision data!");
  }
  plhs[0] = mxCreateNumericMatrix(block_size, out_channels
      , mxSINGLE_CLASS, mxREAL);
  sample_type* output = static_cast<sample_type*>(mxGetData(plhs[0]));
  sample_type*  input = static_cast<sample_type*>(mxGetData(prhs[0]));
#endif

  for (int i = 0; i <= in_channels; ++i)
  {
    inputs[i] = input;
    input += block_size;
  }

  for (int i = 0; i <= out_channels; ++i)
  {
    outputs[i] = output;
    output += block_size;
  }

  engine->audio_callback(block_size, inputs.data(), outputs.data());
}

void source(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  error_init();
  mex::error_no_output(nlhs, plhs, "source");

  std::string command;
  mex::next_arg(nrhs, prhs, command, "'source': second argument must be a "
      "string (the source property to set)!");

  if (command == "position")
  {
    if (nrhs < 1)
    {
      mexErrMsgTxt("'source position' needs a further argument!");
    }
    if (mxIsComplex(prhs[0]))
    {
      mexErrMsgTxt("Complex values are not allowed!");
    }
    if (!mxIsNumeric(prhs[0]))
    {
      mexErrMsgTxt("source positions must be in a numeric matrix!");
    }
    if (mxGetN(prhs[0]) != in_channels)
    {
      mexErrMsgTxt("Number of columns must be the same as number of sources!");
    }
    if (mxGetM(prhs[0]) == 3)
    {
      mexErrMsgTxt("Three-dimensional positions are not supported (yet)!");
    }
    if (mxGetM(prhs[0]) != 2)
    {
      mexErrMsgTxt("Number of rows must be 2 (x and y coordinates)!");
    }

    double* coordinates = mxGetPr(prhs[0]);

    --nrhs; ++prhs;
    mex::error_no_further(nrhs, prhs, "source");

    for (size_t i = 0; i < in_channels; ++i)
    {
      // TODO: handle 3D coordinates

      ssr::NfcHoaRenderer::SourceBase* source = engine->get_source(i + 1);
      // TODO: check if source == nullptr
      source->position(Position(coordinates[i*2], coordinates[i*2+1]));
    }
  }
  else if (command == "orientation")
  {
    // TODO
    mexErrMsgTxt("'source orientation' is not implemented yet!");
  }
  else
  {
    // TODO: more stuff: mute, volume, ...

    mexPrintf("Command: 'source %s'\n", command.c_str());
    mexErrMsgTxt("Unknown command!");
  }
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  try
  {
    std::string command;
    mex::next_arg(nrhs, prhs, command
        , "First argument must be a string (e.g. 'help')!");

    if (command == "help")
    {
      help(nlhs, plhs, nrhs, prhs);
    }
    else if (command == "init")
    {
      init(nlhs, plhs, nrhs, prhs);
    }
    else if (command == "process")
    {
      process(nlhs, plhs, nrhs, prhs);
    }
    else if (command == "source")
    {
      source(nlhs, plhs, nrhs, prhs);
    }
    else if (command == "block_size")
    {
      error_init();
      mex::error_no_further(nrhs, prhs, "block_size");
      mex::error_one_output(nlhs, plhs, "block_size");
      plhs[0] = mxCreateDoubleScalar(block_size);
    }
    else if (command == "out_channels")
    {
      error_init();
      mex::error_no_further(nrhs, prhs, "out_channels");
      mex::error_one_output(nlhs, plhs, "out_channels");
      plhs[0] = mxCreateDoubleScalar(out_channels);
    }
    // Only "clear" shall be documented, the others are hidden features
    else if (command == "free" || command == "delete" || command == "clear")
    {
      mex::error_no_further(nrhs, prhs, "clear");
      mex::error_no_output(nlhs, plhs, "clear");
      // This is safe even if engine wasn't initialized before:
      engine.reset();
    }
    else
    {
      mexPrintf("Command: '%s'\n", command.c_str());
      mexErrMsgTxt("Unknown command!");
    }
  }
  catch (std::exception& e)
  {
    mexErrMsgTxt(e.what());
  }
  catch (...)
  {
    mexErrMsgTxt("Unknown exception!");
  }
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
