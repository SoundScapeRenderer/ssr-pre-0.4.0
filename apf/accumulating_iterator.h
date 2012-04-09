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
/// Accumulating output iterator.

#ifndef APF_ACCUMULATING_ITERATOR_H
#define APF_ACCUMULATING_ITERATOR_H

#include <iterator>  // for std::output_iterator_tag
#include "apf/iterator_macros.h"

namespace apf
{

/** An output iterator which adds on assignment.
 * Whenever the operator= of a pointee is called, its operator+= is invoked.
 * This is done by the helper class output_proxy.
 *
 * The idea to this iterator comes from boost::function_output_iterator:
 * http://www.boost.org/doc/libs/release/libs/iterator/doc/function_output_iterator.html
 * @tparam I Base iterator type
 * @see make_accumulating_iterator (helper function)
 * @ingroup apf_iterators
 **/
template<typename I>
class accumulating_iterator
{
  private:
    typedef accumulating_iterator self;

  public:
    typedef std::output_iterator_tag iterator_category;
    typedef void                     value_type;
    typedef void                     difference_type;
    typedef void                     pointer;
    typedef void                     reference;

    APF_ITERATOR_CONSTRUCTORS(accumulating_iterator, I, _base_iterator)

    /// Helper class.
    class output_proxy
    {
      public:
        /// Constructor. @param i the base iterator
        explicit output_proxy(I& i) : _i(i) {}

        /// Assignment operator.
        /// Value @p v is added to the current value on assignment.
        template<typename V>
        output_proxy& operator=(const V& v) { *_i += v; return *this; }

      private:
        I& _i;
    };

    /// Dereference operator.
    /// @return a temporary object of type output_proxy
    output_proxy operator*()
    {
      assert(no_nullptr(_base_iterator));
      return output_proxy(_base_iterator);
    }

    // operator-> doesn't make sense!

    // straightforward operators:
    APF_ITERATOR_OUTPUT_PREINCREMENT(_base_iterator)
    APF_ITERATOR_OUTPUT_POSTINCREMENT

    APF_ITERATOR_BASE(I, _base_iterator)

  private:
    I _base_iterator;
};

/** Helper function to create an accumulating_iterator.
 * The template parameter is optional because it can be inferred from the
 * parameter @p base_iterator. Example:
 * @code make_accumulating_iterator(some_iterator) @endcode
 * @param base_iterator the base iterator
 * @return an accumulating_iterator
 * @ingroup apf_iterators
 **/
template<typename I>
accumulating_iterator<I>
make_accumulating_iterator(I base_iterator)
{
  return accumulating_iterator<I>(base_iterator);
}

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
