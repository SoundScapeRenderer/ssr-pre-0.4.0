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
/// Iterator that casts an item to a given type on dereferenciation.

#ifndef APF_CAST_ITERATOR_H
#define APF_CAST_ITERATOR_H

#include <iterator> // for std::iterator_traits
#include "apf/iterator_macros.h"

namespace apf
{

/** Iterator that casts items to @p T* on dereferenciation.
 * There is an @em extra dereference inside the dereference operator, similar to
 * @c boost::indirect_iterator
 * (http://boost.org/doc/libs/release/libs/iterator/doc/indirect_iterator.html).
 * But before that, the pointer is casted to the desired type @p T.
 * @tparam T Pointee type to be casted to
 * @tparam I Base iterator type
 * @pre @c value_type of @p I must be a pointer to a compatible type!
 * @see make_cast_iterator() (helper function)
 * @see cast_proxy, cast_proxy_const
 * @ingroup apf_iterators
 **/
template<typename T, typename I>
class cast_iterator
{
  private:
    typedef cast_iterator self;

  public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef typename std::iterator_traits<I>::difference_type difference_type;
    typedef typename std::iterator_traits<I>::iterator_category
                                                              iterator_category;

    APF_ITERATOR_CONSTRUCTORS(cast_iterator, I, _base_iterator)

    // auto-generated copy ctor and assignment operator are OK.

    /// Dereference operator.
    /// @return Reference to current item, casted to @p T.
    reference operator*() const
    {
      assert(no_nullptr(_base_iterator));
      return *this->operator->();
    }

    /// Arrow operator.
    /// @return Pointer to current item, casted to @p T.
    pointer operator->() const
    {
      assert(no_nullptr(_base_iterator));
      // I::value_type must be a pointer to something!
      return _cast_helper(*_base_iterator);
    }

    /// Subscript operator.
    /// @param n index
    /// @return Reference to n-th item, casted to @p T.
    reference operator[](difference_type n) const
    {
      assert(no_nullptr(_base_iterator));
      return *_cast_helper(_base_iterator[n]);
    }

    // straightforward operators:
    APF_ITERATOR_RANDOMACCESS_EQUAL(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_LESS(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREINCREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREDECREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_ADDITION_ASSIGNMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_DIFFERENCE(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_POSTINCREMENT
    APF_ITERATOR_RANDOMACCESS_POSTDECREMENT
    APF_ITERATOR_RANDOMACCESS_UNEQUAL
    APF_ITERATOR_RANDOMACCESS_OTHER_COMPARISONS
    APF_ITERATOR_RANDOMACCESS_THE_REST

    APF_ITERATOR_BASE(I, _base_iterator)

  private:
    /// Do the actual cast.
    /// @param ptr Pointer to be casted
    /// @return @p ptr casted to T*
    /// @pre @p ptr must be a pointer to something. This is automagically
    ///   asserted (at compile time).
    template<typename X>
    pointer _cast_helper(X* ptr) const
    {
      return static_cast<pointer>(ptr);
    }

    I _base_iterator;
};

/** Helper function to create a cast_iterator.
 * The second template parameter is optional because it can be inferred from the
 * parameter @p base_iterator. Example:
 * @code make_cast_iterator<my_target_type>(some_iterator) @endcode
 * @param base_iterator the base iterator
 * @return a cast_iterator
 * @ingroup apf_iterators
 **/
template<typename T, typename I>
cast_iterator<T, I>
make_cast_iterator(I base_iterator)
{
  return cast_iterator<T, I>(base_iterator);
}

/** Encapsulate a container of base pointers (const version).
 * The underlying container cannot be modified.
 * @see cast_proxy
 **/
template<typename T, typename Container>
class cast_proxy_const
{
  public:
    typedef cast_iterator<const T, typename Container::const_iterator> iterator;
    typedef typename Container::size_type size_type;

    // implicit conversion is desired, therefore no "explicit" keyword
    cast_proxy_const(const Container& l) : _l(l) {}

    iterator begin() const { return iterator(_l.begin()); }
    iterator end() const { return iterator(_l.end()); }
    size_type size() const { return _l.size(); }

  private:
    const Container& _l;
};

/** Encapsulate a container of base pointers.
 * @tparam T Target type
 * @tparam Container type of (STL-like) container
 * @see cast_iterator
 **/
template<typename T, typename Container>
class cast_proxy
{
  public:
    typedef cast_iterator<T, typename Container::iterator> iterator;
    typedef typename Container::size_type size_type;

    // implicit conversion is desired, therefore no "explicit" keyword
    cast_proxy(Container& l) : _l(l) {}

    iterator begin() const { return iterator(_l.begin()); }
    iterator end() const { return iterator(_l.end()); }
    size_type size() const { return _l.size(); }

  private:
    Container& _l;
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
