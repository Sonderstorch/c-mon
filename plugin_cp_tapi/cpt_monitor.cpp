
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

#include "cpt_monitor.h"
#include "cpt_gui.h"

#include <process.h>

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// {780D0149-DFB0-46e3-889A-4DD267ABAE19}
static const UUID pluginUUID =
{ 0x780d0149, 0xdfb0, 0x46e3, { 0x88, 0x9a, 0x4d, 0xd2, 0x67, 0xab, 0xae, 0x19 } };

static const CGUID GPLUGIN_GID(pluginUUID);

// ---------------------------------------------------------------------------

wxDEFINE_EVENT(EVT_CPT_NEW_CALL, wxCommandEvent);

// ---------------------------------------------------------------------------

TCallStatus getCallStatus(CTafCall *pCall)
{
  TCallStatus status = TCALL_UNKNOWN;
  switch (pCall->getState())
  {
  case LINECALLSTATE_OFFERING:            status = TCALL_OFFERING;      break;
  case LINECALLSTATE_ACCEPTED:            status = TCALL_CONNECTED;     break;
  case LINECALLSTATE_CONNECTED:           status = TCALL_CONNECTED;     break;
  case LINECALLSTATE_DISCONNECTED:        status = TCALL_DISCONNECTED;  break;
  case LINECALLSTATE_ONHOLD:              status = TCALL_ONHOLD;        break;
  case LINECALLSTATE_CONFERENCED:         status = TCALL_CONFERENCED;   break;
  case LINECALLSTATE_ONHOLDPENDCONF:      status = TCALL_CONFERENCED;   break;
  case LINECALLSTATE_ONHOLDPENDTRANSFER:  status = TCALL_CONFERENCED;   break;
  case LINECALLSTATE_DIALTONE:            status = TCALL_DIALTONE;      break;
  case LINECALLSTATE_DIALING:             status = TCALL_DIALING;       break;
  case LINECALLSTATE_RINGBACK:            status = TCALL_RINGBACK;      break;
  case LINECALLSTATE_BUSY:                status = TCALL_BUSY;          break;
  case LINECALLSTATE_PROCEEDING:          status = TCALL_PROCEEDING;    break;
  }
  return status;
}

// ---------------------------------------------------------------------------


CTapiCallMonitor::CTapiCallMonitor()
  : m_strName(wxT("TAPI"))
{
  CPTPREFS::init();
  m_pTafMgr = CTafManager::getInstance();
  m_pTafMgr->registerObserver(this);
}

CTapiCallMonitor::~CTapiCallMonitor()
{
  m_pTafMgr->deregisterObserver(this);
  delete m_pTafMgr;
  CPTPREFS::free();
}

// ---------------------------------------------------------------------------

wxWindow* CTapiCallMonitor::getGUI(wxWindow* parent)
{
  return new CCptGui(parent, this, 1500);
}

CGUID CTapiCallMonitor::getID() const
{
  return GPLUGIN_GID;
}

unsigned CTapiCallMonitor::getType() const
{
  return PLUGIN_ID;
}

const wxString& CTapiCallMonitor::getName() const
{
  return m_strName;
}

void CTapiCallMonitor::getLines(TLineList& rLines)
{
  m_pTafMgr->getLines(rLines);
}

// ---------------------------------------------------------------------------

void CTapiCallMonitor::registerCallObserver(ICallObserver *pObserver)
{
  m_Listeners.push_back(pObserver);
}

void CTapiCallMonitor::deregisterCallObserver(ICallObserver *pObserver)
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

void CTapiCallMonitor::makeCall(const std::string& strNumber)
{
  m_pTafMgr->makeCall(strNumber);
}

// ---------------------------------------------------------------------------

void CTapiCallMonitor::start(bool bWait)
{
  m_pTafMgr->init(bWait);
}

void CTapiCallMonitor::stop()
{
  m_pTafMgr->shutdown();
}

bool CTapiCallMonitor::isRunning()
{
  return m_pTafMgr->isRunning();
}

void CTapiCallMonitor::OnTafEvent(ITafEventHandler::ETAFEVENT evt, void *info)
{
  std::vector<ICallObserver*>::iterator it = m_Listeners.begin();
  switch (evt)
  {
    case TAF_EVT_NEW_CALL:
    {
      CTafCall *pCall = static_cast<CTafCall *>(info);
      CTafCall::TTafCallType t = pCall->getType();
      TCallStatus status = getCallStatus(pCall);
      TCallType type = TCALL_TYPE_UNKNOWN;
      switch (t) {
      case CTafCall::TCT_INCOMING: type = TCALL_TYPE_INCOMING; break;
      case CTafCall::TCT_OUTGOING: type = TCALL_TYPE_OUTGOING; break;
      }
      for (; it != m_Listeners.end(); ++it) {
        (*it)->newCall(PLUGIN_ID, pCall->getId(), type, status);
      }
      break;
    }
    case TAF_EVT_CALL_INFO_UPDATE:
    {
      TCallInfo ci;
      CTafCall *pCall = static_cast<CTafCall *>(info);
      ci.m_strCallerAddress = pCall->getCallerAddress();
      ci.m_strCalledAddress = pCall->getCalledAddress();
      ci.m_strCallerName = pCall->getCallerName();
      ci.m_strCalledName = pCall->getCalledName();
      for (; it != m_Listeners.end(); ++it) {
        (*it)->callInfo(PLUGIN_ID, pCall->getId(), ci);
      }
      break;
    }
    case TAF_EVT_CALL_STATE_UPDATE:
    {
      CTafCall *pCall = static_cast<CTafCall *>(info);
      TCallStatus status = getCallStatus(pCall);
      for (; it != m_Listeners.end(); ++it) {
        (*it)->callStatus(PLUGIN_ID, pCall->getId(), status);
      }
      break;
    }
    case TAF_EVT_CALL_DELETED:
    {
      CTafCall *pCall = static_cast<CTafCall *>(info);
      for (; it != m_Listeners.end(); ++it) {
        (*it)->callStatus(PLUGIN_ID, pCall->getId(), TCALL_IDLE);
      }
      break;
    }
  }
}
