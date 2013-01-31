#ifndef PSQL_AB_H_INCLUDED
#define PSQL_AB_H_INCLUDED

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

#include "soci.h"
#include "soci-postgresql.h"

#include "glob/contact.h"
#include "plugin/addressbook.h"

class CPSQLAddressbook : public CAddressbookApi
{
public:
  CPSQLAddressbook(CGUID id);
  virtual ~CPSQLAddressbook();

  bool isOk() const { return m_is_ok; }
  const std::string& getLastError() const { return m_strLastError; }

  CGUID    getID() const;
  unsigned getType() const;
  const wxString& getName() const;

  void     getEntries(TContactList& list);
  void     insertUpdateEntry(CContact& contact);
  void     deleteEntry(const CContact& contact);

  void     selectSortOrder(TContactSortOrder sf, bool bAsc);

  void     setNameAndNumberFilter(const wxString& strName,
                                  const wxString& strNumber);

  bool     resolve(const wxString& strNumber, CContact& resolvedContact);

protected:
  void insertUpdatePhone(CPhone& phone);

protected:
  CGUID                 m_id;
  bool                  m_is_ok;
  soci::connection_pool m_conn_pool;
  std::string           m_strLastError;
  std::string           m_strNameFilter;
  std::string           m_strNumberFilter;
  TContactSortOrder     m_nSortOrder;
  bool                  m_bSortAsc;
  wxString              m_strName;
};

#endif /* PSQL_AB_H_INCLUDED */
