
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
#include "mainapp.h"
#include "contactspage.h"
#include "csvimportdlg.h"
#include "contactdlg.h"
#include "pluginloader.h"
#include "resources.h"
#include "dialdlg.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#include <algorithm>

#define wxLOG_COMPONENT "GUI"

// ----------------------------------------------------------------------------

enum {
  CMD_ADD_ADDRESS = 1000,
  CMD_EDIT_ADDRESS,
  CMD_DEL_ADDRESS,
  CMD_IMPORT,
  CMD_EXPORT,
  CMD_CALL_DLG,
  ID_CHOICE_ABOOK,
  ID_SEARCH_CTRL
};

// ----------------------------------------------------------------------------
// CContactListCtrl
// ----------------------------------------------------------------------------

#define MAX_PHONES 5

enum {
  ID_CTX_MAKE_CALL  = 1220,
  ID_CTX_SELECT_CALL = 1230
};

// ----

BEGIN_EVENT_TABLE(CContactListCtrl, wxListCtrl)
  EVT_TIMER(wxID_ANY, CContactListCtrl::OnTimer)
  EVT_LIST_ITEM_ACTIVATED(wxID_ANY, CContactListCtrl::OnItemActivated)
  EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, CContactListCtrl::OnRightClick)
  EVT_MENU(CMD_ADD_ADDRESS, CContactListCtrl::OnAddAddress)
  EVT_MENU(CMD_EDIT_ADDRESS, CContactListCtrl::OnEditAddress)
  EVT_MENU(CMD_DEL_ADDRESS, CContactListCtrl::OnDelAddress)
  EVT_MENU(ID_CTX_SELECT_CALL, CContactListCtrl::OnCtxMakeCallDlg)
  EVT_MENU_RANGE(ID_CTX_MAKE_CALL, ID_CTX_MAKE_CALL+MAX_PHONES, CContactListCtrl::OnCtxMakeQuickCall)
  EVT_UPDATE_UI_RANGE(ID_CTX_MAKE_CALL, ID_CTX_SELECT_CALL, CContactListCtrl::OnUpdateCallAndEdit)
  EVT_UPDATE_UI(CMD_EDIT_ADDRESS, CContactListCtrl::OnUpdateCallAndEdit)
  EVT_UPDATE_UI(CMD_ADD_ADDRESS, CContactListCtrl::OnUpdateAdd)
  EVT_UPDATE_UI(CMD_DEL_ADDRESS, CContactListCtrl::OnUpdateDel)
END_EVENT_TABLE()

// ----

CContactListCtrl::CContactListCtrl(wxWindow* parent, wxWindowID id)
  : wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
                  wxLC_REPORT|wxLC_VIRTUAL)
  , m_pAddressbook(0)
{
  m_Timer.SetOwner(this, GetId());
  m_attrEven.SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
  m_attrOdd.SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
  InsertColumn(Column_Org,    _("Organization"),    wxLIST_FORMAT_LEFT, 100);
  InsertColumn(Column_LN,     _("Last Name"),       wxLIST_FORMAT_LEFT, 130);
  InsertColumn(Column_FN,     _("First Name"),      wxLIST_FORMAT_LEFT, 100);
  InsertColumn(Column_SN,     _("Sorted Name"),     wxLIST_FORMAT_LEFT, 200);
  InsertColumn(Column_Email,  _("Email"),           wxLIST_FORMAT_LEFT, 100);
  InsertColumn(Column_Phone,  _("Phone Number(s)"), wxLIST_FORMAT_LEFT, 220);
  DelayedUpdate();
}

CContactListCtrl::~CContactListCtrl()
{
}

wxString CContactListCtrl::OnGetItemText(long item, long column) const
{
  wxString strRC;
  if ((item >= 0) && (item < (long)m_liEntries.size()))
  {
    switch (column) {
      case Column_Org  : strRC = m_liEntries[item].getOrganization(); break;
      case Column_LN   : strRC = m_liEntries[item].getLN(); break;
      case Column_FN   : strRC = m_liEntries[item].getFN(); break;
      case Column_SN   : strRC = m_liEntries[item].getSN(); break;
      case Column_Email: strRC = m_liEntries[item].getEmail(); break;
      case Column_Phone: {
          const TPhoneList& rPhones = m_liEntries[item].getConstPhones();
          size_t count = rPhones.size();
          if (count)
          {
            strRC = rPhones[0].getNumber();
            if (rPhones.size() > 1) {
              for (int i = 1; i < std::min(count, (size_t)MAX_PHONES); ++i) {
                strRC += wxT("; ") + rPhones[i].getNumber();
              }
              if (count > MAX_PHONES) {
                strRC += wxString::Format(_(" (and %ld more)"), count-5);
              }
            }
          }
        }
        break;
    }
  }
  return strRC;
}

