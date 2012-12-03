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

/// @file
/// Several more or less useful iterators and some macros.

#ifndef APF_ITERATOR_H
#define APF_ITERATOR_H

#include <cassert>  // for assert()
#include <iterator>  // for std::iterator_traits, std::output_iterator_tag, ...
#include <tr1/type_traits>  // for std::tr1::remove_reference

#include "apf/math.h"  // for wrap()

/** @defgroup apf_iterators Iterators
 * TODO: overview of iterators?
 *
 * @see apf_iterator_macros
 **/

/** @defgroup apf_iterator_macros Iterator macros
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
 * @see apf_iterators
 **/

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
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_CONSTRUCTORS(iterator_name, base_iterator_type, base_member) \
  /** Constructor from base iterator. @param base_iterator base iterator **/ \
  explicit iterator_name(base_iterator_type base_iterator) \
    : base_member(base_iterator) { assert(apf::no_nullptr(base_member)); } \
  /** Default constructor. @note This constructor creates a singular iterator. Another iterator_name can be assigned to it, but nothing else works. **/ \
  iterator_name() : base_member() {}

/// Get the base iterator.
/// @param base_iterator_type Typename of the base iterator
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_BASE(base_iterator_type, base_member) \
  /** Get the base iterator, inspired by std::reverse_iterator::base(). **/ \
  base_iterator_type base() const { assert(apf::no_nullptr(base_member)); \
    return (base_member); }

// Output Iterator Requirements

/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_OUTPUT_DEREFERENCE(base_member) \
  /** Straightforward dereference operator. **/ \
  reference operator*() const { assert(apf::no_nullptr(base_member)); \
    return *(base_member); }

/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_OUTPUT_PREINCREMENT(base_member) \
  /** Straightforward preincrement operator. **/ \
  self& operator++() { assert(apf::no_nullptr(base_member)); \
    ++(base_member); return *this; }

/// Postincrement operator (using preincrement operator).
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_OUTPUT_POSTINCREMENT \
  /** Postincrement operator (using preincrement operator). **/ \
  self operator++(int) { self tmp = *this; ++(*this); return tmp; }

// Input Iterator Requirements

/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_INPUT_DEREFERENCE APF_ITERATOR_OUTPUT_DEREFERENCE

/// Straightforward arrow operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_INPUT_ARROW(base_member) \
  /** Straightforward arrow operator. **/ \
  pointer operator->() const { assert(apf::no_nullptr(base_member)); \
    return (base_member); }

/// Straightforward equality operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_INPUT_EQUAL(base_member) \
  /** Straightforward equality operator. **/ \
  bool operator==(const self& rhs) const { \
    assert(apf::no_nullptr(base_member) && apf::no_nullptr(rhs.base_member)); \
    return ((base_member) == (rhs.base_member)); }

/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_INPUT_PREINCREMENT APF_ITERATOR_OUTPUT_PREINCREMENT

/// Postincrement operator (using preincrement operator).
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_INPUT_POSTINCREMENT APF_ITERATOR_OUTPUT_POSTINCREMENT

/// Unequality operator (using equality operator)
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_INPUT_UNEQUAL \
  /** Unequality operator (using equality operator). **/ \
  bool operator!=(const self& rhs) const { return !operator==(rhs); } \

// Forward Iterator Requirements

/// Straightforward equality operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_FORWARD_EQUAL APF_ITERATOR_INPUT_EQUAL
/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_FORWARD_DEREFERENCE APF_ITERATOR_INPUT_DEREFERENCE
/// Straightforward arrow operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_FORWARD_ARROW APF_ITERATOR_INPUT_ARROW
/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_FORWARD_PREINCREMENT APF_ITERATOR_INPUT_PREINCREMENT

/// Postincrement operator (using preincrement operator).
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_FORWARD_POSTINCREMENT APF_ITERATOR_INPUT_POSTINCREMENT
/// Unequality operator (using equality operator).
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_FORWARD_UNEQUAL APF_ITERATOR_INPUT_UNEQUAL

// Bidirectional Iterator Requirements

/// Straightforward equality operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_BIDIRECTIONAL_EQUAL APF_ITERATOR_FORWARD_EQUAL
/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_BIDIRECTIONAL_DEREFERENCE APF_ITERATOR_FORWARD_DEREFERENCE
/// Straightforward arrow operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_BIDIRECTIONAL_ARROW APF_ITERATOR_FORWARD_ARROW
/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_BIDIRECTIONAL_PREINCREMENT APF_ITERATOR_FORWARD_PREINCREMENT

