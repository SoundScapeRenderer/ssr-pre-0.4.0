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
 * Connection class (definition).
 *
 * $LastChangedDate: 2012-11-13 14:03:13 +0100 (Die, 13. Nov 2012) $
 * $LastChangedRevision: 1972 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef CONNECTION_H
#define CONNECTION_H

#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*
#endif

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>

#include "networksubscriber.h"
#include "commandparser.h"

namespace ssr
{

class Publisher;

/// Connection class.
class Connection : public boost::enable_shared_from_this<Connection>
{
  public:
    /// Ptr to Connection
    typedef boost::shared_ptr<Connection> pointer;
    typedef boost::asio::ip::tcp::socket socket_t;

    static pointer create(boost::asio::io_service &io_service
        , Publisher &controller);

    void start();
    void write(std::string &writestring);

    /// @return Reference to socket
    socket_t& socket() { return _socket; }

    ~Connection();

  private:
    Connection(boost::asio::io_service &io_service, Publisher &controller);

    void start_read();
    void read_handler(const boost::system::error_code &error, size_t size);
    void write_handler(boost::shared_ptr<std::string> str_ptr
        , const boost::system::error_code &error, size_t bytes_transferred);

    void timeout_handler(const boost::system::error_code &e);

    /// TCP/IP socket
    socket_t _socket;
    /// Buffer for incoming messages.  
    boost::asio::streambuf _streambuf;
    /// @see Connection::timeout_handler
    boost::asio::deadline_timer _timer;

    /// Reference to Controller
    Publisher &_controller;
    /// Subscriber obj
    NetworkSubscriber _subscriber;
    /// Commandparser obj 
    CommandParser _commandparser;
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