wxListItemAttr* CContactListCtrl::OnGetItemAttr(long item) const
{
  if ((item % 2) == 0) {
    return (wxListItemAttr*) &m_attrEven;
  } else {
    return (wxListItemAttr*) &m_attrOdd;
  }
}

void CContactListCtrl::Update()
{
  m_pAddressbook = wxGetApp().getCurrentAddressbook();
  if (m_pAddressbook) m_pAddressbook->getEntries(m_liEntries);
  SetItemCount(m_liEntries.size());
  Refresh();
}

void CContactListCtrl::DelayedUpdate()
{
  m_Timer.Start(500, wxTIMER_ONE_SHOT);
}

void CContactListCtrl::OnTimer(wxTimerEvent& e)
{
  Update();
}

void CContactListCtrl::AddAddress()
{
  CContact contact;
  CContactDialog dlg(wxGetApp().GetTopWindow(), contact);
  dlg.CenterOnParent();
  if (dlg.ShowModal() == wxID_OK) {
    dlg.GetData(contact);
    if (m_pAddressbook) m_pAddressbook->insertUpdateEntry(contact);
    Update();
  }
}

void CContactListCtrl::EditAddress()
{
  int idx = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if ((idx >= 0) && (idx < m_liEntries.size())) {
    CContact& rContact = m_liEntries[idx];
    CContactDialog dlg(wxGetApp().GetTopWindow(), rContact);
    dlg.CenterOnParent();
    if (dlg.ShowModal() == wxID_OK) {
      dlg.GetData(rContact);
      if (m_pAddressbook) m_pAddressbook->insertUpdateEntry(rContact);
      Update();
    }
  }
}

void CContactListCtrl::DelAddress()
{
  int      count;
  wxString strMessage;
  if ((count = GetSelectedItemCount()) == 1) {
    strMessage = _("Are you sure to delete the selected contact?");
  } else {
    strMessage = wxString::Format(
        _("Are you sure to delete the selected %d contacts?"), count);
  }
  if (wxMessageBox(strMessage, wxMessageBoxCaptionStr, wxICON_QUESTION|wxYES_NO) == wxYES)
  {
    long item = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1) {
      if (m_pAddressbook) m_pAddressbook->deleteEntry(m_liEntries[item]);
      item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
    Update();
  }
}