/// Straightforward predecrement operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_BIDIRECTIONAL_PREDECREMENT(base_member) \
  /** Straightforward predecrement operator. **/ \
  self& operator--() { assert(apf::no_nullptr(base_member)); \
    --(base_member); return *this; }

/// Postincrement operator (using preincrement operator).
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_BIDIRECTIONAL_POSTINCREMENT APF_ITERATOR_FORWARD_POSTINCREMENT
/// Unequality operator (using equality operator).
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_BIDIRECTIONAL_UNEQUAL APF_ITERATOR_FORWARD_UNEQUAL

/// Postdecrement operator (using predecrement operator).
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_BIDIRECTIONAL_POSTDECREMENT \
  /** Postdecrement operator (using predecrement operator). **/ \
  self operator--(int) { self tmp = *this; --(*this); return tmp; }

// Random Access Iterator Requirements

/// Straightforward equality operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_EQUAL APF_ITERATOR_BIDIRECTIONAL_EQUAL
/// Straightforward dereference operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_DEREFERENCE APF_ITERATOR_BIDIRECTIONAL_DEREFERENCE
/// Straightforward arrow operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_ARROW APF_ITERATOR_BIDIRECTIONAL_ARROW
/// Straightforward preincrement operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_PREINCREMENT APF_ITERATOR_BIDIRECTIONAL_PREINCREMENT
/// Straightforward predecrement operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_PREDECREMENT APF_ITERATOR_BIDIRECTIONAL_PREDECREMENT

/// Straightforward addition/assignment operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_ADDITION_ASSIGNMENT(base_member) \
  /** Straightforward addition/assignment operator. **/ \
  self& operator+=(difference_type n) { assert(apf::no_nullptr(base_member)); \
    (base_member) += n; return *this; }

/// Straightforward difference operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_DIFFERENCE(base_member) \
  /** Straightforward difference operator. **/ \
  friend difference_type operator-(const self& lhs, const self& rhs) { \
    assert(apf::no_nullptr(lhs.base_member) \
        && apf::no_nullptr(rhs.base_member)); \
    return ((lhs.base_member) - (rhs.base_member)); }

/// Straightforward subscript operator (using + and dereference operator).
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_SUBSCRIPT \
  /** Straightforward subscript operator (using + and dereference op). **/ \
  reference operator[](difference_type n) const { \
    return *(*this + n); }

/// Straightforward less-than operator.
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_LESS(base_member) \
  /** Straightforward less-than operator. **/ \
  friend bool operator<(const self& lhs, const self& rhs) { \
    assert(apf::no_nullptr(lhs.base_member) \
        && apf::no_nullptr(rhs.base_member)); \
    return (lhs.base_member) < (rhs.base_member); }

/// Unequality operator (using equality operator).
/// @param base_member Name of the member variable holding the base iterator.
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_UNEQUAL APF_ITERATOR_BIDIRECTIONAL_UNEQUAL

/// Other comparisons (>, <=, >=).
/// All are using the less-than operator.
/// @ingroup apf_iterator_macros
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
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_POSTINCREMENT APF_ITERATOR_BIDIRECTIONAL_POSTINCREMENT

/// Postdecrement operator (using predecrement operator)
/// @ingroup apf_iterator_macros
#define APF_ITERATOR_RANDOMACCESS_POSTDECREMENT APF_ITERATOR_BIDIRECTIONAL_POSTDECREMENT

/// The rest of the random access iterator requirements.
///   - Addition operator (iterator plus integer, using +=)
///   - Addition operator (integer plus iterator, using +=)
///   - Subtraction/assignment operator (using +=)
///   - Subtraction operator (using +=)
/// @ingroup apf_iterator_macros
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

namespace apf
{

/// Convenience class providing begin() and end().
/// The derived class can manipulate the protected members _begin and _end.
/// @warning If you use this as base class, don't use a base class pointer (or
/// reference). This class has a public non-virtual destructor, which is
/// generally @b not recommended for base classes.
template<typename I>
class has_begin_and_end
{
  public:
    typedef I iterator;
    typedef typename std::iterator_traits<I>::reference reference;
    typedef size_t size_type;

    /// Default constructor. Singular iterators are created.
    has_begin_and_end() : _begin(), _end() {}

