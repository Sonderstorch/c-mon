
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

#include "glob/prefs.h"
#include "glob/contact.h"
#include "db/backend.h"
#include "journal/journalmodel.h"
#include "resolver/resolver.h"
#include "resolver/address.h"
#include "resolver/resolvermodel.h"
#include "log/filelog.h"

#include "mainapp.h"
#include "mainframe.h"
#include "pluginloader.h"
#include "resources.h"
#include "notification.h"

#include "wx/stdpaths.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator 
#endif

#define wxLOG_COMPONENT "GUI"

IMPLEMENT_APP(CMainApp)

// -----------------------------------------------------------------------
//! CMainApp
// -----------------------------------------------------------------------

enum
{
  NEW_CALL_EVENT,
  UPDATE_CALL_STATE_EVENT,
  UPDATE_CALL_INFO_EVENT,
  RESOLVER_RESPONSE
};

BEGIN_EVENT_TABLE(CMainApp, wxApp)
  EVT_IDLE(CMainApp::OnIdle)
  EVT_THREAD(NEW_CALL_EVENT, CMainApp::OnNewCall)
  EVT_THREAD(UPDATE_CALL_STATE_EVENT, CMainApp::OnCallStateUpdate)
  EVT_THREAD(UPDATE_CALL_INFO_EVENT, CMainApp::OnCallInfoUpdate)
  EVT_THREAD(RESOLVER_RESPONSE, CMainApp::OnResolverResponse)
END_EVENT_TABLE()

CMainApp::CMainApp() 
  : m_pNotificationMgr(0)
  , m_pFrame(0)
  , m_pPrefs(0)
  , m_pDB(0)
  , m_pResolverModel(0)
  , m_pJournalModel(0)
  , m_pSingleInstance(0)
  , m_pPluginLoader(0)
  , m_pCurrentAddressbook(0)
{
}

bool CMainApp::OnInit()
{
  m_pSingleInstance = new wxSingleInstanceChecker(wxT("c'mon"));
  if (m_pSingleInstance->IsAnotherRunning()) {
    delete m_pSingleInstance;
    return false;
  }

  // Check wether user dir exists
  wxFileName fn;
  fn.Assign(wxStandardPaths::Get().GetUserLocalDataDir(), wxT("cmon"), wxEmptyString);
  if (!fn.DirExists()) {
    fn.Mkdir();
  }

  // Initialize Log
  wxLog::SetActiveTarget(new
      CFileLog(CGlobalPreferences::getLogFileName(), 1024*1024, 5));

  // Initialize the locale catalogs we'll be using
  wxLocale::AddCatalogLookupPathPrefix(".");
  m_Locale.Init(wxLANGUAGE_DEFAULT);
  m_Locale.AddCatalog("wxstd");
  m_Locale.AddCatalog("hzshared");
  m_Locale.AddCatalog("gui");

  // Initialize all GUI resources (fonts, bitmaps, etc.)
  wxInitAllImageHandlers();
  RESOURCES::init();

  // Load preferences from registry
  m_pPrefs = GetGlobalPrefs();

  // Load plug-in DLLs
  m_pPluginLoader = new CPluginLoader();
  m_pPluginLoader->init();

  // Init Sockets
  wxSocketBase::Initialize();

  // Create/Open database
  m_pDB = new CDBBackend(m_pPrefs->getDBFileName());
  if (!m_pDB->isOk()) {
    wxMessageBox(wxString::FromUTF8(m_pDB->getLastError().c_str()),
        wxMessageBoxCaptionStr, wxICON_ERROR|wxOK);
  }
  // Create Journal model
  m_pJournalModel = new CJournalModel(m_pDB);
  // Restore resolvers
  m_pResolverModel = new CResolverModel(m_pDB);

  // Resync Resolvers
  const TAddressbookMap& books = m_pPluginLoader->getAddressbooks();
  TAddressbookMap::const_iterator bit = books.begin();
  for (; bit != books.end(); ++bit) {
    if (!m_pResolverModel->hasEntry((*bit).second->getName())) {
      CAddressBookResolver *pR = new CAddressBookResolver((*bit).second);
      pR->setName((*bit).second->getName());
      pR->enable();
      m_pResolverModel->prependEntry(pR);
    }
  }

  // Create main application window
  m_pFrame = new CMainFrame();

  // Create call notification system
  m_pNotificationMgr = new CNotificationMgr(this);
  // Register for call monitor notifications
  const TCallMonitorMap& cms = m_pPluginLoader->getCallMonitors();
  TCallMonitorMap::const_iterator cit = cms.begin();
  for (; cit != cms.end(); ++cit) {
    (*cit).second->registerCallObserver(this);
  }
  // Finally show main window
  if (!m_pPrefs->getPrefs().startHidden()) {
    m_pFrame->Show();
  }
  SetTopWindow(m_pFrame);

  return true;
}

