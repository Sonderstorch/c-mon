#ifndef JOURNALMODEL_H_INCLUDED
#define JOURNALMODEL_H_INCLUDED

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

#include "journal/journalentry.h"
#include "journal/journalfilter.h"

#include <string>
#include <vector>
#include <map>

// ---

class CDBBackend;

// ---

class IJournalListener
{
public:
  IJournalListener() {};
  virtual ~IJournalListener() {};

  virtual void OnJournalUpdate() = 0;
};

// ---

enum TJournalSortOrder {
  J_SORT_TYPE,
  J_SORT_TIME,
  J_SORT_CALLER_NAME,
  J_SORT_CALLER_NR,
  J_SORT_CALLED_NR,
  J_SORT_DURATION,
  J_SORT_RESULT
};

// ----------------------------------------------------------------------------

class CJournalModel
{
public:
  CJournalModel(CDBBackend *pDB);
  virtual ~CJournalModel();

  const std::string& getLastError() const { return m_strLastError; }

  bool getEntry(long long key, CJournalEntry& entry);
  void getEntries(TJournalList& list);
  void insertUpdateEntry(CJournalEntry& entry);

  void registerListener(IJournalListener *p);
  void unregisterListener(IJournalListener *p);

  const TJournalFilterList& getFilters() const { return m_Filters; }
  void selectFilter(int idx);
  void disableFilter();

  void selectSortOrder(TJournalSortOrder sf, bool bAsc);

  void setNameAndNumberFilter(const wxString& strName, const wxString& strNumber);

protected:
  void notifyListeners();
  void updateFilters();
  void purgeEntries();

protected:
  std::string                     m_strNameAndNumberCond;
  int                             m_nCurrentFilter;
  TJournalSortOrder               m_nSortOrder;
  bool                            m_bSortAsc;
  std::string                     m_strLastError;
  std::vector<IJournalListener *> m_Listeners;
  TJournalFilterList              m_Filters;
  CDBBackend                     *m_pDB;
};

// ----------------------------------------------------------------------------

class TJournalKey
{
  public:
    TJournalKey(unsigned providerId, unsigned callId) {
      m_pid = providerId;
      m_cid = callId;
    }
    bool operator ==(const TJournalKey& r) const {
      return (m_pid == r.m_pid) && (m_cid == r.m_cid);
    }
    bool operator <(const TJournalKey& r) const {
      if(m_pid == r.m_pid) {
        return m_cid < r.m_cid;
      }
      return m_pid < r.m_pid;
    }
  protected:
    unsigned m_pid;
    unsigned m_cid;
};
typedef std::map<TJournalKey, CJournalEntry *> TJournalMap;

// ----------------------------------------------------------------------------

#endif /* JOURNALMODEL_H_INCLUDED */
