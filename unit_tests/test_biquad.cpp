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

// Tests for BiQuad and Cascade.

#include "apf/biquad.h"

#include "catch/catch.hpp"

TEST_CASE("BiQuad", "Test BiQuad")
{

// TODO: more tests!

SECTION("basic", "only instantiations and very basic stuff")
{
  apf::BiQuad<double> a;
  apf::BiQuad<float> b;

  apf::SosCoefficients<double> c(0.1, 0.1, 0.1, 0.1, 0.1);
  a = c;

  CHECK(a.b0 == 0.1);

  apf::Cascade<apf::BiQuad<double> > d(25);
  apf::Cascade<apf::BiQuad<float> > e(25);
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
