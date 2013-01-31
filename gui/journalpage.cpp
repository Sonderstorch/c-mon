
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
#include "journalpage.h"
#include "journal/journalfilter.h"
#include "journal/journalmodel.h"
#include "glob/contact.h"
#include "glob/prefs.h"
#include "mainapp.h"
#include "dialdlg.h"
#include "resources.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI/JOURNAL"

// ----------------------------------------------------------------------------
// CJournalFilterListCtrl
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CJournalFilterListCtrl, wxListCtrl)
  EVT_LIST_ITEM_SELECTED(wxID_ANY, CJournalFilterListCtrl::OnFilterSelected)
END_EVENT_TABLE()

// ----

CJournalFilterListCtrl::CJournalFilterListCtrl(CJournalModel& model,
                                               wxWindow* parent,
                                               wxWindowID id)
  : wxListCtrl(parent, id, wxDefaultPosition, wxSize(230, -1),
                  wxLC_REPORT|wxLC_NO_HEADER|wxLC_AUTOARRANGE|
                  wxLC_VIRTUAL|wxLC_SINGLE_SEL)
  , m_model(model)
{
  wxImageList *il = new wxImageList(16, 16, false, 4);
  il->Add(RESOURCES::getBitmap(BMP_FOLDER));
  il->Add(RESOURCES::getBitmap(BMP_CALL_INCOMING));
  il->Add(RESOURCES::getBitmap(BMP_CALL_OUTGOING));
  il->Add(RESOURCES::getBitmap(BMP_CALL_ABSENCE));
  AssignImageList(il, wxIMAGE_LIST_SMALL);

  SetItemCount(m_model.getFilters().size());
  SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

  m_model.registerListener(this);
}

CJournalFilterListCtrl::~CJournalFilterListCtrl()
{
  m_model.unregisterListener(this);
}

wxString CJournalFilterListCtrl::OnGetItemText(long item, long column) const
{
  wxString strRC;
  const TJournalFilterList& rList = m_model.getFilters();
  if (item < (long)rList.size()) {
    switch (column) {
      case 0: break;
      case 1: strRC = wxString::FromUTF8(rList[item]->getName().c_str());
              break;
      case 2: strRC = wxString::Format(wxT("%6d"), rList[item]->getMatchedCount());
              break;
    }
  }
  return strRC;
}

int CJournalFilterListCtrl::OnGetItemImage(long item) const
{
  int img = 0;
  const TJournalFilterList& rList =  m_model.getFilters();
  if (item < (long)rList.size()) {
    switch (rList[item]->getType()) {
    case CJournalFilter::FT_BUILTIN_INCOMING        : img = 1; break;
    case CJournalFilter::FT_BUILTIN_OUTGOING        : img = 2; break;
    case CJournalFilter::FT_BUILTIN_CALLS_IN_ABSENCE: img = 3; break;
    }
  }
  return img;
}

void CJournalFilterListCtrl::OnFilterSelected(wxListEvent& event)
{
  m_model.selectFilter(event.GetIndex());
}

void CJournalFilterListCtrl::OnJournalUpdate()
{
  Refresh();
}

// ----------------------------------------------------------------------------
// CJournalListCtrl
// ----------------------------------------------------------------------------

enum {
  ID_CTX_JL_REVERSE_LOOKUP = 1234
};

BEGIN_EVENT_TABLE(CJournalListCtrl, wxListCtrl)
  EVT_TIMER(wxID_ANY, CJournalListCtrl::OnTimer)
  EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, CJournalListCtrl::OnRightClick)
  EVT_LIST_COL_CLICK(wxID_ANY, CJournalListCtrl::OnColumnClick)
  EVT_MENU(ID_CTX_JL_REVERSE_LOOKUP, CJournalListCtrl::OnCtxReverseLookup)
END_EVENT_TABLE()

// ----

