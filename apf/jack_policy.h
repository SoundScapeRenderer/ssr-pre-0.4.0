/******************************************************************************
 * Copyright (c) 2006-2012 Quality & Usability Lab                            *
 *                         Deutsche Telekom Laboratories, TU Berlin           *
 *                         Ernst-Reuter-Platz 7, 10587 Berlin, Germany        *
 *                                                                            *
 * This file is part of the Audio Processing Framework (APF).                 *
 *                                                                            *
 * The APF is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The APF is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 *                                                http://tu-berlin.de/?id=apf *
 ******************************************************************************/

/// @file
/// JACK policy for MimoProcessor's interface_policy.

#ifndef APF_JACK_POLICY_H
#define APF_JACK_POLICY_H

#include <cassert> // for assert()

#include "apf/jackclient.h"
#include "apf/parameter_map.h"
#include "stringtools.h"

#ifndef APF_MIMOPROCESSOR_INTERFACE_POLICY
#define APF_MIMOPROCESSOR_INTERFACE_POLICY apf::jack_policy
#endif

namespace apf
{

/// @c interface_policy using JACK.
/// Some of the functions are directly taken from JackClient.
/// @see MimoProcessor
/// @ingroup apf_policies
class jack_policy : public JackClient
{
  public:
    typedef sample_t sample_type;
    class Input;
    class Output;

    using JackClient::activate;
    using JackClient::deactivate;
    using JackClient::sample_rate;

    nframes_t block_size() const
    {
      return this->buffer_size();
    }

  protected:
    /// Constructor
    /// @param p Parameters, only the parameter @c "name" (for the name of the
    ///   JACK client) is supported.
    explicit jack_policy(const parameter_map& p = parameter_map())
      : JackClient(p.get("name", "MimoProcessor"), use_jack_process_callback)
    {}

    ~jack_policy() {}  ///< Protected dtor.

  private:
    template<typename X> class Xput;

    struct i_am_in
    {
      typedef const sample_type* iterator;
      static const bool is_input = true;
      static std::string prefix_name() { return "input_prefix"; }
      static std::string default_prefix() { return "in_"; }
    };

    struct i_am_out
    {
      typedef sample_type* iterator;
      static const bool is_input = false;
      static std::string prefix_name() { return "output_prefix"; }
      static std::string default_prefix() { return "out_"; }
    };

    virtual int jack_process_callback(nframes_t nframes)
    {
      (void)nframes;
      assert(nframes == this->block_size());

      // call member function which is implemented in derived class
      // (template method design pattern)
      _process();

      return 0;
    }

    virtual void _process() = 0;
};

// Helper class to avoid code duplication in Input and Output
template<typename X>
class jack_policy::Xput
{
  public:
    typedef typename X::iterator iterator;

    void fetch_buffer()
    {
      _begin = static_cast<sample_type*>(
          jack_port_get_buffer(_port, _parent.block_size()));
      _end   = _begin + _parent.block_size();
    }

    std::string port_name() const { return _port_name; }

  protected:
     Xput(jack_policy& parent, const parameter_map& p);

    ~Xput() { _parent.unregister_port(_port); }

    iterator _begin;
    iterator _end;

  private:
    jack_policy& _parent;
    JackClient::port_t* _port;  // JACK port corresponding to this object. 

    JackClient::port_t* _init_port(const parameter_map& p, jack_policy& parent);

    const std::string _port_name;  // actual JACK port name
};

template<typename X>
JackClient::port_t*
jack_policy::Xput<X>::_init_port(const parameter_map& p, jack_policy& parent)
{
  std::string name;

  // first, try port_name
  if (p.has_key("port_name"))
  {
    name = p["port_name"];
  }
  else
  {
    // then concatenate "input_prefix"/"output_prefix" with "id"
    // if the prefix isn't specified, it's replaced by a default string
    // worst case: duplicate string -> port registration will fail!

    std::string id;

    if (p.has_key("id"))
    {
      id = p.get("id", "");
    }
    else
    {
      static int next_id = 1;
      id = str::A2S(next_id++);
    }

    name = p.get(X::prefix_name(), X::default_prefix()) + id;
  }

  return X::is_input
    ? parent.register_in_port(name) : parent.register_out_port(name);
}

template<typename X>
jack_policy::Xput<X>::Xput(jack_policy& parent, const parameter_map& p)
  : _parent(parent)
  , _port(_init_port(p, _parent))
  // get actual port name and save it to member variable
  , _port_name(_port ? jack_port_name(_port) : "")
{
  // optionally connect to jack_port
  std::string connect_to = p.get("connect_to", "");
  if (connect_to != "")
  {
    if (X::is_input)
    {
      _parent.connect_ports(connect_to, _port_name);
    }
    else
    {
      _parent.connect_ports(_port_name, connect_to);
    }
  }
}

class jack_policy::Input : public Xput<i_am_in>
{
  protected:
    Input(jack_policy& parent, const parameter_map& p)
      : Xput<i_am_in>(parent, p)
    {}
};

class jack_policy::Output : public Xput<i_am_out>
{
  protected:
    Output(jack_policy& parent, const parameter_map& p)
      : Xput<i_am_out>(parent, p)
    {}
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
