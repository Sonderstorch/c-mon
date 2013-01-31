#ifndef MAINFRAME_H_INCLUDED
#define MAINFRAME_H_INCLUDED

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

class CMainPanel; // forward
class CMainFrame;

// ------

class CMainTrayIcon : public wxTaskBarIcon
{
public:
  CMainTrayIcon(CMainFrame *parent) : m_pParent(parent) {  }
  virtual ~CMainTrayIcon()                              {  }

  void OnLeftButtonDClick(wxTaskBarIconEvent&);

  void OnMenuRestore(wxCommandEvent&);
  void OnMenuExit(wxCommandEvent&);
  void OnUpdateRestore(wxUpdateUIEvent&);

  virtual wxMenu *CreatePopupMenu();

protected:
  CMainFrame  *m_pParent;

  DECLARE_EVENT_TABLE()
};

// ------

class CMainFrame : public wxFrame 
{
public:
  enum {
    PAGE_JOURNAL,
    PAGE_CONTACTS,
    PAGE_SETTINGS,
    PAGE_ABOUT,
//    PAGE_LOG,
    PAGE_MAX
  };
  CMainFrame();
  virtual ~CMainFrame();

  virtual bool ShouldPreventAppExit() const { return false; }

  void OnCloseWindow(wxCloseEvent&);
  void OnRibbonPageChanged(wxRibbonBarEvent& e);

protected:
  void ShowPage(size_t i);

private:
  CMainTrayIcon    *m_pTrayIcon;
  wxRibbonBar*      m_pRibbon;
  bool              m_bShowHideHint;
  size_t            m_nCurrentPage;
  wxSizerItem *     m_Page[PAGE_MAX];

  DECLARE_EVENT_TABLE()
};

// -----------------------------

enum
{
  ID_WINDOW_TOP = wxID_HIGHEST,
  ID_MENU_LAST
};

#endif // MAINFRAME_H_INCLUDED
