
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

#include "cpf_monitor.h"
#include "cpf_gui.h"

#include <process.h>
#include <boost/tokenizer.hpp>
#include <string>
#include <mstcpip.h>

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// {C8128F19-539C-4e8e-8490-807F93287A07}
static const UUID pluginUUID =
{ 0xc8128f19, 0x539c, 0x4e8e, { 0x84, 0x90, 0x80, 0x7f, 0x93, 0x28, 0x7a, 0x7 } };

static const CGUID GPLUGIN_GID(pluginUUID);

// ---------------------------------------------------------------------------

wxDEFINE_EVENT(EVT_CPF_NEW_CALL, wxCommandEvent);

// ---------------------------------------------------------------------------

CFritzCallMonitor::CFritzCallMonitor()
  : m_bMonitorIsRunning(false)
  , m_bMonitorIsConnected(false)
  , m_bRunMonitor(false)
  , m_nThreadId(0)
  , m_nLastError(0)
  , m_hThread(INVALID_HANDLE_VALUE)
  , m_socket(INVALID_SOCKET)
  , m_strName(wxT("FritzBox Phone"))
{
  CPFRITZPREFS::init();
  InitializeCriticalSection(&m_csThread);
  m_hEndEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // manually reset, initially not set
  WSADATA wsd;
  WSAStartup(MAKEWORD(2,0), &wsd);
  init();
}

CFritzCallMonitor::~CFritzCallMonitor()
{
  shutdown();
  WSACleanup();
  CloseHandle(m_hEndEvent);
  CPFRITZPREFS::free();
}

// ---------------------------------------------------------------------------

wxWindow* CFritzCallMonitor::getGUI(wxWindow* parent)
{
  return new CCpfGui(parent, this, 1500);
}

CGUID CFritzCallMonitor::getID() const
{
  return GPLUGIN_GID;
}

unsigned CFritzCallMonitor::getType() const
{
  return PLUGIN_ID;
}

const wxString& CFritzCallMonitor::getName() const
{
  return m_strName;
}

// ---------------------------------------------------------------------------

DWORD CFritzCallMonitor::thread(void *arg)
{
  CFritzCallMonitor *pObj = reinterpret_cast<CFritzCallMonitor *>(arg);
  pObj->monitor();
  return 0;
}

// ---------------------------------------------------------------------------

void CFritzCallMonitor::registerCallObserver(ICallObserver *pObserver)
{
  m_Listeners.push_back(pObserver);
}

void CFritzCallMonitor::deregisterCallObserver(ICallObserver *pObserver)
{
  std::vector<ICallObserver*>::iterator it = m_Listeners.begin();
  for (; it != m_Listeners.end(); ++it) {
    if (*it != pObserver) continue;
    m_Listeners.erase(it);
    break;
  }
}

// ---------------------------------------------------------------------------

void CFritzCallMonitor::registerGUI(IGuiNotification *pObserver)
{
  m_GUIs.push_back(pObserver);
}

void CFritzCallMonitor::deregisterGUI(IGuiNotification *pObserver)
{
  std::vector<IGuiNotification*>::iterator it = m_GUIs.begin();
  for (; it != m_GUIs.end(); ++it) {
    if (*it != pObserver) continue;
    m_GUIs.erase(it);
    break;
  }
}

void CFritzCallMonitor::notifyGUIs()
{
  std::vector<IGuiNotification*>::iterator it = m_GUIs.begin();
  for (; it != m_GUIs.end(); ++it) {
    (*it)->OnGuiNotification(0);
  }
}

// ---------------------------------------------------------------------------

void CFritzCallMonitor::makeCall(const std::string& strNumber)
{
  // TODO
}

// ---------------------------------------------------------------------------

bool CFritzCallMonitor::isRunning()
{
  return m_bMonitorIsRunning;
}

bool CFritzCallMonitor::isConnected()
{
  return m_bMonitorIsConnected;
}

void CFritzCallMonitor::init(bool bWait /*= true*/)
{
  int count = 0;
  m_nLastError = 0;
  startMonitor();
  while (bWait && !m_bMonitorIsRunning && (m_nLastError == 0) && (count < 10)) {
    ::Sleep(250);
    ++count;
  }
}

void CFritzCallMonitor::shutdown()
{
  if (m_bMonitorIsRunning) {
    m_bRunMonitor = false;
    SetEvent(m_hEndEvent);
    ::WaitForSingleObject(m_hThread, 10000);
    m_nLastError = 0;
  }
}

