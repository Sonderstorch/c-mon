
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

// $Revision: 15 $ $Date: 2012-09-23 19:45:41 +0200 (Sun, 23 Sep 2012) $

#include "stdwx.h"
#include "psql_ab_connection.h"

#include <string>
#include <sstream>

#include <boost/algorithm/string.hpp>

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

using namespace soci;
using namespace std;

// ----------------------------------------------------------------------------

CPSqlConnection::CPSqlConnection()
  : m_nPort(0)
{
  m_adminCmds.push_back(
      "SET client_encoding = 'UTF8';\n"
      "SET check_function_bodies = false;\n"
      "SET client_min_messages = warning;\n"
      "SET search_path = public, pg_catalog;\n"
      "SET default_tablespace = '';\n"
      "SET default_with_oids = false;\n"
  );
  m_adminCmds.push_back(
      "DROP DATABASE IF EXISTS <DBNAME>;\n"
  );
  m_adminCmds.push_back(
      "DROP ROLE IF EXISTS <DBUSER>;\n"
      "CREATE ROLE <DBUSER> WITH LOGIN PASSWORD '<DBPWD>' CREATEDB;\n"
  );
  m_adminCmds.push_back(
      "CREATE DATABASE <DBNAME> WITH OWNER = <DBUSER> ENCODING = 'UTF8';\n"
  );
  m_adminCmds.push_back(
      "GRANT ALL PRIVILEGES ON DATABASE <DBNAME> to <DBUSER>;\n"
  );
  m_userCmds.push_back(
      "CREATE SEQUENCE contacts_id_seq;\n"
         "CREATE TABLE contacts (\n"
             "   id      integer DEFAULT nextval('contacts_id_seq') NOT NULL,\n"
             "   fn      varchar(60),\n"
             "   mn      varchar(60),\n"
             "   ln      varchar(60),\n"
             "   nn      varchar(60),\n"
             "   sn      varchar(150),\n"
             "   title   varchar(60),\n"
             "   org     varchar(60),\n"
             "   email   varchar(60),\n"
             "   picture text,\n"
             "   CONSTRAINT contacts_pkey PRIMARY KEY (id)\n"
             ");\n"
         "CREATE INDEX contacts_idx_001 ON contacts(fn);\n"
         "CREATE SEQUENCE numbers_id_seq;\n"
         "CREATE TABLE numbers (\n"
             "   id          integer DEFAULT nextval('numbers_id_seq') NOT NULL,\n"
             "   cref        integer NOT NULL,\n"
             "   type        integer NOT NULL,\n"
             "   canonical   varchar(30) NOT NULL,\n"
             "   note        varchar(60),\n"
             "   CONSTRAINT numbers_pkey PRIMARY KEY (id),\n"
             "   FOREIGN KEY(cref) REFERENCES contacts(id) ON DELETE CASCADE\n"
             ");\n"
         "CREATE INDEX numbers_idx_001 ON numbers(canonical);\n"
  );

}

CPSqlConnection::~CPSqlConnection()
{
  disconnect();
}

bool CPSqlConnection::connect()
{
  bool rc = false;

  stringstream ss;

  ss << "host="       << m_strHost
     << " port="      << m_nPort
     << " dbname="    << m_strDatabase;
  if (!m_strUser.empty() && !m_strPassword.empty()) {
     ss << " user="      << m_strUser
        << " password="  << m_strPassword;
  }
  ss << " options='-c client_encoding=UTF8'";

  disconnect();

  try
  {
    int count = 0;
    m_session.open(postgresql, ss.str().c_str());
    m_session << "select count(*) from contacts", into(count);
    m_session << "select count(*) from numbers", into(count);
    rc = true;
  }
  catch (const soci_error& e) {
    m_strLastError = e.what();
  }
  catch (const std::exception& e) {
    m_strLastError = e.what();
  }
  return rc;
}

void CPSqlConnection::disconnect()
{
  m_session.close();
}

bool CPSqlConnection::createSchema(const std::string& adminDb,
                                   const std::string& adminUser,
                                   const std::string& adminPwd)
{
  bool rc = false;

  disconnect();

  stringstream ss;

  try
  {
    // connect as admin and issue admin commands
    ss << "host="       << m_strHost
       << " port="      << m_nPort
       << " dbname="    << adminDb
       << " user="      << adminUser
       << " password="  << adminPwd;
    m_session.open(postgresql, ss.str().c_str());
    vector<string>::const_iterator it;
    for (it = m_adminCmds.begin(); it != m_adminCmds.end(); ++it) {
      m_session << replaceKeywords(*it);
    }
    m_session.close();
    // connect as db owner and issue owner commands
    ss.str(""); ss.clear();
    ss << "host="       << m_strHost
       << " port="      << m_nPort
       << " dbname="    << m_strDatabase;
    if (!m_strUser.empty() && !m_strPassword.empty()) {
       ss << " user="      << m_strUser
          << " password="  << m_strPassword;
    }
    m_session.open(postgresql, ss.str().c_str());
    for (it = m_userCmds.begin(); it != m_userCmds.end(); ++it) {
      m_session << replaceKeywords(*it);
    }
    m_session.close();
    rc = true;
  }
  catch (const soci_error& e) {
    m_strLastError = e.what();
  }
  catch (const std::exception& e) {
    m_strLastError = e.what();
  }
  return rc;
}

string CPSqlConnection::replaceKeywords(const std::string& strCmd)
{
  string cmd = strCmd;
  boost::replace_all(cmd, "<DBNAME>", m_strDatabase);
  boost::replace_all(cmd, "<DBUSER>", m_strUser);
  if (!m_strPassword.empty()) {
    boost::replace_all(cmd, "<DBPWD>", m_strPassword);
  }
  else {
    boost::replace_all(cmd, "<DBPWD>", "NULL");
  }
  return cmd;
}

string CPSqlConnection::getSchema()
{
  string schema;
  vector<string>::const_iterator it;
  for (it = m_adminCmds.begin(); it != m_adminCmds.end(); ++it) {
    schema += *it;
    schema += "\n";
  }
  for (it = m_userCmds.begin(); it != m_userCmds.end(); ++it) {
    schema += *it;
    schema += "\n";
  }
  return schema;
}
