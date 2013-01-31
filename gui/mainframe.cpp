
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

#include "glob/prefs.h"

#include "mainapp.h"
#include "mainframe.h"
#include "notification.h"
#include "aboutpage.h"
#include "contactspage.h"
#include "journalpage.h"
#include "settingspage.h"
#include "logpage.h"
#include "resources.h"


#ifdef __WXMSW__
#  include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI"

// ----------------------------------------------------------------------------
// CMainTrayIcon
// ----------------------------------------------------------------------------

enum
{
    TI_MENU_RESTORE = 11000,
    TI_MENU_EXIT
};

BEGIN_EVENT_TABLE(CMainTrayIcon, wxTaskBarIcon)
    EVT_MENU(TI_MENU_RESTORE, CMainTrayIcon::OnMenuRestore)
    EVT_MENU(TI_MENU_EXIT,    CMainTrayIcon::OnMenuExit)
    EVT_TASKBAR_LEFT_DCLICK  (CMainTrayIcon::OnLeftButtonDClick)
    EVT_UPDATE_UI(TI_MENU_RESTORE, CMainTrayIcon::OnUpdateRestore)
END_EVENT_TABLE()

void CMainTrayIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{
  wxGetApp().getFrameWindow()->Show();
  wxGetApp().getPrefs().enableHiddenStart(false);
}

void CMainTrayIcon::OnMenuRestore(wxCommandEvent&)
{
  wxGetApp().getFrameWindow()->Show();
  wxGetApp().getPrefs().enableHiddenStart(false);
}

void CMainTrayIcon::OnMenuExit(wxCommandEvent&)
{
  wxGetApp().getFrameWindow()->Close(true);
}

void CMainTrayIcon::OnUpdateRestore(wxUpdateUIEvent& e)
{
  e.Enable(!wxGetApp().getFrameWindow()->IsVisible());
}

wxMenu *CMainTrayIcon::CreatePopupMenu()
{
  wxMenu *menu = new wxMenu;
  menu->Append(TI_MENU_RESTORE, _("&Restore main window"));
  menu->AppendSeparator();
  menu->Append(TI_MENU_EXIT,    _("E&xit"));
  return menu;
}

// ----------------------------------------------------------------------------
// CMainFrame
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CMainFrame, wxFrame)
EVT_CLOSE(CMainFrame::OnCloseWindow)
EVT_RIBBONBAR_PAGE_CHANGED(wxID_ANY, CMainFrame::OnRibbonPageChanged)
END_EVENT_TABLE()

CMainFrame::CMainFrame()
  : wxFrame(NULL, wxID_ANY, wxT("c'mon"),
            wxDefaultPosition, wxSize(800, 600),
            wxDEFAULT_FRAME_STYLE)
{
  // add main client window
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  m_pRibbon = new wxRibbonBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxRIBBON_BAR_FLOW_HORIZONTAL  |
      wxRIBBON_BAR_SHOW_PAGE_LABELS |
      wxRIBBON_BAR_SHOW_PAGE_ICONS  |
      wxRIBBON_BAR_ALWAYS_SHOW_TABS);
  wxRibbonPage* page = new wxRibbonPage(m_pRibbon, wxID_ANY, _("Journal"),
      RESOURCES::getBitmap(BMP_TB_JOURNAL));
  page = new wxRibbonPage(m_pRibbon, wxID_ANY, _("Contacts"),
      RESOURCES::getBitmap(BMP_TB_CONTACTS));
  page = new wxRibbonPage(m_pRibbon, wxID_ANY, _("Settings"),
      RESOURCES::getBitmap(BMP_TB_SETTINGS));
  page = new wxRibbonPage(m_pRibbon, wxID_ANY, _("About"),
      RESOURCES::getBitmap(BMP_TB_ABOUT));
//  page = new wxRibbonPage(m_pRibbon, wxID_ANY, _("Log"),
//      RESOURCES::getBitmap(BMP_TB_LOG));
  m_pRibbon->Realize();

//  CLogPage *logPage = new CLogPage(this);
  sizer->Add(m_pRibbon, wxSizerFlags(0).Expand());
  m_Page[PAGE_JOURNAL]  = sizer->Add(new CJournalPage(wxGetApp().getJournalModel(), this), wxSizerFlags(1).Expand());
  m_Page[PAGE_CONTACTS] = sizer->Add(new CContactPage(this), wxSizerFlags(1).Expand());
  m_Page[PAGE_SETTINGS] = sizer->Add(new CSettingsPage(wxGetApp().getResolverModel(), this), wxSizerFlags(1).Expand().Border(wxALL, 10));
  m_Page[PAGE_ABOUT]    = sizer->Add(new CAboutPage(this), wxSizerFlags(1).Expand());
//  m_Page[PAGE_LOG]      = sizer->Add(logPage, wxSizerFlags(1).Expand());
  ShowPage(wxGetApp().getPrefs().getSelectedPage());
  m_pRibbon->SetActivePage(m_nCurrentPage);
  SetSizerAndFit(sizer);

  m_pTrayIcon = new CMainTrayIcon(this);
  wxIcon icon_tray(wxICON(AAPHONERED));
  m_pTrayIcon->SetIcon(icon_tray, _("c'mon telephony monitor"));
  wxIcon icon_main(wxICON(AAPHONERED));
  SetIcon(icon_main);

  // CreateStatusBar(1);

  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));

  wxRect rect(0,0,100,100);
  if (wxGetApp().getGlobalPrefs().readFrameSize(rect)) {
    SetSize(rect, wxSIZE_ALLOW_MINUS_ONE);
  }

  m_bShowHideHint = ! wxGetApp().getPrefs().startHidden();
}

CMainFrame::~CMainFrame()
{
  wxGetApp().getGlobalPrefs().savePrefs();
  m_pTrayIcon->Destroy();
}

void CMainFrame::ShowPage(size_t idx)
{
  Freeze();
  for (size_t i = 0; i < sizeof(m_Page)/sizeof(m_Page[0]); ++i) {
    m_Page[i]->Show(i == idx);
  }
  m_nCurrentPage = idx;
  Layout();
  Refresh(true);
  Thaw();
}

void CMainFrame::OnRibbonPageChanged(wxRibbonBarEvent& WXUNUSED(evt))
{
  int idx = m_pRibbon->GetActivePage();
  wxGetApp().getPrefs().setSelectedPage(idx);
  ShowPage(idx);
}

void CMainFrame::OnCloseWindow(wxCloseEvent& e)
{
  wxGetApp().getGlobalPrefs().writeFrameSize(GetScreenRect());
  if (e.CanVeto() && IsVisible())
  {
    Hide();
    wxGetApp().getPrefs().enableHiddenStart(true);
    if (m_bShowHideHint) {
      m_pTrayIcon->ShowBalloon(_("c'mon telephony monitor"),
        _("Note: c'mon has not been quit, just hidden. "
          "Right-click on this icon and use the context menu "
          "to show it again or to quit for real."), 3000,
          wxICON_INFORMATION);
      m_bShowHideHint = false;  // Only once
    }
  }
  else {
    wxFrame::OnCloseWindow(e);
  }
}
