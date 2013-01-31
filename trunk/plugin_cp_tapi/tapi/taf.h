#ifndef TAPIFW_H_INCLUDED
#define TAPIFW_H_INCLUDED

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

// $Revision: 10 $ $Date: 2012-09-21 21:35:29 +0200 (Fri, 21 Sep 2012) $

#include "tapi2dll.h"

#include <map>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// TAPI Framework (TAF)
// ---------------------------------------------------------------------------

struct TLineInfo
{
  std::string strName;
  DWORD       lid;  // Permanent Line ID

  TLineInfo() {
    lid = (DWORD)-1;
  }
  TLineInfo(const std::string& name, DWORD id) {
    strName = name;
    lid = id;
  }
};
typedef std::vector<TLineInfo> TLineList;

// ---------------------------------------------------------------------------

class CTafLock
{
public:
  CTafLock(LPCRITICAL_SECTION pCS) : m_pCS(pCS) {
    ::EnterCriticalSection(m_pCS);
  }
  ~CTafLock() {
    ::LeaveCriticalSection(m_pCS);
  }
protected:
  LPCRITICAL_SECTION m_pCS;
};

// ---------------------------------------------------------------------------

class CTafCall
{
public:
  enum TTafCallType {
    TCT_UNKNOWN,
    TCT_INCOMING,
    TCT_OUTGOING
  };

  CTafCall(DWORD id);
  virtual ~CTafCall();

  unsigned getId() const { return m_nId; }
  const std::string& getCallerAddress() const { return m_strCallerAddress; }
  const std::string& getCalledAddress() const { return m_strCalledAddress; }
  const std::string& getCallerName() const    { return m_strCallerName; }
  const std::string& getCalledName() const    { return m_strCalledName; }

  TTafCallType getType();
  DWORD        getState();

  void handleMessage(const LINEMESSAGE& m);

protected:
  bool queryInfo();
  void switchState(DWORD toState);

protected:
  DWORD          m_nId;
  DWORD          m_nState;
  LPLINECALLINFO m_pInfo;
  TTafCallType   m_nType;
  std::string    m_strCalledAddress;
  std::string    m_strCalledName;
  std::string    m_strCallerAddress;
  std::string    m_strCallerName;
  bool           m_bReported;
};
typedef std::map<unsigned, CTafCall *> TCallMap;

// ---------------------------------------------------------------------------

class CTafLine
{
public:
  CTafLine(DWORD idx);
  virtual ~CTafLine();

  bool init();

  bool open();
  void close();

  bool isOpen();

  bool hasMediaModes(DWORD modes);
  bool hasFeatures(DWORD features);

  void makeCall(const std::string& number);

  const std::string& getName();
  DWORD              getIdx();
  DWORD              getPermanentId();

  void handleMessage(const LINEMESSAGE& m);

protected:
  bool negotiateVersion();
  bool queryCaps();
  void closeCalls();

protected:
  DWORD         m_nIdx;
  DWORD         m_nTAPIVersion;
  std::string   m_strName;
  HLINE         m_hLine;
  LPLINEDEVCAPS m_pCaps;
  TCallMap      m_mapCalls;
};
typedef std::map<unsigned, CTafLine *> TLineMap;

// ---------------------------------------------------------------------------

class ITafEventHandler
{
public:
  enum ETAFEVENT {
    TAF_EVT_LINES_CHANGED,
    TAF_EVT_RUNSTATE_CHANGED,
    TAF_EVT_NEW_CALL,
    TAF_EVT_CALL_STATE_UPDATE,
    TAF_EVT_CALL_INFO_UPDATE,
    TAF_EVT_CALL_DELETED
  };

  virtual void OnTafEvent(ITafEventHandler::ETAFEVENT evt, void *info) = 0;

protected:
  ITafEventHandler() {  };
  virtual ~ITafEventHandler() {  };
};

// ---------------------------------------------------------------------------

class CTafManager
{
public:
  static CTafManager *getInstance();
  virtual ~CTafManager();

  void registerObserver(ITafEventHandler *);
  void deregisterObserver(ITafEventHandler *);

  void init(bool bWait = true);
  void shutdown();

  bool isRunning();

  void makeCall(const std::string& strNumber);

  void getLines(TLineList&);

protected:
  HLINEAPP getTapiHandle() { return m_hTAPI; }

  void newCall(const CTafCall& call);
  void updateCall(const CTafCall& call);
  void updateCallState(const CTafCall& call);
  void deleteCall(const CTafCall& call);

  void notifyObservers(ITafEventHandler::ETAFEVENT evt, void *info = 0);

private:
  CTafManager();

  static DWORD thread(void *arg);
  void monitor();

  void parseTapiMsg(LINEMESSAGE m);

  bool initTapi();
  bool isTapiOk();
  void shutdownTapi();

  void queryLocation();

  void addLine(DWORD idx);
  void removeLine(DWORD idx);

  void startMonitor();
  void stopMonitor();

  void handleTapiMessages();

protected:
  TAPI             *m_pTAPI;         //<! TAPI DLL
  HLINEAPP          m_hTAPI;         //<! TAPI handle
  DWORD             m_nTAPIVersion;  //<! TAPI version
  HANDLE            m_hThread;
  unsigned          m_nThreadId;
  bool              m_bRunMonitor;
  volatile bool     m_bMonitorIsRunning;
  TLineMap          m_mapLines;
  CRITICAL_SECTION  m_csLines;

  std::vector<ITafEventHandler *> m_Listeners;

  std::string               m_strCountryCode;                 // Laenderkennung
  std::string               m_strAreaCode;                    // Ortsvorwahl
  std::string               m_strOutsideAccess;               // Amtsleitung
  std::string               m_strNationalDialingPrefix;       // Ferngespraech
//  std::string       m_strInternationalDialingPrefix;  // Auslandsvorwahl

protected:
  static CTafManager *m_pInstance;

  friend class CTafLine;
  friend class CTafCall;
};

#endif /* TAPIFW_H_INCLUDED */
