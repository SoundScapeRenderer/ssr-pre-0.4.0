/******************************************************************************
 * Copyright © 2012      Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
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
 * http://SoundScapeRenderer.github.com                  ssr@spatialaudio.net *
 ******************************************************************************/

/// @file
/// Audio player using ecasound (definition).

#ifndef SSR_AUDIOPLAYER_H
#define SSR_AUDIOPLAYER_H

#include <eca-control-interface.h>
#include <map>
#include <string>
#include <stdexcept> // for std::runtime_error
#include <memory>

#include "apf/misc.h"  // for NonCopyable

/** Loads audiofiles for playback using ecasound (with JACK transport).
 * For each (multichannel) soundfile an ecasound instance is opened.
 **/
class AudioPlayer : apf::NonCopyable
{
  public:
    typedef std::auto_ptr<AudioPlayer> ptr_t; ///< auto_ptr to AudioPlayer

    // using default ctor.

    virtual ~AudioPlayer();

    /// Open audio file with ecasound and return corresponding JACK port.
    std::string get_port_name(const std::string& audio_file_name,
        int channel, bool loop);
    /// get length (in samples) of given audio file.
    long int get_file_length(const std::string& audio_file_name) const;

  private:
    class Soundfile; // nested class

    /// map of Soundfiles, indexed by strings.
    typedef std::map<std::string, Soundfile*> soundfile_map_t;
    /// map to associate an ecasound instance with a filename
    soundfile_map_t _file_map;
};

/** Plays a single (possibly multichannel) audio file.
 * Playback can be started using JACK transport.
 **/
class AudioPlayer::Soundfile : apf::NonCopyable
{
  public:
    typedef std::auto_ptr<Soundfile> ptr_t; ///< auto_ptr to Soundfile

    /// exception to be thrown by ctor.
    struct soundfile_error : public std::runtime_error
    {
      soundfile_error(const std::string& s): std::runtime_error(s) {}
    };

    Soundfile(const std::string& filename, bool loop,
        const std::string& prefix = "channel") throw (soundfile_error);

    /// "named constructor" for %Soundfile objects
    static ptr_t create(const std::string& filename, bool loop = false);

    ~Soundfile();

    int get_channels() const;            ///< get number of channels
    std::string get_client_name() const; ///< get name of JACK client
    long int get_length() const;         ///< get length (in samples)

    /// output prefix used for %Soundfile channels
    const std::string output_prefix;

  private:
    ECA_CONTROL_INTERFACE _eca;           ///< interface to the ecasound library
    const std::string _filename;          ///< name of input sound file
    const std::string _escaped_filename;  ///< name of input sound file with escaped white spaces
    std::string _client_name;             ///< name of JACK client used by ecasound
    std::string _sample_format;           ///< format of input sound file (e.g. s16_le)
    int _channels;                        ///< number of channels in input sound file
    int _sample_rate;                     ///< sample rate of input sound file
    long int _length_samples;             ///< length of soundfile (in samples)

    std::string _get_escaped_filename(const std::string& filename);
};

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
