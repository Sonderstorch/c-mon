
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

// $Revision: 33 $ $Date: 2012-10-01 21:52:52 +0200 (Mo, 01 Okt 2012) $

#include "stdwx.h"
#include "psql_ab_gui.h"
#include "psql_ab_connection.h"
#include "psql_ab_prefs.h"

#include <libpq-fe.h>

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// ----------------------------------------------------------------------------
// CTextMessageDlg
// ----------------------------------------------------------------------------

class CTextMessageDlg : public wxDialog
{
public:
  CTextMessageDlg(wxWindow* parent, const wxString& msg, const wxString& title);
};

// ---

CTextMessageDlg::CTextMessageDlg(wxWindow* parent,
                                 const wxString& msg,
                                 const wxString& title)
  : wxDialog(parent, wxID_ANY, title, wxDefaultPosition,
           wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
  wxBoxSizer *sizerOut = new wxBoxSizer(wxVERTICAL);
  sizerOut->Add(new wxTextCtrl(this, wxID_ANY, msg,
      wxDefaultPosition, wxSize(500, 400), wxTE_MULTILINE),
      1, wxEXPAND|wxALL, 5);
  sizerOut->Add(CreateStdDialogButtonSizer(wxOK), 0, wxALL|wxALIGN_CENTER, 5);
  SetSizerAndFit(sizerOut);
  CentreOnParent();
}

// ----------------------------------------------------------------------------
// CCredentialsDlg
// ----------------------------------------------------------------------------

enum {
  ID_EDIT_000,
  ID_EDIT_001,
  ID_EDIT_002
};

class CCredentialsDlg : public wxDialog
{
public:
  CCredentialsDlg(wxWindow* parent,
      const wxString& strRootDb,
      const wxString& strRootUser,
      const wxString& strRootPwd);

  void OnChange(wxCommandEvent& e);
  void DoUpdate();

  wxString m_strRootDb;
  wxString m_strRootUser;
  wxString m_strRootPwd;

protected:
  wxStdDialogButtonSizer *m_btnSizer;
};

CCredentialsDlg::CCredentialsDlg(wxWindow* parent,
                                 const wxString& strRootDb,
                                 const wxString& strRootUser,
                                 const wxString& strRootPwd)
  : wxDialog(parent, wxID_ANY, _("Database Credentials"), wxDefaultPosition,
             wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
  , m_strRootDb(strRootDb)
  , m_strRootUser(strRootUser)
  , m_strRootPwd(strRootPwd)
{
  wxBoxSizer *sizerOut = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 3, 2, 0, 0 );
  fgSizer1->AddGrowableCol( 1 );
  fgSizer1->SetFlexibleDirection( wxBOTH );
  fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  wxTextCtrl *txt1 = new wxTextCtrl(this, ID_EDIT_000, m_strRootDb, wxDefaultPosition,
                          wxSize(300, -1), 0, wxGenericValidator(&m_strRootDb));
  wxTextCtrl *txt2 = new wxTextCtrl(this, ID_EDIT_001, m_strRootUser, wxDefaultPosition,
                          wxSize(300, -1), 0, wxGenericValidator(&m_strRootUser));
  wxTextCtrl *txt3 = new wxTextCtrl(this, ID_EDIT_002, m_strRootPwd, wxDefaultPosition,
                          wxSize(300, -1), wxTE_PASSWORD, wxGenericValidator(&m_strRootPwd));

  fgSizer1->Add(new wxStaticText(this, wxID_ANY, _("&Root Database:")), 0, wxLEFT|wxTOP, 5);
  fgSizer1->Add(txt1, 1, wxEXPAND|wxTOP|wxLEFT, 5);

  fgSizer1->Add(new wxStaticText(this, wxID_ANY, _("Root &User:")), 0, wxLEFT|wxTOP, 5);
  fgSizer1->Add(txt2, 1, wxEXPAND|wxTOP|wxLEFT, 5);

  fgSizer1->Add(new wxStaticText(this, wxID_ANY, _("Root &Password:")), 0, wxLEFT|wxTOP, 5);
  fgSizer1->Add(txt3, 1, wxEXPAND|wxTOP|wxLEFT, 5);

  sizerOut->Add(fgSizer1, 1, wxEXPAND);
  m_btnSizer = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
  sizerOut->Add(m_btnSizer, 0, wxALL|wxALIGN_CENTER, 5);

  SetSizerAndFit(sizerOut);
  CentreOnParent();

  TransferDataToWindow();

  txt1->Connect( ID_EDIT_000, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CCredentialsDlg::OnChange),NULL,this);
  txt2->Connect( ID_EDIT_001, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CCredentialsDlg::OnChange),NULL,this);
  txt3->Connect( ID_EDIT_002, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CCredentialsDlg::OnChange),NULL,this);

  txt1->SetFocus();
  txt1->SelectAll();

  DoUpdate();
}

