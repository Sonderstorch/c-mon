
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

// $Revision: 42 $ $Date: 2012-10-23 23:15:58 +0200 (Tue, 23 Oct 2012) $

#include "stdwx.h"

#include "cpd_gui.h"
#include "cpd_impl.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// --------------------------------------------------------------------------

enum {
  ID_BTN_TEST,
  ID_BTN_TEST_IN,
  ID_BTN_TEST_OUT
};

CCpDummyGui::CCpDummyGui(wxWindow* parent, CCallMonitorApi *pCallMonitor,
                         wxWindowID id, const wxPoint& pos, const wxSize& size,
                         long style )
  : wxPanel( parent, id, pos, size, style )
  , m_pCallMonitor(reinterpret_cast<CDummyCallMonitor *>(pCallMonitor))
{
  wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* bSizer2 = new wxBoxSizer(wxHORIZONTAL);
  wxButton *btn = new wxButton(this, ID_BTN_TEST, wxT("Test"));
  btn->Connect( ID_BTN_TEST, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CCpDummyGui::OnTest),NULL,this);
  bSizer2->Add(btn, 0, wxLEFT, 5);
  btn = new wxButton(this, ID_BTN_TEST_IN, wxT("Test In"));
  btn->Connect( ID_BTN_TEST_IN, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CCpDummyGui::OnTest),NULL,this);
  bSizer2->Add(btn, 0, wxLEFT, 5);
  btn = new wxButton(this, ID_BTN_TEST_OUT, wxT("Test Out"));
  btn->Connect( ID_BTN_TEST_OUT, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CCpDummyGui::OnTest),NULL,this);
  bSizer2->Add(btn, 0, wxLEFT, 5);
  bSizer1->Add(bSizer2, 0, wxEXPAND|wxALL, 5);
  SetSizerAndFit(bSizer1);
}

CCpDummyGui::~CCpDummyGui()
{
}

// --------------------------------------------------------------------------

void CCpDummyGui::OnTest(wxCommandEvent& e)
{
  static int test1Idx = 0;
  static wxString number;
  if (e.GetId() == ID_BTN_TEST) {
    m_pCallMonitor->Test();
  }
  else if (e.GetId() == ID_BTN_TEST_IN) {
    if (test1Idx == 0) {
      wxTextEntryDialog dlg(this, wxT("Nummer"));
      if (dlg.ShowModal() == wxID_OK) {
        m_pCallMonitor->newCall(4711, dlg.GetValue());
        m_pCallMonitor->newCall(4712, dlg.GetValue());
        m_pCallMonitor->newCall(4713, dlg.GetValue());
        m_pCallMonitor->newCall(4714, dlg.GetValue());
        m_pCallMonitor->newCall(4715, dlg.GetValue());
        ++test1Idx;
      }
    } else if (test1Idx == 1) {
      m_pCallMonitor->updateCall(4713, TCALL_CONNECTED);
      ++test1Idx;
    } else if (test1Idx == 2) {
      m_pCallMonitor->updateCall(4711, TCALL_IDLE);
      ++test1Idx;
    } else if (test1Idx == 3) {
      m_pCallMonitor->updateCall(4715, TCALL_IDLE);
      ++test1Idx;
    } else if (test1Idx == 4) {
      m_pCallMonitor->updateCall(4714, TCALL_IDLE);
      ++test1Idx;
    } else if (test1Idx == 5) {
      m_pCallMonitor->updateCall(4712, TCALL_IDLE);
      ++test1Idx;
    } else if (test1Idx == 6) {
      m_pCallMonitor->updateCall(4713, TCALL_IDLE);
      test1Idx = 0;
    }
  }
  else if (e.GetId() == ID_BTN_TEST_OUT) {
    if (number.IsEmpty()) {
      wxTextEntryDialog dlg(this, wxT("Nummer"));
      if (dlg.ShowModal() == wxID_OK) {
        number = dlg.GetValue();
        m_pCallMonitor->TestOutgoing(number);
      }
    }
    else {
      if (m_pCallMonitor->TestOutgoing(number)) {
        number.Empty();
      }
    }
  }
}
