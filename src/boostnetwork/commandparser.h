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
 * CommandParser class (definition). 
 *
 * $LastChangedDate: 2012-07-09 21:37:27 +0200 (Mon, 09. Jul 2012) $
 * $LastChangedRevision: 1886 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <iostream>
#include <queue>
#include <string>

#include "ssr_global.h"
#include "xmlparser.h"

namespace ssr
{

class Publisher;

/** Parses a XML string and maps to Controller.
 * This class is the bridge between the network interface and the Controller.
 * Incoming XML messages (in ASDF-format) are parsed and the appropriate 
 * functions of Controller called.  
 **/
class CommandParser
{
  public:
    CommandParser(Publisher& controller);
    ~CommandParser();

    void parse_cmd(std::string &cmd);
    //void parse_cmd_old(std::string &cmd)
  private:
    Publisher& _controller;
    //Subscriber& _scene;
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