void CCredentialsDlg::DoUpdate()
{
  bool bEnable = (!m_strRootDb.IsEmpty() &&
                  !m_strRootUser.IsEmpty() &&
                  !m_strRootPwd.IsEmpty());
  m_btnSizer->GetAffirmativeButton()->Enable(bEnable);
}

void CCredentialsDlg::OnChange(wxCommandEvent& e)
{
  TransferDataFromWindow();
  DoUpdate();
}

// ---------------------------------------------------------------------------

enum {
  ID_EDIT1,
  ID_EDIT2,
  ID_EDIT3,
  ID_EDIT4,
  ID_EDIT5,
  ID_BTN_TEST,
  ID_BTN_CREATEDB,
  ID_BTN_SCHEMA,
  ID_CHECK_OS_USER
};

CPSQLAddressbookGui::CPSQLAddressbookGui( wxWindow *parent,
                                          wxWindowID id,
                                          const wxPoint& pos,
                                          const wxSize& size,
                                          long style)
  : wxPanel( parent, id, pos, size, style )
{
  CPSqlPrefs& rPrefs = PSQLPREFS::getPrefs();

  m_pFont = new wxFont(12, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Calibri"));

  wxBoxSizer *bSizer1 = new wxBoxSizer( wxVERTICAL );

  wxStaticText *txt = new wxStaticText(this, wxID_ANY, _("Addressbook for PostgreSQL"));
  txt->SetFont(*m_pFont);
  bSizer1->Add(txt, 0, wxLEFT|wxRIGHT|wxTOP, 5);
  bSizer1->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
      wxDefaultSize, wxHORIZONTAL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 8, 2, 0, 0 );
  fgSizer1->AddGrowableCol( 1 );
  fgSizer1->SetFlexibleDirection( wxBOTH );
  fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  wxTextCtrl *txt1 = new wxTextCtrl( this, ID_EDIT1, wxEmptyString, wxDefaultPosition,
      wxSize(400, -1), 0, wxGenericValidator(&rPrefs.m_strHost) );
  wxTextCtrl *txt2 = new wxTextCtrl( this, ID_EDIT2, wxEmptyString, wxDefaultPosition,
      wxSize(400, -1), 0, wxGenericValidator((int *)&rPrefs.m_nHostPort) );
  wxTextCtrl *txt3 = new wxTextCtrl( this, ID_EDIT3, wxEmptyString, wxDefaultPosition,
      wxSize(400, -1), 0, wxGenericValidator(&rPrefs.m_strDatabase) );
  m_txtUser = new wxTextCtrl( this, ID_EDIT4, wxEmptyString, wxDefaultPosition,
      wxSize(400, -1), 0, wxGenericValidator(&rPrefs.m_strUsername) );
  m_txtPwd = new wxTextCtrl( this, ID_EDIT5, wxEmptyString, wxDefaultPosition,
      wxSize(400, -1), wxTE_PASSWORD, wxGenericValidator(&rPrefs.m_strPassword) );
  m_txtPwdNote = new wxStaticText( this, wxID_ANY,
        _("NOTE: The password will be stored in clear-text"),
        wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
  m_chkUseOSUser = new wxCheckBox(this, ID_CHECK_OS_USER,
      _("Use user account of operating system"),
      wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_VERTICAL,
      wxGenericValidator(&rPrefs.m_bUseOSUser));
  m_chkUseOSUser->SetToolTip(
      _("If enabled, your PostgreSQL server needs to be\n"
        "configured for single-sign-on (SSPI or GSSAPI)"));
  fgSizer1->Add( new wxStaticText( this, wxID_ANY, _("&Server:"),
      wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT ),
      0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  fgSizer1->Add( txt1, 0, wxALL, 5 );

  fgSizer1->Add( new wxStaticText( this, wxID_ANY, _("&Port:"),
      wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT ),
      0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  fgSizer1->Add( txt2, 0, wxALL, 5 );

  fgSizer1->Add( new wxStaticText( this, wxID_ANY, _("&Database:"),
      wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT ),
      0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  fgSizer1->Add( txt3, 0, wxALL, 5 );

  fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
  fgSizer1->Add( m_chkUseOSUser, 0, wxALL, 5 );

  fgSizer1->Add( new wxStaticText( this, wxID_ANY, _("&User:"),
      wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT ),
      0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  fgSizer1->Add( m_txtUser, 0, wxALL, 5 );

  fgSizer1->Add( new wxStaticText( this, wxID_ANY, _("Pass&word:"),
      wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT ),
      0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  fgSizer1->Add( m_txtPwd, 0, wxALL, 5 );
  fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
  fgSizer1->Add( m_txtPwdNote, 0, wxALL, 5 );

  wxBoxSizer *btnSizer = new wxBoxSizer( wxHORIZONTAL );
  m_btnTest = new wxButton( this, ID_BTN_TEST, _("Test"),
      wxDefaultPosition, wxDefaultSize, 0 );
  btnSizer->Add( m_btnTest, 0, 0, 0 );
  m_btnCreate = new wxButton( this, ID_BTN_CREATEDB, _("Create Database..."),
      wxDefaultPosition, wxDefaultSize, 0 );
  btnSizer->Add( m_btnCreate, 0, wxLEFT, 5 );
  wxButton *btnSchema = new wxButton( this, ID_BTN_SCHEMA, _("Display Schema..."),
      wxDefaultPosition, wxDefaultSize, 0 );
  btnSizer->Add( btnSchema, 0, wxLEFT, 5 );

  fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
  fgSizer1->Add(btnSizer, 0, wxALL, 5);

  bSizer1->Add(fgSizer1, 0, wxEXPAND, 0);

  SetSizerAndFit(bSizer1);

  TransferDataToWindow();

  txt1->Connect( ID_EDIT1, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CPSQLAddressbookGui::OnChange),NULL,this);
  txt2->Connect( ID_EDIT2, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CPSQLAddressbookGui::OnChange),NULL,this);
  txt3->Connect( ID_EDIT3, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CPSQLAddressbookGui::OnChange),NULL,this);
  m_txtUser->Connect( ID_EDIT4, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CPSQLAddressbookGui::OnChange),NULL,this);
  m_txtPwd->Connect( ID_EDIT5, wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(CPSQLAddressbookGui::OnChange),NULL,this);
  m_chkUseOSUser->Connect(ID_CHECK_OS_USER, wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(CPSQLAddressbookGui::OnChange),NULL,this);
  m_btnTest->Connect( ID_BTN_TEST, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CPSQLAddressbookGui::OnTest),NULL,this);
  m_btnCreate->Connect( ID_BTN_CREATEDB, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CPSQLAddressbookGui::OnCreateDB),NULL,this);
  btnSchema->Connect( ID_BTN_SCHEMA, wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(CPSQLAddressbookGui::OnDisplaySchema),NULL,this);

  OnUpdate();
}

