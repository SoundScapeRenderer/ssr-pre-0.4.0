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
 * Command line parsing, and config-file related stuff (implementation).
 *
 * $LastChangedDate: 2012-11-14 15:47:54 +0100 (Mit, 14. Nov 2012) $
 * $LastChangedRevision: 1986 $
 * $LastChangedBy: geier.matthias $
 **/

#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*, HAVE_*, WITH_*
#endif

#include <cassert>      // for assert()
#include <getopt.h>     // for getopt_long()
#include <cstdlib>      // for getenv(), ...
#include <cstring>
#include <stdio.h>
#include "configuration.h"
#include "posixpathtools.h"
#include "apf/stringtools.h"
#include "ssr_global.h" // for ssr::verbose, WARNING(), ...
#include "xmlparser.h"  // TODO: move this somewhere else

using posixpathtools::make_path_relative_to_current_dir;
using apf::str::S2A;

// only needed in this file
enum CONFIG_ERRORS {CONFIG_SUCCESS, CONFIG_NOVALUE, CONFIG_EMPTY, CONFIG_NOQUOTE, CONFIG_QUOTE};

const int BUF_SIZE = 4096;

namespace // anonymous
{
  /// show version and compiled-in features
  void print_version_details()
  {
    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
      " It's ... " << std::flush;
    sleep(3);
    std::cout <<
      "the " PACKAGE_NAME
#ifndef NDEBUG
      ", compiled in debug mode!"
#endif
      "\n              version " PACKAGE_VERSION ", aka \"" NICKNAME "\""
      "\n Following compile-time features are activated:\n"
      " new renderer"
#ifdef ENABLE_GUI
      ", GUI"
#endif
#ifdef ENABLE_IP_INTERFACE
      ", IP interface"
#endif
#ifdef ENABLE_INTERSENSE
      ", InterSense "
#ifdef HAVE_INTERSENSE_404
      "(>= v4.04)"
#else
      "(< v4.04)"
#endif
#endif // ENABLE_INTERSENSE
#ifdef ENABLE_POLHEMUS
      ", Polhemus"
#endif
#ifdef ENABLE_RAZOR
      ", Razor AHRS"
#endif
#ifdef ENABLE_ECASOUND
      ", Ecasound"
#endif
      "\n\n"
      SSR_AUTHORS
      "\n\n";
  }
}

/** parse command line options and configuration file(s)
 * @param argc number of command line arguments.
 * @param argv the arguments themselves.
 * @param conf structure with the configuration options
 * @return @b true if main program shall continue (It shall exit if e.g. only
 * version information or the help screen was shown).
 **/
