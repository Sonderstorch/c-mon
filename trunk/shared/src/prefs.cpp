
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

#include "stdwx.h"
#include "glob/prefs.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator 
#endif

// --------------------------------------------------------------------

static CGlobalPreferences *gpGlobalPrefs = 0;

CGlobalPreferences *GetGlobalPrefs()
{
  if (gpGlobalPrefs == 0) gpGlobalPrefs = new CGlobalPreferences();
  return gpGlobalPrefs;
}

// --------------------------------------------------------------------

CGeneralPrefs::CGeneralPrefs()
{
  m_bStartHidden = false;
  m_nSelectedPage = 3;
  m_bExtNrByLAC = false;
  m_bExtNrByLength = false;
  m_nIntNrLength = 0;
  m_bAddACIfShortLen = false;
  m_nLocalNrMaxLen = 0;
  m_nExtNrChangeMode = PREF_EXTNR_CHANGE_MODE_NOCHANGE;
  m_bDelJournalByAge = false;
  m_nMaxJournalAge = 365;
  m_bDelJournalByCount = false;
  m_nMaxJournalCount = 500;
}

void CGeneralPrefs::load(wxConfig *pC)
{
  wxString strCC  = wxString::FromUTF8(m_strCC.c_str(), m_strCC.length());
  wxString strAC  = wxString::FromUTF8(m_strAC.c_str(), m_strAC.length());
  wxString strLAC = wxString::FromUTF8(m_strLAC.c_str(), m_strLAC.length());
  wxString strAB; m_uuidAB.toString(strAB);

  pC->Read(wxT("StartHidden"),        &m_bStartHidden);
  pC->Read(wxT("SelectedPage"),       &m_nSelectedPage);
  pC->Read(wxT("Callprovider"),       &m_strCP);
  pC->Read(wxT("Addressbook"),        &strAB);
  pC->Read(wxT("CC"),                 &strCC);
  pC->Read(wxT("AC"),                 &strAC);
  pC->Read(wxT("AddACIfShortLen"),    &m_bAddACIfShortLen);
  pC->Read(wxT("LocalNrMaxLen"),      &m_nLocalNrMaxLen);
  pC->Read(wxT("LAC"),                &strLAC);
  pC->Read(wxT("ExtNrByLAC"),         &m_bExtNrByLAC);
  pC->Read(wxT("ExtNrByLength"),      &m_bExtNrByLength);
  pC->Read(wxT("IntNrLength"),        &m_nIntNrLength);
  pC->Read(wxT("ExtNrChangeModeOut"), &m_nExtNrChangeMode);
  pC->Read(wxT("DelJournalByAge"),    &m_bDelJournalByAge);
  pC->Read(wxT("DelJournalByCount"),  &m_bDelJournalByCount);
  pC->Read(wxT("MaxJournalAge"),      &m_nMaxJournalAge);
  pC->Read(wxT("MaxJournalCount"),    &m_nMaxJournalCount);

  m_strCC  = strCC.ToUTF8().data();
  m_strAC  = strAC.ToUTF8().data();
  m_strLAC = strLAC.ToUTF8().data();
  m_uuidAB.fromString(strAB);
}

void CGeneralPrefs::save(wxConfig *pC)
{
  wxString strCC  = wxString::FromUTF8(m_strCC.c_str(), m_strCC.length());
  wxString strAC  = wxString::FromUTF8(m_strAC.c_str(), m_strAC.length());
  wxString strLAC = wxString::FromUTF8(m_strLAC.c_str(), m_strLAC.length());
  wxString strAB; m_uuidAB.toString(strAB);

  pC->Write(wxT("StartHidden"),        m_bStartHidden);
  pC->Write(wxT("SelectedPage"),       m_nSelectedPage);
  pC->Write(wxT("Callprovider"),       m_strCP);
  pC->Write(wxT("Addressbook"),        strAB);
  pC->Write(wxT("CC"),                 strCC);
  pC->Write(wxT("AC"),                 strAC);
  pC->Write(wxT("AddACIfShortLen"),    m_bAddACIfShortLen);
  pC->Write(wxT("LocalNrMaxLen"),      m_nLocalNrMaxLen);
  pC->Write(wxT("LAC"),                strLAC);
  pC->Write(wxT("ExtNrByLAC"),         m_bExtNrByLAC);
  pC->Write(wxT("ExtNrBYLength"),      m_bExtNrByLength);
  pC->Write(wxT("IntNrLength"),        m_nIntNrLength);
  pC->Write(wxT("ExtNrChangeModeOut"), m_nExtNrChangeMode);
  pC->Write(wxT("DelJournalByAge"),    m_bDelJournalByAge);
  pC->Write(wxT("DelJournalByCount"),  m_bDelJournalByCount);
  pC->Write(wxT("MaxJournalAge"),      m_nMaxJournalAge);
  pC->Write(wxT("MaxJournalCount"),    m_nMaxJournalCount);
}