CJournalListCtrl::CJournalListCtrl(CJournalModel& model, wxWindow* parent, wxWindowID id)
  : wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
                  wxLC_REPORT|wxLC_VIRTUAL|
                  wxLC_SINGLE_SEL)
  , m_model(model)
{
  m_nSortColumn = -1;
  m_bSortAsc    = false;
  m_RegExNoNumber.Compile(wxT("[^[:digit:]]"), wxRE_ADVANCED);
  m_Timer.SetOwner(this, GetId());
  wxImageList *il = new wxImageList(16, 16, false, 4);
  il->Add(RESOURCES::getBitmap(BMP_PHONE));
  il->Add(RESOURCES::getBitmap(BMP_CALL_INCOMING));
  il->Add(RESOURCES::getBitmap(BMP_CALL_OUTGOING));
  il->Add(RESOURCES::getBitmap(BMP_BULLETUP));
  il->Add(RESOURCES::getBitmap(BMP_BULLETDOWN));
  AssignImageList(il, wxIMAGE_LIST_SMALL);
  m_attrEven.SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
  m_attrOdd.SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
  m_model.registerListener(this);
  InsertColumn(Column_Type, wxEmptyString,     wxLIST_FORMAT_CENTER, 30);
  InsertColumn(Column_Time,     _("Time"),     wxLIST_FORMAT_LEFT,  125);
  InsertColumn(Column_Name,     _("Name"),     wxLIST_FORMAT_LEFT,  150);
  InsertColumn(Column_Caller,   _("Caller"),   wxLIST_FORMAT_LEFT,  115);
  InsertColumn(Column_Called,   _("Called"),   wxLIST_FORMAT_LEFT,  100);
  InsertColumn(Column_Duration, _("Duration"), wxLIST_FORMAT_LEFT,   80);
  InsertColumn(Column_Result,   _("Result"),   wxLIST_FORMAT_LEFT,   80);
  ToggleColumnSort(1);
  DelayedUpdate();
}

CJournalListCtrl::~CJournalListCtrl()
{
  m_model.unregisterListener(this);
}

wxString CJournalListCtrl::OnGetItemText(long item, long column) const
{
  wxString strRC;
  if (item < m_liEntries.size())
  {
    const CJournalEntry& entry = m_liEntries[item];
    switch (column) {
      case 0: break;
      case 1: strRC = entry.getCreation().IsValid() ?
                              entry.getCreation().Format() :
                              wxT("--.--.-- --:--");
              break;
      case 2: if (entry.getType() == CJournalEntry::J_INCOMING) {
                strRC = entry.getCallerName();
              } else if (entry.getType() == CJournalEntry::J_OUTGOING) {
                strRC = entry.getCalledName();
              }
              break;
      case 3: strRC = entry.getCallerAddress();
              break;
      case 4: strRC = entry.getCalledAddress();
              break;
      case 5: {
                unsigned secs = entry.getDurationInSec().GetValue();
                unsigned mins = secs / 60;
                secs -= (mins * 60);
                strRC = wxString::Format(wxT("%3u:%02u"), mins, secs);
              }
              break;
      case 6: {
                CJournalEntry::TResult res = entry.getResult();
                switch(res) {
                case CJournalEntry::JR_OK:         strRC = _("Ok"); break;
                case CJournalEntry::JR_BUSY:       strRC = _("Busy"); break;
                case CJournalEntry::JR_NOTREACHED: strRC = _("Not Reached"); break;
                case CJournalEntry::JR_FAILED:     strRC = _("Failed"); break;
                }
              }
              break;
    }
  }
  return strRC;
}

int CJournalListCtrl::OnGetItemImage(long item) const
{
  int img = 0;
  if (item < m_liEntries.size()) {
    switch (m_liEntries[item].getType()) {
    case CJournalEntry::J_INCOMING        : img = 1; break;
    case CJournalEntry::J_OUTGOING        : img = 2; break;
    }
  }
  return img;
}

wxListItemAttr* CJournalListCtrl::OnGetItemAttr(long item) const
{
  if ((item % 2) == 0) {
    return (wxListItemAttr*) &m_attrEven;
  } else {
    return (wxListItemAttr*) &m_attrOdd;
  }
}

void CJournalListCtrl::Update()
{
  m_model.getEntries(m_liEntries);
  SetItemCount(m_liEntries.size());
  Refresh();
}

void CJournalListCtrl::DelayedUpdate()
{
  m_Timer.Start(500, wxTIMER_ONE_SHOT);
}

void CJournalListCtrl::OnTimer(wxTimerEvent& e)
{
  Update();
}