ssr::conf_struct ssr::configuration(int& argc, char* argv[])
{
  // TODO: Move this somewhere else? Now it's used for reproduction setup and
  // scene, mabe make two separate instances?
  XMLParser::Init();

  conf_struct conf;

  // borrowed from ardour sources: cut path from program name
  const char *exec_name = strrchr(argv[0], '/');
  if (exec_name == NULL) exec_name = argv[0]; else exec_name++;
  conf.exec_name = exec_name;

#ifndef NDEBUG
  // Because of this warning, "make check" fails for debug builds (on purpose).
  WARNING(conf.exec_name << " was compiled for debugging!");
#endif

  // hard coded default values:
  conf.stand_ampl_ref_dist = 3.0f; // meters
#ifdef ENABLE_GUI
  conf.gui = true;
#else
  conf.gui = false;
#endif
#ifdef ENABLE_IP_INTERFACE
  conf.ip_server = true;
#else
  conf.ip_server = false;
#endif
  conf.server_port = 4711;

  conf.freewheeling = false;
  conf.scene_file_name = "";
  conf.renderer_params.set("reproduction_setup"
      , SSR_DATA_DIR"/default_setup.asd");
  conf.xml_schema = SSR_DATA_DIR"/asdf.xsd";
  conf.audio_recorder_file_name = ""; // default: no recording
  conf.renderer_params.set("threads", 1);  // TODO: obtain reasonable default

  conf.input_port_prefix = "system:capture_";
  conf.output_port_prefix = "system:playback_";
  conf.path_to_gui_images = SSR_DATA_DIR"/images";
  conf.path_to_scene_menu = "./scene_menu.conf";

  // for WFS renderer
  conf.renderer_params.set("prefilter_file"
      , SSR_DATA_DIR"/default_wfs_prefilter.wav");
  conf.renderer_params.set("delayline_size", 100000); // in samples
  conf.renderer_params.set("initial_delay", 1000);    // in samples

  // for binaural renderer
  conf.renderer_params.set("hrir_size", 0); // "0" means use all that are there
  conf.renderer_params.set("hrir_file", SSR_DATA_DIR"/default_hrirs.wav");

  // for AAP renderer
  conf.ambisonics_order = 0; // "0" means use maximum that makes sense
  conf.in_phase_rendering = false;

#if defined(ENABLE_POLHEMUS)
  conf.tracker = "polhemus";
#elif defined(ENABLE_INTERSENSE)
  conf.tracker = "intersense";
#elif defined(ENABLE_RAZOR)
  conf.tracker = "razor";
#else
  conf.tracker = "";
#endif
  // USB ports have to be checked first!
  conf.tracker_ports = "/dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyS0 /dev/tty.usbserial-00001004 /dev/tty.usbserial-00002006";

  conf.loop = false; // temporary solution!

  // load system-wide config file (Mac)
  load_config_file("/Library/SoundScapeRenderer/ssr.conf",conf);
  // load system-wide config file (Linux et al)
  load_config_file("/etc/ssr.conf",conf);
  // load user config file (Mac)
  std::string filename = getenv("HOME");
  filename += "/Library/SoundScapeRenderer/ssr.conf";
  load_config_file(filename.c_str(),conf);
  // load user config file (Linux et al.)
  filename = getenv("HOME");
  filename += "/.ssr/ssr.conf";
  load_config_file(filename.c_str(),conf);

  const std::string usage_string =
"\nUSAGE: " + conf.exec_name + " [OPTIONS] <scene-file>"
#ifdef ENABLE_GUI
//" [-- <GUI options>]"
#endif
"\n";

  const std::string help_string =
"\nThe SoundScape Renderer (SSR) is a tool for real-time "
"spatial audio reproduction\n"
"providing a variety of rendering algorithms.\n"
"\n"
"OPTIONS:\n"
"\n"
"Renderer-specific options:\n"
"    --hrirs=FILE       Load the HRIRs for binaural renderer from FILE\n"
"    --hrir-size=VALUE  Maximum IR length (binaural and BRS renderer)\n"
"    --prefilter=FILE   Load WFS prefilter from FILE\n"
"-o, --ambisonics-order=VALUE Ambisonics order to use (default: maximum)\n"
"    --in-phase-rendering     Use in-phase rendering for Ambisonics\n"
"\n"
"JACK options:\n"
"-n, --name=NAME        Set JACK client name to NAME\n"
"    --input-prefix=PREFIX    Input  port prefix "
                                              "(default: \"system:capture_\")\n"
"    --output-prefix=PREFIX   Output port prefix "
                                             "(default: \"system:playback_\")\n"
"-f, --freewheel        Use JACK in freewheeling mode\n"
"\n"
"General options:\n"
"-c, --config=FILE      Read configuration from FILE\n"
"-s, --setup=FILE       Load reproduction setup from FILE\n"
"    --threads=N        Number of audio threads (default N=1)\n"
"-r, --record=FILE      Record the audio output of the renderer to FILE\n"
#ifndef ENABLE_ECASOUND
"                       (disabled at compile time!)\n"
#endif
// TODO: --loop is a temporary option, should rather be done in scene file
"    --loop             Loop all audio files\n"
"    --master-volume-correction=VALUE\n"
"                       Correction of the master volume in dB "
                                                         "(default: 0 dB)\n"
#ifdef ENABLE_IP_INTERFACE
"-i, --ip-server[=PORT] Start IP server (default on)\n"
"                       A port can be specified: --ip-server=5555\n"
"-I, --no-ip-server     Don't start IP server\n"
#else
"-i, --ip-server        Start IP server (not enabled at compile time!)\n"
"-I, --no-ip-server     Don't start IP server (default)\n"
#endif
#ifdef ENABLE_GUI       
"-g, --gui              Start GUI (default)\n"
"-G, --no-gui           Don't start GUI\n"
#else
"-g, --gui              Start GUI (not enabled at compile time!)\n"
"-G, --no-gui           Don't start GUI (default)\n"
#endif
#if defined(ENABLE_INTERSENSE) || defined(ENABLE_POLHEMUS) || defined(ENABLE_RAZOR)
"-t, --tracker=TYPE     Start tracker, possible value(s):"
#if defined(ENABLE_POLHEMUS)
" polhemus"
#endif
#if defined(ENABLE_INTERSENSE)
" intersense"
#endif
#if defined(ENABLE_RAZOR)
" razor"
#endif
"\n"
"    --tracker-port=PORT\n"
"                       A serial port can be specified, e.g. /dev/ttyS1\n"
"-T, --no-tracker       Don't start tracker\n"
#else
"-t, --tracker          Start tracker (not enabled at compile time!)\n"
"-T, --no-tracker       Don't start tracker (default)\n"
#endif
"\n"
"-h, --help             Show this very help information. "
                                                     "You just typed that!\n"
"-v, --verbose          Increase verbosity level (up to -vvv)\n"
"-V, --version          Show version information and exit\n"
"\n"

#ifdef ENABLE_GUI
//"\n"
//" --               All arguments after \"--\" are passed on to the GUI.\n"
#else
//"\n"
//" --               All arguments after \"--\" are ignored.\n"
#endif
;

  // the special argument "--" forces the end of option scanning

  const struct option longopts[] =
  {
    {"hrirs",        required_argument, NULL,  0 },
    {"hrir-size",    required_argument, NULL,  0 },
    {"prefilter",    required_argument, NULL,  0 },
    {"ambisonics-order",required_argument,NULL,'o'},
    {"in-phase-rendering", no_argument, NULL,  0 },

    {"name",         required_argument, NULL, 'n'},
    {"input-prefix", required_argument, NULL,  0 },
    {"output-prefix",required_argument, NULL,  0 },
    {"freewheel",    no_argument,       NULL, 'f'},

    {"config",       required_argument, NULL, 'c'},
    {"setup",        required_argument, NULL, 's'},
    {"threads",      required_argument, NULL,  0 },
    {"record",       required_argument, NULL, 'r'},
    {"loop",         no_argument,       NULL,  0 },
    {"master-volume-correction", required_argument, NULL, 0},
    {"ip-server",    optional_argument, NULL, 'i'},
    {"no-ip-server", no_argument,       NULL, 'I'},
    {"gui",          no_argument,       NULL, 'g'},
    {"no-gui",       no_argument,       NULL, 'G'},
    {"tracker",      required_argument, NULL, 't'},
    {"no-tracker",   no_argument,       NULL, 'T'},
    {"tracker-port", required_argument, NULL,  0 },

    {"help",         no_argument,       NULL, 'h'},
    {"verbose",      no_argument,       NULL, 'v'},
    {"version",      no_argument,       NULL, 'V'},

    // for some obscure reasons, the last element has to be filled with zeroes
    {0, 0, 0, 0}
  };
  // one colon: required argument; two colons: optional argument
  // if first character is '-', non-option arguments return 1 (see case 1 below)
  const char *optstring = "-c:fgGhi::In:o:r:s:t:TvV?";

  int opt;
  int longindex = 0;

  int non_option_parameters = 0;

  while ((opt = getopt_long(argc, argv, optstring, longopts, &longindex))
    != -1)
  {
    switch (opt)
    {
      case 0:
        // long option without a short arg

        if (strcmp("hrirs", longopts[longindex].name) == 0)
        {
          conf.renderer_params.set("hrir_file", optarg);
        }
        else if (strcmp("hrir-size", longopts[longindex].name) == 0)
        {
          conf.renderer_params.set("hrir_size", optarg);
          assert(conf.renderer_params.get("hrir_size", 0) >= 1);
        }
        else if (strcmp("prefilter", longopts[longindex].name) == 0)
        {
          conf.renderer_params.set("prefilter_file", optarg);
        }
        else if (strcmp("in-phase-rendering", longopts[longindex].name) == 0)
        {
          conf.in_phase_rendering = true;
        }
        else if (strcmp("input-prefix", longopts[longindex].name) == 0)
        {
          conf.input_port_prefix = optarg;
        }
        else if (strcmp("output-prefix", longopts[longindex].name) == 0)
        {
          conf.output_port_prefix = optarg;
        }
        else if (strcmp("threads", longopts[longindex].name) == 0)
        {
          conf.renderer_params.set("threads", optarg);
        }
        else if (strcmp("loop", longopts[longindex].name) == 0)
        {
          conf.loop = true;
        }
        else if (strcmp("master-volume-correction",longopts[longindex].name)==0)
        {
          conf.renderer_params.set("master_volume_correction", optarg);
        }
        else if (strcmp("tracker-port", longopts[longindex].name) == 0)
        {
          conf.tracker_ports = optarg;
        }
        break;

      case 1:
        // if first character of optstring is '-',
        // non-option parameters (e.g. filenames) arrive here.
        non_option_parameters++;
        if (non_option_parameters == 1)
        {
          conf.scene_file_name = optarg;
        }
        else
        {
          ERROR("For now, only one non-option parameter "
              "(= scene file) is allowed!");
          WARNING("Ignoring '" << optarg << "'.");
        }
        break;

      case 'c':
        if (load_config_file(optarg, conf) == EXIT_FAILURE)
        {
          throw std::logic_error("Couldn't load config file \""
              + std::string(optarg) + "\"!");
        }
        break;

      case 'f':
        conf.freewheeling = true;
        break;

      case 'g':
        // enable GUI
        conf.gui = true;
        break;

      case 'G':
        conf.gui = false;
        break;

      case 'h':
        // show help message
        std::cout << usage_string << help_string;
        exit(EXIT_SUCCESS);
        break;

      case 'i':
        conf.ip_server = true;
#ifdef ENABLE_IP_INTERFACE
        if (optarg)
        {
          if (!S2A(optarg, conf.server_port))
          {
            ERROR("Invalid server port specified!");
          }
        }
#endif
        break;

      case 'I':
        conf.ip_server = false;
        break;

      case 'n':
        conf.renderer_params.set("name", optarg);
        break;

      case 'o':
        conf.ambisonics_order = atoi(optarg);
        break;

      case 'r':
        conf.audio_recorder_file_name = optarg;
        break;

      case 's':
        conf.renderer_params.set("reproduction_setup", optarg);
        break;
      case 't':
        conf.tracker = optarg;
        break;
      case 'T':
        conf.tracker = "";
        break;
      case 'v':
        // increase verbosity
        ssr::verbose++;
        break;

      case 'V':
        print_version_details();
        exit(EXIT_SUCCESS);
        break;

      case '?':
        // here we deal with unknown/invalid options
        // getopt() already prints an error message for unknown options
        std::cout << usage_string;
        std::cout << "Type '" << conf.exec_name << " --help' "
          "for more information.\n\n";
        exit(EXIT_FAILURE);
        break;

      default:
        // should not happen?
        throw std::logic_error("BUG: parsing input arguments.");
    }
  }

  // remove arguments up to (and including) '--', but leave argv[0].
  // "optind" is set to the index of the next argument by getopt_long()
  for (int i = optind; i < argc; ++i)
  {
    argv[1 + i - optind] = argv[i];
  }
  argc = 1 + argc - optind;

  conf.renderer_params.set("xml_schema", conf.xml_schema);

  if (!conf.freewheeling)
  {
    conf.renderer_params.set("system_output_prefix", conf.output_port_prefix);
  }

  return conf;
}