void  CFritzCallMonitor::startMonitor()
{
  if (m_bMonitorIsRunning) return;

  m_bRunMonitor = true;
  m_hThread = ::CreateThread(NULL, 0,
      (LPTHREAD_START_ROUTINE)&CFritzCallMonitor::thread, this, 0,
      (LPDWORD)&m_nThreadId);
}

void  CFritzCallMonitor::stopMonitor()
{
  if (!m_bMonitorIsRunning) return;
  m_bRunMonitor = false;
}

// ---------------------------------------------------------------------------

// The Fritz!Box Phone sends the following strings via TCP/IP:

// Outgoing calls: <dd.mm.yy HH:MM:SS>;CALL;<CallID>;<Extension>;<Caller Number>;<Called Number>;<Line-Name>
// Incoming calls: <dd.mm.yy HH:MM:SS>;RING;<CallID>;<Caller Number>;<Called Number>;
// Call accepted : <dd.mm.yy HH:MM:SS>;CONNECT;<CallID>;<Extension>;<Number>;
// End of call   : <dd.mm.yy HH:MM:SS>;DISCONNECT;<CallID>;<Duration in Seconds>;

void CFritzCallMonitor::handleFritzMessage(const std::string& msg)
{
  std::string date;
  std::string cmd;
  std::string args[5];
  int         argc;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(";", "", boost::keep_empty_tokens);
  tokenizer tokens(msg, sep);

  tokenizer::iterator it = tokens.begin();
  if (it == tokens.end()) return;
  date = *it; ++it;
  if (it == tokens.end()) return;
  cmd = *it; ++it;
  for (argc = 0; (argc < 5) && (it != tokens.end()); ++it, ++argc) {
    args[argc] = *it;
  }
  if ((cmd == "CALL") && (argc >= 4)) {
    OnNewOutCall( atoi(args[0].c_str()), args[2], args[3]);
  }
  else if ((cmd == "RING") && (argc >= 3)) {
    OnNewInCall(atoi(args[0].c_str()), args[1], args[2]);
  }
  else if ((cmd == "CONNECT") && (argc >= 1)) {
    OnConnect(atoi(args[0].c_str()));
  }
  else if ((cmd == "DISCONNECT") && (argc >= 2)) {
    OnDisconnect(atoi(args[0].c_str()), atoi(args[1].c_str()));
  }
  else {
    wxLogWarning(wxT("Unknown Fritz! message: %s"), msg.c_str());
  }
}

void CFritzCallMonitor::OnNewInCall(int cid, const std::string& strCaller, const std::string& strCalled)
{
  TCallInfo ci;
  wxLogMessage(wxT("New incoming call %d: %s -> %s"), cid, strCaller.c_str(), strCalled.c_str());
  ci.m_strCallerAddress = strCaller;
  ci.m_strCalledAddress = strCalled;
  std::vector<ICallObserver*>::iterator it = m_Listeners.begin();
  for (; it != m_Listeners.end(); ++it) {
    (*it)->newCall(PLUGIN_ID, cid, TCALL_TYPE_INCOMING, TCALL_OFFERING);
    (*it)->callInfo(PLUGIN_ID, cid, ci);
  }
}

void CFritzCallMonitor::OnNewOutCall(int cid, const std::string& strCaller, const std::string& strCalled)
{
  TCallInfo ci;
  wxLogMessage(wxT("New outgoing call %d: %s -> %s"), cid, strCaller.c_str(), strCalled.c_str());
  ci.m_strCallerAddress = strCaller;
  ci.m_strCalledAddress = strCalled;
  std::vector<ICallObserver*>::iterator it = m_Listeners.begin();
  for (; it != m_Listeners.end(); ++it) {
    (*it)->newCall(PLUGIN_ID, cid, TCALL_TYPE_OUTGOING, TCALL_DIALING);
    (*it)->callInfo(PLUGIN_ID, cid, ci);
  }
}

void CFritzCallMonitor::OnConnect(int cid)
{
  wxLogMessage(wxT("Call %d connected"), cid);
  std::vector<ICallObserver*>::iterator it = m_Listeners.begin();
  for (; it != m_Listeners.end(); ++it) {
    (*it)->callStatus(PLUGIN_ID, cid, TCALL_CONNECTED);
  }
}

