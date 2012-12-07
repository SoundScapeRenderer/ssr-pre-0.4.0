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
/// Audio player using ecasound (implementation).

#include <jack/jack.h> // for jack_client_name_size()
#include <algorithm>

#include "audioplayer.h"
#include "maptools.h"
#include "ssr_global.h"
#include "apf/stringtools.h"
#include "ptrtools.h"
#include "posixpathtools.h"

using ptrtools::is_null; using ptrtools::destroy; using maptools::get_item;

/// delete the file map.
AudioPlayer::~AudioPlayer()
{
  if (!_file_map.empty())
  {
    maptools::purge(_file_map);
    VERBOSE2("AudioPlayer dtor: file map deleted.");
  }
  else
  {
    VERBOSE2("AudioPlayer dtor.");
  }
}

/** _.
 * If the file is already opened, an existing instance of ecasound is used.
 * @param audio_file_name name of the audio file, what did you think?
 * @param channel select a channel of a multichannel file (starting with 1)
 * @param loop temporary solution for a loop mode
 * @return Name of the JACK port.
 * @warning If @a audio_file_name uses symbolic links or such things it can
 * happen that one file is opened several times.
 **/
std::string AudioPlayer::get_port_name(const std::string& audio_file_name,
    int channel, bool loop)
{
  assert(channel >= 0);

  const Soundfile* registered_file = get_item(_file_map, audio_file_name);
  if (registered_file != NULL)
  {
    VERBOSE2("AudioPlayer: Input file '" + audio_file_name
        + "' already registered.");
    if (channel > registered_file->get_channels())
    {
      ERROR("AudioPlayer: Channel " << channel << " doesn't exist in '"
          + audio_file_name + "'!");
      return "";
    }
  }
  else // file not yet registered
  {
    Soundfile::ptr_t temp = Soundfile::create(audio_file_name, loop);
    if (is_null(temp))
    {
      WARNING("AudioPlayer: Initialization of soundfile '" + audio_file_name
          + "' failed!");
      return "";
    }
    if (channel > temp->get_channels())
    {
      ERROR("AudioPlayer: Channel " << channel << " doesn't exist in '"
          + audio_file_name + "'!");
      // if wrong channel is requested, audiofile is not registered.
      return "";
    }
    registered_file = temp.get();
    _file_map[audio_file_name] = temp.release();
  }
  return registered_file->get_client_name() + ":"
    + registered_file->output_prefix + "_" + apf::str::A2S(channel);
}

/** _.
 * @param audio_file_name the audio file you want to know the length of.
 * @warning If the file wasn't loaded before, 0 is returned!
 **/
long int AudioPlayer::get_file_length(const std::string& audio_file_name) const
{
  const Soundfile* const file = get_item(_file_map, audio_file_name);
  return file ? file->get_length() : 0;
}

/** ctor.
 * @param filename name of soundfile
 * @param loop enable loop mode
 * @param prefix prefix used for channel names
 * @throw soundfile_error
 * @attention The soundfile must have the same sampling rate as the JACK server.
 * @warning If the name of the soundfile (including path) is longer than the
 * maximum allowed JACK client-name, it gets truncated. But if another file is
 * truncated to the same client-name, ecasound probably wants to add "_2" to
 * distinguish them but this will then be longer than the allowed length.
 * \par
 * UPDATE: This is already taken care of, but only up to "_99". So if you have
 * more than 99 equal client-names, you will get a problem. Hopefully, you
 * don't have that many ...
 **/
