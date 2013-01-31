
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

#include "stdwx.h"

#include "soci.h"
#include "soci-sqlite3.h"

#include "glob/prefs.h"
#include "db/backend.h"
#include "journal/journalmodel.h"
#include "mainapp.h"

#include <sstream>
#include <iomanip>
#include <string>

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI/JOURNAL"

using namespace soci;
using namespace std;

// ---------------------------------------------------------------------------
// CJournalModel
// ---------------------------------------------------------------------------

CJournalModel::CJournalModel(CDBBackend *pDB)
  : m_nCurrentFilter(-1)
  , m_nSortOrder(J_SORT_TIME)
  , m_bSortAsc(false)
  , m_pDB(pDB)
{
  stringstream ss;

  ss << "(type == " << (int)CJournalEntry::J_INCOMING << ")";
  m_Filters.push_back(new CJournalFilter(_("Incoming"),
      ss.str(), CJournalFilter::FT_BUILTIN_INCOMING));

  ss.str(""); ss.clear();
  ss << "(type == " << (int)CJournalEntry::J_OUTGOING << ")";
  m_Filters.push_back(new CJournalFilter(_("Outgoing"),
      ss.str(), CJournalFilter::FT_BUILTIN_OUTGOING));

  ss.str(""); ss.clear();
  ss << "(type == " << (int)CJournalEntry::J_INCOMING
     << " and res == " << (int)CJournalEntry::JR_NOTREACHED
     << ")";
  m_Filters.push_back(new CJournalFilter(_("Calls in absence"),
      ss.str(), CJournalFilter::FT_BUILTIN_CALLS_IN_ABSENCE));

  m_Filters.push_back(new CJournalFilter(_("All calls"), ""));

  m_Filters.push_back(new CJournalFilter(_("Today"),
      "(date(tm1) == date('now'))"));
  m_Filters.push_back(new CJournalFilter(_("This month"),
      "(strftime('%Y%m', tm1) == strftime('%Y%m', 'now'))"));
  m_Filters.push_back(new CJournalFilter(_("This week"),
      "(strftime('%Y%W', tm1) == strftime('%Y%W', 'now'))"));
//
//  m_Filters.push_back(new CJournalFilter(_("Older than limit"),
//      "((julianday('now') - julianday(tm1)) > 5)"));

  updateFilters();
}

CJournalModel::~CJournalModel()
{
  for (size_t i = 0; i < m_Filters.size(); ++i) delete m_Filters[i];
}

void CJournalModel::registerListener(IJournalListener *p)
{
  m_Listeners.push_back(p);
}

void CJournalModel::unregisterListener(IJournalListener *p)
{
  std::vector<IJournalListener*>::iterator it = m_Listeners.begin();
  while (it != m_Listeners.end()) {
    if (*it == p) {
      m_Listeners.erase(it);
      return;
    }
    it++;
  }
}

void CJournalModel::notifyListeners()
{
  std::vector<IJournalListener*>::iterator it = m_Listeners.begin();
  for (; it != m_Listeners.end(); ++it) {
    (*it)->OnJournalUpdate();
  }
}

void CJournalModel::setNameAndNumberFilter(const wxString& strName, const wxString& strNumber)
{
  std::string strNameUTF8   = strName.ToUTF8().data();
  std::string strNumberUTF8 = strNumber.ToUTF8().data();

  m_strNameAndNumberCond = "";

  if (strNameUTF8.length()) {
    m_strNameAndNumberCond += "(( cln like '%" + strNameUTF8 + "%') ";
    m_strNameAndNumberCond += "or (cdn like '%" + strNameUTF8 + "%')) ";
  }
  if (strNumberUTF8.length()) {
    if (m_strNameAndNumberCond.length()) {
      m_strNameAndNumberCond += " and ";
    }
    m_strNameAndNumberCond += "(( clnr like '%" + strNumberUTF8 + "%') ";
    m_strNameAndNumberCond += "or (cdnr like '%" + strNumberUTF8 + "%')) ";
  }

  updateFilters();
  notifyListeners();
}

