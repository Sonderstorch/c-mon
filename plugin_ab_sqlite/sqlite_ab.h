#ifndef SQLITE_AB_H_INCLUDED
#define SQLITE_AB_H_INCLUDED

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

// $Revision: 14 $ $Date: 2012-09-21 22:37:23 +0200 (Fri, 21 Sep 2012) $

#include "soci.h"
#include "soci-sqlite3.h"

#include "glob/contact.h"
#include "plugin/addressbook.h"

#include <vector>

class CSQLiteAddressbook : public CAddressbookApi
{
public:
  CSQLiteAddressbook(CGUID id, const wxString& strName, const wxString& strPath);
  virtual ~CSQLiteAddressbook();

  bool isOk() const { return m_is_ok; }
  const std::string& getLastError() const { return m_strLastError; }

  CGUID    getID() const;
  unsigned getType() const;
  const wxString& getName() const;

  void getEntries(TContactList& list);
  void insertUpdateEntry(CContact& contact);
  void deleteEntry(const CContact& contact);

  void selectSortOrder(TContactSortOrder sf, bool bAsc);

  void setNameAndNumberFilter(const wxString& strName,
                              const wxString& strNumber);

  bool resolve(const wxString& strNumber, CContact& resolvedContact);

  // ---

  void setName(const wxString& strName);
  const wxString& getPath() const;

  void markForDeletion();
  bool isMarkedForDeletion();

protected:
  void insertUpdatePhone(CPhone& phone);

protected:
  void initSchema();

protected:
  CGUID                 m_id;
  bool                  m_is_ok;
  soci::connection_pool m_conn_pool;
  wxString              m_strName;
  wxString              m_strPath;
  std::string           m_strLastError;
  std::string           m_strNameFilter;
  std::string           m_strNumberFilter;
  TContactSortOrder     m_nSortOrder;
  bool                  m_bSortAsc;
  bool                  m_bMarkedForDeletion;
};

typedef std::vector<CSQLiteAddressbook *> TAddressbooks;

#endif /* SQLITE_AB_H_INCLUDED */
