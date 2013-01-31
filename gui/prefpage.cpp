
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

// $Revision: 46 $ $Date: 2012-11-18 18:55:31 +0100 (Sun, 18 Nov 2012) $

#include "stdwx.h"

#include "glob/prefs.h"
#include "glob/contact.h"

#include "prefpage.h"
#include "mainapp.h"
#include "resources.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#include <string>
#include <set>

#define wxLOG_COMPONENT "GUI"

using namespace std;

// --------------------------------------------------------------------------

void CPrefsPage::UIData::fromPrefs()
{
  CGeneralPrefs& p = wxGetApp().getPrefs();
  strCC            = p.getCC();
  strAC            = p.getAC();
  bAddACIfShortLen = p.addACIfShortLen();
  nLocalNrMaxLen   = p.getLocalNrMaxLen();
  strLAC           = p.getLAC();
  bExtNrByLAC      = p.isExternalByLAC();
  bExtNrByLen      = p.isExternalByLen();
  nIntNrLen        = p.getInternalNrLen();
  nNrChangeMode    = p.getExternalNrChangeMode();
  bDelJournalByAge = p.getDelJournalByAge();
  nMaxJournalAge   = p.getMaxJournalAge();
  bDelJournalByCount = p.getDelJournalByCount();
  nMaxJournalCount = p.getMaxJournalCount();
}

void CPrefsPage::UIData::toPrefs()
{
  CGeneralPrefs& p = wxGetApp().getPrefs();
  p.setCC(strCC.ToStdString());
  p.setAC(strAC.ToStdString());
  p.setACIfShortLen(bAddACIfShortLen);
  p.setLocalNrMaxLen(nLocalNrMaxLen);
  p.setLAC(strLAC.ToStdString());
  p.setExternalByLAC(bExtNrByLAC);
  p.setExternalByLen(bExtNrByLen);
  p.setInternalNrLen(nIntNrLen);
  p.setExternalNrChangeMode(nNrChangeMode);
  p.setDelJournalByAge(bDelJournalByAge);
  p.setMaxJournalAge(nMaxJournalAge);
  p.setDelJournalByCount(bDelJournalByCount);
  p.setMaxJournalCount(nMaxJournalCount);
}

// --------------------------------------------------------------------------

#define LEFT_COLUMN_SIZE 200

enum {
  ID_EDIT_CC,
  ID_EDIT_AC,
  ID_CHECK_LOCAL_NR_LEN,
  ID_EDIT_LOCAL_NR_LEN,
  ID_EDIT_LAC,
  ID_LAC_MODE,
  ID_CHECK_NR_START,
  ID_CHECK_NR_LEN,
  ID_EDIT_NR_LEN,
  ID_CHECK_JOURNAL_AGE,
  ID_SPIN_JOURNAL_AGE,
  ID_CHECK_JOURNAL_COUNT,
  ID_SPIN_JOURNAL_COUNT
};

BEGIN_EVENT_TABLE(CPrefsPage, wxScrolled<wxPanel>)
  EVT_CHOICE(ID_EDIT_CC,                  CPrefsPage::OnChange)
  EVT_TEXT(ID_EDIT_AC,                    CPrefsPage::OnChange)
  EVT_TEXT(ID_EDIT_LOCAL_NR_LEN,          CPrefsPage::OnChange)
  EVT_TEXT(ID_EDIT_LAC,                   CPrefsPage::OnChange)
  EVT_TEXT(ID_EDIT_NR_LEN,                CPrefsPage::OnChange)
  EVT_CHOICE(ID_LAC_MODE,                 CPrefsPage::OnChange)
  EVT_CHECKBOX(ID_CHECK_LOCAL_NR_LEN,     CPrefsPage::OnChange)
  EVT_CHECKBOX(ID_CHECK_NR_START,         CPrefsPage::OnChange)
  EVT_CHECKBOX(ID_CHECK_NR_LEN,           CPrefsPage::OnChange)
  EVT_CHECKBOX(ID_CHECK_JOURNAL_COUNT,    CPrefsPage::OnChange)
  EVT_CHECKBOX(ID_CHECK_JOURNAL_AGE,      CPrefsPage::OnChange)
  EVT_TEXT(ID_SPIN_JOURNAL_AGE,           CPrefsPage::OnChange)
  EVT_TEXT(ID_SPIN_JOURNAL_COUNT,         CPrefsPage::OnChange)
  EVT_SPINCTRL(ID_SPIN_JOURNAL_AGE,       CPrefsPage::OnChangeSpin)
  EVT_SPINCTRL(ID_SPIN_JOURNAL_COUNT,     CPrefsPage::OnChangeSpin)
  EVT_TIMER(wxID_ANY,                     CPrefsPage::OnTimer)
  EVT_UPDATE_UI_RANGE(ID_LAC_MODE, ID_EDIT_NR_LEN,  CPrefsPage::OnUpdateDialrules)
  EVT_UPDATE_UI(ID_EDIT_LOCAL_NR_LEN,               CPrefsPage::OnUpdateLocalNrLen)
  EVT_UPDATE_UI(ID_SPIN_JOURNAL_AGE,                CPrefsPage::OnUpdateJournalAge)
  EVT_UPDATE_UI(ID_SPIN_JOURNAL_COUNT,              CPrefsPage::OnUpdateJournalCount)
