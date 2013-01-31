
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

// $Revision: 47 $ $Date: 2012-11-18 22:38:42 +0100 (Sun, 18 Nov 2012) $

#include "stdwx.h"

#include "cpd_impl.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// ---------------------------------------------------------------------------

// {9F9B0ADE-31C0-4028-9511-C6015DB15F60}
static const UUID pluginUUID =
{ 0x9f9b0ade, 0x31c0, 0x4028, { 0x95, 0x11, 0xc6, 0x1, 0x5d, 0xb1, 0x5f, 0x60 } };

static const CGUID GPLUGIN_GID(pluginUUID);

// ---------------------------------------------------------------------------

CDummyCallMonitor::CDummyCallMonitor()
  : m_strName(wxT("Dummy Call Monitor"))
{
}

CDummyCallMonitor::~CDummyCallMonitor()
{
}

// ---------------------------------------------------------------------------

CGUID CDummyCallMonitor::getID() const
{
  return GPLUGIN_GID;
}

unsigned CDummyCallMonitor::getType() const
{
  return PLUGIN_ID;
}

const wxString& CDummyCallMonitor::getName() const
{
  return m_strName;
}

// ---------------------------------------------------------------------------

void CDummyCallMonitor::registerCallObserver(ICallObserver *pObserver)
{
  m_Listeners.push_back(pObserver);
}

void CDummyCallMonitor::deregisterCallObserver(ICallObserver *pObserver)
{
  std::vector<ICallObserver*>::iterator it = m_Listeners.begin();
  while (it != m_Listeners.end()) {
    if (*it == pObserver) {
      m_Listeners.erase(it);
      return;
    }
    it++;
  }
}

void CDummyCallMonitor::makeCall(const std::string& strNumber)
{
}

// ---------------------------------------------------------------------------

void CDummyCallMonitor::newCall(int id, const wxString& strNumber)
{
  TCallInfo info;
  info.m_strCallerAddress = strNumber;
  info.m_strCalledAddress = "22";
  std::vector<ICallObserver*>::iterator it;
  for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) {
    (*it)->newCall(PLUGIN_ID, id, TCALL_TYPE_INCOMING, TCALL_OFFERING);
    (*it)->callInfo(PLUGIN_ID, id, info);
  }
}

void CDummyCallMonitor::updateCall(int id, TCallStatus status)
{
  std::vector<ICallObserver*>::iterator it;
  for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) {
    (*it)->callStatus(PLUGIN_ID, id, status);
  }
}

bool CDummyCallMonitor::TestOutgoing(const wxString& strNumber)
{
  static int tstIdx = 0;
  std::vector<ICallObserver*>::iterator it;
  if (tstIdx == 0) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->newCall(PLUGIN_ID, 5884,
        TCALL_TYPE_OUTGOING, TCALL_RINGBACK);
    ++tstIdx;
  }
  else if (tstIdx == 1) {
    TCallInfo info;
    info.m_strCallerAddress = "22";
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callInfo(PLUGIN_ID, 5884, info);
    info.m_strCalledAddress = strNumber.ToUTF8().data();
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callInfo(PLUGIN_ID, 5884, info);
    ++tstIdx;
  }
  else if (tstIdx == 2) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callStatus(PLUGIN_ID, 5884, TCALL_CONNECTED);
    ++tstIdx;
  }
  else if (tstIdx == 3) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callStatus(PLUGIN_ID, 5884, TCALL_DISCONNECTED);
    ++tstIdx;
  }
  else if (tstIdx == 4) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callStatus(PLUGIN_ID, 5884, TCALL_IDLE);
    tstIdx = 0;
  }
  return (tstIdx == 0);
}

void CDummyCallMonitor::Test()
{
  static int tstIdx = 0;
  std::vector<ICallObserver*>::iterator it;
  if (tstIdx == 0) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->newCall(PLUGIN_ID, 1234,
        TCALL_TYPE_INCOMING, TCALL_OFFERING);
    ++tstIdx;
  }
  else if (tstIdx == 1) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->newCall(PLUGIN_ID, 1235,
        TCALL_TYPE_OUTGOING, TCALL_RINGBACK);
    ++tstIdx;
  }
  else if (tstIdx == 2) {
    TCallInfo info;
    info.m_strCallerAddress = "418";
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callInfo(PLUGIN_ID, 1234, info);
    info.m_strCalledAddress = "22";
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callInfo(PLUGIN_ID, 1235, info);
    ++tstIdx;
  }
  else if (tstIdx == 3) {
    TCallInfo info;
    info.m_strCallerAddress = "418";
    info.m_strCalledAddress = "263";
    info.m_strCalledName    = "Carola Dimpflbacher";
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callInfo(PLUGIN_ID, 1234, info);
    ++tstIdx;
  }
  else if (tstIdx == 4) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callStatus(PLUGIN_ID, 1234, TCALL_CONNECTED);
    ++tstIdx;
  }
  else if (tstIdx == 5) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callStatus(PLUGIN_ID, 1234, TCALL_DISCONNECTED);
    ++tstIdx;
  }
  else if (tstIdx == 6) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callStatus(PLUGIN_ID, 1234, TCALL_IDLE);
    ++tstIdx;
  }
  else if (tstIdx == 7) {
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it) (*it)->callStatus(PLUGIN_ID, 1235, TCALL_IDLE);
    tstIdx = 0;
  }
}
