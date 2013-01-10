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

// Tests for the Convolver.

#include "apf/convolver.h"

#include "catch/catch.hpp"

#define CHECK_RANGE(left, right, range) \
  for (int i = 0; i < range; ++i) { \
    INFO("i = " << i); \
    CHECK((left)[i] == Approx((right)[i])); }

namespace c = apf::conv;

TEST_CASE("Convolver", "Test Convolver")
{

float test_signal[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.2f, 0.3f, 0.4f
  , 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.0f, 0.0f, 0.0f, 0.0f };
float zeros[20] = { 0.0f };

c::Input conv_input(8, c::min_partitions(8, 16));
c::Output conv_output(conv_input);

float* result;

float filter[16] = { 0.0f };
filter[10] = 5.0f;
filter[11] = 4.0f;
filter[12] = 3.0f;

SECTION("silence", "")
{
  conv_input.add_block(test_signal);
  result = conv_output.convolve();

  CHECK_RANGE(result, zeros, 8);

  conv_input.add_block(test_signal);
  result = conv_output.convolve();

  CHECK_RANGE(result, zeros, 8);
}

SECTION("impulse", "")
{
  float one = 1.0f;

  conv_input.add_block(test_signal);
  conv_output.set_filter(&one, (&one)+1);
  result = conv_output.convolve();

  CHECK_RANGE(result, test_signal, 8);

  conv_input.add_block(test_signal + 8);
  result = conv_output.convolve();

  CHECK_RANGE(result, test_signal + 8, 8);
}

SECTION("... and more", "")
{
  conv_output.set_filter(filter, filter + 16);

  float input[8] = { 0.0f };

  input[1] = 1.0f;
  conv_input.add_block(input);
  result = conv_output.convolve();

  CHECK_RANGE(result, zeros, 8);

  input[1] = 2.0f;
  conv_input.add_block(input);
  CHECK_FALSE(conv_output.queues_empty());
  conv_output.rotate_queues();
  result = conv_output.convolve();

  float expected[8] = { 0.0f };
  expected[3] = 5.0f;
  expected[4] = 4.0f;
  expected[5] = 3.0f;

  CHECK_RANGE(result, expected, 8);

  input[1] = 0.0f;
  conv_input.add_block(input);
  CHECK(conv_output.queues_empty());
  //conv_output.rotate_queues();  // not necessary, because queues are empty
  result = conv_output.convolve();

  expected[3] = 10.0f;
  expected[4] =  8.0f;
  expected[5] =  6.0f;

  CHECK_RANGE(result, expected, 8);

  conv_input.add_block(input);
  CHECK(conv_output.queues_empty());
  //conv_output.rotate_queues();  // not necessary, because queues are empty
  result = conv_output.convolve();

  CHECK_RANGE(result, zeros, 8);

  CHECK(conv_output.queues_empty());
}

SECTION("StaticConvolverOutput impulse", "")
{
  float one = 1.0f;

  c::Input sconv_input(8, 1);
  c::StaticOutput sconv_output(sconv_input, &one, (&one)+1);

  sconv_input.add_block(test_signal);
  result = sconv_output.convolve();

  CHECK_RANGE(result, test_signal, 8);

  sconv_input.add_block(test_signal + 8);
  result = sconv_output.convolve();

  CHECK_RANGE(result, test_signal + 8, 8);
}

SECTION("StaticConvolverOutput frequency domain", "")
{
  float one = 1.0f;
  // 7 partitions, blocksize 8
  c::filter_t fd_filter(std::make_pair(7, 16));

  c::Input sconv_input(8, 7);
  conv_input.prepare_filter(&one, (&one)+1, fd_filter);
  c::StaticOutput sconv_output(sconv_input, fd_filter);

  // TODO: check if there are really 7 partitions

  sconv_input.add_block(test_signal);
  result = sconv_output.convolve();

  CHECK_RANGE(result, test_signal, 8);
}

SECTION("conv::Simple", "")
{
  // Note: not the full filter is given (the rest is zero anyway)
  c::Simple sconv(8, filter, filter + 13);

  float input[8] = { 0.0f };

  input[1] = 1.0f;
  sconv.add_block(input);
  result = sconv.convolve();

  CHECK_RANGE(result, zeros, 8);

  input[1] = 2.0f;
  sconv.add_block(input);
  result = sconv.convolve();

  float expected[8] = { 0.0f };
  expected[3] = 5.0f;
  expected[4] = 4.0f;
  expected[5] = 3.0f;

  CHECK_RANGE(result, expected, 8);

  input[1] = 0.0f;
  sconv.add_block(input);
  result = sconv.convolve();

  expected[3] = 10.0f;
  expected[4] =  8.0f;
  expected[5] =  6.0f;

  CHECK_RANGE(result, expected, 8);

  sconv.add_block(input);
  result = sconv.convolve();

  CHECK_RANGE(result, zeros, 8);
}

// TODO: test copy_nested() and transform_nested()!

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