/******************************************************************************/

/* this function determines whether a line is empty or a comment */
static int is_comment_or_empty(const char *line){
  while (*line && isspace(*line))
    line++;

  return (*line == '#') || (!*line);
}

/******************************************************************************/

/* This function takes a line from the configuration file and splits
 * it into a key- and value-pair. If a problem occures, the function
 * returns a non-zero value and the user will be scolded.
 * NOTE: no buffer overflows in the while-loops because key and
 * value are as big as line is.
 */
static int parse(const char *line, char *key, char *value)
{
  /* is the value-string quoted? */
  int quoted = 0;

  /* skip leading whitespaces */
  while (*line && isspace(*line))
    line++;

  /* read the key which is a single word, so stop when
   * whitespaces or the = occurs
   */
  while (*line && !isspace(*line) && *line != '=')
    *(key++) = *(line++);

  /* null-terminate the key-string */
  *key = '\0';

  /* skip whitespaces between key and = */
  while (*line && isspace(*line))
    line++;

  /* huh, no value? */
  if (!*line || *line != '=')
    return CONFIG_NOVALUE;

  /* skip whitespaces and the = */
  while (*line && (isspace(*line) || *line == '='))
    line++;

  /* empty value but no quotes -> warning */
  if (!*line)
    return CONFIG_EMPTY;

  /* if the value-string is quoted, skip the " */
  if (*line && *line == '"')
  {
    quoted = 1;
    line++;
  }

  /* read the value until end of line, newline, carriage return
   * or a terminating "
   */
  while (*line && *line != '"' && *line != '\r' && *line != '\n')
    *(value++) = *(line++);

  /* null-terminate the value-string */
  *value = '\0';

  /* quoted value but no end of quote? */
  if (quoted && *line != '"')
    return CONFIG_NOQUOTE;

  /* an ending quote without beginning? */
  if (!quoted && *line == '"')
    return CONFIG_QUOTE;

  return CONFIG_SUCCESS;
}
/******************************************************************************/

