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

/** @file
 * Some macros to avoid code duplication in iterator (adaptor) classes.
 * For most of the macros you need special typedefs in your iterator class:
 * @c self, @c reference, @c pointer, @c difference_type, ...
 *
 * The assignment operator isn't provided here because normally the
 * auto-generated assignment operator can be used.
 *
 * @note A default constructor is a requirement for every iterator. If you
 *   implement a special constructor, you also have to implement a default
 *   constructor!
 *
 * @ingroup apf_iterators
 **/

#ifndef APF_ITERATOR_MACROS_H
#define APF_ITERATOR_MACROS_H

#include <cassert>  // for assert()

namespace apf
{
/// Check for null-pointer
/// @return @b true if @p in != 0, else @b false
template<typename T> bool no_nullptr(T* in) { return in != 0; }
/// Dummy overload for non-pointers
/// @return Always @b true
/// @note We can only check if plain pointers are NULL, use @c _GLIBCXX_DEBUG
/// to check for singular iterators!
template<typename T> bool no_nullptr(T&) { return true; }
}

/// Straightforward default constructor and constructor from base iterator.
/// @param iterator_name Name of the iterator class
/// @param base_iterator_type Typename of the base iterator
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_CONSTRUCTORS(iterator_name, base_iterator_type, base_member) \
  /** Constructor from base iterator. @param base_iterator base iterator **/ \
  explicit iterator_name(base_iterator_type base_iterator) \
    : base_member(base_iterator) { assert(apf::no_nullptr(base_member)); } \
  /** Default constructor. @note This constructor creates a singular iterator. Another iterator_name can be assigned to it, but nothing else works. **/ \
  iterator_name() : base_member() {}

/// Get the base iterator.
/// @param base_iterator_type Typename of the base iterator
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_BASE(base_iterator_type, base_member) \
  /** Get the base iterator, inspired by std::reverse_iterator::base(). **/ \
  base_iterator_type base() const { assert(apf::no_nullptr(base_member)); \
    return (base_member); }

/// @name Output Iterator Requirements
//@{

/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_OUTPUT_DEREFERENCE(base_member) \
  /** Straightforward dereference operator. **/ \
  reference operator*() const { assert(apf::no_nullptr(base_member)); \
    return *(base_member); }

/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_OUTPUT_PREINCREMENT(base_member) \
  /** Straightforward preincrement operator. **/ \
  self& operator++() { assert(apf::no_nullptr(base_member)); \
    ++(base_member); return *this; }

/// Postincrement operator (using preincrement operator).
#define APF_ITERATOR_OUTPUT_POSTINCREMENT \
  /** Postincrement operator (using preincrement operator). **/ \
  self operator++(int) { self tmp = *this; ++(*this); return tmp; }

//@}

/// @name Input Iterator Requirements
//@{

/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_INPUT_DEREFERENCE APF_ITERATOR_OUTPUT_DEREFERENCE

/// Straightforward arrow operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_INPUT_ARROW(base_member) \
  /** Straightforward arrow operator. **/ \
  pointer operator->() const { assert(apf::no_nullptr(base_member)); \
    return (base_member); }

/// Straightforward equality operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_INPUT_EQUAL(base_member) \
  /** Straightforward equality operator. **/ \
  bool operator==(const self& rhs) const { \
    assert(apf::no_nullptr(base_member) && apf::no_nullptr(rhs.base_member)); \
    return ((base_member) == (rhs.base_member)); }

/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_INPUT_PREINCREMENT APF_ITERATOR_OUTPUT_PREINCREMENT

/// Postincrement operator (using preincrement operator).
#define APF_ITERATOR_INPUT_POSTINCREMENT APF_ITERATOR_OUTPUT_POSTINCREMENT

/// Unequality operator (using equality operator)
#define APF_ITERATOR_INPUT_UNEQUAL \
  /** Unequality operator (using equality operator). **/ \
  bool operator!=(const self& rhs) const { return !operator==(rhs); } \

//@}

/// @name Forward Iterator Requirements
//@{

/// Straightforward equality operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_FORWARD_EQUAL APF_ITERATOR_INPUT_EQUAL
/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_FORWARD_DEREFERENCE APF_ITERATOR_INPUT_DEREFERENCE
/// Straightforward arrow operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_FORWARD_ARROW APF_ITERATOR_INPUT_ARROW
/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_FORWARD_PREINCREMENT APF_ITERATOR_INPUT_PREINCREMENT

/// Postincrement operator (using preincrement operator).
#define APF_ITERATOR_FORWARD_POSTINCREMENT APF_ITERATOR_INPUT_POSTINCREMENT
/// Unequality operator (using equality operator).
#define APF_ITERATOR_FORWARD_UNEQUAL APF_ITERATOR_INPUT_UNEQUAL

//@}

/// @name Bidirectional Iterator Requirements
//@{

/// Straightforward equality operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_BIDIRECTIONAL_EQUAL APF_ITERATOR_FORWARD_EQUAL
/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_BIDIRECTIONAL_DEREFERENCE APF_ITERATOR_FORWARD_DEREFERENCE
/// Straightforward arrow operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_BIDIRECTIONAL_ARROW APF_ITERATOR_FORWARD_ARROW
/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_BIDIRECTIONAL_PREINCREMENT APF_ITERATOR_FORWARD_PREINCREMENT