    /// Constructor with begin and end
    has_begin_and_end(iterator b, iterator e) : _begin(b), _end(e) {}

    /// Constructor with begin and length
    template<typename Distance>
    has_begin_and_end(iterator b, Distance length)
      : _begin(b)
      , _end(b)
    {
      std::advance(_end, length);
    }

    // auto-generated copy constructor is used

    /// Get begin.
    /// @note There is no const-ness propagation. The const-ness of the original
    ///   iterator I is maintained.
    iterator begin() const { return _begin; }

    /// Get end.
    /// @note There is no const-ness propagation. The const-ness of the original
    ///   iterator I is maintained.
    iterator end() const { return _end; }

    /// Subscript operator.
    /// @note This only works if @p I is a random access iterator!
    reference operator[](size_type n) const { return _begin[n]; }

  protected:
    iterator _begin;  ///< can be assigned to in derived class
    iterator _end;    ///< can be assigned to in derived class
};

/// Helper class for apf::cast_proxy and apf::transform_proxy.
/// @see iterator_proxy_const
template<typename I, typename Container>
class iterator_proxy
{
  public:
    typedef I iterator;
    typedef typename Container::size_type size_type;
    typedef typename std::iterator_traits<I>::value_type value_type;

    // implicit conversion is desired, therefore no "explicit" keyword
    iterator_proxy(Container& l) : _l(l) {}

    iterator begin() const { return iterator(_l.begin()); }
    iterator end() const { return iterator(_l.end()); }
    size_type size() const { return _l.size(); }

  private:
    Container& _l;
};

/// Helper class for cast_proxy_const and transform_proxy_const.
/// @see iterator_proxy
template<typename I, typename Container>
class iterator_proxy_const
{
  public:
    typedef I iterator;
    typedef typename Container::size_type size_type;
    typedef typename std::iterator_traits<I>::value_type value_type;

    // implicit conversion is desired, therefore no "explicit" keyword
    iterator_proxy_const(const Container& l) : _l(l) {}

    iterator begin() const { return iterator(_l.begin()); }
    iterator end() const { return iterator(_l.end()); }
    size_type size() const { return _l.size(); }

  private:
    const Container& _l;
};

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

/** Encapsulate a container of base pointers.
 * @tparam T Target type
 * @tparam Container type of (STL-like) container
 * @see cast_proxy_const, cast_iterator
 **/
template<typename T, typename Container>
struct cast_proxy : iterator_proxy<
                      cast_iterator<T, typename Container::iterator>, Container>
{
  typedef T value_type;

  cast_proxy(Container& l)
    : iterator_proxy<cast_iterator<
        T, typename Container::iterator>, Container>(l)
  {}
};

/** Helper function to create a cast_proxy.
 **/
template<typename T, typename Container>
cast_proxy<T, Container>
make_cast_proxy(Container& l)
{
  return cast_proxy<T, Container>(l);
}

/** Encapsulate a container of base pointers (const version).
 * The underlying container cannot be modified.
 * @see cast_proxy
 **/
template<typename T, typename Container>
struct cast_proxy_const : iterator_proxy_const<
          cast_iterator<const T, typename Container::const_iterator>, Container>
{
  typedef T value_type;

  cast_proxy_const(const Container& l)
    : iterator_proxy_const<cast_iterator<
        const T, typename Container::const_iterator>, Container>(l)
  {}
};

/** Helper function to create a cast_proxy_const.
 **/
template<typename T, typename Container>
cast_proxy_const<T, Container>
make_cast_proxy_const(Container& l)
{
  return cast_proxy_const<T, Container>(l);
}

/** Circular iterator class.
 * Creates an iterator which can be infinitely iterated. When reaching the end
 * of the range, it just starts again at the beginning. And vice versa.
 * @tparam I the iterator type on which the circular iterator is based on.
 * <tt>circular_iterator@<I@></tt> has the same @c iterator_category as @c I
 * itself. It only really works with random access iterators, however.
 * @ingroup apf_iterators
 **/
template<typename I>
class circular_iterator
{
  private:
    typedef circular_iterator self;

  public:
    /// @name Type Definitions from the Underlying Iterator
    //@{
    typedef typename std::iterator_traits<I>::iterator_category
                                                              iterator_category;
    typedef typename std::iterator_traits<I>::value_type      value_type;
    typedef typename std::iterator_traits<I>::difference_type difference_type;
    typedef typename std::iterator_traits<I>::pointer         pointer;
    typedef typename std::iterator_traits<I>::reference       reference;
    //@}