void CJournalListCtrl::OnJournalUpdate()
{
  DelayedUpdate();
}

void CJournalListCtrl::OnColumnClick(wxListEvent& event)
{
  ToggleColumnSort(event.GetColumn());
  TJournalSortOrder so;
  switch (m_nSortColumn) {
  case 0: so = J_SORT_TYPE; break;
  case 1: so = J_SORT_TIME; break;
  case 2: so = J_SORT_CALLER_NAME; break;
  case 3: so = J_SORT_CALLER_NR; break;
  case 4: so = J_SORT_CALLED_NR; break;
  case 5: so = J_SORT_DURATION; break;
  case 6: so = J_SORT_RESULT; break;
  default: so = J_SORT_TIME; break;
  }
  m_model.selectSortOrder(so, m_bSortAsc);
}

void CJournalListCtrl::ToggleColumnSort(int col)
{
  wxListItem item;
  item.SetMask(wxLIST_MASK_IMAGE);

  if (m_nSortColumn != -1) {
    // Clear icon
    item.SetImage(-1);
    SetColumn(m_nSortColumn, item);
  }
  if (col == m_nSortColumn) {
    m_bSortAsc = !m_bSortAsc;
  }
  // Set icon
  m_nSortColumn = col;
  item.SetImage(m_bSortAsc ? 3 : 4);
  SetColumn(m_nSortColumn, item);
}

void CJournalListCtrl::OnRightClick(wxListEvent& e)
{
  long idx = e.GetIndex();
  if (idx < m_liEntries.size()) {
    wxMenu menu;
    menu.SetClientData((void *) idx);
    if (m_liEntries[idx].getType() == CJournalEntry::J_INCOMING) {
      menu.Append(ID_CTX_JL_REVERSE_LOOKUP, _("Lookup Caller"));
    } else if (m_liEntries[idx].getType() == CJournalEntry::J_OUTGOING) {
      menu.Append(ID_CTX_JL_REVERSE_LOOKUP, _("Lookup Called"));
    }
    PopupMenu(&menu);
  }
}

void CJournalListCtrl::OnCtxReverseLookup(wxCommandEvent& e)
{
  int idx = (int) static_cast<wxMenu *>(e.GetEventObject())->GetClientData();
  if (idx < m_liEntries.size()) {
    CContact resolvedContact;
    CJournalEntry::TType type = m_liEntries[idx].getType();
    wxString toResolve = (type == CJournalEntry::J_INCOMING) ?
        m_liEntries[idx].getCallerAddress() :
        m_liEntries[idx].getCalledAddress();
    m_RegExNoNumber.ReplaceAll(&toResolve, wxEmptyString);
    if (wxGetApp().reverseLookup(toResolve, resolvedContact)) {
      if (type == CJournalEntry::J_INCOMING) {
        m_liEntries[idx].setCallerName(resolvedContact.getSN());
      } else {
        m_liEntries[idx].setCalledName(resolvedContact.getSN());
      }
      m_model.insertUpdateEntry(m_liEntries[idx]);
    }
  }
}

void CJournalListCtrl::DoCall()
{
  CDialDialog dlg(this, NULL);
  int idx = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if ((idx >= 0) && (idx < m_liEntries.size())) {
    CJournalEntry& rEntry = m_liEntries[idx];
    if (rEntry.getType() == CJournalEntry::J_INCOMING) {
      dlg.SetNumber(rEntry.getCallerAddress());
    }
    else if (rEntry.getType() == CJournalEntry::J_OUTGOING) {
      dlg.SetNumber(rEntry.getCalledAddress());
    }
  }
  if (dlg.ShowModal() == wxID_OK) {
    wxGetApp().getPrefs().setDefaultCallProvider(dlg.m_strCallProvider);
    CCallMonitorApi *pCM = wxGetApp().getDefaultCallProvider();
    if (pCM) {
      wxLogMessage(wxT("Dialing %s using %s ..."), dlg.m_strNumber,
          wxGetApp().getPrefs().getDefaultCallProvider());
      pCM->makeCall(dlg.m_strNumber.ToUTF8().data());
    }
  }
}