AudioPlayer::Soundfile::Soundfile(const std::string& filename, bool loop,
    const std::string& prefix) throw (soundfile_error) :
  output_prefix(prefix),
  _filename(filename),
  _escaped_filename(posixpathtools::get_escaped_filename(filename)),
  _client_name(""),
  _channels(0)
{
  // first we have to load the multichannel files to find out
  // correct number of channels in the chainsetup.
  // Only then can we make another chainsetup with the correct number of
  // channels.
  // If you know how to avoid this ugly workaround, please tell me!

  _eca.command("cs-add dummy_chainsetup_for_getting_number_of_channels");
  _eca.command("c-add dummy_chain");

  _eca.command("ao-add jack"); // with "null" this doesn't work.
  _eca.command("ai-add " + _escaped_filename);
  _eca.command("cs-connect");
  if (_eca.error())
  {
    throw soundfile_error("AudioPlayer::Soundfile: " + _eca.last_error());
  }
  // file is loaded just to get its format.
  _eca.command("ai-get-format");
  std::string input_format = _eca.last_string();
  _eca.command("cs-disconnect");
  _eca.command("c-remove");
  _eca.command("cs-remove");

  // we have to extract the number of channels of the input file:

  // parse input_format to find number of channels
  // (between commas: "bits,channels,samplerate")

  std::string str(input_format);                  // make a copy of input_format
  std::replace(str.begin(), str.end(), ',', ' '); // replace commas with spaces
  std::istringstream iss(str);                    // convert to string stream
  // save format, number of channels and sample rate in member variables
  iss >> _sample_format >> _channels >> _sample_rate;
  if (iss.fail())
  {
    throw soundfile_error("Couldn't convert string to integer "
        "for channel count");
  }
  assert(_sample_rate >= 1);
// end of ugly work-around. ////////////////////////////////////////////////////

  _eca.command("cs-add real_chainsetup");
  _eca.command("c-add real_chain");
  // apply the format of the input file to the chain setup
  _eca.command("cs-set-audio-format " + input_format);

  if (loop) _eca.command("ai-add audioloop," + _escaped_filename);
  else      _eca.command("ai-add "           + _escaped_filename);
  _eca.command("ao-add jack_generic," + this->output_prefix);

  // check if filename is too long for a JACK portname.
  // if yes, truncate filename (keep the end)
  int max_size = jack_client_name_size();  // #include <jack/jack.h>
#ifdef __APPLE__
  // TODO this is a workaround, and might not work on every system or with every version of jack.
  // on OS X jack_client_name_size() returns 64, but 52 seems to be the max supported size.
  //_client_name = "a123456789a123456789a123456789a123456789a123456789a"; // length = 51 + \0 = 52
  max_size = 52;
#endif
  max_size--; // max_size includes the terminating \0 character!
  max_size -= 3; // to allow ecasound to append a number up to "_99"
  max_size--; // we will add a special character at the beginning (maybe '['?)
  _client_name = _escaped_filename;
  assert(max_size >= 0);
  if (_escaped_filename.size() > static_cast<size_t>(max_size))
  {
    _client_name = _escaped_filename.substr(_escaped_filename.size() - max_size);
    // to visualize the truncation
    _client_name[0] = '<';
  }
  // to group the inputs in an alphabetic list of clients (e.g. in qjackctl)
  _client_name.insert(0,"["); // see max_size-- above!
  std::replace(_client_name.begin(), _client_name.end(), '/', '_');

  // set the name of the JACK client (using the truncated audio-filename)
  // do not send, only receive transport msg.
  // this must be done AFTER adding a chain
  _eca.command("-G:jack," + _client_name + ",recv");

  _eca.command("cs-connect");
  if (_eca.error())
  {
    throw soundfile_error("AudioPlayer::Soundfile: " + _eca.last_error());
  }

  // after cs-connect, get information about the input file:
  //_eca.command("ai-get-length");
  //_length = _eca.last_float();
  _eca.command("ai-get-length-samples");
  _length_samples = _eca.last_long_integer();

  _eca.command("engine-launch");
  if (_eca.error())
  {
    _eca.command("cs-disconnect");
    throw soundfile_error("AudioPlayer::Soundfile: " + _eca.last_error());
  }
  // It takes a little time until the client is available
  // This is a little ugly, but I don't know a better way to do it.
  // If you know one, tell me, please!
  usleep(ssr::usleeptime);
  VERBOSE2("Added '" + _escaped_filename + "', format: '" + input_format + "'.");
}

/// disconnects from ecasound.
AudioPlayer::Soundfile::~Soundfile()
{
  // TODO: check if ecasound is really running.
  _eca.command("cs-disconnect"); // implies "stop" and "engine-halt"
  VERBOSE2("AudioPlayer::Soundfile: '" + _escaped_filename + "' disconnected.");
}

AudioPlayer::Soundfile::ptr_t AudioPlayer::Soundfile::create(
    const std::string& filename, bool loop)
{
  ptr_t temp; // temp = NULL
  try
  {
    temp.reset(new Soundfile(filename, loop));
  }
  catch(soundfile_error& e)
  {
    ERROR(e.what());
  }
  return temp;
}

int AudioPlayer::Soundfile::get_channels() const
{
  return _channels;
}

std::string AudioPlayer::Soundfile::get_client_name() const
{
  return _client_name;
}

long int AudioPlayer::Soundfile::get_length() const
{
  return _length_samples;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
