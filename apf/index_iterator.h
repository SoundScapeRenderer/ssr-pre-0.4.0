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
/// Iterator with a built-in number.

#ifndef APF_INDEX_ITERATOR_H
#define APF_INDEX_ITERATOR_H

#include <iterator>  // for std::input_iterator_tag

#include "apf/iterator_macros.h"

namespace apf
{

/** Iterator with a built-in number.
 * This can be used, for example, as a base iterator in function_input_iterator.
 * @tparam T type of the number
 * @ingroup apf_iterators
 **/
template<typename T>
class index_iterator
{
  private:
    typedef index_iterator self;

  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef T reference;
    typedef T difference_type;
    typedef void pointer;

    /// Constructor. @param start Starting index
    explicit index_iterator(T start = T())
      : _number(start)
    {}

    /// Dereference operator. @return The current index
    reference operator*() const { return _number; }

    // operator-> doesn't make sense!

    APF_ITERATOR_RANDOMACCESS_EQUAL(_number)
    APF_ITERATOR_RANDOMACCESS_PREINCREMENT(_number)
    APF_ITERATOR_RANDOMACCESS_PREDECREMENT(_number)
    APF_ITERATOR_RANDOMACCESS_ADDITION_ASSIGNMENT(_number)
    APF_ITERATOR_RANDOMACCESS_DIFFERENCE(_number)
    APF_ITERATOR_RANDOMACCESS_LESS(_number)
    APF_ITERATOR_RANDOMACCESS_SUBSCRIPT
    APF_ITERATOR_RANDOMACCESS_UNEQUAL
    APF_ITERATOR_RANDOMACCESS_OTHER_COMPARISONS
    APF_ITERATOR_RANDOMACCESS_POSTINCREMENT
    APF_ITERATOR_RANDOMACCESS_POSTDECREMENT
    APF_ITERATOR_RANDOMACCESS_THE_REST

  private:
    T _number;
};

/** Helper function to create an index_iterator.
 * The template parameter is optional because it can be inferred from the
 * parameter @p start. Example:
 * @code make_index_iterator(42) @endcode
 * @param start the start index
 * @return an index_iterator
 * @ingroup apf_iterators
 **/
template<typename T>
index_iterator<T>
make_index_iterator(T start)
{
  return index_iterator<T>(start);
}

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