// ----------------------------------------------------------------------------
// CJournalPage
// ----------------------------------------------------------------------------

enum {
  CMD_CALL_DLG = 0x1278,
  CMD_DIAL_DLG,
  ID_SEARCH_CTRL
};

BEGIN_EVENT_TABLE(CJournalPage, wxPanel)
  EVT_TIMER(wxID_ANY, CJournalPage::OnTimer)
  EVT_TEXT(ID_SEARCH_CTRL, CJournalPage::OnSearchUpdate)
  EVT_SEARCHCTRL_SEARCH_BTN(ID_SEARCH_CTRL, CJournalPage::OnSearch)
  EVT_SEARCHCTRL_CANCEL_BTN(ID_SEARCH_CTRL, CJournalPage::OnSearchCancel)
  EVT_BUTTON(CMD_CALL_DLG, CJournalPage::OnCall)
  EVT_BUTTON(CMD_DIAL_DLG, CJournalPage::OnDialNumber)
  EVT_UPDATE_UI(CMD_CALL_DLG, CJournalPage::OnUpdateCall)
END_EVENT_TABLE()

CJournalPage::CJournalPage(CJournalModel& model, wxWindow* parent, wxWindowID id)
  : wxPanel(parent, id, wxDefaultPosition, wxSize(0,0),
      wxCLIP_CHILDREN|wxNO_BORDER|wxTAB_TRAVERSAL)
  , m_model(model)
{
  Create(parent, id);
  m_RegExNumber.Compile(wxT("[[:digit:]]"), wxRE_ADVANCED);
  m_RegExNoNumber.Compile(wxT("[^[:digit:]]"), wxRE_ADVANCED);
}

CJournalPage::~CJournalPage()
{
}

