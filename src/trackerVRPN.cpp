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
 * VRPN tracker (implementation).
 *
 * $LastChangedDate: 2012-02-08 20:27:32 +0100 (Wed, 08 Feb 2012) $
 * $LastChangedRevision: 1680 $
 * $LastChangedBy: geier.matthias $
 **/


#include <unistd.h>  // for write(), fsync(), close(), ...
#include <termios.h> // for cfsetispeed(), ...
#include <fcntl.h>   // for open(), ...
#include <sstream>   // for std::stringstream
#include <poll.h>    // for poll(), pollfd, ...
#include <iostream>
#include <fstream>
#include <string>

#include "trackerVRPN.h"
#include "controller.h"
#include "ssr_global.h"
#include "ptrtools.h"
#include "stringtools.h"



//#include <libquat>
 // For preload and accumulate settings

using namespace std;

using stringtools::A2S;
#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif
#ifndef _REENTRANT
#error You need to compile with _REENTRANT defined since this uses threads!
#endif
unsigned tracker_stride = 1;
double azi_vrpn_cor = 0.0;

class device_info {
    public:
	char		    *name;
	vrpn_Tracker_Remote *tkr;
	vrpn_Button_Remote  *btn;
	vrpn_Analog_Remote  *ana;
	vrpn_Dial_Remote    *dial;
        vrpn_Text_Receiver  *text;
};
device_info *dev;
TrackerVRPN *tracky;
double value = 0.0f;
string adress = "";


class t_user_callback {
    public:
	char		    t_name[vrpn_MAX_TEXT_LEN];
        vector<unsigned>    t_counts;
};

void	VRPN_CALLBACK handle_tracker_pos_quat_vrpn (void *userdata, const vrpn_TRACKERCB t)
{
	t_user_callback	*t_data = (t_user_callback *)userdata;
        // Make sure we have a count value for this sensor
        while (t_data->t_counts.size() <= static_cast<unsigned>(t.sensor)) {
          t_data->t_counts.push_back(0);

        }

	// See if we have gotten enough reports from this sensor that we should
	// print this one.  If so, print and reset the count.
	if ( ++t_data->t_counts[t.sensor] >= tracker_stride ) {
		t_data->t_counts[t.sensor] = 0;
		// get quaternions information
		double w = t.quat[0];
		double x = t.quat[1];
		double y = t.quat[2];
		double z = t.quat[3];

		//calculate yaw(azimut) from quaternions
		double azi_vrpn = atan2(2*(w*x+y*z),1-2*(x*x+y*y))*(180/M_PI);



		//parse information to controller
		tracky->setValue(azi_vrpn);


	}
}

const unsigned MAX_DEVICES = 50;


TrackerVRPN::TrackerVRPN(Controller& controller
    , const std::string& ports)
  : Tracker()
  , _controller(controller)
  , _stopped(false)
  , _az_corr(90.0f)
  , _thread_id(0)
{
//	ERROR("VRPN Tracker chosen!");
	device_info device_list[1];
	dev =  &device_list[0];
	// gets adress from vrpn.cfg
	this->getAdress();
	tracky = this;
        ERROR(adress);

	dev->name = const_cast<char*>(adress.c_str());
	dev->tkr = new vrpn_Tracker_Remote(adress.c_str());
	vrpn_Tracker_Remote *tkr = dev->tkr;
	// set Updarte Rate (120)
	dev->tkr->set_update_rate(120);


	t_user_callback *tc1 = new t_user_callback;

    // Fill in the user-data callback information
    strncpy(tc1->t_name, dev->name, sizeof(tc1->t_name));
    tkr->register_change_handler(tc1, handle_tracker_pos_quat_vrpn);



  _start();
  // wait until tracker has started
  usleep(50000);

  this->calibrate();
}

TrackerVRPN::~TrackerVRPN()
{
  // if thread was started
  if (_thread_id) _stop();
  close(_tracker_port);
}

TrackerVRPN::ptr_t
TrackerVRPN::create(Controller& controller, const std::string& ports)
{

  ptr_t temp; // temp = NULL
  try
  {
    temp.reset(new TrackerVRPN(controller, ports));
  }
  catch(std::runtime_error& e)
  {

    ERROR(e.what());

  }
  return temp;
}

int
TrackerVRPN::_open_serial_port(const char *portname)
{
  int port;
  int flags;
  
  // O_NDELAY allows open even with no carrier detect (e.g. needed for Razor)
  if ((port = open(portname, O_RDWR | O_NOCTTY | O_NDELAY)) != -1)
  {
    // clear O_NDELAY to make I/O blocking again
    if (((flags = fcntl(port, F_GETFL, 0)) != -1) &&
        (fcntl(port, F_SETFL, flags & ~O_NDELAY)) != -1)
    {
      return port;
    }
  }
  
  // something didn't work
  close(port);
  return -1;
}

void
TrackerVRPN::calibrate()
{

  _az_corr = _current_data.azimuth + 90.0f;
}

void
TrackerVRPN::_start()
{
  // create thread
	 ERROR("pre tread");
  pthread_create(&_thread_id , NULL, _thread, this);
  VERBOSE("Starting tracker ...");
}

void
TrackerVRPN::_stop()
{
  // dummy
	ERROR("stoping tread");
  void *thread_exit_status;

  _stopped = true;
  pthread_join(_thread_id , &thread_exit_status);
}

void*
TrackerVRPN::_thread(void *arg)
{
  return reinterpret_cast<TrackerVRPN*> (arg)->thread(NULL);
}

void*
TrackerVRPN::thread(void *arg)
{


  while (!_stopped)
  {

		device_info device_list[1];
		dev =  &device_list[0];
		tracky = this;

		dev->name = const_cast<char*>(adress.c_str());
		dev->tkr = new vrpn_Tracker_Remote(adress.c_str());
		vrpn_Tracker_Remote *tkr = dev->tkr;
		t_user_callback *tc1 = new t_user_callback;

	    // Fill in the user-data callback information
	    strncpy(tc1->t_name, dev->name, sizeof(tc1->t_name));
	    dev->tkr->register_change_handler(tc1, handle_tracker_pos_quat_vrpn);
	  // set Update Rate (120)
	    dev->tkr->set_update_rate(120);

	  	dev->tkr->mainloop();

	  	// Prevents VRPN from running wild / consuming to much resources
	  	vrpn_SleepMsecs(10);

  };

  return arg;
}
void TrackerVRPN::setValue(double azi_vrpn){

	//   sets new value to _controller and _current_data
	_current_data.azimuth =azi_vrpn;
	_controller.set_reference_orientation(
        Orientation(-azi_vrpn + _az_corr));

}

void TrackerVRPN::getAdress(){



	  string line;
	  // reading data from config file Format NAME@ADRESS
	  ifstream vrpn_file ("vrpn.cfg");
	  if (vrpn_file.is_open())
	  {
		while (! vrpn_file.eof() )
		{

		  getline (vrpn_file,line);
		  if (line[0]== '#'){
			  line = "";
		  }else{
			  break;
		  }
		}
		vrpn_file.close();
	  }
	  adress = line;

}



// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
