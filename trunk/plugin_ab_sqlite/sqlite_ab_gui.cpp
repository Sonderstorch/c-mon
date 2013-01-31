
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
#include "sqlite_ab.h"
#include "sqlite_ab_gui.h"
#include "sqlite_ab_main.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// ----------------------------------------------------------------------------
// CPhoneBookDetailDlg
// ----------------------------------------------------------------------------

enum {
  ID_EDIT_000,
  ID_EDIT_001,
  ID_BTN_000
};

class CPhoneBookDetailDlg : public wxDialog
{
public:
  CPhoneBookDetailDlg(
      wxWindow* parent,
      const wxString& strDbName,
      const wxString& strDbFile);

  void OnSelectFile(wxCommandEvent& e);
  void OnChange(wxCommandEvent& e);
  void DoUpdate();

  virtual bool Validate();

  wxString m_strDbName;
  wxString m_strDbFile;

protected:
  wxTextCtrl             *m_pTxtPath;
  wxStdDialogButtonSizer *m_btnSizer;
  CSQLiteAddressbookGui  *m_pGUI;
  bool                    m_bRenameOnly;
};

CPhoneBookDetailDlg::CPhoneBookDetailDlg(wxWindow* parent,
                                         const wxString& strDbName,
                                         const wxString& strDbFile)
  : wxDialog(parent, wxID_ANY, _("Phonebook"), wxDefaultPosition,
             wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
  , m_strDbName(strDbName)
  , m_strDbFile(strDbFile)
  , m_pGUI(reinterpret_cast<CSQLiteAddressbookGui *>(parent))
  , m_bRenameOnly(!strDbFile.IsEmpty())
{
  wxBoxSizer *sizerOut = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 3, 3, 0, 0 );
  fgSizer1->AddGrowableCol( 1 );
  fgSizer1->SetFlexibleDirection( wxBOTH );
  fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  wxTextCtrl *txt1 = new wxTextCtrl(this, ID_EDIT_000, m_strDbName, wxDefaultPosition,
                          wxSize(300, -1), 0, wxGenericValidator(&m_strDbName));
  m_pTxtPath = new wxTextCtrl(this, ID_EDIT_001, m_strDbFile, wxDefaultPosition,
                          wxSize(300, -1), wxTE_READONLY, wxGenericValidator(&m_strDbFile));

  fgSizer1->Add(new wxStaticText(this, wxID_ANY, _("&Name:")), 0,
      wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP, 5);
  fgSizer1->Add(txt1, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT|wxRIGHT, 5);
  fgSizer1->AddSpacer(10);

  fgSizer1->Add(new wxStaticText(this, wxID_ANY, _("Database &File:")), 0,
      wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP, 5);
  fgSizer1->Add(m_pTxtPath, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT|wxRIGHT, 5);
  wxButton *btn = new wxButton(this, ID_BTN_000, wxT("..."),
      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  btn->SetToolTip(_T("Select database file"));
  btn->Enable(!m_bRenameOnly);  // if we have a file already, disable button
  fgSizer1->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

  sizerOut->Add(fgSizer1, 1, wxEXPAND);
  m_btnSizer = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
  sizerOut->Add(m_btnSizer, 0, wxALL|wxALIGN_CENTER, 5);

  SetSizerAndFit(sizerOut);
  CentreOnParent();

  TransferDataToWindow();

  txt1->Connect( ID_EDIT_000, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CPhoneBookDetailDlg::OnChange),NULL,this);
  m_pTxtPath->Connect( ID_EDIT_001, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CPhoneBookDetailDlg::OnChange),NULL,this);
  btn->Connect( ID_BTN_000, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CPhoneBookDetailDlg::OnSelectFile), NULL, this);

  txt1->SetFocus();
  txt1->SelectAll();

  DoUpdate();
}

void CPhoneBookDetailDlg::DoUpdate()
{
  bool bEnable = (!m_strDbName.IsEmpty() &&
                  !m_strDbFile.IsEmpty());
  m_btnSizer->GetAffirmativeButton()->Enable(bEnable);
}

