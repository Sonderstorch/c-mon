
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
#include "sqlite_ab_prefs.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator 
#endif

// --------------------------------------------------------------------

wxConfig                         *SQLITEPREFS::m_pConfig = NULL;
std::vector<IPrefChangeListener*> SQLITEPREFS::m_Listeners;
std::map<wxString, wxString>      SQLITEPREFS::m_DbFiles;

// --------------------------------------------------------------------

void SQLITEPREFS::init()
{
  m_pConfig = new wxConfig(wxT("c'mon"));
  readDatabases();
}

void SQLITEPREFS::free()
{
  delete m_pConfig;
}

void SQLITEPREFS::registerListener(IPrefChangeListener *l)
{
  m_Listeners.push_back(l);
}

void SQLITEPREFS::unregisterListener(IPrefChangeListener *l)
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

void SQLITEPREFS::notify()
{
  for (size_t i = 0; i < m_Listeners.size(); ++i) {
    m_Listeners[i]->OnPrefChange();
  }
}

void SQLITEPREFS::getDbFiles(TDbFiles& list)
{
  list = m_DbFiles;
}

void SQLITEPREFS::setDbFiles(const TDbFiles& list)
{
  m_DbFiles = list;
  writeDatabases();
}

void SQLITEPREFS::readDatabases()
{
  wxString strKey, strValue;
  long idx = 0;

  m_DbFiles.clear();

  m_pConfig->SetPath(wxT("Addressbooks/SQLite"));
  if (m_pConfig->GetFirstEntry(strKey, idx)) {
    do {
      if (m_pConfig->Read(strKey, &strValue)) {
        m_DbFiles[strKey] = strValue;
      }
    } while(m_pConfig->GetNextEntry(strKey, idx));
  }
  m_pConfig->SetPath(wxT("../.."));
}

void SQLITEPREFS::writeDatabases()
{
  wxString strKey;
  long idx = 0;

  m_pConfig->DeleteGroup(wxT("Addressbooks/SQLite"));

  TDbFiles::const_iterator it = m_DbFiles.begin();
  while (it != m_DbFiles.end()) {
    strKey = wxString::Format(wxT("Addressbooks/SQLite/%s"), it->first);
    m_pConfig->Write(strKey, it->second);
    it++;
  }
}
