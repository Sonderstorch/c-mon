
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
#include "psql_ab_prefs.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator 
#endif

// --------------------------------------------------------------------


wxConfig                         *PSQLPREFS::m_pConfig = NULL;
CPSqlPrefs                        PSQLPREFS::m_Prefs;
std::vector<IPrefChangeListener*> PSQLPREFS::m_Listeners;

// --------------------------------------------------------------------

CPSqlPrefs::CPSqlPrefs()
{
  m_strHost = wxT("PSqlServer");
  m_nHostPort = 5432;
  m_bUseOSUser = true;
}

// --------------------------------------------------------------------

void PSQLPREFS::init()
{
  m_pConfig = new wxConfig(wxT("c'mon"));
  readPrefs();
}

void PSQLPREFS::free()
{
  delete m_pConfig;
}

void PSQLPREFS::registerListener(IPrefChangeListener *l)
{
  m_Listeners.push_back(l);
}

void PSQLPREFS::unregisterListener(IPrefChangeListener *l)
{
  std::vector<IPrefChangeListener*>::iterator it = m_Listeners.begin();
  while (it != m_Listeners.end()) {
    if (*it == l) {
      m_Listeners.erase(it);
      return;
    }
    it++;
  }
}

void PSQLPREFS::notify()
{
  for (size_t i = 0; i < m_Listeners.size(); ++i) {
    m_Listeners[i]->OnPrefChange();
  }
}

void PSQLPREFS::readPrefs()
{
  m_pConfig->Read(wxT("Addressbooks/PostgreSQL/Host"),
    &m_Prefs.m_strHost, m_Prefs.m_strHost);
  m_pConfig->Read(wxT("Addressbooks/PostgreSQL/Port"),
    (long*)&m_Prefs.m_nHostPort, (long)m_Prefs.m_nHostPort);
  m_pConfig->Read(wxT("Addressbooks/PostgreSQL/Database"),
    &m_Prefs.m_strDatabase, m_Prefs.m_strDatabase);
  m_pConfig->Read(wxT("Addressbooks/PostgreSQL/UseOSUser"),
    &m_Prefs.m_bUseOSUser, m_Prefs.m_bUseOSUser);
  m_pConfig->Read(wxT("Addressbooks/PostgreSQL/Username"),
    &m_Prefs.m_strUsername, m_Prefs.m_strUsername);
  m_pConfig->Read(wxT("Addressbooks/PostgreSQL/Password"),
    &m_Prefs.m_strPassword, m_Prefs.m_strPassword);
}

void PSQLPREFS::savePrefs()
{
  m_pConfig->Write(wxT("Addressbooks/PostgreSQL/Host"),      m_Prefs.m_strHost);
  m_pConfig->Write(wxT("Addressbooks/PostgreSQL/Port"),      m_Prefs.m_nHostPort);
  m_pConfig->Write(wxT("Addressbooks/PostgreSQL/Database"),  m_Prefs.m_strDatabase);
  m_pConfig->Write(wxT("Addressbooks/PostgreSQL/UseOSUser"), m_Prefs.m_bUseOSUser);
  m_pConfig->Write(wxT("Addressbooks/PostgreSQL/Username"),  m_Prefs.m_strUsername);
  m_pConfig->Write(wxT("Addressbooks/PostgreSQL/Password"),  m_Prefs.m_strPassword);
}

CPSqlPrefs& PSQLPREFS::getPrefs()
{
  return m_Prefs;
}