CPSQLAddressbookGui::~CPSQLAddressbookGui()
{
  delete m_pFont;
}

// --------------------------------------------------------------------------

void CPSQLAddressbookGui::OnUpdate()
{
  CPSqlPrefs& rPrefs = PSQLPREFS::getPrefs();

  bool bBtnEnable = (rPrefs.m_strHost     != wxEmptyString) &&
                    (rPrefs.m_nHostPort   != 0) &&
                    (rPrefs.m_strDatabase != wxEmptyString) &&
                    (rPrefs.m_bUseOSUser || (
                     (rPrefs.m_strUsername != wxEmptyString) &&
                     (rPrefs.m_strPassword != wxEmptyString)));
  m_txtUser->Enable(!rPrefs.m_bUseOSUser);
  m_txtPwd->Enable(!rPrefs.m_bUseOSUser);
  m_txtPwdNote->Enable(!rPrefs.m_bUseOSUser);
  m_btnTest->Enable(bBtnEnable);
  m_btnCreate->Enable(bBtnEnable);
}

// --------------------------------------------------------------------------

void CPSQLAddressbookGui::OnChange(wxCommandEvent& e)
{
  CPSqlPrefs oldPrefs = PSQLPREFS::getPrefs();
  TransferDataFromWindow();
  CPSqlPrefs& newPrefs = PSQLPREFS::getPrefs();
  if (!oldPrefs.useOSUser() && newPrefs.useOSUser()) {
    m_txtUser->SetValue(wxGetUserId());
    m_txtPwd->SetValue(wxEmptyString);
  }
  if (!(oldPrefs == PSQLPREFS::getPrefs())) {
    PSQLPREFS::savePrefs();
  }
  OnUpdate();
}