int CMainApp::OnExit()
{
  m_pPrefs->savePrefs();
  wxApp::OnExit();
  return 0;
}

void CMainApp::OnIdle(wxIdleEvent& evt)
{
}

void CMainApp::CleanUp()
{
  // Unregister from call monitor plug-ins
  const TCallMonitorMap& cms = m_pPluginLoader->getCallMonitors();
  TCallMonitorMap::const_iterator it = cms.begin();
  for (; it != cms.end(); ++it) {
    (*it).second->registerCallObserver(this);
  }
  // Shutdown background thread
  // TODO: May create one background thread under wxApp control and
  //       queue jobs into it from whoever needs it.
  m_pResolverModel->shutdown();
  // Delete notifications
  delete m_pNotificationMgr;
  // Call base class (destroys windows)
  wxApp::CleanUp();
  // Unload plug-ins
  delete m_pPluginLoader;
  // Shutdown Sockets
  wxSocketBase::Shutdown();
  // Delete Journal
  delete m_pJournalModel;
  // Delete Resolvers
  delete m_pResolverModel;
  // Free DB
  delete m_pDB;
  // Free preferences
  delete m_pPrefs;
  // Free resources
  RESOURCES::free();
  delete m_pSingleInstance;

  delete wxLog::SetActiveTarget(NULL);
}

wxLocale& CMainApp::getLocale()
{
  return m_Locale;
}

CJournalModel& CMainApp::getJournalModel()
{
  return *m_pJournalModel;
}

CResolverModel& CMainApp::getResolverModel()
{
  return *m_pResolverModel;
}

CGeneralPrefs& CMainApp::getPrefs()
{
  return m_pPrefs->getPrefs();
}

CGlobalPreferences& CMainApp::getGlobalPrefs()
{
  return *m_pPrefs;
}

CPluginLoader& CMainApp::getPluginLoader()
{
  return *m_pPluginLoader;
}

CAddressbookApi *CMainApp::getAddressbook(const CGUID& gid)
{
  return m_pPluginLoader->getAddressbook(gid);
}

const TAddressbookMap& CMainApp::getAddressbooks() const
{
  return m_pPluginLoader->getAddressbooks();
}

CAddressbookApi *CMainApp::getCurrentAddressbook()
{
  return m_pCurrentAddressbook;
}

void CMainApp::setCurrentAddressbook(const CGUID& gid)
{
  m_pPrefs->getPrefs().setSelectedABook(gid);
  m_pCurrentAddressbook = getAddressbook(gid);
}

CCallMonitorApi *CMainApp::getDefaultCallProvider()
{
  // TODO: Implement default handling
  // (i.e. Dialog with 'make this default' option)
  CCallMonitorApi *api = m_pPluginLoader->getCallMonitor(
      m_pPrefs->getPrefs().getDefaultCallProvider());
  if (api == NULL) {
    wxArrayString choices;
    m_pPluginLoader->getCallMonitorNames(choices);
    wxSingleChoiceDialog dlg(m_pFrame,
        _("Please choose the call provider that shall be used as default:"),
        _("Choose default call provider"), choices);
    if (dlg.ShowModal() == wxID_OK) {
      m_pPrefs->getPrefs().setDefaultCallProvider(choices[dlg.GetSelection()]);
      api = m_pPluginLoader->getCallMonitor(choices[dlg.GetSelection()]);
    }
  }
  return api;
}

bool CMainApp::reverseLookup(const wxString& strNumber,
                             CContact& resolvedContact)
{
  return m_pResolverModel->resolve(strNumber, resolvedContact);
}

CJournalEntry *CMainApp::findActiveEntry(long long dbKey)
{
  CJournalEntry *rc = NULL;
  TJournalMap::const_iterator it = m_ActiveCalls.begin();
  for (; it != m_ActiveCalls.end(); ++it) {
    if (it->second->getDbKey() == dbKey) {
      rc = it->second;
      break;
    }
  }
  return rc;
}

bool CMainApp::findJournalEntry(long long dbKey, CJournalEntry& e)
{
  return m_pJournalModel->getEntry(dbKey, e);
}

void CMainApp::displayNotification(CJournalEntry& e)
{
  m_pNotificationMgr->notify(e);
}

void CMainApp::closeNotification(CJournalEntry& e)
{
  m_pNotificationMgr->killWnd(e.getNotification());
}

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

struct TEventInfoNewCall
{
  unsigned    cid;
  TCallType   type;
  TCallStatus status;
};

