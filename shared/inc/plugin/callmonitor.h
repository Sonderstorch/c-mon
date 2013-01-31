#ifndef CALLMONITORAPI_H_INCLUDED
#define CALLMONITORAPI_H_INCLUDED

// This file is part of c'mon, a lightweight telephony monitor.
//
// Copyright (C) 2012 Peter Schaefer-Hutter ("THE AUTHOR")
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>

// ----------------------------------------------------------------------------
// NOTE: This License does NOT permit incorporating this file (or parts or
//       changed versions of it) into proprietary programs that are
//       distributed to third-parties in binary form only.
// ----------------------------------------------------------------------------

// You can contact the original author by email at peter.schaefer@gmx.de
// if you need different license-terms or other modifications to c'mon.

// $Revision: 43 $ $Date: 2012-10-25 23:30:03 +0200 (Thu, 25 Oct 2012) $

#include <wx/wx.h>

#include <string>
#include <vector>

#include "plugin/types.h"
#include "plugin/generic.h"

// Call Monitor Interface ----------------------------------------------------

enum TCallType
{
  TCALL_TYPE_UNKNOWN,
  TCALL_TYPE_INCOMING,
  TCALL_TYPE_OUTGOING
};

enum TCallStatus
{
  TCALL_UNKNOWN,
  // Incoming
  TCALL_OFFERING,
  // Outgoing
  TCALL_WAIT_DIALTONE,
  TCALL_DIALTONE,
  TCALL_DIALING,
  TCALL_PROCEEDING,
  TCALL_RINGBACK,
  TCALL_BUSY,
  // In & Out
  TCALL_CONNECTED,
  TCALL_DISCONNECTED,
  TCALL_IDLE,
  // Specials
  TCALL_ONHOLD,
  TCALL_CONFERENCED
};

struct TCallInfo
{
  std::string m_strCalledAddress;
  std::string m_strCallerAddress;
  std::string m_strCalledName;
  std::string m_strCallerName;
};

class ICallObserver
{
  public:
    ICallObserver() {  };
    virtual ~ICallObserver() {  };

    virtual void newCall(unsigned pid, unsigned cid, TCallType type, TCallStatus status) = 0;
    virtual void callStatus(unsigned pid, unsigned cid, TCallStatus status) = 0;
    virtual void callInfo(unsigned pid, unsigned cid, const TCallInfo& info) = 0;
};

class CCallMonitorApi : public IPluginModule
{
public:
  virtual ~CCallMonitorApi() {};

  virtual void registerCallObserver(ICallObserver *pObserver) = 0;
  virtual void deregisterCallObserver(ICallObserver *pObserver) = 0;

  virtual void makeCall(const std::string& strNumber) = 0;

protected:
  CCallMonitorApi()  { };
};

// Callmonitor Plugin Interface -----------------------------------------------

class CCallMonitorPlugin : public IPluginApi
{
public:
  virtual ~CCallMonitorPlugin() {};

protected:
  CCallMonitorPlugin()  { };
};

#endif /* CALLMONITORAPI_H_INCLUDED */
