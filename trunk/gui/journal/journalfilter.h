#ifndef JOURNALFILTER_H_INCLUDED
#define JOURNALFILTER_H_INCLUDED

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

#include <vector>
#include <string>

// ----------------------------------------------------------------------------

class CJournalFilter
{
public:
  enum TFilterType {
    FT_BUILTIN_INCOMING,
    FT_BUILTIN_OUTGOING,
    FT_BUILTIN_CALLS_IN_ABSENCE,
    FT_USERDEFINED
  };

public:
  CJournalFilter();
  CJournalFilter(const wxString& name,
                 const std::string& strCondition,
                 TFilterType type = FT_USERDEFINED);

  virtual ~CJournalFilter();

  const wxString&  getName() const { return m_strName; }
  TFilterType      getType() const { return m_Type; }
  size_t           getMatchedCount();

protected:
  void                setMatchedCount(size_t count);
  const std::string&  getCondition() const { return m_strCondition; }

protected:
  wxString    m_strName;
  std::string m_strCondition;
  TFilterType m_Type;
  size_t      m_nCount;

  friend class CJournalModel;
};
typedef std::vector<CJournalFilter *> TJournalFilterList;

#endif /* JOURNALFILTER_H_INCLUDED */