void CPhoneBookDetailDlg::OnChange(wxCommandEvent&)
{
  TransferDataFromWindow();
  DoUpdate();
}

void CPhoneBookDetailDlg::OnSelectFile(wxCommandEvent&)
{
  wxFileDialog dlg(this, _("Select database file"),
      wxEmptyString, wxEmptyString,
      _("DB Files (*.db)|*.db||"), wxFD_OPEN);
  if (dlg.ShowModal() == wxID_OK) {
    m_pTxtPath->SetValue(dlg.GetPath());
  }
}

bool CPhoneBookDetailDlg::Validate()
{
  TransferDataFromWindow();
  bool rc = m_pGUI->ValidateName(m_strDbName);
  if (!rc) {
    wxMessageBox(_("A phone-book entry having the same name already exists."),
        wxMessageBoxCaptionStr, wxOK|wxCENTRE|wxICON_ERROR, this);
    return rc;
  }
  if (!m_bRenameOnly) {
    rc = m_pGUI->ValidatePath(m_strDbFile);
    if (!rc) {
      wxMessageBox(_("A phone-book entry for the same database file already exists."),
          wxMessageBoxCaptionStr, wxOK|wxCENTRE|wxICON_ERROR, this);
    }
  }
  return rc;
}

// ---------------------------------------------------------------------------

enum {
  ID_BTN_ADD    = 1600,
  ID_BTN_DELETE
};

// ---------------------------------------------------------------------------

CSQLiteAddressbookGui::CSQLiteAddressbookGui(CSQLiteAddressbookPlugin *plugin,
                                             wxWindow *parent,
                                             wxWindowID id,
                                             const wxPoint& pos,
                                             const wxSize& size,
                                             long style)
  : wxPanel( parent, id, pos, size, style )
  , m_pPlugin(plugin)
{
  m_pFont = new wxFont(12, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Calibri"));

  wxBoxSizer *sizer = new wxBoxSizer( wxVERTICAL );

  wxStaticText *txt = new wxStaticText(this, wxID_ANY, _("Local Addressbook"));
  txt->SetFont(*m_pFont);
  sizer->Add(txt, 0, wxLEFT|wxRIGHT|wxTOP, 5);
  sizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
      wxDefaultSize, wxHORIZONTAL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  sizer->Add(new wxStaticText(this, wxID_ANY, _("Database files:")),
      wxSizerFlags(0).Expand().Border(wxTOP|wxLEFT, 5));

  wxBoxSizer *iSizer1 = new wxBoxSizer( wxHORIZONTAL );
  m_pList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition,
                                    wxSize(350, 160), wxLC_REPORT);
  m_pList->InsertColumn(1, _("Name"), wxLIST_FORMAT_LEFT, 120);
  m_pList->InsertColumn(2, _("Path"), wxLIST_FORMAT_LEFT, 250);
  m_pList->InsertColumn(2, _("State"), wxLIST_FORMAT_LEFT, 250);
  iSizer1->Add(m_pList, wxSizerFlags(2).Expand());

  wxBoxSizer *iSizer2 = new wxBoxSizer( wxVERTICAL );
  m_pBtnAdd = new wxButton(this, ID_BTN_ADD, _("&Register"));
  m_pBtnAdd->SetToolTip(_("Create new phonebook database file or register existing file"));
  iSizer2->Add(m_pBtnAdd, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
  m_pBtnDel = new wxButton(this, ID_BTN_DELETE, _("&Unregister"));
  m_pBtnDel->SetToolTip(_("Unregister a phonebook database"));
  iSizer2->Add(m_pBtnDel, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

  iSizer1->Add(iSizer2, wxSizerFlags(1).Expand());

  sizer->Add(iSizer1, wxSizerFlags(0).Expand().Border(wxLEFT, 5));

  wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
  hSizer->Add(new wxStaticText(this, wxID_ANY, _("NOTE:")), 0, wxLEFT, 10);
  hSizer->Add(new wxStaticText(this,
      wxID_ANY, _("Putting database files on a network share and using them\n"
                  "simultaneously from several computers is not recommended\n"
                  "and may lead to data corruption.")), 0, wxLEFT, 5);
  sizer->Add( hSizer, 0, wxALL|wxEXPAND, 5 );

  m_pBtnAdd->Connect( ID_BTN_ADD, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CSQLiteAddressbookGui::OnAdd), NULL, this);
  m_pBtnDel->Connect( ID_BTN_DELETE, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CSQLiteAddressbookGui::OnDel), NULL, this);

  m_pBtnDel->Connect( ID_BTN_DELETE, wxEVT_UPDATE_UI,
      wxUpdateUIEventHandler(CSQLiteAddressbookGui::OnUpdateDel), NULL, this);

  m_pList->Connect( wxID_ANY, wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
      wxListEventHandler(CSQLiteAddressbookGui::OnEdit), NULL, this);

  Populate();

  SetSizerAndFit(sizer);
}

