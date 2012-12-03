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
 * InterSense tracker (definition).
 *
 * $LastChangedDate: 2012-07-09 21:37:27 +0200 (Mon, 09. Jul 2012) $
 * $LastChangedRevision: 1886 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef TRACKERINTERSENSE_H
#define TRACKERINTERSENSE_H

#include <pthread.h>
#include <isense.h>
#include <string>
#include <memory>
#include <stdexcept> // for std::runtime_error

#include "tracker.h"

namespace ssr
{

class Publisher;

/// Intersense InertiaCube3 head tracker 
class TrackerInterSense : public Tracker
{
  public:
    typedef std::auto_ptr<TrackerInterSense> ptr_t;

    virtual ~TrackerInterSense(); ///< destructor

    /// "named constructor"
  static ptr_t create(Publisher& controller, const std::string& ports = "",
        const unsigned int read_interval = 20);

    virtual void calibrate();

  private:
    /// constructor
    TrackerInterSense(Publisher& controller, const std::string& ports, const unsigned int read_interval);

    Publisher& _controller;

    /// interval in ms to wait after each read cycle
    unsigned int _read_interval;

    bool _stopped; ///< stops the tracking thread

    ISD_TRACKER_HANDLE _tracker_h; ///< tracker handle

    void _start(); ///< start the tracking thread
    void _stop();  ///< stop the tracking thread

    // thread related stuff
    pthread_t _thread_id;
    static void* _thread(void*);
    void* thread(void*);
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