    /// @name Constructors
    /// Copy ctor and assignment operator are auto-generated by the compiler.
    //@{

    /// Constructor with explicit current iterator.
    /// @param begin begin of the original iterator range
    /// @param end end of said range
    /// @param current current iterator within the range
    circular_iterator(I begin, I end, I current)
      : _begin(begin)
      , _end(end)
      , _current(current)
    {
      assert(no_nullptr(_begin) && no_nullptr(_end) && no_nullptr(_current));
      assert(_begin != _end);
      assert(_current != _end);
    }

    /// Constructor with implicit current iterator.
    /// @param begin begin of the original iterator range
    /// @param end end of said range
    /// @note The current iterator is set to @a begin.
    circular_iterator(I begin, I end)
      : _begin(begin)
      , _end(end)
      , _current(begin)
    {
      assert(no_nullptr(_begin) && no_nullptr(_end) && no_nullptr(_current));
      assert(_begin != _end);
    }

    // Although this constructor is normally useless, it's good for unit tests.
    /// Constructor from one iterator.
    /// @param begin begin of the original iterator range
    /// @note The resulting iterator is of limited use, because there is only
    ///   one location where it will ever point to.
    ///   Let's call it a @em stagnant iterator.
    circular_iterator(I begin)
      : _begin(begin)
      , _end(begin + 1)
      , _current(begin)
    {
      assert(no_nullptr(_begin) && no_nullptr(_end) && no_nullptr(_current));
      assert(_begin != _end);
    }

    /// Default constructor.
    /// @note This constructor creates a singular iterator. Another
    /// circular_iterator can be assigned to it, but nothing else works.
    circular_iterator()
      : _begin()
      , _end()
      , _current()
    {}

    //@}

    /// @name Operators
    /// operator<, operator<=, operator>, operator>= don't make sense!
    //@{

    /// Preincrement operator.
    self&
    operator++()
    {
      assert(no_nullptr(_begin) && no_nullptr(_end) && no_nullptr(_current));
      ++_current;
      if (_current == _end) _current = _begin;
      return *this;
    }

    /// Predecrement operator.
    self&
    operator--()
    {
      assert(no_nullptr(_begin) && no_nullptr(_end) && no_nullptr(_current));
      if (_current == _begin) _current = _end;
      --_current;
      return *this;
    }

    /// Addition/assignment operator.
    self&
    operator+=(difference_type n)
    {
      assert(no_nullptr(_begin) && no_nullptr(_end) && no_nullptr(_current));
      difference_type length = _end     - _begin;
      difference_type index  = _current - _begin;
      index += n;
      _current = _begin + apf::math::wrap(index, length);
      return *this;
    }

    /// Difference operator.
    /// @note Always returns a non-negative difference.
    /// @warning This operator only works when @a a and @a b are iterators for
    ///   the same data (i.e. they were constructed with identical @a begin and
    ///   @a end iterators)! This is not checked internally, @b you have to do
    ///   that!
    /// @param lhs the iterator to the left of the minus sign
    /// @param rhs the iterator on the right side
    friend
    difference_type
    operator-(const self& lhs, const self& rhs)
    {
      assert(no_nullptr(lhs._begin)   && no_nullptr(rhs._begin));
      assert(no_nullptr(lhs._end)     && no_nullptr(rhs._end));
      assert(no_nullptr(lhs._current) && no_nullptr(rhs._current));
      assert(lhs._begin == rhs._begin);
      assert(lhs._end   == rhs._end  );

      difference_type d = lhs._current - rhs._current;

      if (d < 0)
      {
        d = lhs._current - lhs._begin + rhs._end - rhs._current;
      }
      // if lhs and rhs are the same, the difference is of course zero.
      return d;
    }

    // straightforward operators:
    APF_ITERATOR_RANDOMACCESS_DEREFERENCE(_current)
    APF_ITERATOR_RANDOMACCESS_ARROW(_current)
    APF_ITERATOR_RANDOMACCESS_EQUAL(_current)
    APF_ITERATOR_RANDOMACCESS_UNEQUAL
    APF_ITERATOR_RANDOMACCESS_SUBSCRIPT
    APF_ITERATOR_RANDOMACCESS_POSTINCREMENT
    APF_ITERATOR_RANDOMACCESS_POSTDECREMENT
    APF_ITERATOR_RANDOMACCESS_THE_REST

