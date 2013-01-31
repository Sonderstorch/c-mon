#ifndef PSQL_AB_CONNECTION_H_INCLUDED
#define PSQL_AB_CONNECTION_H_INCLUDED

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

#include "soci.h"
#include "soci-postgresql.h"

#include <string>
#include <vector>

class CPSqlConnection
{
public:
  CPSqlConnection();
  virtual ~CPSqlConnection();

  bool connect();
  void disconnect();

  const std::string& getLastError() const { return m_strLastError; }

  unsigned getPort() const {   return m_nPort;  }
  void setPort(unsigned port) { m_nPort = port; }

  std::string getDatabase() const { return m_strDatabase; }
  void setDatabase(std::string strDatabase) {  m_strDatabase = strDatabase; }

  std::string getHost() const { return m_strHost; }
  void setHost(std::string strHost) {  m_strHost = strHost; }

  std::string getUser() const { return m_strUser; }
  void setUser(std::string strUser) { m_strUser = strUser; }

  std::string getPass() const { return m_strPassword; }
  void setPass(std::string strPass) { m_strPassword = strPass; }

  bool createSchema(const std::string& adminDb,
                    const std::string& adminUser,
                    const std::string& adminPwd);

  std::string getSchema();

protected:
  std::string replaceKeywords(const std::string& strCmd);

protected:
  std::string   m_strHost;
  unsigned      m_nPort;
  std::string   m_strDatabase;
  std::string   m_strUser;
  std::string   m_strPassword;
  std::string   m_strLastError;
  soci::session m_session;
  std::vector<std::string> m_adminCmds;
  std::vector<std::string> m_userCmds;
};

#endif /* PSQL_AB_CONNECTION_H_INCLUDED */