bool CJournalPage::Create(wxWindow* parent, wxWindowID id)
{
  m_Timer.SetOwner(this, id);

#ifdef USE_SPLIT_WINDOW

  wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );

  m_pSplitter = new wxSplitterWindow(this, wxID_ANY,
      wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

  m_listCtrl2 = new CJournalFilterListCtrl(m_model, m_pSplitter, wxID_ANY);
  m_listCtrl2->InsertColumn(1, wxEmptyString, wxLIST_FORMAT_CENTER, 25);
  m_listCtrl2->InsertColumn(2, _("Name"), wxLIST_FORMAT_LEFT, 150);
  m_listCtrl2->InsertColumn(3, _("Count"), wxLIST_FORMAT_RIGHT, 40);

  wxPanel *panel = new wxPanel(m_pSplitter, wxID_ANY);

  wxBoxSizer* bSizer2a = new wxBoxSizer( wxVERTICAL );
  wxBoxSizer* bSizer2b = new wxBoxSizer( wxHORIZONTAL );

  wxButton *btn = new wxButton( panel, CMD_CALL_DLG, _("Call..."));
  btn->SetBitmap(RESOURCES::getBitmap(BMP_HANDSET));
  btn->SetToolTip(_("Call selected entry"));
  bSizer2b->Add(btn, 0, wxALIGN_CENTER_VERTICAL, 0 );

  btn = new wxButton( panel, CMD_DIAL_DLG, _("Dial..."));
  btn->SetBitmap(RESOURCES::getBitmap(BMP_DIAL));
  btn->SetToolTip(_("Enter a number to call"));
  bSizer2b->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );

  bSizer2b->AddStretchSpacer(1);

  m_searchCtrl = new wxSearchCtrl( panel, ID_SEARCH_CTRL,
      wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
  #ifndef __WXMAC__
  m_searchCtrl->ShowSearchButton( true );
  #endif
  m_searchCtrl->ShowCancelButton( true );
  bSizer2b->Add( m_searchCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 0);

  m_listCtrl1 = new CJournalListCtrl(m_model, panel, wxID_ANY);

  bSizer2a->Add( bSizer2b, 0, wxEXPAND|wxTOP, 5 );
  bSizer2a->Add( m_listCtrl1, 1, wxEXPAND|wxTOP, 5 );

  panel->SetSizerAndFit(bSizer2a);

  m_pSplitter->SplitVertically(m_listCtrl2, panel, 0);

  bSizer1->Add( m_pSplitter, 1, wxALL|wxEXPAND, 5 );

#else

  wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );

  wxBoxSizer* bSizer2a = new wxBoxSizer( wxHORIZONTAL );
  wxBoxSizer* bSizer2b = new wxBoxSizer( wxHORIZONTAL );

  bSizer2a->AddStretchSpacer(1);

  wxButton *btn = new wxButton( this, CMD_CALL_DLG, _("Call..."));
  btn->SetBitmap(RESOURCES::getBitmap(BMP_HANDSET));
  btn->SetToolTip(_("Call selected entry"));
  bSizer2b->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 10 );
  btn = new wxButton( this, CMD_DIAL_DLG, _("Dial..."));
  btn->SetBitmap(RESOURCES::getBitmap(BMP_DIAL));
  btn->SetToolTip(_("Enter a number to call"));
  bSizer2b->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
  bSizer2b->AddStretchSpacer(1);
  m_searchCtrl = new wxSearchCtrl( this, ID_SEARCH_CTRL,
      wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
#ifndef __WXMAC__
  m_searchCtrl->ShowSearchButton( true );
#endif
  m_searchCtrl->ShowCancelButton( true );
  bSizer2b->Add( m_searchCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

  bSizer2a->Add( bSizer2b, 3, wxEXPAND|wxTOP, 5 );
  bSizer1->Add( bSizer2a, 0, wxEXPAND|wxTOP, 5 );

  wxBoxSizer* bSizer3 = new wxBoxSizer( wxHORIZONTAL );

  m_listCtrl2 = new CJournalFilterListCtrl(m_model, this, wxID_ANY);
  m_listCtrl2->InsertColumn(1, wxEmptyString, wxLIST_FORMAT_CENTER, 25);
  m_listCtrl2->InsertColumn(2, _("Name"), wxLIST_FORMAT_LEFT, 150);
  m_listCtrl2->InsertColumn(3, _("Count"), wxLIST_FORMAT_RIGHT, 40);
  bSizer3->Add( m_listCtrl2, 1, wxEXPAND, 0 );

  m_listCtrl1 = new CJournalListCtrl(m_model, this, wxID_ANY);
  bSizer3->Add( m_listCtrl1, 3, wxLEFT|wxEXPAND, 5 );
  bSizer1->Add( bSizer3, 1, wxALL|wxEXPAND, 5 );
#endif

  SetSizerAndFit(bSizer1);

  return true;
}

void CJournalPage::OnDialNumber(wxCommandEvent& e)
{
  CDialDialog dlg(this, NULL);
  if (dlg.ShowModal() == wxID_OK) {
    wxGetApp().getPrefs().setDefaultCallProvider(dlg.m_strCallProvider);
    CCallMonitorApi *pCM = wxGetApp().getDefaultCallProvider();
    if (pCM) {
      wxLogMessage(wxT("Dialing %s using %s ..."), dlg.m_strNumber,
          wxGetApp().getPrefs().getDefaultCallProvider());
      pCM->makeCall(dlg.m_strNumber.ToUTF8().data());
    }
  }
}

void CJournalPage::OnCall(wxCommandEvent& e)
{
  m_listCtrl1->DoCall();
}

void CJournalPage::OnUpdateCall(wxUpdateUIEvent& e)
{
  e.Enable(m_listCtrl1->GetSelectedItemCount() > 0);
}

void CJournalPage::DoSearch()
{
  wxString strText, strNumber;
  strText = m_searchCtrl->GetValue();
  strNumber = strText;
  m_RegExNoNumber.ReplaceAll(&strNumber, wxEmptyString);
  m_RegExNumber.ReplaceAll(&strText, wxEmptyString);
  m_model.setNameAndNumberFilter(strText, strNumber);
}

void CJournalPage::OnSearch(wxCommandEvent&)
{
  DoSearch();
}

void CJournalPage::OnSearchCancel(wxCommandEvent&)
{
  m_searchCtrl->Clear();
  DoSearch();
}

void CJournalPage::OnSearchUpdate(wxCommandEvent&)
{
  m_Timer.Start(300, wxTIMER_ONE_SHOT);
}

void CJournalPage::OnTimer(wxTimerEvent& e)
{
  DoSearch();
}
