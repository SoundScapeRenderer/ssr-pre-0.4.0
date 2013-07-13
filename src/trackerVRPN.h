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
 * Polhemus tracker (definition).
 *
 * $LastChangedDate: 2012-02-08 20:27:32 +0100 (Wed, 08 Feb 2012) $
 * $LastChangedRevision: 1680 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef TRACKERVRPN_H
#define TRACKERVRPN_H

#include <pthread.h>
#include <string>
#include <memory>
#include <stdexcept> // for std::runtime_error

#include "tracker.h"
#include "vrpn/Header/vrpn_Shared.h"
#include "vrpn/Header/vrpn_Tracker.h"
#include "vrpn/Header/vrpn_Button.h"
#include "vrpn/Header/vrpn_Analog.h"
#include "vrpn/Header/vrpn_Dial.h"
#include "vrpn/Header/vrpn_Text.h"
#include "vrpn/Header/vrpn_FileConnection.h"
class Controller; // forward declaration

/// Polhemus Fastrack tracker 
class TrackerVRPN : public Tracker
{
  public:
    typedef std::auto_ptr<TrackerVRPN> ptr_t;

    virtual ~TrackerVRPN(); ///< destructor

    /// "named constructor"
    static ptr_t create(Controller& controller, const std::string& portsz);

    virtual void calibrate();
    void setValue(double azi_vrpn);

    void getAdress();

  private:
    /// constructor
    TrackerVRPN(Controller& controller, const std::string& ports);

    struct tracker_data_t
    {
      float header;
      float x;
      float y;
      float z;
      float azimuth;
      float elevation;
      float roll;

      // contructor
      tracker_data_t()
        : header(0.0f), x(0.0f), y(0.0f), z(0.0f)
        , azimuth(0.0f), elevation(0.0f), roll(0.0f)
      {}
    };

    Controller& _controller;

    tracker_data_t _current_data;

    int _tracker_port;
    int _open_serial_port(const char *portname);
  
    volatile bool _stopped; ///< stops the tracking thread

    float _az_corr; ///< correction of the azimuth due to calibration
    void _start(); ///< start the tracking thread
    void _stop();  ///< stop the tracking thread
    // thread related stuff
    pthread_t _thread_id;
    static void* _thread(void*);
    void* thread(void*);
};

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
