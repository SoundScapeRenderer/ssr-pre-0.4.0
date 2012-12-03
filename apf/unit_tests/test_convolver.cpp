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

TEST_CASE("Convolver", "Test Convolver")
{

float test_signal[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.2f, 0.3f, 0.4f
  , 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.0f, 0.0f, 0.0f, 0.0f };
float zeros[20] = { 0.0f };

apf::Convolver conv(8, 4);

float* result;

float filter[16] = { 0.0f };
filter[10] = 5.0f;
filter[11] = 4.0f;
filter[12] = 3.0f;

SECTION("silence", "")
{
  conv.add_input_block(test_signal);
  result = conv.convolve_signal();

  CHECK_RANGE(result, zeros, 8);

  conv.add_input_block(test_signal);
  result = conv.convolve_signal();

  CHECK_RANGE(result, zeros, 8);
}

SECTION("impulse", "")
{
  float one = 1.0f;

  conv.add_input_block(test_signal);
  conv.set_filter(&one, (&one)+1);
  result = conv.convolve_signal();

  CHECK_RANGE(result, test_signal, 8);

  conv.add_input_block(test_signal + 8);
  result = conv.convolve_signal();

  CHECK_RANGE(result, test_signal + 8, 8);
}

SECTION("... and more", "")
{
  conv.set_filter(filter, filter + 16);

  float input[8] = { 0.0f };

  input[1] = 1.0f;
  conv.add_input_block(input);
  result = conv.convolve_signal();

  CHECK_RANGE(result, zeros, 8);

  input[1] = 2.0f;
  conv.add_input_block(input);
  CHECK_FALSE(conv.queues_empty());
  conv.rotate_queues();
  result = conv.convolve_signal();

  float expected[8] = { 0.0f };
  expected[3] = 5.0f;
  expected[4] = 4.0f;
  expected[5] = 3.0f;

  CHECK_RANGE(result, expected, 8);

  input[1] = 0.0f;
  conv.add_input_block(input);
  CHECK_FALSE(conv.queues_empty());
  conv.rotate_queues();
  result = conv.convolve_signal();

  expected[3] = 10.0f;
  expected[4] =  8.0f;
  expected[5] =  6.0f;

  CHECK_RANGE(result, expected, 8);

  conv.add_input_block(input);
  CHECK_FALSE(conv.queues_empty());
  conv.rotate_queues();
  result = conv.convolve_signal();

  CHECK_RANGE(result, zeros, 8);

  CHECK(conv.queues_empty());
}

SECTION("StaticConvolver impulse", "")
{
  float one = 1.0f;

  apf::StaticConvolver sconv(8, &one, (&one)+1);

  sconv.add_input_block(test_signal);
  result = sconv.convolve_signal();

  CHECK_RANGE(result, test_signal, 8);

  sconv.add_input_block(test_signal + 8);
  result = sconv.convolve_signal();

  CHECK_RANGE(result, test_signal + 8, 8);
}

SECTION("StaticConvolver frequency domain", "")
{
  float one = 1.0f;
  // 7 partitions, blocksize 8
  apf::StaticConvolver::filter_t fd_filter(std::make_pair(7, 16));

  conv.prepare_filter(&one, (&one)+1, fd_filter);
  apf::StaticConvolver sconv(8, fd_filter);

  // TODO: check if there are really 7 partitions

  sconv.add_input_block(test_signal);
  result = sconv.convolve_signal();

  CHECK_RANGE(result, test_signal, 8);
}

SECTION("more stuff with StaticConvolver", "")
{
  // Note: not the full filter is given (the rest is zero anyway)
  apf::StaticConvolver sconv(8, filter, filter + 13);

  float input[8] = { 0.0f };

  input[1] = 1.0f;
  sconv.add_input_block(input);
  result = sconv.convolve_signal();

  CHECK_RANGE(result, zeros, 8);

  input[1] = 2.0f;
  sconv.add_input_block(input);
  result = sconv.convolve_signal();

  float expected[8] = { 0.0f };
  expected[3] = 5.0f;
  expected[4] = 4.0f;
  expected[5] = 3.0f;

  CHECK_RANGE(result, expected, 8);

  input[1] = 0.0f;
  sconv.add_input_block(input);
  result = sconv.convolve_signal();

  expected[3] = 10.0f;
  expected[4] =  8.0f;
  expected[5] =  6.0f;

  CHECK_RANGE(result, expected, 8);

  sconv.add_input_block(input);
  result = sconv.convolve_signal();

  CHECK_RANGE(result, zeros, 8);
}

// TODO: test copy_nested() and transform_nested()!

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