/* Read the .conf file with Server Settings */
int ssr::load_config_file(const char *filename, conf_struct& conf){

  FILE *file;
  char line[BUF_SIZE], key[BUF_SIZE], value[BUF_SIZE];

  char line_number=0;

  if ((file = fopen(filename,"r"))==NULL) {
    if (verbose) fprintf(stderr,"Cannot open %s !\n",filename);
    return (EXIT_FAILURE); //need to create one?
  }

  /* we can read the file */
  while (fgets(line,sizeof(line),file)){
    line_number++;

    /* skip comments and empty lines */
    if (is_comment_or_empty(line))continue;

    /* parse line and test for errors */
    switch(parse(line, key, value))
    {
      case CONFIG_SUCCESS:
        break;

      /* no value specified: error */
      case CONFIG_NOVALUE:
        fprintf(stderr, "%s:%u: no value found\n",filename, line_number);
        return EXIT_FAILURE;

      /* quoted string not finished: error */
      case CONFIG_NOQUOTE:
        fprintf(stderr, "%s:%u: no end of quote found\n",filename, line_number);
        return EXIT_FAILURE;

      /* ending quote without starting a quoted string: error */
      case CONFIG_QUOTE:
        fprintf(stderr, "%s:%u: end of quote found, but no start\n", filename, line_number);
        return EXIT_FAILURE;

      /* empty value not quoted: warning */
      case CONFIG_EMPTY:
        fprintf(stderr, "%s:%u: suggest quotes around empty values\n", filename, line_number);
        break;
    }//switch

    VERBOSE2(key << " = " << value);

    if (!strcmp(key, "NAME"))
    {
      conf.renderer_params.set("name", value);
    }
    else if (!strcmp(key, "NUMBER_OF_THREADS"))
    {
      conf.renderer_params.set("threads", value);
    }
    else if (!strcmp(key, "MASTER_VOLUME_CORRECTION"))
    {
      conf.renderer_params.set("master_volume_correction", value);
    }
    else if (!strcmp(key, "STANDARD_AMPLITUDE_REFERENCE_DISTANCE"))
    {
      conf.stand_ampl_ref_dist = atof(value);
    }
    else if (!strcmp(key, "SCENE_FILE_NAME"))
    {
      conf.scene_file_name = make_path_relative_to_current_dir(value, filename);
    }
    else if (!strcmp(key, "SCENE_MENU"))
    {
      conf.path_to_scene_menu
        = make_path_relative_to_current_dir(value, filename);
    }
    else if (!strcmp(key, "PLAYBACK_SETUP_FILE_NAME"))
    {
      conf.renderer_params.set("reproduction_setup"
          , make_path_relative_to_current_dir(value, filename));
    }
    else if (!strcmp(key, "XML_SCHEMA_FILE_NAME"))
    {
      conf.xml_schema = make_path_relative_to_current_dir(value, filename);
    }
    else if (!strcmp(key, "AUDIO_RECORDER_FILE_NAME"))
    {
      conf.audio_recorder_file_name
        = make_path_relative_to_current_dir(value, filename);
    }
    else if (!strcmp(key, "RENDERER_TYPE"))
    {
      WARNING("\"RENDERER_TYPE\" is deprecated, don't use it anymore!");
    }
    else if (!strcmp(key, "WFS_PREFILTER"))
    {
      conf.renderer_params.set("prefilter_file"
          , make_path_relative_to_current_dir(value, filename));
    }
    else if (!strcmp(key, "DELAYLINE_SIZE"))
    {
      conf.renderer_params.set("delayline_size", value);
      assert(conf.renderer_params.get<int>("delayline_size") >= 0);
    }
    else if (!strcmp(key, "INITIAL_DELAY"))
    {
      conf.renderer_params.set("initial_delay", value);
      assert(conf.renderer_params.get<int>("initial_delay") >= 0);
    }
    else if (!strcmp(key, "HRIR_FILE_NAME"))
    {
      conf.renderer_params.set("hrir_file"
          , make_path_relative_to_current_dir(value, filename));
    }
    else if (!strcmp(key, "HRIR_SIZE"))
    {
      conf.renderer_params.set("hrir_size", value);
      assert(conf.renderer_params.get("hrir_size", 0) >= 1);
    }
    else if (!strcmp(key, "AMBISONICS_ORDER"))
    {
      conf.ambisonics_order = atoi(value);
    }
    else if (!strcmp(key, "IN_PHASE_RENDERING"))
    {
      if (!strcasecmp(value,"TRUE")) conf.in_phase_rendering = true;
      else if (!strcasecmp(value,"true")) conf.in_phase_rendering = true;
      else if (!strcasecmp(value,"FALSE")) conf.in_phase_rendering = false;
      else if (!strcasecmp(value,"false")) conf.in_phase_rendering = false;
      else ERROR("I don't understand the option '" << value
          << "' for in-phase rendering.");
    }
    else if (!strcmp(key, "INPUT_PREFIX"))
    {
      conf.input_port_prefix = value;
    }
    else if (!strcmp(key, "OUTPUT_PREFIX"))
    {
      conf.output_port_prefix = value;
    }
    else if (!strcmp(key, "PATH_TO_GUI_IMAGES"))
    {
      conf.path_to_gui_images
        = make_path_relative_to_current_dir(value, filename);
    }
    else if (!strcmp(key, "FREEWHEEL"))
    {
      if (!strcasecmp(value, "yes")) conf.freewheeling = true;
      else conf.freewheeling = false;
    }
    else if (!strcmp(key, "GUI"))
    {
      if (!strcasecmp(value, "on")) conf.gui = true;
      else conf.gui= false;
    }
    else if (!strcmp(key, "NETWORK_INTERFACE"))
    {
      #ifdef ENABLE_IP_INTERFACE
      if (!strcasecmp(value, "on")) conf.ip_server= true;
      else conf.ip_server= false;
      #endif
    }
    else if (!strcmp(key, "SERVER_PORT"))
    {
      #ifdef ENABLE_IP_INTERFACE
      conf.server_port = atoi(value);
      #endif
    }
    else if (!strcmp(key, "VERBOSE"))
    {
      ssr::verbose = atoi(value);
    }
    else if (!strcmp(key, "TRACKER"))
    {
      conf.tracker = value;
    }
    else if (!strcmp(key, "TRACKER_PORTS"))
    {
      conf.tracker_ports = value;
    }
    else
    {
      printf("%s:%u unknown option \"%s\"\n",filename, line_number, key);
      return EXIT_FAILURE;
    }
  }//while
  return EXIT_SUCCESS;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