void CPSQLAddressbookGui::OnTest(wxCommandEvent& e)
{
  CPSqlConnection db;
  CPSqlPrefs& p = PSQLPREFS::getPrefs();

  db.setHost(p.getHost().ToUTF8().data());
  db.setPort(p.getPort());
  db.setDatabase(p.getDatabase().ToUTF8().data());
  db.setUser(p.getUsername().ToUTF8().data());
  db.setPass(p.getPassword().ToUTF8().data());

  if (db.connect()) {
    db.disconnect();
    wxMessageBox(_("Successfully connected to database."));
  } else {
    db.disconnect();  // to be sure
    wxString strError = wxString::FromUTF8(db.getLastError().c_str());
    if (!strError.Length()) {
      // On errors early in the connection phase PostgreSQL still hasn't the
      // client encoding switched to UTF8 - so try ISO8859_1 also
      strError = wxString::From8BitData(db.getLastError().c_str());
    }
    wxMessageBox(wxString::Format(_("Failed to connect to database:\n%s"), strError));
  }
}


void CPSQLAddressbookGui::OnCreateDB(wxCommandEvent& e)
{
  CPSqlConnection db;
  CPSqlPrefs& p = PSQLPREFS::getPrefs();

  db.setHost(p.getHost().ToUTF8().data());
  db.setPort(p.getPort());
  db.setDatabase(p.getDatabase().ToUTF8().data());
  db.setUser(p.getUsername().ToUTF8().data());
  db.setPass(p.getPassword().ToUTF8().data());

  CCredentialsDlg dlg(this, wxT("postgres"), wxT("postgres"), wxEmptyString);
  if (dlg.ShowModal() == wxID_OK)
  {
    if (db.createSchema(dlg.m_strRootDb.ToUTF8().data(),
                        dlg.m_strRootUser.ToUTF8().data(),
                        dlg.m_strRootPwd.ToUTF8().data()))
    {
      wxMessageBox(_("Successfully created database."));
    }
    else
    {
      wxString strError = wxString::FromUTF8(db.getLastError().c_str());
      if (!strError.Length()) {
        strError = wxString::From8BitData(db.getLastError().c_str());
      }
      wxMessageBox(wxString::Format(_("Failed to create database:\n%s"), strError));
    }
  }
}

void CPSQLAddressbookGui::OnDisplaySchema(wxCommandEvent& e)
{
  CPSqlConnection db;
  CTextMessageDlg dlg(this, db.getSchema().c_str(), _("Database Schema"));
  dlg.ShowModal();
}