void CFritzCallMonitor::OnDisconnect(int cid, int dur)
{
  wxLogMessage(wxT("Call %d disconnected (%d secs)"), cid, dur);
  std::vector<ICallObserver*>::iterator it = m_Listeners.begin();
  for (; it != m_Listeners.end(); ++it) {
    (*it)->callStatus(PLUGIN_ID, cid, TCALL_IDLE);
  }
}

// ---------------------------------------------------------------------------

void CFritzCallMonitor::monitor()
{
  struct fd_set set;
  timeval       to;
  char          buf[1024];
  int           rc;
  int           retryCount = 1;

  std::string   line;
  std::string   cmd;

  to.tv_sec  = 5;
  to.tv_usec = 0;

  m_bMonitorIsRunning = true;
  notifyGUIs();
  do
  {
    wxLogMessage(wxT("(Re)try to connect..."));
    if (initSocket())
    {
      retryCount = 1;
      m_bMonitorIsConnected = true;
      notifyGUIs();
      wxLogMessage(wxT("Connected successfully"));
      while (m_bRunMonitor)
      {
        FD_ZERO(&set);
        FD_SET(m_socket, &set);
        if ((rc = ::select(m_socket + 1, &set, NULL, NULL, &to)) > 0)
        {
          if (FD_ISSET(m_socket, &set))
          {
            if ((rc = ::recv(m_socket, (char *)&buf, sizeof(buf), 0)) > 0)
            {
              line.append(buf, rc);
              size_t pos = line.find('\n');
              while (pos != std::string::npos) {
                cmd = line.substr(0, pos);
                handleFritzMessage(cmd);
                line.erase(0, pos+1);
                pos = line.find('\n');
              }
            }
            else if (rc == 0) {
              // graceful termination
              wxLogWarning(wxT("Fritz!Box closed socket"));
              line.clear();
              break;
            }
            else {
              // socket error
              wxLogError(wxT("Socket error on recv(): %d"), WSAGetLastError());
              line.clear();
              break;
            }
          }
        }
        else if (rc < 0) {
          // socket error
          wxLogError(wxT("Socket error on select(): %d"), WSAGetLastError());
          line.clear();
          break;
        }
        // else timeout on select()
      }
      m_bMonitorIsConnected = false;
      closeSocket();
      notifyGUIs();
    }
    else {
      if (retryCount <= 60) ++retryCount; // max waiting time 30 seconds
      WaitForSingleObject(m_hEndEvent, 1000*retryCount/2);
    }
  } while (m_bRunMonitor);

  wxLogMessage(wxT("Monitor stopped"));
  m_bMonitorIsRunning = false;
  notifyGUIs();
}

// ---------------------------------------------------------------------------

bool CFritzCallMonitor::initSocket()
{
  bool rc = false;

  addrinfo  hint;
  addrinfo *pAddrInfo;

  std::string fbHost = CPFRITZPREFS::getPrefs().getFritzAddr();

  // Resolve Fritz!Box name to IPv4 and Port 1012
  memset(&hint, 0, sizeof(hint));
  hint.ai_family   = AF_INET;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_protocol = IPPROTO_TCP;
  if (getaddrinfo(fbHost.c_str(), "1012", &hint, &pAddrInfo) != 0) {
    m_nLastError = WSAGetLastError();
    wxLogMessage(wxT("Error %d resolving Fritz!Box name (%s)"),
        m_nLastError, fbHost.c_str());
    return rc;
  }

  closeSocket();

  DWORD rcDummy = 0;
  tcp_keepalive ka;
  ka.onoff             = 1;
  ka.keepalivetime     = 30000;
  ka.keepaliveinterval = 1000;

  m_socket = ::socket(pAddrInfo->ai_family, pAddrInfo->ai_socktype, 0);
  BOOL optval = 1;
  if ((m_socket != INVALID_SOCKET) &&
      (WSAIoctl(m_socket, SIO_KEEPALIVE_VALS, (LPVOID)&ka, sizeof(ka), NULL, 0, &rcDummy, NULL, NULL) == 0) &&
      (::connect(m_socket, pAddrInfo->ai_addr, pAddrInfo->ai_addrlen) == 0)) {
    rc = true;
  }
  else {
    m_nLastError = WSAGetLastError();
    wxLogMessage(wxT("Error %d opening socket"), m_nLastError);
    closeSocket();
  }

  freeaddrinfo(pAddrInfo);

  return rc;
}

void CFritzCallMonitor::closeSocket()
{
  if (m_socket != INVALID_SOCKET) {
    ::closesocket(m_socket);
    m_socket = INVALID_SOCKET;
  }
}
