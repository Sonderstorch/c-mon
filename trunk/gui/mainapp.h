#ifndef MAINAPP_H_INCLUDED
#define MAINAPP_H_INCLUDED

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

#include "plugin/callmonitor.h"
#include "journal/journalmodel.h"
#include "pluginloader.h"

class CNotificationMgr;
class CMainFrame;
class CDBBackend;
class CGlobalPreferences;
class CResolverModel;
class CJournalModel;
class CJournalEntry;
class CGeneralPrefs;
class CPluginLoader;
class CAddressbookApi;
class CContact;

// ----------------------------------------------------------------------------

class CMainApp : public wxApp, public ICallObserver
{
public:
  CMainApp();

  virtual bool OnInit();
  virtual int OnExit();
  void OnIdle(wxIdleEvent&);

  CMainFrame *getFrameWindow() { return m_pFrame; }

  DECLARE_EVENT_TABLE()

public:
  // Overrides from ICallObserver
  virtual void newCall(unsigned pid, unsigned cid, TCallType type, TCallStatus status);
  virtual void callStatus(unsigned pid, unsigned cid, TCallStatus status);
  virtual void callInfo(unsigned pid, unsigned cid, const TCallInfo& info);

  // Callback from Resolver
  void resolverResponse(unsigned cid, unsigned data, const CContact& contact);

  const TJournalMap& getActiveCalls() const { return m_ActiveCalls; }

  CJournalModel&      getJournalModel();
  CResolverModel&     getResolverModel();
  CGeneralPrefs&      getPrefs();
  CGlobalPreferences& getGlobalPrefs();
  CPluginLoader&      getPluginLoader();
  wxLocale&           getLocale();

  CCallMonitorApi    *getDefaultCallProvider();

  CAddressbookApi       *getAddressbook(const CGUID& gid);
  const TAddressbookMap& getAddressbooks() const;

  CAddressbookApi       *getCurrentAddressbook();
  void                   setCurrentAddressbook(const CGUID& gid);

  bool reverseLookup(const wxString& strNumber, CContact& resolvedContact);

  void displayNotification(CJournalEntry& e);
  void closeNotification(CJournalEntry& e);

  virtual void CleanUp();

protected:
  void OnNewCall(wxThreadEvent& event);
  void OnCallStateUpdate(wxThreadEvent& event);
  void OnCallInfoUpdate(wxThreadEvent& event);
  void OnResolverResponse(wxThreadEvent& event);

  void ResolveCalled(const std::string& toResolve, CJournalEntry *pJ);
  void ResolveCaller(const std::string& toResolve, CJournalEntry *pJ);

  CJournalEntry *findActiveEntry(long long dbKey);
  bool findJournalEntry(long long dbKey, CJournalEntry& e);

protected:
  wxLocale                 m_Locale;
  TJournalMap              m_ActiveCalls;
  CNotificationMgr        *m_pNotificationMgr;
  CMainFrame              *m_pFrame;
  CDBBackend              *m_pDB;
  CResolverModel          *m_pResolverModel;
  CJournalModel           *m_pJournalModel;
  CPluginLoader           *m_pPluginLoader;
  CGlobalPreferences      *m_pPrefs;
  CAddressbookApi         *m_pCurrentAddressbook;
  wxSingleInstanceChecker *m_pSingleInstance;
};

DECLARE_APP(CMainApp)

#endif // MAINAPP_H_INCLUDED