END_EVENT_TABLE()


CPrefsPage::CPrefsPage(wxWindow* parent, wxWindowID id)
  : wxScrolled<wxPanel>( parent, id, wxDefaultPosition, wxDefaultSize,
                          wxTAB_TRAVERSAL|wxNO_BORDER|wxVSCROLL )
{
  m_Timer.SetOwner(this, id);
  m_bInitialUpdate = true;
  m_Data.fromPrefs();

  set<string> regions;
  CPhone::getSupportedRegions(&regions);

  wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxStaticText *txt = new wxStaticText(this, wxID_ANY, _("Location"));
  txt->SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_BOLD));
  bSizer1->Add(txt, 0, wxLEFT|wxRIGHT|wxTOP, 5);
  bSizer1->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
      wxDefaultSize, wxHORIZONTAL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  wxBoxSizer *bSizer = new wxBoxSizer(wxHORIZONTAL);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("Country Code:"),
                    wxDefaultPosition, wxSize(LEFT_COLUMN_SIZE, -1),
                    wxALIGN_RIGHT), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_choiceCC = new wxChoice(this, ID_EDIT_CC,
      wxDefaultPosition, wxDefaultSize, 0, NULL, 0,
      wxGenericValidator(&m_Data.strCC));
  for (set<string>::const_iterator it = regions.begin(); it != regions.end(); ++it) {
    wxString region = wxString(*it);
    m_choiceCC->Append(region);
  }
  m_choiceCC->SetToolTip(_("The international country code of your country"));
  bSizer->Add(m_choiceCC, 0, wxALL, 5);
  bSizer1->Add(bSizer, 0, wxEXPAND, 0);

  bSizer = new wxBoxSizer(wxHORIZONTAL);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("Area Code:"),
                    wxDefaultPosition, wxSize(LEFT_COLUMN_SIZE, -1),
                    wxALIGN_RIGHT), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_edtAC = new wxTextCtrl(this, ID_EDIT_AC,
      wxEmptyString, wxDefaultPosition, wxDefaultSize, 0,
      wxGenericValidator(&m_Data.strAC));
  m_edtAC->SetToolTip(_("The access code for your city/area without\n"
                        "the national dialing prefix (e.g 202 for Washington, DC)"));
  m_edtAC->SetMaxLength(15);
  bSizer->Add(m_edtAC, 0, wxALL, 5);
  bSizer1->Add(bSizer, 0, wxEXPAND, 0);

  bSizer = new wxBoxSizer(wxHORIZONTAL);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("Add area code, if:"),
                    wxDefaultPosition, wxSize(LEFT_COLUMN_SIZE, -1),
                    wxALIGN_RIGHT), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_chkLocalNrLength = new wxCheckBox(this, ID_CHECK_LOCAL_NR_LEN,
      _("Nr. has less than"),
      wxDefaultPosition, wxDefaultSize, 0,
      wxGenericValidator(&m_Data.bAddACIfShortLen));
  bSizer->Add(m_chkLocalNrLength, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  m_edtLocalNrLength = new wxTextCtrl(this, ID_EDIT_LOCAL_NR_LEN, wxEmptyString,
      wxDefaultPosition, wxSize(50, -1), 0,
      wxGenericValidator(&m_Data.nLocalNrMaxLen));
  bSizer->Add(m_edtLocalNrLength, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("digits"),
                    wxDefaultPosition, wxDefaultSize, 0),
                    0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  bSizer1->Add( bSizer, 0, wxEXPAND, 0);

  txt = new wxStaticText(this, wxID_ANY, _("External calls"));
  txt->SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_BOLD));
  bSizer1->Add(txt, 0, wxLEFT|wxRIGHT|wxTOP, 5);
  bSizer1->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
      wxDefaultSize, wxHORIZONTAL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  bSizer = new wxBoxSizer(wxHORIZONTAL);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("Line access code:"),
                    wxDefaultPosition, wxSize(LEFT_COLUMN_SIZE, -1),
                    wxALIGN_RIGHT), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_edtLAC = new wxTextCtrl(this, ID_EDIT_LAC,
      wxEmptyString, wxDefaultPosition, wxDefaultSize, 0,
      wxGenericValidator(&m_Data.strLAC));
  m_edtLAC->SetToolTip(_("The code to get an outside line (behind a PBX)"));
  bSizer->Add(m_edtLAC, 0, wxALL, 5);
  bSizer1->Add(bSizer, 0, wxEXPAND, 0);

  bSizer = new wxBoxSizer( wxHORIZONTAL );
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("External call, if:"),
                    wxDefaultPosition, wxSize(LEFT_COLUMN_SIZE, -1),
                    wxALIGN_RIGHT), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_chkNrStart = new wxCheckBox(this, ID_CHECK_NR_START,
      _("Nr. starts with line access code"),
      wxDefaultPosition, wxDefaultSize, 0,
      wxGenericValidator(&m_Data.bExtNrByLAC));
  bSizer->Add(m_chkNrStart, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  bSizer1->Add(bSizer, 0, wxEXPAND, 5);

  bSizer = new wxBoxSizer(wxHORIZONTAL);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    wxEmptyString, wxDefaultPosition,
                    wxSize(LEFT_COLUMN_SIZE, -1), 0), 0, wxALL, 5);
  m_chkNrLength = new wxCheckBox(this, ID_CHECK_NR_LEN,
      _("Nr. has more than"),
      wxDefaultPosition, wxDefaultSize, 0,
      wxGenericValidator(&m_Data.bExtNrByLen));
  bSizer->Add(m_chkNrLength, 0, wxALL, 5);
  m_edtNrLength = new wxTextCtrl(this, ID_EDIT_NR_LEN, wxEmptyString,
      wxDefaultPosition, wxSize(50, -1), 0,
      wxGenericValidator(&m_Data.nIntNrLen));
  bSizer->Add(m_edtNrLength, 0, wxALL, 5);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("digits"),
                    wxDefaultPosition, wxDefaultSize, 0),
                    0, wxALL, 5);
  bSizer1->Add(bSizer, 0, wxEXPAND, 5);

  wxString choices[] = {
      _("leave number unchanged"),
      _("add line access code"),
      _("remove line access code")
  };

  bSizer = new wxBoxSizer(wxHORIZONTAL);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                      _("On outgoing external call:"),
                      wxDefaultPosition, wxSize(LEFT_COLUMN_SIZE, -1),
                      wxALIGN_RIGHT), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxChoice *pCombo = new wxChoice(this, ID_LAC_MODE,
      wxDefaultPosition, wxDefaultSize,
      sizeof(choices)/sizeof(choices[0]), choices, 0,
      wxGenericValidator(&m_Data.nNrChangeMode));
  bSizer->Add(pCombo, 0, wxALL, 5);
  bSizer1->Add(bSizer, 0, wxEXPAND, 5);

  txt = new wxStaticText(this, wxID_ANY, _("Journal"));
  txt->SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_BOLD));
  bSizer1->Add(txt, 0, wxLEFT|wxRIGHT|wxTOP, 5);
  bSizer1->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
      wxDefaultSize, wxHORIZONTAL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  bSizer = new wxBoxSizer( wxHORIZONTAL );
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("Delete entries, when:"),
                    wxDefaultPosition, wxSize(LEFT_COLUMN_SIZE, -1),
                    wxALIGN_RIGHT), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_chkJournalAge = new wxCheckBox(this, ID_CHECK_JOURNAL_AGE,
      _("They are older than"),
      wxDefaultPosition, wxDefaultSize, 0,
      wxGenericValidator(&m_Data.bDelJournalByAge));
  bSizer->Add(m_chkJournalAge, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  m_spinJournalAge = new wxSpinCtrl(this, ID_SPIN_JOURNAL_AGE, wxEmptyString,
      wxDefaultPosition, wxSize(50, -1), 0, 1, 3650);
  m_spinJournalAge->SetValidator(wxGenericValidator(&m_Data.nMaxJournalAge));
  bSizer->Add(m_spinJournalAge, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("days"),
                    wxDefaultPosition, wxDefaultSize, 0),
                    0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  bSizer1->Add(bSizer, 0, wxEXPAND, 5);

  bSizer = new wxBoxSizer( wxHORIZONTAL );
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    wxEmptyString, wxDefaultPosition,
                    wxSize(LEFT_COLUMN_SIZE, -1), 0), 0, wxALL, 5);
  m_chkJournalCount = new wxCheckBox(this, ID_CHECK_JOURNAL_AGE,
      _("Journal has more than"),
      wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_VERTICAL,
      wxGenericValidator(&m_Data.bDelJournalByCount));
  bSizer->Add(m_chkJournalCount, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  m_spinJournalCount = new wxSpinCtrl(this, ID_SPIN_JOURNAL_COUNT, wxEmptyString,
      wxDefaultPosition, wxSize(50, -1), 0, 10, 999999);
  m_spinJournalCount->SetValidator(wxGenericValidator(&m_Data.nMaxJournalCount));
  bSizer->Add(m_spinJournalCount, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  bSizer->Add(new wxStaticText(this, wxID_ANY,
                    _("entries"),
                    wxDefaultPosition, wxDefaultSize, 0),
                    0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  bSizer1->Add(bSizer, 0, wxEXPAND, 5);

  TransferDataToWindow();

  SetSizerAndFit( bSizer1 );

  Update();

  m_bInitialUpdate = false;
}

CPrefsPage::~CPrefsPage()
{
}

// --------------------------------------------------------------------------

void CPrefsPage::OnUpdateDialrules(wxUpdateUIEvent& e)
{
  bool enable = !m_edtLAC->GetValue().IsEmpty();
  if (e.GetId() == ID_EDIT_NR_LEN) {
    enable = enable && m_chkNrLength->IsChecked();
  }
  e.Enable(enable);
}

void CPrefsPage::OnUpdateLocalNrLen(wxUpdateUIEvent& e)
{
  e.Enable(m_chkLocalNrLength->IsChecked());
}

void CPrefsPage::OnUpdateJournalAge(wxUpdateUIEvent& e)
{
  e.Enable(m_chkJournalAge->IsChecked());
}

void CPrefsPage::OnUpdateJournalCount(wxUpdateUIEvent& e)
{
  e.Enable(m_chkJournalCount->IsChecked());
}

// --------------------------------------------------------------------------

void CPrefsPage::DoChange()
{
  CGeneralPrefs& currPrefs = wxGetApp().getPrefs();
  CGeneralPrefs oldPrefs = currPrefs;
  TransferDataFromWindow();
  m_Data.toPrefs();
  if (!(oldPrefs == currPrefs)) {
    wxGetApp().getGlobalPrefs().savePrefs();
  }
}

void CPrefsPage::OnChange(wxCommandEvent& e)
{
  if (!m_bInitialUpdate) {
    m_Timer.Start(250, wxTIMER_ONE_SHOT);
  }
}

void CPrefsPage::OnChangeSpin(wxSpinEvent& e)
{
  m_Timer.Start(250, wxTIMER_ONE_SHOT);
}

void CPrefsPage::OnTimer(wxTimerEvent& e)
{
  DoChange();
}
