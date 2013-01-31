
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

#include "soci.h"
#include "soci-sqlite3.h"

#include "db/backend.h"
#include "glob/contact.h"
#include "resolver/resolver.h"
#include "resolver/address.h"
#include "resolver/rexweb.h"
#include "resolver/resolvermodel.h"
#include "mainapp.h"

#include <sstream>
#include <string>

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI/RESOLVER"

using namespace soci;
using namespace std;

// ---------------------------------------------------------------------------
// CResolverModel
// ---------------------------------------------------------------------------

CResolverModel::CResolverModel(CDBBackend *pDB)
  : wxThreadHelper()
{
  m_pDB = pDB;
  loadResolvers();
  m_bTerminate = false;
  CreateThread();
  GetThread()->Run();
}

CResolverModel::~CResolverModel()
{
  clearResolvers();
}

void CResolverModel::shutdown()
{
  if (GetThread() && GetThread()->IsRunning()) {
    m_bTerminate = true;  // self-destruct
    m_semaQ.Post();       // wake thread (just in case)
    GetThread()->Wait();
  }
}

void CResolverModel::clearResolvers()
{
  wxMutexLocker lock(m_mtxResolvers);
  for (TResolverList::iterator it = m_Resolvers.begin(); it != m_Resolvers.end(); ++it) delete (*it);
  m_Resolvers.clear();
}

bool CResolverModel::hasEntry(const wxString strName)
{
  wxMutexLocker lock(m_mtxResolvers);
  TResolverList::iterator it = m_Resolvers.begin();
  for (; it != m_Resolvers.end(); ++it) {
    if ((*it)->getName() == strName) {
      return true;
    }
  }
  return false;
}

bool CResolverModel::prependEntry(CResolver *pR)
{
  wxMutexLocker lock(m_mtxResolvers);
  if (hasEntry(pR->getName())) return false;
  m_Resolvers.push_front(pR);
  TResolverList::iterator it = m_Resolvers.begin();
  for (int pos = 0; it != m_Resolvers.end(); ++it, ++pos) {
    (*it)->setPosition(pos);
  }
  saveEntries();
  return true;
}

bool CResolverModel::appendEntry(CResolver *pR)
{
  wxMutexLocker lock(m_mtxResolvers);
  if (hasEntry(pR->getName())) return false;
  int pos = 0;
  for (TResolverList::iterator it = m_Resolvers.begin(); it != m_Resolvers.end(); ++it) {
    if ((*it)->getPosition() > pos) pos = (*it)->getPosition();
  }
  pR->setPosition(pos);
  m_Resolvers.push_back(pR);
  saveEntries();
  return true;
}

bool CResolverModel::moveUp(size_t idx)
{
  bool rc = false;
  wxMutexLocker lock(m_mtxResolvers);
  if (idx < m_Resolvers.size())
  {
    TResolverList::iterator current = m_Resolvers.begin();
    std::advance(current, idx);
    if (current != m_Resolvers.begin()) {
      TResolverList::iterator newpos = current; --newpos;
      m_Resolvers.insert(newpos, 1, (*current));
      m_Resolvers.erase(current);
      TResolverList::iterator it = m_Resolvers.begin();
      for (int pos = 0; it != m_Resolvers.end(); ++it, ++pos) {
        (*it)->setPosition(pos);
      }
      saveEntries();
      rc = true;
    }
  }
  return rc;
}

bool CResolverModel::moveDown(size_t idx)
{
  bool rc = false;
  wxMutexLocker lock(m_mtxResolvers);
  if (idx < m_Resolvers.size())
  {
    TResolverList::iterator current = m_Resolvers.begin();
    std::advance(current, idx);
    if (current != m_Resolvers.end()) {
      TResolverList::iterator newpos = current; ++newpos; ++newpos;
      m_Resolvers.insert(newpos, 1, (*current));
      m_Resolvers.erase(current);
      TResolverList::iterator it = m_Resolvers.begin();
      for (int pos = 0; it != m_Resolvers.end(); ++it, ++pos) {
        (*it)->setPosition(pos);
      }
      saveEntries();
      rc = true;
    }
  }
  return rc;
}

bool CResolverModel::toggle(size_t idx)
{
  bool rc = false;
  wxMutexLocker lock(m_mtxResolvers);
  if (idx < m_Resolvers.size())
  {
    TResolverList::iterator it = m_Resolvers.begin();
    std::advance(it, idx);
    (*it)->enable(!(*it)->isEnabled());
    saveEntries();
    rc = true;
  }
  return rc;
}

