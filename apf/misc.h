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
/// Miscellaneous helper classes.

#ifndef APF_MISC_H
#define APF_MISC_H

#include <iterator>  // for std::advance()

namespace apf
{

/// Classes derived from this class cannot be copied.
/// Private copy constructor and copy assignment ensure that.
class NonCopyable
{
  protected:
     NonCopyable() {} ///< Protected default constructor
    ~NonCopyable() {} ///< Protected non-virtual destructor

  private:
    /// @name Deactivated copy ctor and assignment operator
    //@{
    NonCopyable(const NonCopyable&);
    NonCopyable& operator=(const NonCopyable&);
    //@}
};

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
    iterator   end() const { return _end; }

  protected:
    iterator _begin;  ///< can be assigned to in derived class
    iterator _end;    ///< can be assigned to in derived class
};

} // namespace classtools

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
