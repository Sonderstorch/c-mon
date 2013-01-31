#include "stdwx.h"
#include "cpt_prefs.h"

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

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator 
#endif

// --------------------------------------------------------------------


wxConfig                         *CPTPREFS::m_pConfig = NULL;
CCptPrefs                         CPTPREFS::m_Prefs;
std::vector<IPrefChangeListener*> CPTPREFS::m_Listeners;

// --------------------------------------------------------------------

CCptPrefs::CCptPrefs()
{
}

// --------------------------------------------------------------------

void CPTPREFS::init()
{
  m_pConfig = new wxConfig(wxT("c'mon"));
  readPrefs();
}

void CPTPREFS::free()
{
  delete m_pConfig;
}

void CPTPREFS::registerListener(IPrefChangeListener *l)
{
  m_Listeners.push_back(l);
}

void CPTPREFS::unregisterListener(IPrefChangeListener *l)
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

void CPTPREFS::notify()
{
  for (size_t i = 0; i < m_Listeners.size(); ++i) {
    m_Listeners[i]->OnPrefChange();
  }
}

void CPTPREFS::readLines()
{
  unsigned nLineId;
  wxString strGroup, strKey;
  long idx = 0;

  m_pConfig->SetPath(wxT("Tapi/Lines"));
  size_t nCount = m_pConfig->GetNumberOfEntries();
  m_Prefs.m_vSelectedLines.clear();
  m_Prefs.m_vSelectedLines.reserve(nCount);
  for (size_t i = 0; i < nCount; ++i) {
    strKey = wxString::Format(wxT("%02d"), i);
    if (m_pConfig->Read(strKey, (long *)&nLineId)) {
      m_Prefs.m_vSelectedLines.push_back(nLineId);
    }
  }
  m_pConfig->SetPath(wxT("../.."));
}

void CPTPREFS::saveLines()
{
  wxString strKey;
  m_pConfig->DeleteGroup(wxT("Tapi/Lines"));
  for (size_t i = 0; i < m_Prefs.m_vSelectedLines.size(); ++i) {
    strKey = wxString::Format(wxT("Tapi/Lines/%02d"), i);
    m_pConfig->Write(strKey, m_Prefs.m_vSelectedLines[i]);
  }
}

void CPTPREFS::readPrefs()
{
  readLines();
}

void CPTPREFS::savePrefs()
{
  saveLines();
}

CCptPrefs& CPTPREFS::getPrefs()
{
  return m_Prefs;
}
