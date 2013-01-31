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

// $Revision: 40 $ $Date: 2012-10-18 23:19:11 +0200 (Thu, 18 Oct 2012) $

#include "stdwx.h"
#include "glob/uuid.h"

CGUID::CGUID()
{
  UuidCreate(&m_UUID);
}

CGUID::CGUID(const UUID& u)
{
  m_UUID = u;
}

CGUID::CGUID(const CGUID& u)
{
  memcpy(&m_UUID, &u, sizeof(m_UUID));
}

CGUID::CGUID(const wxString& str)
{
  fromString(str);
}

bool CGUID::operator< (const CGUID& u) const
{
  return ( memcmp(&m_UUID, &u, sizeof(UUID)) < 0 );
}

bool CGUID::operator== (const CGUID& u) const
{
  return ( memcmp(&m_UUID, &u, sizeof(UUID)) == 0 );
}

void CGUID::operator= (const CGUID& u)
{
  memcpy(&m_UUID, &u, sizeof(m_UUID));
}

void CGUID::fromString(const wxString& str)
{
  UUID gid;
#ifdef UNICODE
  RPC_STATUS rc = UuidFromString((RPC_WSTR)str.wc_str(), &gid);
#else
  RPC_STATUS rc = UuidFromString((RPC_CSTR)str.c_str(), &gid);
#endif
  if (rc == RPC_S_OK) {
    m_UUID = gid;
  } else {
    memset(&m_UUID, 0, sizeof(m_UUID));
  }
}

void CGUID::toString(wxString& str)
{
#ifdef UNICODE
  wchar_t* szGUID = 0;
  RPC_WSTR *pszGUID = (RPC_WSTR *)&szGUID;
#else
  char * szGUID = 0;
  RPC_CSTR *pszGUID = (RPC_CSTR *)&szGUID;
#endif
  RPC_STATUS rc = UuidToString(&m_UUID, pszGUID);
  if (rc == RPC_S_OK) {
    str = szGUID;
    RpcStringFree(pszGUID);
  } else {
    str.Clear();
  }
}