void CContactListCtrl::DoCallDlg()
{
  const CContact *pContact = NULL;
  int idx = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if ((idx >= 0) && (idx < m_liEntries.size())) {
    pContact = &m_liEntries[idx];
  }
  CDialDialog dlg(this, pContact);
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

void CContactListCtrl::OnAddAddress(wxCommandEvent& e)
{
  AddAddress();
}

void CContactListCtrl::OnEditAddress(wxCommandEvent& e)
{
  EditAddress();
}

void CContactListCtrl::OnDelAddress(wxCommandEvent& e)
{
  DelAddress();
}

void CContactListCtrl::OnItemActivated(wxListEvent& e)
{
  // EditAddress(e.GetIndex());
  DoCallDlg();
}

void CContactListCtrl::OnRightClick(wxListEvent& e)
{
  wxMenu menu;
  if (GetSelectedItemCount() == 1) {
    // Single selection
    CContact& rContact = m_liEntries[e.GetIndex()];
    const TPhoneList& rPhones = rContact.getConstPhones();
    if (rPhones.size()) {
      wxMenu *submenu = new wxMenu();
      TPhoneList::const_iterator it = rPhones.begin();
      for (int i = 0; it != rPhones.end() && (i < MAX_PHONES); ++it, ++i) {
        submenu->Append(ID_CTX_MAKE_CALL + i,
            wxString::Format(wxT("%s (%s)"), it->getNumber(),
                CPhone::getNumberTypeName(it->getType())));
      }
      if (rPhones.size() > MAX_PHONES) {
        submenu->Append(ID_CTX_SELECT_CALL, _("More..."));
      }
      menu.Append(ID_CTX_SELECT_CALL, _("Call..."));
      menu.AppendSubMenu(submenu, _("Quick Call"));
      menu.AppendSeparator();
    }
    menu.Append(CMD_ADD_ADDRESS, _("Add..."));
    menu.Append(CMD_EDIT_ADDRESS, _("Edit..."));
    menu.AppendSeparator();
    menu.Append(CMD_DEL_ADDRESS, _("Delete"));
  }
  else {
    // Multiple selection
    menu.Append(CMD_ADD_ADDRESS, _("Add..."));
    menu.AppendSeparator();
    menu.Append(CMD_DEL_ADDRESS, _("Delete"));
  }
  PopupMenu(&menu);
}

void CContactListCtrl::OnCtxMakeQuickCall(wxCommandEvent& e)
{
  int idx = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if ((idx >= 0) && (idx < m_liEntries.size()))
  {
    CContact& rContact = m_liEntries[idx];
    const TPhoneList& rPhones = rContact.getConstPhones();
    int phoneIdx = e.GetId() - ID_CTX_MAKE_CALL;
    if ((phoneIdx >= 0) && (phoneIdx < rPhones.size())) {
      wxString strNumber;
      rPhones[phoneIdx].getCallableNumber(strNumber);
      CCallMonitorApi *pCM = wxGetApp().getDefaultCallProvider();
      if (pCM) {
        wxLogMessage(wxT("Dialing %s using %s ..."), strNumber,
            wxGetApp().getPrefs().getDefaultCallProvider());
        pCM->makeCall(strNumber.ToUTF8().data());
      } else {
        wxLogMessage(wxT("Can't dial -> no valid default call provider"));
      }
    }
  }
}

void CContactListCtrl::OnCtxMakeCallDlg(wxCommandEvent& e)
{
  DoCallDlg();
}

void CContactListCtrl::OnUpdateAdd(wxUpdateUIEvent& e)
{
  e.Enable(wxGetApp().getCurrentAddressbook() != NULL);
}

void CContactListCtrl::OnUpdateDel(wxUpdateUIEvent& e)
{
  e.Enable(GetSelectedItemCount() > 0);
}

void CContactListCtrl::OnUpdateCallAndEdit(wxUpdateUIEvent& e)
{
  e.Enable(GetSelectedItemCount() == 1);
}

// ----------------------------------------------------------------------------
// CContactPage
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CContactPage, wxPanel)
  EVT_TIMER(wxID_ANY, CContactPage::OnTimer)
  EVT_TEXT(ID_SEARCH_CTRL, CContactPage::OnSearchUpdate)
  EVT_SEARCHCTRL_SEARCH_BTN(ID_SEARCH_CTRL, CContactPage::OnSearch)
  EVT_SEARCHCTRL_CANCEL_BTN(ID_SEARCH_CTRL, CContactPage::OnSearchCancel)
  EVT_BUTTON(CMD_CALL_DLG, CContactPage::OnCall)
  EVT_BUTTON(CMD_ADD_ADDRESS, CContactPage::OnAddAddress)
  EVT_BUTTON(CMD_EDIT_ADDRESS, CContactPage::OnEditAddress)
  EVT_BUTTON(CMD_DEL_ADDRESS, CContactPage::OnDelAddress)
  EVT_BUTTON(CMD_IMPORT, CContactPage::OnImport)
  EVT_BUTTON(CMD_EXPORT, CContactPage::OnExport)
  EVT_CHOICE(ID_CHOICE_ABOOK, CContactPage::OnChoiceAbook)
  EVT_UPDATE_UI(CMD_CALL_DLG, CContactPage::OnUpdateCallAndEdit)
  EVT_UPDATE_UI(CMD_ADD_ADDRESS, CContactPage::OnUpdateAddAndImport)
  EVT_UPDATE_UI(CMD_IMPORT, CContactPage::OnUpdateAddAndImport)
  EVT_UPDATE_UI(CMD_DEL_ADDRESS, CContactPage::OnUpdateDel)
  EVT_UPDATE_UI(CMD_EDIT_ADDRESS, CContactPage::OnUpdateCallAndEdit)
  EVT_UPDATE_UI(CMD_EXPORT, CContactPage::OnUpdateExport)
END_EVENT_TABLE()

CContactPage::CContactPage(wxWindow* parent, wxWindowID id)
  : wxPanel(parent, id, wxDefaultPosition, wxSize(0,0), wxCLIP_CHILDREN)
{
  Create(parent, id);
  m_RegExNumber.Compile(wxT("[[:digit:]]"), wxRE_ADVANCED);
  m_RegExNoNumber.Compile(wxT("[^[:digit:]]"), wxRE_ADVANCED);
}

CContactPage::~CContactPage()
{
}