CSQLiteAddressbookGui::~CSQLiteAddressbookGui()
{
  delete m_pFont;
}

void CSQLiteAddressbookGui::OnAdd(wxCommandEvent&)
{
  CPhoneBookDetailDlg dlg(this, wxT(""), wxT(""));
  if (dlg.ShowModal() == wxID_OK) {
    m_pPlugin->addAddressbook(dlg.m_strDbName, dlg.m_strDbFile);
    Populate();
    int idx = m_pList->FindItem(-1, dlg.m_strDbName);
    if (idx != -1) m_pList->SetItemState(idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  }
}

void CSQLiteAddressbookGui::OnEdit(wxListEvent& e)
{
  wxString name, path;
  const TAddressbooks& books = m_pPlugin->getAddressbooks();
  int item = e.GetIndex();
  if (item < books.size()) {
    name = books[item]->getName();
    path = books[item]->getPath();
    CPhoneBookDetailDlg dlg(this, name, path);
    if (dlg.ShowModal() == wxID_OK) {
      reinterpret_cast<CSQLiteAddressbook *>(books[item])->
          setName(dlg.m_strDbName);
      Populate();
      int idx = m_pList->FindItem(-1, dlg.m_strDbName);
      if (idx != -1) m_pList->SetItemState(idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
  }
}

void CSQLiteAddressbookGui::OnDel(wxCommandEvent&)
{
  const TAddressbooks& books = m_pPlugin->getAddressbooks();
  long item = m_pList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item < books.size()) {
    reinterpret_cast<CSQLiteAddressbook *>(books[item])->markForDeletion();
    Populate();
    m_pList->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  }
}

void CSQLiteAddressbookGui::OnUpdateDel(wxUpdateUIEvent& e)
{
  e.Enable(m_pList->GetSelectedItemCount() > 0);
}

void CSQLiteAddressbookGui::Populate()
{
  wxString name, path;
  bool bIsDeleted;
  const TAddressbooks& books = m_pPlugin->getAddressbooks();
  m_pList->DeleteAllItems();
  for (int i = 0; i < books.size(); ++i) {
    name = books[i]->getName();
    path = books[i]->getPath();
    bIsDeleted = books[i]->isMarkedForDeletion();
    int idx = m_pList->InsertItem(m_pList->GetItemCount(), name);
    m_pList->SetItem(idx, 1, path);
    wxString state = (bIsDeleted ? _("removed on restart") : wxT(""));
    m_pList->SetItem(idx, 2, state);
  }
}

bool CSQLiteAddressbookGui::ValidateName(wxString& strDbName)
{
  return !m_pPlugin->addressBookNameExists(strDbName);
}

bool CSQLiteAddressbookGui::ValidatePath(wxString& strDbPath)
{
  return !m_pPlugin->addressBookPathExists(strDbPath);
}