void CResolverModel::loadResolvers()
{
  if (m_pDB->isOk())
  {
    int pos, enabled, type;
    std::string name,v1,v2,v3,v4,v5;
    indicator i_pos,i_enabled,i_type,i_v1,i_v2,i_v3, i_v4,i_v5;
    wxMutexLocker lock(m_mtxResolvers);
    soci::session& session = m_pDB->getSession();
    try
    {
      statement st = (
        session.prepare <<
        "select name,type,pos,enabled,v1,v2,v3,v4,v5 from resolver order by pos",
          into(name), into(type, i_type), into(pos, i_pos), into(enabled, i_enabled),
          into(v1, i_v1), into(v2, i_v2), into(v3, i_v3), into(v4, i_v4), into(v5, i_v5)
      );
      st.execute();
      clearResolvers();
      while(st.fetch())
      {
        if (i_type != i_ok) continue;
        if (type == RT_ADDRESSBOOK) {
          wxString strUUID = wxString::FromUTF8(name.c_str(), name.length());
          CGUID gid(strUUID);
          CAddressbookApi *pBook = wxGetApp().getAddressbook(gid);
          CAddressBookResolver *pR = new CAddressBookResolver(pBook);
          pR->setName(strUUID);
          pR->setPosition((i_pos == i_ok) ? pos : 0);
          pR->enable(((pBook != NULL) && (i_enabled == i_ok)) ? (enabled == 1) : false);
          pR->setDirty(false);
          m_Resolvers.push_back(pR);
        }
        else if (type == RT_ONLINE_REGEX) {
          CRegExWebResolver *pR = new CRegExWebResolver();
          pR->setName(wxString::FromUTF8(name.c_str(), name.length()));
          if (i_v1 == i_ok) pR->setURL(wxString::FromUTF8(v1.c_str(), v1.length()));
          if (i_v2 == i_ok) pR->setRegEx(wxString::FromUTF8(v2.c_str(), v2.length()));
          pR->setPosition((i_pos == i_ok) ? pos : 0);
          pR->enable((i_enabled == i_ok) ? (enabled == 1) : false);
          pR->setDirty(false);
          m_Resolvers.push_back(pR);
        }
      }
      st.clean_up();
    }
    catch (const soci_error& e) {
      m_strLastError = e.what();
      wxLogMessage(wxString::Format(wxT("Error %s"), m_strLastError));
    }
    catch (const std::exception& e) {
      m_strLastError = e.what();
      wxLogMessage(wxString::Format(wxT("Error %s"), m_strLastError));
    }
  }
}


void CResolverModel::saveEntries()
{
  if (!m_pDB->isOk()) return;

  wxMutexLocker lock(m_mtxResolvers);

  soci::session& session = m_pDB->getSession();

  TResolverList::iterator it = m_Resolvers.begin();
  for (; it != m_Resolvers.end(); ++it)
  {
    if (!(*it)->isDirty()) continue;
    std::string name  = (*it)->getName().ToUTF8().data();
    int type          = (*it)->getType();
    int pos           = (*it)->getPosition();
    int enabled       = ((*it)->isEnabled() ? 1 : 0);
    try
    {
      int count = 0;
      session.begin();
      session << "select count(*) from resolver where name=:name", into(count), use(name);
      if ((*it)->getType() == RT_ADDRESSBOOK)
      {
        if (count == 0) {
          session << "insert into resolver (name,type,pos,enabled) "
                     "values (:name,:type,:pos,:enabled)",
                     use(name), use(type), use(pos), use(enabled);
        } else {
          session << "update resolver set type=:type,pos=:pos,enabled=:enabled "
                     "where name = :name",
                     use(type), use(pos), use(enabled), use(name);
        }
      }
      else if ((*it)->getType() == RT_ONLINE_REGEX)
      {
        CRegExWebResolver *pR = reinterpret_cast<CRegExWebResolver *>(*it);
        std::string url   = pR->getURL().ToUTF8().data();
        std::string regex = pR->getRegEx().ToUTF8().data();
        if (count == 0) {
          session << "insert into resolver (name,type,pos,enabled,v1,v2) "
                     "values (:name,:type,:pos,:enabled,:url,:regex)",
                     use(name), use(type), use(pos), use(enabled), use(url), use(regex);
        } else {
          session << "update resolver set type=:type,pos=:pos,enabled=:enabled,v1=:url,v2=:regex "
                     "where name = :name",
                     use(type), use(pos), use(enabled), use(url), use(regex), use(name);
        }
      }
      session.commit();
      (*it)->setDirty(false);
    }
    catch (const std::exception& e) {
      m_strLastError = e.what();
      wxLogMessage(wxString::Format(wxT("Error %s"), m_strLastError));
      session.rollback();
    }
  }
}

