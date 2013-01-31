#ifndef CPF_GUI_H_INCLUDED
#define CPF_GUI_H_INCLUDED

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

#include "cpf_monitor.h"

class CStatusIndicator;

class CCpfGui : public wxPanel, public IGuiNotification
{
public:
  CCpfGui(wxWindow *parent,
          CFritzCallMonitor *pCallMonitor,
          wxWindowID id = wxID_ANY,
          const wxPoint& pos = wxDefaultPosition,
          const wxSize& size = wxSize( 500,300 ),
          long style = wxTAB_TRAVERSAL | wxNO_BORDER );
  virtual ~CCpfGui();

  void InitiateUpdate();

protected:
  void OnChange(wxCommandEvent&);
  void OnStartStop(wxCommandEvent& e);

  void OnGuiNotification(long info);

  void OnRefresh(wxCommandEvent&);
  void UpdateStatus();

protected:
  wxFont            *m_pFont;
  wxTextCtrl        *m_txtAddress;
  CStatusIndicator  *m_pStatusRunning;
  CStatusIndicator  *m_pStatusConnected;
  wxButton          *m_btnStartStop;

protected:
  CFritzCallMonitor *m_pCallMonitor;
};

#endif /* CPF_GUI_H_INCLUDED */