void CJournalModel::selectFilter(int idx)
{
  if ((idx >= 0) && (idx < m_Filters.size())) {
    m_nCurrentFilter = idx;
  }
  notifyListeners();
}

void CJournalModel::disableFilter()
{
  m_nCurrentFilter = -1;
  notifyListeners();
}

void CJournalModel::updateFilters()
{
  if (!m_pDB->isOk()) return;

  soci::session& session = m_pDB->getSession();

  string strStatement;
  int count;
  TJournalFilterList::iterator it = m_Filters.begin();
  for (; it != m_Filters.end(); ++it)
  {
    count = 0;
    string strCond = (*it)->getCondition();
    if (m_strNameAndNumberCond.length()) {
      if (strCond.length()) {
        strCond += " and ";
      }
      strCond += m_strNameAndNumberCond;
    }
    strStatement = "select count(*) from calls";
    if (strCond.length()) {
      strStatement += " where " + strCond;
    }
    try {
      session << strStatement, into(count);
    }
    catch (const soci_error& e) {
      m_strLastError = e.what();
      wxLogMessage(wxString::Format(wxT("Error %s"), m_strLastError));
    }
    catch (const std::exception& e) {
      m_strLastError = e.what();
      wxLogMessage(wxString::Format(wxT("Error %s"), m_strLastError));
    }
    (*it)->setMatchedCount(count);
  }
}

void CJournalModel::selectSortOrder(TJournalSortOrder sf, bool bAsc)
{
  m_nSortOrder = sf;
  m_bSortAsc   = bAsc;
  notifyListeners();
}

void CJournalModel::insertUpdateEntry(CJournalEntry& j)
{
  if (j.isDirty())
  {
    if (m_pDB->isOk())
    {
      int dbkey    = (int) j.getDbKey();
      int type     = (int) j.getType();
      int result   = (int) j.getResult();
      int duration = (int) j.getDurationInSec().GetValue();
      std::string strTM1;
      if (j.getCreation().IsValid()) {
        strTM1 = j.getCreation().FormatISOCombined().c_str();
      }
      std::string strTM2;
      if (j.getBegin().IsValid()) {
        strTM2 = j.getBegin().FormatISOCombined().c_str();
      }
      std::string strClnr = j.getCallerAddress().ToAscii().data();
      std::string strCln  = j.getCallerName().ToUTF8().data();
      std::string strCdnr = j.getCalledAddress().ToAscii().data();
      std::string strCdn  = j.getCalledName().ToUTF8().data();

      bool bPurge = false; // do purge only after inserts

      soci::session& session = m_pDB->getSession();
      try
      {
        session.begin();
        if (dbkey == 0) {
          session << "select value from config where kind='nextcallid' limit 1", into(dbkey);
          size_t next_id = dbkey + 1;
          session << "update config set value = :val where kind='nextcallid'", use(next_id);
          j.setDbKey(dbkey);
          session << "insert into calls (id,type,res,tm1,tm2,dur,clnr,cln,cdnr,cdn) "
                     "values (:id,:type,:res,:tm1,:tm2,:dur,:clnr,:cln,:cdnr,:cdn)",
                     use(dbkey), use(type), use(result), use(strTM1), use(strTM2),
                     use(duration), use(strClnr), use(strCln), use(strCdnr), use(strCdn);
          bPurge = true;
        } else {
          session << "update calls set type=:type,res=:res,tm1=:tm1,tm2=:tm2,"
                     "dur=:dur,clnr=:clnr,cln=:cln,cdnr=:cdnr,cdn=:cdn "
                     "where id == :id",
                     use(type), use(result), use(strTM1), use(strTM2), use(duration),
                     use(strClnr), use(strCln), use(strCdnr), use(strCdn), use(dbkey);
        }
        session.commit();
        j.setDirty(false);
        if (bPurge) purgeEntries();
        updateFilters();
        notifyListeners();
      }
      catch (const std::exception& e) {
        m_strLastError = e.what();
        wxLogMessage(wxString::Format(wxT("Error %s"), m_strLastError));
        session.rollback();
      }
    }
    else {
      j.setDirty(false);
    }
  }
}