bool CResolverModel::deleteEntry(size_t idx)
{
  bool rc = false;

  if (!m_pDB->isOk()) return rc;

  wxMutexLocker lock(m_mtxResolvers);

  TResolverList::iterator it = m_Resolvers.begin();
  std::advance(it, idx);
  const CResolver *pR = (*it);
  soci::session& session = m_pDB->getSession();
  std::string name  = pR->getName().ToUTF8().data();
  try
  {
    session.begin();
    session << "delete from resolver where name=:name ", use(name);
    session.commit();
    m_Resolvers.erase(it);
    delete pR;
    rc = true;
  }
  catch (const std::exception& e) {
    m_strLastError = e.what();
    wxLogMessage(wxString::Format(wxT("Error %s"), m_strLastError));
    session.rollback();
  }
  return rc;
}

size_t CResolverModel::getEntryCount() const
{
  size_t sz = 0;
  {
    wxMutexLocker lock(const_cast<wxMutex&>(m_mtxResolvers));
    sz = m_Resolvers.size();
  }
  return sz;
}

const CResolver *CResolverModel::getEntry(size_t idx) const
{
  CResolver *pR = NULL;
  wxMutexLocker lock(const_cast<wxMutex&>(m_mtxResolvers));
  if (idx < m_Resolvers.size()) {
    TResolverList::const_iterator it = m_Resolvers.begin();
    std::advance(it, idx);
    pR = *it;
  }
  return pR;
}

bool CResolverModel::resolve(const wxString& strNumber,
                             CContact& resolvedContact)
{
  bool rc = false;
  wxMutexLocker lock(m_mtxResolvers);
  TResolverList::iterator it = m_Resolvers.begin();
  for (; it != m_Resolvers.end(); ++it) {
    CResolver *pR = *it;
    if (pR->isEnabled()) {
      wxLogMessage(wxString::Format(wxT("Resolving %s using %s..."),
          strNumber, pR->getName()));
      rc = pR->resolve(strNumber, resolvedContact);
      if (rc) {
        wxLogMessage(wxString::Format(wxT("Success: %s -> %s"),
            strNumber, resolvedContact.getSN()));
        break;
      }
    }
  }
  return rc;
}

void CResolverModel::resolve(unsigned id, const wxString& strNationalNumber,
                             const wxString& strE164Number, bool bInternational,
                             unsigned userdata)
{
  wxCriticalSectionLocker lock(m_csRequestList);
  m_RequestList.push_back(TResolveRequest(id, userdata, strNationalNumber,
                                          strE164Number, bInternational));
  m_semaQ.Post();
}

void CResolverModel::processRequest(const TResolveRequest& req)
{
  wxLogMessage(wxT("RESOLVER: %s / %s -> %sonline resolvable"),
      req.m_strNumberNational, req.m_strNumberE164,
      req.m_bOnlineResolvable ? wxEmptyString : wxT("not "));
  bool bFound = false;
  CContact resolvedContact;
  wxMutexLocker lock(m_mtxResolvers);
  TResolverList::iterator it = m_Resolvers.begin();
  for (; it != m_Resolvers.end(); ++it)
  {
    CResolver *pR = *it;
    if (pR->isEnabled()) {
      bool isOnlineResolver = (pR->getType() == RT_ONLINE_REGEX);
      if (isOnlineResolver) {
        if (!req.m_bOnlineResolvable) continue;
        wxLogMessage(wxString::Format(wxT("Online resolving %s using %s ..."),
            pR->getName(), req.m_strNumberNational));
        bFound = pR->resolve(req.m_strNumberNational, resolvedContact);
      }
      else {
        wxLogMessage(wxString::Format(wxT("Local resolving %s using %s ..."),
            pR->getName(), req.m_strNumberE164));
        bFound = pR->resolve(req.m_strNumberE164, resolvedContact);
      }
      if (bFound) {
        wxGetApp().resolverResponse(req.m_nCallId, req.m_nUserData, resolvedContact);
        break;
      }
    }
  }
}

wxThread::ExitCode CResolverModel::Entry()
{
  // Worker thread function
  TResolveRequest req;
  while (!GetThread()->TestDestroy()) {
    if (m_semaQ.Wait() == wxSEMA_NO_ERROR) {
      if (!m_bTerminate) {
        m_csRequestList.Enter();
        req = m_RequestList.front();
        m_RequestList.pop_front();
        m_csRequestList.Leave();
        processRequest(req);
      }
      else {
        break;
      }
    }
  }
  return (wxThread::ExitCode) 0; // success
}