struct TEventInfoCallStatus
{
  unsigned    cid;
  TCallStatus status;
};

struct TEventInfoCallInfo
{
  unsigned  cid;
  TCallInfo info;
};

// ----------------------------------------------------------------------------

CJournalEntry::TType CallType2JournalType(TCallType type)
{
  CJournalEntry::TType jtype = CJournalEntry::J_UNKNOWN;
  if (type == TCALL_TYPE_INCOMING) {
    jtype = CJournalEntry::J_INCOMING;
  } else if (type == TCALL_TYPE_OUTGOING){
    jtype = CJournalEntry::J_OUTGOING;
  }
  return jtype;
}

CJournalEntry::TState CallState2JournalState(TCallStatus status)
{
  CJournalEntry::TState jstate = CJournalEntry::JS_UNKNOWN;
  switch (status)
  {
  case TCALL_OFFERING:
  case TCALL_WAIT_DIALTONE:
  case TCALL_DIALTONE:
  case TCALL_DIALING:
  case TCALL_PROCEEDING:
  case TCALL_RINGBACK:
    jstate = CJournalEntry::JS_PENDING;
    break;
  case TCALL_CONNECTED:
    jstate = CJournalEntry::JS_CONNECTED;
    break;
  case TCALL_BUSY:
    jstate = CJournalEntry::JS_BUSY;
    break;
  case TCALL_DISCONNECTED:
    jstate = CJournalEntry::JS_DISCONNECTED;
    break;
  case TCALL_ONHOLD:
  case TCALL_CONFERENCED:
    jstate = CJournalEntry::JS_ONHOLD;
    break;
  }
  return jstate;
}

// ----------------------------------------------------------------------------
// Plug-in call-backs
// ----------------------------------------------------------------------------

// NOTE: Possibly on other threads, therefore GUI-Update must be done by
//       posting an event.

void CMainApp::newCall(unsigned pid, unsigned cid, TCallType type, TCallStatus status)
{
  TEventInfoNewCall info;
  wxThreadEvent event( wxEVT_THREAD, NEW_CALL_EVENT );
  event.SetInt(pid);
  info.cid    = cid;
  info.type   = type;
  info.status = status;
  event.SetPayload(info);
  wxQueueEvent(this, event.Clone());
}

void CMainApp::callStatus(unsigned pid, unsigned cid, TCallStatus status)
{
  TEventInfoCallStatus info;
  wxThreadEvent event( wxEVT_THREAD, UPDATE_CALL_STATE_EVENT );
  event.SetInt(pid);
  info.cid    = cid;
  info.status = status;
  event.SetPayload(info);
  wxQueueEvent(this, event.Clone());
}

void CMainApp::callInfo(unsigned pid, unsigned cid, const TCallInfo& cinf)
{
  TEventInfoCallInfo info;
  wxThreadEvent event( wxEVT_THREAD, UPDATE_CALL_INFO_EVENT );
  event.SetInt(pid);
  info.cid  = cid;
  info.info = cinf;
  event.SetPayload(info);
  wxQueueEvent(this, event.Clone());
}

void CMainApp::resolverResponse(unsigned cid, unsigned data, const CContact& contact)
{
  wxThreadEvent event( wxEVT_THREAD, RESOLVER_RESPONSE );
  event.SetInt(cid);
  event.SetExtraLong(data);
  event.SetPayload(contact);
  wxQueueEvent(this, event.Clone());
}
// ----------------------------------------------------------------------------
// Call-back-triggered events (now on the GUI thread)
// ----------------------------------------------------------------------------

enum {
  RESOLVE_CALLER = 1,
  RESOLVE_CALLED = 2
};

void CMainApp::OnNewCall(wxThreadEvent& event)
{
  unsigned pid = event.GetInt();
  TEventInfoNewCall info = event.GetPayload<TEventInfoNewCall>();
  CJournalEntry *pCall = new CJournalEntry(pid, info.cid, CallType2JournalType(info.type));
  pCall->setState(CallState2JournalState(info.status));
  m_pJournalModel->insertUpdateEntry(*pCall);
  m_ActiveCalls.insert(std::make_pair(TJournalKey(pid, info.cid), pCall));
  displayNotification(*pCall);
}

void CMainApp::OnCallStateUpdate(wxThreadEvent& event)
{
  unsigned pid = event.GetInt();
  TEventInfoCallStatus info = event.GetPayload<TEventInfoCallStatus>();
  TJournalMap::iterator it = m_ActiveCalls.find(TJournalKey(pid, info.cid));
  if (it != m_ActiveCalls.end()) {
    if (info.status == TCALL_IDLE) {
      delete (it->second);
      m_ActiveCalls.erase(it);
    } else {
      it->second->setState(CallState2JournalState(info.status));
      m_pJournalModel->insertUpdateEntry(*(it->second));
    }
  }
}