/// Straightforward predecrement operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_BIDIRECTIONAL_PREDECREMENT(base_member) \
  /** Straightforward predecrement operator. **/ \
  self& operator--() { assert(apf::no_nullptr(base_member)); \
    --(base_member); return *this; }

/// Postincrement operator (using preincrement operator).
#define APF_ITERATOR_BIDIRECTIONAL_POSTINCREMENT APF_ITERATOR_FORWARD_POSTINCREMENT
/// Unequality operator (using equality operator).
#define APF_ITERATOR_BIDIRECTIONAL_UNEQUAL APF_ITERATOR_FORWARD_UNEQUAL

/// Postdecrement operator (using predecrement operator).
#define APF_ITERATOR_BIDIRECTIONAL_POSTDECREMENT \
  /** Postdecrement operator (using predecrement operator). **/ \
  self operator--(int) { self tmp = *this; --(*this); return tmp; }

//@}

/// @name Random Access Iterator Requirements
//@{

/// Straightforward equality operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_RANDOMACCESS_EQUAL APF_ITERATOR_BIDIRECTIONAL_EQUAL
/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_RANDOMACCESS_DEREFERENCE APF_ITERATOR_BIDIRECTIONAL_DEREFERENCE
/// Straightforward arrow operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_RANDOMACCESS_ARROW APF_ITERATOR_BIDIRECTIONAL_ARROW
/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_RANDOMACCESS_PREINCREMENT APF_ITERATOR_BIDIRECTIONAL_PREINCREMENT
/// Straightforward predecrement operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_RANDOMACCESS_PREDECREMENT APF_ITERATOR_BIDIRECTIONAL_PREDECREMENT

/// Straightforward addition/assignment operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_RANDOMACCESS_ADDITION_ASSIGNMENT(base_member) \
  /** Straightforward addition/assignment operator. **/ \
  self& operator+=(difference_type n) { assert(apf::no_nullptr(base_member)); \
    (base_member) += n; return *this; }

/// Straightforward difference operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_RANDOMACCESS_DIFFERENCE(base_member) \
  /** Straightforward difference operator. **/ \
  friend difference_type operator-(const self& lhs, const self& rhs) { \
    assert(apf::no_nullptr(lhs.base_member) \
        && apf::no_nullptr(rhs.base_member)); \
    return ((lhs.base_member) - (rhs.base_member)); }

/// Straightforward subscript operator (using + and dereference operator).
#define APF_ITERATOR_RANDOMACCESS_SUBSCRIPT \
  /** Straightforward subscript operator (using + and dereference op). **/ \
  reference operator[](difference_type n) const { \
    return *(*this + n); }

/// Straightforward less-than operator.
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_RANDOMACCESS_LESS(base_member) \
  /** Straightforward less-than operator. **/ \
  friend bool operator<(const self& lhs, const self& rhs) { \
    assert(apf::no_nullptr(lhs.base_member) \
        && apf::no_nullptr(rhs.base_member)); \
    return (lhs.base_member) < (rhs.base_member); }

/// Unequality operator (using equality operator).
/// @param base_member Name of the member variable holding the base iterator.
#define APF_ITERATOR_RANDOMACCESS_UNEQUAL APF_ITERATOR_BIDIRECTIONAL_UNEQUAL

/// Other comparisons (>, <=, >=).
/// All are using the less-than operator.
#define APF_ITERATOR_RANDOMACCESS_OTHER_COMPARISONS \
  /** Straightforward greater-than operator (using less-than operator). **/ \
  friend bool operator>(const self& lhs, const self& rhs) \
    { return rhs < lhs; } \
  /** Straightforward less-or-equal operator (using less-than operator). **/ \
  friend bool operator<=(const self& lhs, const self& rhs) \
    { return !(rhs < lhs); } \
  /** Straightforward greater-or-equal operator (using less-than operator). **/\
  friend bool operator>=(const self& lhs, const self& rhs) \
    { return !(lhs < rhs); }

/// Postincrement operator (using preincrement operator).
#define APF_ITERATOR_RANDOMACCESS_POSTINCREMENT APF_ITERATOR_BIDIRECTIONAL_POSTINCREMENT

/// Postdecrement operator (using predecrement operator)
#define APF_ITERATOR_RANDOMACCESS_POSTDECREMENT APF_ITERATOR_BIDIRECTIONAL_POSTDECREMENT

/// The rest of the random access iterator requirements.
///   - Addition operator (iterator plus integer, using +=)
///   - Addition operator (integer plus iterator, using +=)
///   - Subtraction/assignment operator (using +=)
///   - Subtraction operator (using +=)
#define APF_ITERATOR_RANDOMACCESS_THE_REST \
  /** Addition operator (iterator plus integer, using +=). **/ \
  self operator+(difference_type n) const { self tmp(*this); return tmp += n; }\
  /** Addition operator (integer plus iterator, using +=). **/ \
  friend self operator+(difference_type n, const self& it) { \
    self temp(it); return temp += n; } \
  /** Subtraction/assignment operator (using +=). **/ \
  self& operator-=(difference_type n) { *this += -n; return *this; } \
  /** Subtraction operator (using +=). **/ \
  self operator-(difference_type n) const { self tmp(*this); return tmp += -n; }

//@}

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
