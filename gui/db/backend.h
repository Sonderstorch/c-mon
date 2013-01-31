#ifndef BACKEND_H_INCLUDED
#define BACKEND_H_INCLUDED

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

#include "soci.h"
#include <string>

class CDBBackend
{
public:
  CDBBackend(const std::string& strDBFile);
  ~CDBBackend();

  bool isOk() const                       { return m_is_ok;        }
  const std::string& getLastError() const { return m_strLastError; }

  soci::session& getSession() { return m_session; }

protected:
  bool           m_is_ok;
  soci::session  m_session;
  std::string    m_strLastError;
};

#endif /* BACKEND_H_INCLUDED */