// --------------------------------------------------------------------

CGlobalPreferences::CGlobalPreferences()
{
  wxFileName fn;
  m_pConfig = new wxConfig(wxT("c'mon"));
  fn.Assign(wxStandardPaths::Get().GetUserLocalDataDir(), wxEmptyString);
  if (!fn.DirExists()) fn.Mkdir(wxS_DEFAULT);
  readPrefs();
  if (m_GeneralPrefs.getCC().empty()) {
    TCHAR chBuf[16];
    if (::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME,
                        chBuf, sizeof(chBuf)/sizeof(TCHAR)) > 0) {
      wxString strCC = chBuf; strCC.MakeUpper();
      m_GeneralPrefs.setCC(strCC.ToStdString());
    }
  }
}

CGlobalPreferences::~CGlobalPreferences()
{
  savePrefs();
  delete m_pConfig;
}

void CGlobalPreferences::readUILayout(wxString& strInfo)
{
  strInfo = m_pConfig->Read(wxT("GUI"));
}

void CGlobalPreferences::writeUILayout(const wxString& strInfo)
{
  m_pConfig->Write(wxT("GUI"), strInfo);
}

bool CGlobalPreferences::readFrameSize(wxRect& rect)
{
  bool bSuccess = m_pConfig->Exists(wxT("Window"));
  if (bSuccess) {
    // override with stored size
    bool bSuccess1 = m_pConfig->Read(wxT("Window/PosX"), &rect.x);
    bool bSuccess2 = m_pConfig->Read(wxT("Window/PosY"), &rect.y);
    bool bSuccess3 = m_pConfig->Read(wxT("Window/Width"), &rect.width);
    bool bSuccess4 = m_pConfig->Read(wxT("Window/Height"), &rect.height);
    bSuccess = bSuccess1 && bSuccess2 && bSuccess3 && bSuccess4;
  }
  // TODO: Determine if stored position is still on current screen!
  // wxSize scr = wxGetDisplaySize();
  return bSuccess;
}

void CGlobalPreferences::writeFrameSize(const wxRect& rect)
{
  m_pConfig->Write(wxT("Window/PosX"), rect.x);
  m_pConfig->Write(wxT("Window/PosY"), rect.y);
  m_pConfig->Write(wxT("Window/Width"), rect.width);
  m_pConfig->Write(wxT("Window/Height"), rect.height);
}

CGeneralPrefs& CGlobalPreferences::getPrefs()
{
  return m_GeneralPrefs;
}

void CGlobalPreferences::readPrefs()
{
  m_GeneralPrefs.load(m_pConfig);
}

void CGlobalPreferences::savePrefs()
{
  m_GeneralPrefs.save(m_pConfig);
}

std::string CGlobalPreferences::getProfileFileName(const std::string& fn,
                                                   const std::string ext)
{
  wxFileName f;
  f.Assign(wxStandardPaths::Get().GetUserLocalDataDir(),
      wxString::FromUTF8(fn.c_str(), fn.length()),
      wxString::FromUTF8(ext.c_str(), ext.length()));
  return f.GetFullPath().c_str().AsChar();
}

std::string CGlobalPreferences::getDBFileName()
{
  wxFileName fn;
  fn.Assign(wxStandardPaths::Get().GetUserLocalDataDir(), wxT("cmon"), wxT("db"));
  return fn.GetFullPath().c_str().AsChar();
}

wxString CGlobalPreferences::getLogFileName()
{
  wxFileName fn;
  fn.Assign(wxStandardPaths::Get().GetUserLocalDataDir(), wxT("cmon"), wxT("log"));
  return fn.GetFullPath();
}