bool CJournalModel::getEntry(long long key, CJournalEntry& e)
{
  bool rc = false;
  if (m_pDB->isOk())
  {
    int dbkey, type, result, duration;
    std::string strTM1, strTM2;
    std::string strClnr, strCln, strCdnr, strCdn;
    indicator i_result, i_duration, i_tm1, i_tm2, i_clnr, i_cln, i_cdnr, i_cdn;
    soci::session& session = m_pDB->getSession();
    try
    {
      wxDateTime    dt;
      wxString      str;

      session << "select type,res,tm1,tm2,dur,clnr,cln,cdnr,cdn from calls where id = :key",
          into(type), into(result, i_result),
          into(strTM1, i_tm1), into(strTM2, i_tm2),
          into(duration, i_duration),
          into(strClnr, i_clnr), into(strCln, i_cln),
          into(strCdnr, i_cdnr), into(strCdn, i_cdn), use(key);

      if (session.got_data())
      {
        e.setDbKey(key);
        e.setType(static_cast<CJournalEntry::TType>(type));
        e.setResult(static_cast<CJournalEntry::TResult>((i_result == i_ok) ? result : 0));
        e.setDurationInSec((i_duration == i_ok) ? duration : 0);

        dt = wxInvalidDateTime;
        if (i_tm1 == i_ok) {
          str = wxString::FromAscii(strTM1.c_str());
          dt.ParseISOCombined(str);
        }
        e.setCreation(dt);

        dt = wxInvalidDateTime;
        if (i_tm2 == i_ok) {
          str = wxString::FromAscii(strTM2.c_str());
          dt.ParseISOCombined(str);
        }
        e.setBegin(dt);

        str.Clear();
        if (i_clnr == i_ok) {
          str = wxString::FromAscii(strClnr.c_str(), strClnr.length());
        }
        e.setCallerAddress(str);

        str.Clear();
        if (i_cln == i_ok) {
          str = wxString::FromUTF8(strCln.c_str(), strCln.length());
        }
        e.setCallerName(str);

        str.Clear();
        if (i_cdnr == i_ok) {
          str = wxString::FromAscii(strCdnr.c_str(), strCdnr.length());
        }
        e.setCalledAddress(str);

        str.Clear();
        if (i_cdn == i_ok) {
          str = wxString::FromUTF8(strCdn.c_str(), strCdn.length());
        }
        e.setCalledName(str);

        e.setDirty(false);

        rc = true;
      }
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
  return rc;
}

void CJournalModel::getEntries(TJournalList& list)
{
  if (m_pDB->isOk())
  {
    int dbkey, type, result, duration;
    std::string strTM1, strTM2;
    std::string strClnr, strCln, strCdnr, strCdn;
    indicator i_result, i_duration, i_tm1, i_tm2, i_clnr, i_cln, i_cdnr, i_cdn;
    soci::session& session = m_pDB->getSession();
    try
    {
      stringstream  ss;
      wxDateTime    dt;
      wxString      str;
      CJournalEntry e;
//      ss << "select id,type,res,tm1,tm2,dur,clnr,cln,cdnr,cdn from calls";
      ss << "select id,type,res,tm1,tm2,dur,clnr,cln,cdnr,cdn from calls";
      if (m_strNameAndNumberCond.length() || (m_nCurrentFilter != -1)) {
        string strCond;
        if (m_nCurrentFilter != -1) {
          strCond = m_Filters[m_nCurrentFilter]->getCondition();
        }
        if (m_strNameAndNumberCond.length()) {
          if (strCond.length()) {
            strCond += " and ";
          }
          strCond += m_strNameAndNumberCond;
        }
        if (strCond.length()) {
          ss << " where " << strCond;
        }
      }

      ss << " order by ";
      switch (m_nSortOrder) {
      case J_SORT_TYPE:
        ss << "type ";
        break;
      case J_SORT_TIME:
        ss << "tm1 ";
        break;
      case J_SORT_CALLER_NAME:
        ss << "cln ";
        break;
      case J_SORT_CALLER_NR:
        ss << "clnr ";
        break;
      case J_SORT_CALLED_NR:
        ss << "cdnr ";
        break;
      case J_SORT_DURATION:
        ss << "dur ";
        break;
      case J_SORT_RESULT:
        ss << "res ";
        break;
      };
      ss << (m_bSortAsc ? "asc" : "desc");
      wxLogDebug("getEntries: %s", ss.str().c_str());
      statement st = (session.prepare << ss.str(),
            into(dbkey), into(type), into(result, i_result),
            into(strTM1, i_tm1), into(strTM2, i_tm2),
            into(duration, i_duration),
            into(strClnr, i_clnr), into(strCln, i_cln),
            into(strCdnr, i_cdnr), into(strCdn, i_cdn));
      st.execute();
      list.clear();
      while(st.fetch())
      {
        e.setDbKey(dbkey);
        e.setType(static_cast<CJournalEntry::TType>(type));
        e.setResult(static_cast<CJournalEntry::TResult>((i_result == i_ok) ? result : 0));
        e.setDurationInSec((i_duration == i_ok) ? duration : 0);

        dt = wxInvalidDateTime;
        if (i_tm1 == i_ok) {
          str = wxString::FromAscii(strTM1.c_str());
          dt.ParseISOCombined(str);
        }
        e.setCreation(dt);

        dt = wxInvalidDateTime;
        if (i_tm2 == i_ok) {
          str = wxString::FromAscii(strTM2.c_str());
          dt.ParseISOCombined(str);
        }
        e.setBegin(dt);

        str.Clear();
        if (i_clnr == i_ok) {
          str = wxString::FromAscii(strClnr.c_str(), strClnr.length());
        }
        e.setCallerAddress(str);

        str.Clear();
        if (i_cln == i_ok) {
          str = wxString::FromUTF8(strCln.c_str(), strCln.length());
        }
        e.setCallerName(str);

        str.Clear();
        if (i_cdnr == i_ok) {
          str = wxString::FromAscii(strCdnr.c_str(), strCdnr.length());
        }
        e.setCalledAddress(str);

        str.Clear();
        if (i_cdn == i_ok) {
          str = wxString::FromUTF8(strCdn.c_str(), strCdn.length());
        }
        e.setCalledName(str);

        e.setDirty(false);

        list.push_back(e);
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

void  CJournalModel::purgeEntries()
{
  bool bAge     = wxGetApp().getPrefs().getDelJournalByAge();
  bool bCount   = wxGetApp().getPrefs().getDelJournalByCount();

  if (!(bAge || bCount)) return;

  int maxAge   = wxGetApp().getPrefs().getMaxJournalAge();
  int maxCount = wxGetApp().getPrefs().getMaxJournalCount();

  if (m_pDB->isOk())
  {
    soci::session& session = m_pDB->getSession();
    try
    {
      session.begin();
      if (bAge) {
        // 'use(maxAge)' didn't work here ?!? ...
        stringstream  ss;
        ss << "delete from calls where ((julianday('now') - julianday(tm1)) > " << maxAge << ")";
        session << ss.str();
      }
      if (bCount) {
        session << "delete from calls where id not in (select id from calls order by id desc limit :m)", use(maxCount);
      }
      session.commit();
    }
    catch (const std::exception& e) {
      m_strLastError = e.what();
      wxLogMessage(wxString::Format(wxT("Error %s"), m_strLastError));
      session.rollback();
    }
  }
}
