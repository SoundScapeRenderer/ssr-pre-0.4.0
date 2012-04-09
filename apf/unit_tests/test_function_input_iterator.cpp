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

// Tests for function_input_iterator.

#include "apf/function_input_iterator.h"
#include "iterator_test_macros.h"

#include "catch/catch.hpp"

// Requirements for input iterators:
// http://www.cplusplus.com/reference/std/iterator/InputIterator/

struct three_halves
{
  typedef int argument_type;  // this is optional
  typedef float result_type;  // this is necessary

  float operator()(int in) { return static_cast<float>(in) * 1.5f; }
};

typedef apf::function_input_iterator<int*, three_halves> fii;

TEST_CASE("iterators/function_input_iterator"
    , "Test all functions of function_input_iterator")
{

ITERATOR_TEST_SECTION_BASE(fii, int)
ITERATOR_TEST_SECTION_DEFAULT_CTOR(fii)
ITERATOR_TEST_SECTION_COPY_ASSIGNMENT(fii, int)
ITERATOR_TEST_SECTION_EQUALITY(fii, int)
ITERATOR_TEST_SECTION_INCREMENT(fii, int)
ITERATOR_TEST_SECTION_DECREMENT(fii, int)
ITERATOR_TEST_SECTION_PLUS_MINUS(fii, int)
ITERATOR_TEST_SECTION_LESS(fii, int)

int array[] = { 1, 2, 3 };

SECTION("dereference", "*a; *a++; a[]")
{
  fii iter(array);

  CHECK(*iter == 1.5f);
  CHECK(iter.base() == &array[0]);
  CHECK(*iter++ == 1.5f);
  CHECK(iter.base() == &array[1]);

  CHECK(*iter-- == 3.0f);
  CHECK(iter.base() == &array[0]);

  CHECK(iter[2] == 4.5f);

  // NOTE: operator->() is purposely not implemented!
}

SECTION("test make_function_input_iterator", "namespace-level helper function")
{
  CHECK(*apf::make_function_input_iterator(array, three_halves()) == 1.5f);
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
