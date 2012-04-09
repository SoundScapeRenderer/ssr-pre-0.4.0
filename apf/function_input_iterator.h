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

/// @file
/// Iterator adaptor with a function call at dereferenciation.

#ifndef APF_FUNCTION_INPUT_ITERATOR_H
#define APF_FUNCTION_INPUT_ITERATOR_H

#include <iterator>  // for std::input_iterator_tag, std::iterator_traits

#include "apf/iterator_macros.h"

namespace apf
{

/** Iterator adaptor with a function call at dereferenciation.
 * @tparam I type of base iterator
 * @tparam F function object with inner type @c result_type.
 *   Example: math::raised_cosine
 * @ingroup apf_iterators
 **/
template<typename I, typename F>
class function_input_iterator
{
  private:
    typedef function_input_iterator self;

  public:
    typedef std::input_iterator_tag iterator_category;
    typedef typename F::result_type value_type;
    typedef typename F::result_type reference;
    typedef typename std::iterator_traits<I>::difference_type difference_type;
    typedef void pointer;

    /// Constructor.
    explicit function_input_iterator(I base_iterator = I(), F f = F())
      : _base_iterator(base_iterator)
      , _f(f)
    {}

    /// Dereference operator.
    /// Dereference the base iterator, use it as argument to the stored function
    /// and return the result
    /// @note This is non-const because _f might have state
    reference operator*() { return _f(*_base_iterator); }

    // operator-> doesn't make too much sense!

    APF_ITERATOR_INPUT_EQUAL(_base_iterator)
    APF_ITERATOR_INPUT_PREINCREMENT(_base_iterator)
    APF_ITERATOR_INPUT_POSTINCREMENT
    APF_ITERATOR_INPUT_UNEQUAL

    // because we are generous, we throw in a few more operators, despite the
    // fact that function_input_iterator is an input iterator

    APF_ITERATOR_RANDOMACCESS_LESS(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREDECREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_ADDITION_ASSIGNMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_DIFFERENCE(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_SUBSCRIPT
    APF_ITERATOR_RANDOMACCESS_POSTDECREMENT
    APF_ITERATOR_RANDOMACCESS_OTHER_COMPARISONS
    APF_ITERATOR_RANDOMACCESS_THE_REST

    APF_ITERATOR_BASE(I, _base_iterator)

  private:
    I _base_iterator;
    F _f;
};

/** Helper function to create a function_input_iterator.
 * The template parameters are optional because they can be inferred from the
 * parameters @p base_iterator and @p f. Example:
 * @code make_function_input_iterator(some_iterator, some_function) @endcode
 * @param base_iterator the base iterator
 * @param f the function (normally a function object)
 * @return a function_input_iterator
 * @ingroup apf_iterators
 **/
template<typename I, typename F>
function_input_iterator<I, F>
make_function_input_iterator(I base_iterator, F f)
{
  return function_input_iterator<I, F>(base_iterator, f);
}

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
