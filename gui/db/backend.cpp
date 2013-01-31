
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

#include "stdwx.h"

#include "soci.h"
#include "soci-sqlite3.h"

#include "backend.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

using namespace soci;
using namespace std;

#define wxLOG_COMPONENT "GUI/DB"

// ---------------------------------------------------------------------------
// CDBBackend
// ---------------------------------------------------------------------------

CDBBackend::CDBBackend(const std::string& strDBFile)
  : m_is_ok(false)
{
  try
  {
    m_session.open(soci::sqlite3, strDBFile);
    try {
      int schema_version, count;
      m_session << "select value from config where kind='schema' limit 1", into(schema_version);
      m_session << "select count(*) from calls", into(count);
      m_is_ok = true;
    }
    catch (std::exception const &e) {
      m_session.begin();
      m_session << "drop table if exists config";
      m_session << "create table config ("
                << "value INTEGER NOT NULL,"
                << "kind TEXT NOT NULL)";
      m_session << "insert into config (value, kind) values (1, 'nextcallid')";
      m_session << "insert into config (value, kind) values (1, 'schema')";
      m_session << "drop table if exists calls";
      m_session << "create table calls ("
                << "id INTEGER NOT NULL,"
                << "type INTEGER NOT NULL,"
                << "res INTEGER,"
                << "tm1 TEXT,"
                << "tm2 TEXT,"
                << "dur INTEGER,"
                << "clnr TEXT,"
                << "cln TEXT,"
                << "cdnr TEXT,"
                << "cdn TEXT)";
      m_session << "drop table if exists resolver";
      m_session << "create table resolver ("
                << "name INTEGER NOT NULL,"
                << "type INTEGER,"
                << "pos INTEGER,"
                << "enabled INTEGER,"
                << "v1 TEXT,"
                << "v2 TEXT,"
                << "v3 TEXT,"
                << "v4 TEXT,"
                << "v5 TEXT)";
      m_session.commit();
      m_is_ok = true;
    }
  }
  catch (const std::exception& e) {
    m_strLastError = e.what();
    m_session.rollback();
  }
}

CDBBackend::~CDBBackend()
{
  m_session.close();
}
