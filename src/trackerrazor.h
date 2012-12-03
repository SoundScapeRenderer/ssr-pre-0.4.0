/******************************************************************************
 * Copyright (c) 2006-2012 Quality & Usability Lab                            *
 *                         Deutsche Telekom Laboratories, TU Berlin           *
 *                         Ernst-Reuter-Platz 7, 10587 Berlin, Germany        *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://tu-berlin.de/?id=ssr                  SoundScapeRenderer@telekom.de *
 ******************************************************************************/

/** @file
 * Razor AHRS tracker (definition).
 * See http://dev.qu.tu-berlin.de/projects/sf-razor-9dof-ahrs/wiki
 *
 * $LastChangedDate: 2011-11-30 11:52:04 +0100 (Wed, 30 Nov 2011) $
 * $LastChangedRevision: 1566 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef TRACKERRAZOR_H
#define TRACKERRAZOR_H

#include "tracker.h"  // base class
#include "razor-ahrs/RazorAHRS.h"
#include "publisher.h"

namespace ssr
{

/// Razor AHRS tracker 
class TrackerRazor : public Tracker
{
  public:
    typedef std::auto_ptr<TrackerRazor> ptr_t;

    /// "named constructor"
    static ptr_t create(Publisher& controller, const std::string& ports);
  
    /// destructor
    ~TrackerRazor()
    {
      if (_tracker != NULL) delete _tracker;
    }
  
  virtual void calibrate() { _az_corr = _current_azimuth + 90.0f; }

  private:
    /// constructor
    TrackerRazor(Publisher& controller, const std::string& ports);

    /// Razor AHRS callback functions
    void on_data(const float ypr[])
    {
      _current_azimuth = ypr[0];
      if (_init_az_corr)
      {
        calibrate();
        _init_az_corr = false;
      }
      _controller.set_reference_orientation(Orientation(-_current_azimuth + _az_corr));
    }
    void on_error(const string &msg) { ERROR("Razor AHRS: " << msg); }

    Publisher& _controller;
    volatile float _current_azimuth;
    volatile float _az_corr;
    volatile bool _init_az_corr;

    RazorAHRS* _tracker;
};

}  // namespace ssr

#endif // TRACKERRAZOR_H

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
