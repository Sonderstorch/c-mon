
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

#include "cpf_gui.h"
#include "cpf_monitor.h"
#include "cpf_prefs.h"

#include "glob/controls.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// ----------------------------------------------------------------------------

wxDEFINE_EVENT(EVENT_REFRESH, wxCommandEvent);

enum {
  ID_EDIT1,
  ID_BTN_STARTSTOP
};

CCpfGui::CCpfGui(wxWindow* parent, CFritzCallMonitor *pCallMonitor,
                 wxWindowID id, const wxPoint& pos, const wxSize& size,
                 long style )
  : wxPanel( parent, id, pos, size, style )
  , m_pCallMonitor(pCallMonitor)
{
  CCpFritzPrefs& rPrefs = CPFRITZPREFS::getPrefs();

  // TODO: Move RESOURCES class to shared
  m_pFont = new wxFont(12, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Calibri"));

  m_txtAddress = new wxTextCtrl(this, ID_EDIT1, wxEmptyString,
                            wxDefaultPosition, wxSize(400, -1), 0,
                            wxGenericValidator(&rPrefs.m_strFritzAddr));
  m_txtAddress->SetToolTip(
      _("The IP address or name of the Fritz!Box Phone in your network.\n"
        "Factory default is '192.168.178.1' and 'fritz.box', respectively."));

  m_pStatusRunning = new CStatusIndicator(this, wxID_ANY,
      wxDefaultPosition, wxSize(32,16));
  m_pStatusConnected = new CStatusIndicator(this, wxID_ANY,
      wxDefaultPosition, wxSize(32,16));

  m_btnStartStop = new wxButton( this, ID_BTN_STARTSTOP, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, 0 );

  wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxStaticText *txt = new wxStaticText(this, wxID_ANY, _("Fritz!Box call monitor"));
  txt->SetFont(*m_pFont);
  bSizer1->Add(txt, 0, wxLEFT|wxRIGHT|wxTOP, 5);
  bSizer1->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
      wxDefaultSize, wxHORIZONTAL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 6, 2, 0, 0 );
  fgSizer1->AddGrowableCol( 1 );
  fgSizer1->SetFlexibleDirection( wxBOTH );
  fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  fgSizer1->Add(new wxStaticText(this, wxID_ANY,
                      _("Running/Connected:")),
      0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5);
  wxBoxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
  pSizer->Add(m_pStatusRunning, 0, 0, 0);
  pSizer->Add(m_pStatusConnected, 0, wxLEFT, 5);
  fgSizer1->Add(pSizer, 0, wxALL, 5);

  fgSizer1->Add(new wxStaticText(this, wxID_ANY,
                      _("Network address:")),
      0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5);
  fgSizer1->Add(m_txtAddress, 0, wxALL, 5);

  fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
  fgSizer1->Add(new wxStaticText(this, wxID_ANY,
                      _("Note: You have to enable the call monitor on your Fritz!Box by calling\n"
                        "#96*5* on a connected phone once. To disable it again, dial #96*4*."),
                      wxDefaultPosition, wxDefaultSize,
                      wxALIGN_LEFT), 0, wxALL, 5);

  fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
  fgSizer1->Add( m_btnStartStop, 0, wxALL, 5 );

  bSizer1->Add(fgSizer1, 0, wxEXPAND, 0);

  this->SetSizer( bSizer1 );
  this->Layout();

  TransferDataToWindow();

  m_txtAddress->Connect( ID_EDIT1, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CCpfGui::OnChange),NULL,this);
  m_btnStartStop->Connect( ID_BTN_STARTSTOP, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CCpfGui::OnStartStop),NULL,this);

  Bind(EVENT_REFRESH, &CCpfGui::OnRefresh, this);

  Update();

  m_pCallMonitor->registerGUI(this);

  UpdateStatus();
}

CCpfGui::~CCpfGui()
{
  m_pCallMonitor->deregisterGUI(this);
  delete m_pFont;
}

void CCpfGui::InitiateUpdate()
{
  wxCommandEvent evt(EVENT_REFRESH);
  evt.SetEventObject(this);
  wxPostEvent(this, evt);
}

void CCpfGui::OnGuiNotification(long info)
{
  InitiateUpdate();
}

// --------------------------------------------------------------------------

void CCpfGui::UpdateStatus()
{
  bool bRunning = m_pCallMonitor->isRunning();
  m_pStatusRunning->SetOK(bRunning);
  m_pStatusConnected->SetOK(m_pCallMonitor->isConnected());
  m_btnStartStop->SetLabelText(bRunning ?
      _("Stop Monitor") : _("Start Monitor"));
  m_txtAddress->Enable(!bRunning);
  m_btnStartStop->Enable(true);
}

void CCpfGui::OnChange(wxCommandEvent& e)
{
  CCpFritzPrefs& currPrefs = CPFRITZPREFS::getPrefs();
  CCpFritzPrefs oldPrefs = currPrefs;
  TransferDataFromWindow();
  if (!(oldPrefs == currPrefs)) {
    CPFRITZPREFS::savePrefs();
    CPFRITZPREFS::notify();
  }
}

void CCpfGui::OnRefresh(wxCommandEvent& e)
{
  UpdateStatus();
}

void CCpfGui::OnStartStop(wxCommandEvent& e)
{
  m_btnStartStop->Enable(false);
  bool bRunning = m_pCallMonitor->isRunning();
  if (bRunning) {
    m_pCallMonitor->shutdown();
  }
  else {
    m_pCallMonitor->init(false);
  }
}