    //@}

    APF_ITERATOR_BASE(I, _current)

  private:
    I _begin;   ///< begin of the underlying iterator range
    I _end;     ///< end of said range
    I _current; ///< current position in that range
};

/** Iterator adaptor with a function call at dereferenciation.
 * @tparam I type of base iterator
 * @tparam F Unary function object with inner type @c result_type.
 *   Example: math::raised_cosine
 * @warning The @c result_type of @p F can be any type, but operator->() only
 *   works if it is a reference!
 *   If it's not a reference, you will get an error similar to this:
 *   @code error: lvalue required as unary ‘&’ operand @endcode
 * @see boost::transform_iterator: same idea, but much fancier implementation!
 * http://boost.org/doc/libs/release/libs/iterator/doc/transform_iterator.html
 * @see transform_proxy, transform_proxy_const
 * @ingroup apf_iterators
 **/
template<typename I, typename F>
class transform_iterator
{
  private:
    typedef transform_iterator self;

  public:
    typedef typename std::iterator_traits<I>::iterator_category
                                                              iterator_category;
    /// Can be a reference, but doesn't necessarily have to
    typedef typename F::result_type reference;
    typedef typename std::tr1::remove_reference<reference>::type value_type;
    typedef value_type* pointer;
    typedef typename std::iterator_traits<I>::difference_type difference_type;

    /// Constructor.
    explicit transform_iterator(I base_iterator = I(), F f = F())
      : _base_iterator(base_iterator)
      , _f(f)
    {}

    /// Dereference operator.
    /// Dereference the base iterator, use it as argument to the stored function
    /// and return the result.
    /// @note This is non-const because _f might have state
    reference operator*() { return _f(*_base_iterator); }

    /// Arrow operator.
    /// Dereference the base iterator, use it as argument to the stored function
    /// and return the <em>address of the</em> result.
    /// @warning Only works if @c F::result_type is a reference!
    pointer operator->()
    {
      return &this->operator*();
    }

    APF_ITERATOR_RANDOMACCESS_EQUAL(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREINCREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_LESS(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREDECREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_ADDITION_ASSIGNMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_DIFFERENCE(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_POSTINCREMENT
    APF_ITERATOR_RANDOMACCESS_UNEQUAL
    APF_ITERATOR_RANDOMACCESS_SUBSCRIPT
    APF_ITERATOR_RANDOMACCESS_POSTDECREMENT
    APF_ITERATOR_RANDOMACCESS_OTHER_COMPARISONS
    APF_ITERATOR_RANDOMACCESS_THE_REST

    APF_ITERATOR_BASE(I, _base_iterator)

  private:
    I _base_iterator;
    F _f;
};

/** Helper function to create a transform_iterator.
 * The template parameters are optional because they can be inferred from the
 * parameters @p base_iterator and @p f. Example:
 * @code make_transform_iterator(some_iterator, some_function) @endcode
 * @param base_iterator the base iterator
 * @param f the function (normally a function object)
 * @return a transform_iterator
 * @ingroup apf_iterators
 **/
template<typename I, typename F>
transform_iterator<I, F>
make_transform_iterator(I base_iterator, F f)
{
  return transform_iterator<I, F>(base_iterator, f);
}

/** Wrap a container and provide a transform_iterator instead of the normal one.
 * @tparam F Function to be used for the transform_iterator
 * @tparam Container Type of the container
 * @see transform_proxy_const, transform_iterator
 **/
template<typename F, typename Container>
struct transform_proxy : iterator_proxy<
                 transform_iterator<typename Container::iterator, F>, Container>
{
  transform_proxy(Container& l)
    : iterator_proxy<transform_iterator<
        typename Container::iterator, F>, Container>(l)
  {}
};

/** Wrap a container and provide a transform_iterator (const version).
 * The underlying container cannot be modified.
 * @see transform_proxy
 **/
template<typename F, typename Container>
struct transform_proxy_const : iterator_proxy_const<
           transform_iterator<typename Container::const_iterator, F>, Container>
{
  transform_proxy_const(const Container& l)
    : iterator_proxy_const<transform_iterator<
      typename Container::const_iterator, F>, Container>(l)
  {}
};

/** Iterator with a built-in number.
 * This can be used, for example, as a base iterator in transform_iterator.
 * @tparam T type of the number
 * @warning If @p T is an unsigned type, strange things may happen!
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