void CMainApp::OnCallInfoUpdate(wxThreadEvent& event)
{
  unsigned pid = event.GetInt();
  TEventInfoCallInfo info = event.GetPayload<TEventInfoCallInfo>();
  TJournalMap::iterator it = m_ActiveCalls.find(TJournalKey(pid, info.cid));
  if (it != m_ActiveCalls.end())
  {
    CJournalEntry *pJ = it->second;
    if (pJ->getCalledName().IsEmpty()) {
      pJ->setCalledName(info.info.m_strCalledName);
    }
    if (pJ->getCallerName().IsEmpty()) {
      pJ->setCallerName(info.info.m_strCallerName);
    }
    if (pJ->getCalledAddress().IsEmpty() && !info.info.m_strCalledAddress.empty()) {
      ResolveCalled(info.info.m_strCalledAddress, pJ);
    }
    if (pJ->getCallerAddress().IsEmpty() && !info.info.m_strCallerAddress.empty()) {
      ResolveCaller(info.info.m_strCallerAddress, pJ);
    }
    m_pJournalModel->insertUpdateEntry(*pJ);
  }
}

void CMainApp::ResolveCaller(const std::string& toResolve, CJournalEntry *pJ)
{
  if (!pJ->isResolvingCaller())
  {
    wxString strFormattedNr, strE164Nr;
    CPhone::TNumberKind kind = CPhone::parseAndFormatForResolver(
        toResolve, strFormattedNr, strE164Nr);
    // Set journal entry to formatted number
    pJ->setCallerAddress(strFormattedNr);
    // lookup the caller's number if we haven't done it yet
    if (pJ->getCallerName().IsEmpty() && !strFormattedNr.IsEmpty())
    {
      pJ->setResolvingCaller();
      // We don't want to resolve internal or international numbers or
      // callers that originate from the local connection to be resolved
      // online
      bool bOnlineResolvable = ((pJ->getType() != CJournalEntry::J_OUTGOING) &&
                                (kind != CPhone::INTERNAL) &&
                                (kind != CPhone::INTERNATIONAL));
      m_pResolverModel->resolve(pJ->getDbKey(), strFormattedNr,
                                strE164Nr, bOnlineResolvable,
                                RESOLVE_CALLER);
    }
  }
}

void CMainApp::ResolveCalled(const std::string& toResolve, CJournalEntry *pJ)
{
  if (!pJ->isResolvingCalled())
  {
    wxString strFormattedNr, strE164Nr;
    CPhone::TNumberKind kind = CPhone::parseAndFormatForResolver(
        toResolve, strFormattedNr, strE164Nr);
    // Set journal entry to formatted number
    pJ->setCalledAddress(strFormattedNr);
    // lookup the caller's number if we haven't done it yet
    if (pJ->getCalledName().IsEmpty() && !strFormattedNr.IsEmpty())
    {
      pJ->setResolvingCalled();
      // We don't want to resolve internal, local called or international
      // numbers to be resolved online
      bool bOnlineResolvable = ((pJ->getType() != CJournalEntry::J_INCOMING) &&
                                (kind != CPhone::INTERNAL) &&
                                (kind != CPhone::INTERNATIONAL));
      m_pResolverModel->resolve(pJ->getDbKey(), strFormattedNr,
                                strE164Nr, bOnlineResolvable,
                                RESOLVE_CALLED);
    }
  }
}

void CMainApp::OnResolverResponse(wxThreadEvent& event)
{
  unsigned key = event.GetInt();
  unsigned data = event.GetExtraLong();
  CContact c = event.GetPayload<CContact>();
  CJournalEntry *pE = findActiveEntry(key);
  if (pE != NULL) {
    if (data == RESOLVE_CALLER) {
      pE->setCallerName(c.getSN());
      if (pE->getType() == CJournalEntry::J_INCOMING) {
        pE->setImage(c.getImage());
      }
    } else if (data == RESOLVE_CALLED) {
      pE->setCalledName(c.getSN());
      if (pE->getType() == CJournalEntry::J_OUTGOING) {
        pE->setImage(c.getImage());
      }
    }
    m_pJournalModel->insertUpdateEntry(*pE);
  }
  else {
    CJournalEntry e;
    if (findJournalEntry(key, e)) {
      if (data == RESOLVE_CALLER) e.setCallerName(c.getSN());
      if (data == RESOLVE_CALLED) e.setCalledName(c.getSN());
      m_pJournalModel->insertUpdateEntry(e);
    }
  }
}