bool CContactPage::Create(wxWindow* parent, wxWindowID id)
{
  m_Timer.SetOwner(this, GetId());

  wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );
  wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );
  wxButton *btn = new wxButton( this, CMD_CALL_DLG, _("Call..."));
  btn->SetBitmap(RESOURCES::getBitmap(BMP_HANDSET));
  btn->SetToolTip(_("Call selected entry"));
  bSizer2->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
  bSizer2->AddStretchSpacer(1);
  m_choiceBooks = new wxChoice(this, ID_CHOICE_ABOOK);
  m_choiceBooks->SetToolTip(_("Select phonebook"));
  bSizer2->Add(m_choiceBooks, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT|wxRIGHT, 5 );
  m_searchCtrl = new wxSearchCtrl( this, ID_SEARCH_CTRL,
      wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
#ifndef __WXMAC__
  m_searchCtrl->ShowSearchButton( true );
#endif
  m_searchCtrl->ShowCancelButton( true );
  bSizer2->Add( m_searchCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

  bSizer1->Add( bSizer2, 0, wxEXPAND|wxTOP, 5 );
  m_listCtrl = new CContactListCtrl( this, wxID_ANY );
  bSizer1->Add( m_listCtrl, 1, wxALL|wxEXPAND, 5 );

  wxBoxSizer* bSizer3 = new wxBoxSizer( wxHORIZONTAL );
  btn = new wxButton( this, CMD_ADD_ADDRESS, _("Add..."));
  btn->SetBitmap(RESOURCES::getBitmap(BMP_USER_ADD));
  btn->SetToolTip(_("Add a new contact"));
  bSizer3->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
  btn = new wxButton( this, CMD_EDIT_ADDRESS, _("Edit..."));
  btn->SetBitmap(RESOURCES::getBitmap(BMP_USER));
  btn->SetToolTip(_("Edit the selected contact"));
  bSizer3->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
  btn = new wxButton( this, CMD_DEL_ADDRESS, _("Delete"));
  btn->SetBitmap(RESOURCES::getBitmap(BMP_USER_DEL));
  btn->SetToolTip(_("Delete selected contact(s)"));
  bSizer3->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
  bSizer3->AddStretchSpacer(1);
  btn = new wxButton( this, CMD_IMPORT, _("Import..."));
  btn->SetBitmap(RESOURCES::getBitmap(BMP_IMPORT));
  btn->SetToolTip(_("Import address-book data from a text file\nin CSV format (comma separated values)"));
  bSizer3->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
  btn = new wxButton( this, CMD_EXPORT, _("Export..."));
  bSizer3->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
  btn->SetBitmap(RESOURCES::getBitmap(BMP_EXPORT));
  btn->SetToolTip(_("Export address-book data as UTF8-encoded\ntext file according to RFC 4180 (text/csv)"));
  bSizer1->Add( bSizer3, 0, wxEXPAND|wxBOTTOM, 5 );

  SetSizerAndFit( bSizer1 );

  int selection = 0;
  CGUID gid = wxGetApp().getPrefs().getSelectedABook();
  const TAddressbookMap& books = wxGetApp().getAddressbooks();
  for (TAddressbookMap::const_iterator it = books.begin(); it != books.end(); ++it) {
    int pos = m_choiceBooks->Append(it->second->getName(), it->second);
    if (gid == it->second->getID()) {
      selection = pos;
    }
  }
  m_choiceBooks->SetSelection(selection);

//  if (!m_choiceBooks->SetStringSelection()) {
//    if (!books.IsEmpty()) m_choiceBooks->SetSelection(0);
//  }
//  DoSelectAbook(m_choiceBooks->GetStringSelection());

  return true;
}

void CContactPage::OnUpdateCallAndEdit(wxUpdateUIEvent& e)
{
  e.Enable(m_listCtrl->GetSelectedItemCount() == 1);
}

void CContactPage::OnUpdateDel(wxUpdateUIEvent& e)
{
  e.Enable(m_listCtrl->GetSelectedItemCount() > 0);
}

void CContactPage::OnImport(wxCommandEvent&)
{
  CImportCSVDialog dlg(this);
  dlg.CenterOnParent();
  if ( dlg.ShowModal() == wxID_OK) {
    m_listCtrl->Update();
  }
}

void CContactPage::OnExport(wxCommandEvent&)
{
  wxFileDialog dlg(this, _("Save CSV File"),
      wxEmptyString, wxEmptyString,
      _("CSV Files (*.csv;*.txt)|*.csv;*.txt||"),
      wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

  if (dlg.ShowModal() == wxID_OK) {
    DoExport(dlg.GetPath());
  }
}

void CContactPage::OnCall(wxCommandEvent& e)
{
  m_listCtrl->DoCallDlg();
}

void CContactPage::OnAddAddress(wxCommandEvent& e)
{
  m_listCtrl->AddAddress();
}

void CContactPage::OnEditAddress(wxCommandEvent& e)
{
  m_listCtrl->EditAddress();
}

void CContactPage::OnDelAddress(wxCommandEvent& e)
{
  m_listCtrl->DelAddress();
}

void CContactPage::OnUpdateExport(wxUpdateUIEvent& e)
{
  e.Enable(m_listCtrl->GetItemCount() > 0);
}

void CContactPage::OnUpdateAddAndImport(wxUpdateUIEvent& e)
{
  e.Enable(wxGetApp().getCurrentAddressbook() != NULL);
}

void CContactPage::OnChoiceAbook(wxCommandEvent&)
{
  int sel = m_choiceBooks->GetSelection();
  CAddressbookApi *book = reinterpret_cast<CAddressbookApi *>
    (m_choiceBooks->GetClientData(sel));
  DoSelectAbook(book->getID());
}

void CContactPage::DoSearch()
{
  wxString strText, strNumber;
  CAddressbookApi *pBook = wxGetApp().getCurrentAddressbook();
  if (pBook) {
    strText = m_searchCtrl->GetValue();
    strNumber = strText;
    m_RegExNoNumber.ReplaceAll(&strNumber, wxEmptyString);
    m_RegExNumber.ReplaceAll(&strText, wxEmptyString);
    pBook->setNameAndNumberFilter(strText, strNumber);
    m_listCtrl->Update();
  }
}

void CContactPage::OnSearch(wxCommandEvent&)
{
  DoSearch();
}

void CContactPage::OnSearchCancel(wxCommandEvent&)
{
  m_searchCtrl->Clear();
  DoSearch();
}

void CContactPage::OnSearchUpdate(wxCommandEvent&)
{
  m_Timer.Start(300, wxTIMER_ONE_SHOT);
}

void CContactPage::OnTimer(wxTimerEvent& e)
{
  DoSearch();
}

void CContactPage::DoSelectAbook(const CGUID& gidBook)
{
  wxGetApp().setCurrentAddressbook(gidBook);
  m_listCtrl->DelayedUpdate();
}

wxString CSVQuote(const wxString& str)
{
  wxString rc;
  if (str.Length()) {
    wxString quoted = str;
    quoted.Replace(wxT("\""), wxT("\"\""));
    rc = wxT("\"");
    rc += quoted;
    rc += wxT("\"");
  }
  return rc;
}

bool CContactPage::DoExport(const wxString& strPath)
{
  wxFileOutputStream file(strPath);
  if (!file.IsOk()) return false;

  wxTextOutputStream tos(file, wxEOL_DOS, wxCSConv(wxFONTENCODING_UTF8));

  const TContactList& rEntries = m_listCtrl->getEntries();
  size_t itemCount = rEntries.size();
  wxProgressDialog progress(_("CSV Export"), _("Exporting..."),
      itemCount, this, wxPD_APP_MODAL);
  int numTel = 0;
  for (size_t i = 0; i < itemCount; ++i) {
    if (rEntries[i].getConstPhones().size() > numTel) {
      numTel = rEntries[i].getConstPhones().size();
    }
  }
  wxString strLine;
  strLine += wxT("\"FN\",\"MN\",\"LN\",\"NN\",\"Title\",\"SN\",\"Email\",\"Organization\",\"Image\"");
  for (size_t i = 0; i < numTel; ++i) {
    strLine += ",";
    strLine += wxString::Format(wxT("\"TelNumber%02d\""), i);
  }
  strLine += "\n"; tos.WriteString(strLine);
  for (size_t i = 0; i < itemCount; ++i) {
    strLine.Clear();
    strLine += CSVQuote( rEntries[i].getFN() );
    strLine += ",";
    strLine += CSVQuote( rEntries[i].getMN() );
    strLine += ",";
    strLine += CSVQuote( rEntries[i].getLN() );
    strLine += ",";
    strLine += CSVQuote( rEntries[i].getNN() );
    strLine += ",";
    strLine += CSVQuote( rEntries[i].getTitle() );
    strLine += ",";
    strLine += CSVQuote( rEntries[i].getSN() );
    strLine += ",";
    strLine += CSVQuote( rEntries[i].getEmail() );
    strLine += ",";
    strLine += CSVQuote( rEntries[i].getOrganization() );
    strLine += ",";
    strLine += CSVQuote( rEntries[i].getImage() );
    for (size_t j = 0; j < numTel; ++j) {
      strLine += ",";
      if (j < rEntries[i].getConstPhones().size()) {
        strLine += CSVQuote( rEntries[i].getConstPhones()[j].getNumber() );
      }
    }
    strLine += "\n"; tos.WriteString(strLine);
    progress.Update(i);
  }

  return true;
}
