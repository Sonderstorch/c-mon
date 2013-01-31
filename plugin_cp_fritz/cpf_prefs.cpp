
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
#include "cpf_prefs.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator 
#endif

// --------------------------------------------------------------------


wxConfig                         *CPFRITZPREFS::m_pConfig = NULL;
CCpFritzPrefs                     CPFRITZPREFS::m_Prefs;
std::vector<IPrefChangeListener*> CPFRITZPREFS::m_Listeners;

// --------------------------------------------------------------------

CCpFritzPrefs::CCpFritzPrefs()
{
  m_strFritzAddr = wxT("fritz.box");
}

std::string CCpFritzPrefs::getFritzAddr()
{
  return m_strFritzAddr.ToStdString();
}

// --------------------------------------------------------------------

void CPFRITZPREFS::init()
{
  m_pConfig = new wxConfig(wxT("c'mon"));
  readPrefs();
}

void CPFRITZPREFS::free()
{
  delete m_pConfig;
}

void CPFRITZPREFS::registerListener(IPrefChangeListener *l)
{
  m_Listeners.push_back(l);
}

void CPFRITZPREFS::unregisterListener(IPrefChangeListener *l)
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

void CPFRITZPREFS::notify()
{
  for (size_t i = 0; i < m_Listeners.size(); ++i) {
    m_Listeners[i]->OnPrefChange();
  }
}

void CPFRITZPREFS::readPrefs()
{
  m_pConfig->Read(wxT("Fritz/Address"), &m_Prefs.m_strFritzAddr, m_Prefs.m_strFritzAddr);
}

void CPFRITZPREFS::savePrefs()
{
  m_pConfig->Write(wxT("Fritz/Address"), m_Prefs.m_strFritzAddr);
}

CCpFritzPrefs& CPFRITZPREFS::getPrefs()
{
  return m_Prefs;
}
