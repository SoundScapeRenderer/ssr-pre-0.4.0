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
/// An iterator to slice a sequence with a given step-size (stride).

#ifndef APF_STRIDE_ITERATOR_H
#define APF_STRIDE_ITERATOR_H

#include <iterator>
#include <cassert>
#include "apf/iterator_macros.h"

namespace apf
{

/** A stride iterator.
 * @tparam I Base iterator type
 *
 * Most of the code is taken from the C++ Cookbook, recipe 11.13:
 * http://flylib.com/books/en/2.131.1.171/1/
 *
 * Some modifications are based on this:
 * http://stackoverflow.com/questions/1649529/sorting-blocks-of-l-elements/1649650#1649650
 * @ingroup apf_iterators
 **/
template<class I>
class stride_iterator
{
  private:
    typedef stride_iterator self;

  public:
    /// @name Type Definitions from the Underlying Iterator
    //@{
    typedef typename std::iterator_traits<I>::iterator_category
                                                              iterator_category;
    typedef typename std::iterator_traits<I>::value_type      value_type;
    typedef typename std::iterator_traits<I>::difference_type difference_type;
    typedef typename std::iterator_traits<I>::reference       reference;
    typedef typename std::iterator_traits<I>::pointer         pointer;
    //@}

    /// This is the normal constructor.
    /// @param base_iterator the base iterator
    /// @param step the step size
    explicit stride_iterator(I base_iterator, difference_type step
#ifdef APF_STRIDE_ITERATOR_DEFAULT_STRIDE
        = APF_STRIDE_ITERATOR_DEFAULT_STRIDE
#endif
        )
      : _iter(base_iterator)
      , _step(step)
    {
      assert(no_nullptr(_iter));
    }

    /// Create a stride iterator from another stride iterator.
    /// @param base_iterator the base (stride) iterator
    /// @param step the step size (in addition to the already present step size)
    stride_iterator(stride_iterator<I> base_iterator, difference_type step)
      : _iter(base_iterator.base())
      , _step(base_iterator.step_size() * step)
    {
      assert(no_nullptr(_iter));
    }

    /// Default constructor.
    /// @note This constructor creates a singular iterator. Another
    /// circular_iterator can be assigned to it, but nothing else works.
    stride_iterator()
      : _iter()
      , _step(0)
    {}

    bool
    operator==(const self& rhs) const
    {
      assert(no_nullptr(_iter) && no_nullptr(rhs._iter));
      assert(_step == rhs._step);
      return (_iter == rhs._iter);
    }

    self&
    operator++()
    {
      assert(no_nullptr(_iter));
      std::advance(_iter, _step);
      return *this;
    }

    self&
    operator--()
    {
      assert(no_nullptr(_iter));
      std::advance(_iter, -_step);
      return *this;
    }

    reference
    operator[](difference_type n) const
    {
      assert(no_nullptr(_iter));
      return _iter[n * _step];
    }

    self&
    operator+=(difference_type n)
    {
      assert(no_nullptr(_iter));
      std::advance(_iter, n * _step);
      return *this;
    }

    friend
    difference_type
    operator-(const self& lhs, const self& rhs)
    {
      assert(no_nullptr(lhs._iter) && no_nullptr(rhs._iter));
      assert(lhs._step == rhs._step);
      return (lhs._iter - rhs._iter) / lhs._step;
    }

    friend
    bool
    operator<(const self& lhs, const self& rhs)
    {
      assert(no_nullptr(lhs._iter) && no_nullptr(rhs._iter));
      assert(lhs._step == rhs._step);
      return lhs._iter < rhs._iter;
    }

    friend
    bool
    operator<=(const self& lhs, const self& rhs)
    {
      assert(no_nullptr(lhs._iter) && no_nullptr(rhs._iter));
      assert(lhs._step == rhs._step);
      return lhs._iter <= rhs._iter;
    }

    friend
    bool
    operator>(const self& lhs, const self& rhs)
    {
      assert(no_nullptr(lhs._iter) && no_nullptr(rhs._iter));
      assert(lhs._step == rhs._step);
      return lhs._iter > rhs._iter;
    }

    friend
    bool
    operator>=(const self& lhs, const self& rhs)
    {
      assert(no_nullptr(lhs._iter) && no_nullptr(rhs._iter));
      assert(lhs._step == rhs._step);
      return lhs._iter >= rhs._iter;
    }

    // straightforward operators:
    APF_ITERATOR_RANDOMACCESS_DEREFERENCE(_iter)
    APF_ITERATOR_RANDOMACCESS_ARROW(_iter)
    APF_ITERATOR_RANDOMACCESS_UNEQUAL
    APF_ITERATOR_RANDOMACCESS_POSTINCREMENT
    APF_ITERATOR_RANDOMACCESS_POSTDECREMENT
    APF_ITERATOR_RANDOMACCESS_THE_REST

    APF_ITERATOR_BASE(I, _iter)

    /// Get step size
    difference_type step_size() const { return _step; }

  private:
    I _iter; ///< Base iterator
    // this could be const, except then the iterator wouldn't be copyable:
    difference_type _step; ///< Iterator increment
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
