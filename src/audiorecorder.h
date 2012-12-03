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
 * Audio recorder using ecasound (definition).
 *
 * $LastChangedDate: 2012-08-14 22:33:23 +0200 (Die, 14. Aug 2012) $
 * $LastChangedRevision: 1925 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <eca-control-interface.h>
#include <string>
#include <memory>
#include <stdexcept> // for std::runtime_error

/**
 * Writes a (multichannel) soundfile using ecasound and JACK transport.
 **/
class AudioRecorder
{
  public:
    typedef std::auto_ptr<AudioRecorder> ptr_t; ///< auto_ptr to AudioRecorder

    /// exception to be thrown by ctor.
    struct audiorecorder_error : public std::runtime_error
    {
      audiorecorder_error(const std::string& s): std::runtime_error(s) {}
    };

    ~AudioRecorder();

    AudioRecorder(const std::string& audio_file_name,
        const std::string& format_string, const std::string& record_source,
        const std::string& client_name = "recorder",
        const std::string& input_prefix = "channel") throw(audiorecorder_error);

    bool enable();   ///< enable recording as soon as transport is started
    bool disable();  ///< disable recording

    const std::string client_name;   ///< name of JACK client used by ecasound
    const std::string input_prefix;  ///< prefix used for channels

  private:
    ECA_CONTROL_INTERFACE _eca; ///< interface to ecasound
};

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
