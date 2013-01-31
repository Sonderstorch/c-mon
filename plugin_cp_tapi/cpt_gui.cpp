
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

// $Revision: 16 $ $Date: 2012-09-23 20:48:06 +0200 (Sun, 23 Sep 2012) $

#include "stdwx.h"

#include "cpt_gui.h"
#include "cpt_monitor.h"
#include "cpt_prefs.h"

#include "glob/controls.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

wxDEFINE_EVENT(EVENT_REFRESH, wxCommandEvent);

// --------------------------------------------------------------------------

enum {
  ID_PROVIDERS,
  ID_NR_CHANGE_MODE,
  ID_EDIT_NR_PREFIX,
  ID_CHECK_NR_START,
  ID_EDIT_NR_START,
  ID_CHECK_NR_LEN,
  ID_EDIT_NR_LEN,
  ID_BTN_STARTSTOP
};

CCptGui::CCptGui(wxWindow* parent, CTapiCallMonitor *pCallMonitor,
                 wxWindowID id, const wxPoint& pos, const wxSize& size,
                 long style )
  : wxPanel( parent, id, pos, size, style )
  , m_pCallMonitor(pCallMonitor)
{
  CCptPrefs& rPrefs = CPTPREFS::getPrefs();

  m_pFont = new wxFont(12, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Calibri"));

  m_pStatusRunning = new CStatusIndicator(this, wxID_ANY,
      wxDefaultPosition, wxSize(32,16));

  wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxStaticText *txt = new wxStaticText(this, wxID_ANY, _("TAPI call monitor"));
  txt->SetFont(*m_pFont);
  bSizer1->Add(txt, 0, wxLEFT|wxRIGHT|wxTOP, 5);
  bSizer1->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
      wxDefaultSize, wxHORIZONTAL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 6, 2, 0, 0 );
  fgSizer1->AddGrowableCol( 1 );
  fgSizer1->SetFlexibleDirection( wxBOTH );
  fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  fgSizer1->Add(new wxStaticText(this, wxID_ANY,
                      _("Monitor running:")), 0,
      wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  fgSizer1->Add(m_pStatusRunning, 0, wxALL, 5);

  fgSizer1->Add(new wxStaticText(this, wxID_ANY,
                      _("TAPI-Provider:")), 0,
      wxALIGN_RIGHT|wxALIGN_TOP|wxALL, 5);

  m_cbProviders = new wxCheckListBox(this, ID_PROVIDERS,
      wxDefaultPosition, wxSize(400, 350));
  fgSizer1->Add(m_cbProviders, 0, wxALL, 5);

  fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
  m_btnStartStop = new wxButton(this, ID_BTN_STARTSTOP, wxT("--"));
  fgSizer1->Add(m_btnStartStop, 0, wxALL, 5);

  bSizer1->Add(fgSizer1, 0, wxEXPAND, 0);

  m_btnStartStop->Connect( ID_BTN_STARTSTOP, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CCptGui::OnStartStop),NULL,this);

  this->SetSizer( bSizer1 );
  this->Layout();

  TransferDataToWindow();

  m_cbProviders->Connect( ID_PROVIDERS, wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,
      wxCommandEventHandler(CCptGui::OnChange),NULL,this);

  m_TimerUpdate.SetOwner(this, 1);
  m_TimerLines.SetOwner(this, 2);

  Bind(wxEVT_TIMER, &CCptGui::OnTimer, this);
  Bind(EVENT_REFRESH, &CCptGui::OnRefresh, this);

  CTafManager::getInstance()->registerObserver(this);

  PopulateLines();
  Update();
}

CCptGui::~CCptGui()
{
  CTafManager::getInstance()->deregisterObserver(this);
  delete m_pFont;
}

// --------------------------------------------------------------------------

void CCptGui::PopulateLines()
{
  TLineList lines;
  CCptPrefs& rPrefs = CPTPREFS::getPrefs();
  m_pCallMonitor->getLines(lines);
  m_cbProviders->Clear();
  const TPrefLines& selectedLines = rPrefs.getSelectedLines();
  for (int i = lines.size()-1; i >= 0; --i) {
    int item = m_cbProviders->Insert(lines[i].strName, 0, (void *)lines[i].lid);
    bool bFound = false;
    for (int j = 0; j < selectedLines.size(); ++j) {
      bFound = (lines[i].lid == selectedLines[j]);
      if (bFound) {
        m_cbProviders->Check(item);
        break;
      }
    }
  }
}

// --------------------------------------------------------------------------

void CCptGui::Update()
{
  bool bRunning = m_pCallMonitor->isRunning();
  m_pStatusRunning->SetOK(bRunning);
  m_cbProviders->Enable(!bRunning);
  m_btnStartStop->SetLabelText(bRunning ? _("Stop Monitor") : _("Start Monitor"));
  m_btnStartStop->Enable(true);
}

// --------------------------------------------------------------------------

void CCptGui::OnChange(wxCommandEvent& e)
{
  CCptPrefs& currPrefs = CPTPREFS::getPrefs();
  CCptPrefs oldPrefs = currPrefs;
  TransferDataFromWindow();
  currPrefs.m_vSelectedLines.clear();
  currPrefs.m_vSelectedLines.reserve(m_cbProviders->GetCount());
  for (int i = 0; i < m_cbProviders->GetCount(); ++i) {
    if (m_cbProviders->IsChecked(i)) {
      unsigned lid = (unsigned) m_cbProviders->GetClientData(i);
      currPrefs.m_vSelectedLines.push_back(lid);
    }
  }
  if (!(oldPrefs == currPrefs)) {
    CPTPREFS::savePrefs();
    CPTPREFS::notify();
  }
}

// --------------------------------------------------------------------------

void CCptGui::OnStartStop(wxCommandEvent& e)
{
  m_btnStartStop->Enable(false);
  bool bRunning = m_pCallMonitor->isRunning();
  if (bRunning) {
    m_pCallMonitor->stop();
  }
  else {
    m_pCallMonitor->start(false);
  }
}

// --------------------------------------------------------------------------

void CCptGui::OnTimer(wxTimerEvent& e)
{
  if (e.GetId() == 1) {
    Update();
  }
  if (e.GetId() == 2) {
    PopulateLines();
  }
}

void CCptGui::OnRefresh(wxCommandEvent& e)
{
  if (e.GetExtraLong() == TAF_EVT_LINES_CHANGED) {
    m_TimerLines.Start(1000, wxTIMER_ONE_SHOT);
  } else {   // TAF_EVT_RUNSTATE_CHANGED
    m_TimerUpdate.Start(1000, wxTIMER_ONE_SHOT);
  }
}

void CCptGui::InitiateUpdate(long info)
{
  wxCommandEvent evt(EVENT_REFRESH);
  evt.SetEventObject(this);
  evt.SetExtraLong(info);
  wxPostEvent(this, evt);
}

void CCptGui::OnTafEvent(ITafEventHandler::ETAFEVENT evt, void *info)
{
  switch (evt)
  {
    case TAF_EVT_LINES_CHANGED:
    case TAF_EVT_RUNSTATE_CHANGED:
      InitiateUpdate(evt);
      break;
  }
}
