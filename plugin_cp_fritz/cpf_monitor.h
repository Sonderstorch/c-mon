#ifndef CPF_MONITOR_H_INCLUDED
#define CPF_MONITOR_H_INCLUDED

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

#include "plugin/types.h"
#include "plugin/callmonitor.h"

#include "cpf_prefs.h"

#include <string>
#include <vector>

// ----

#define PLUGIN_ID (TPLUGIN_TYPE_CALLPROVIDER|TPLUGIN_ID_CP_FRITZ)

// ----

class wxWindow;

// ----

class IGuiNotification
{
public:
  virtual ~IGuiNotification() { };
  virtual void OnGuiNotification(long info) = 0;

};

// ----

class CFritzCallMonitor : public CCallMonitorApi
{
  public:
    CFritzCallMonitor();
    virtual ~CFritzCallMonitor();

    wxWindow* getGUI(wxWindow* parent);

    void init(bool bWait = true);
    void shutdown();

    bool isRunning();
    bool isConnected();

    CGUID    getID() const;
    unsigned getType() const;
    const wxString& getName() const;

    void registerCallObserver(ICallObserver *pObserver);
    void deregisterCallObserver(ICallObserver *pObserver);

    void registerGUI(IGuiNotification *pObserver);
    void deregisterGUI(IGuiNotification *pObserver);

    void makeCall(const std::string& strNumber);

    void handleFritzMessage(const std::string& strMsg);

  protected:
    void OnNewInCall(int cid, const std::string& strCaller, const std::string& strCalled);
    void OnNewOutCall(int cid, const std::string& strCaller, const std::string& strCalled);
    void OnConnect(int cid);
    void OnDisconnect(int cid, int dur);

  protected:
    static DWORD thread(void *arg);
    void monitor();

    void startMonitor();
    void stopMonitor();

    bool initSocket();
    void closeSocket();

    void notifyGUIs();

    void interruptibleWait(int millis);

  protected:
    CRITICAL_SECTION  m_csThread;
    HANDLE            m_hThread;
    HANDLE            m_hEndEvent;
    unsigned          m_nThreadId;
    unsigned          m_nLastError;
    bool              m_bRunMonitor;
    wxString          m_strName;

    volatile bool     m_bMonitorIsConnected;
    volatile bool     m_bMonitorIsRunning;

  protected:
    int               m_socket;

  protected:
    std::vector<ICallObserver*>     m_Listeners;
    std::vector<IGuiNotification*>  m_GUIs;
};

#endif /* CPF_MONITOR_H_INCLUDED */
