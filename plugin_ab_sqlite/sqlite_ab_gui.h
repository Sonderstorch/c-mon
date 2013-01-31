#ifndef SQLITE_AB_GUI_H_INCLUDED
#define SQLITE_AB_GUI_H_INCLUDED

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

#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/listctrl.h>
#include <wx/panel.h>

class CSQLiteAddressbookPlugin;

class CSQLiteAddressbookGui : public wxPanel
{
public:
  CSQLiteAddressbookGui(CSQLiteAddressbookPlugin *plugin,
                        wxWindow *parent,
                        wxWindowID id = wxID_ANY,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxSize( 500,300 ),
                        long style = wxTAB_TRAVERSAL | wxNO_BORDER);
  virtual ~CSQLiteAddressbookGui();

  void OnAdd(wxCommandEvent&);
  void OnDel(wxCommandEvent&);

  void OnEdit(wxListEvent&);

  void OnUpdateDel(wxUpdateUIEvent& e);

  bool ValidateName(wxString& strDbName);
  bool ValidatePath(wxString& strDbPath);

protected:
  void Populate();

protected:
  CSQLiteAddressbookPlugin *m_pPlugin;
  wxButton                 *m_pBtnAdd;
  wxButton                 *m_pBtnDel;
  wxListCtrl               *m_pList;
  wxFont                   *m_pFont;
};

#endif /* SQLITE_AB_GUI_H_INCLUDED */
