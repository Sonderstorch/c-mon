
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
#include "glob/contact.h"
#include "csvimportdlg.h"
#include "pluginloader.h"

#include "glob/prefs.h"
#include "mainapp.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI/CVSIMPORT"

enum {
  ID_CB_ENCODING = 1200,
  ID_CB_DELIMITER,
  ID_CB_QUOTE_CHAR,
  ID_IMP_CTRL,
  ID_SPIN_START_LINE,
  ID_BTN_FILE
};

// CImportCSVDialog -----------------------------------------------------------

BEGIN_EVENT_TABLE(CImportCSVDialog, wxDialog)
  EVT_BUTTON(ID_BTN_FILE, CImportCSVDialog::OnOpenFile)
  EVT_BUTTON(wxID_APPLY, CImportCSVDialog::OnImport)
  EVT_CHOICE(ID_CB_ENCODING, CImportCSVDialog::OnChoiceChange)
  EVT_CHOICE(ID_CB_DELIMITER, CImportCSVDialog::OnChoiceChange)
  EVT_CHOICE(ID_CB_QUOTE_CHAR, CImportCSVDialog::OnChoiceChange)
  EVT_SPINCTRL(ID_SPIN_START_LINE, CImportCSVDialog::OnSpinChange)
  EVT_UPDATE_UI(wxID_APPLY, CImportCSVDialog::OnUpdateImportBtn)
END_EVENT_TABLE()

CImportCSVDialog::CImportCSVDialog(wxWindow* parent)
  : wxDialog(parent, wxID_ANY, _("CSV Import"), wxDefaultPosition,
             wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
  wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );
  wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );

  // Encoding
  bSizer2->Add(new wxStaticText(this, wxID_ANY,
      _("&Encoding:")), 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
  m_cbEncoding = new wxChoice(this, ID_CB_ENCODING,
      wxDefaultPosition, wxSize(250, -1));
  m_cbEncoding->Append(wxFontMapper::GetEncodingDescription(wxFONTENCODING_DEFAULT),
      (void *) wxFONTENCODING_DEFAULT);
  for (size_t enc = 0; enc < wxFontMapper::GetSupportedEncodingsCount(); ++enc) {
    wxFontEncoding fenc = wxFontMapper::GetEncoding(enc);
    if (fenc != wxFONTENCODING_DEFAULT) {
      m_cbEncoding->Append(wxFontMapper::GetEncodingDescription(fenc), (void *)fenc);
    }
  }
  m_cbEncoding->Select(0);
  bSizer2->Add(m_cbEncoding, 0, wxLEFT|wxRIGHT, 5);

  // Delimiter
  bSizer2->Add(new wxStaticText(this, wxID_ANY,
      _("&Field-Separator:")), 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
  m_cbDelimiter = new wxChoice(this, ID_CB_DELIMITER,
      wxDefaultPosition, wxSize(80, -1));
  m_cbDelimiter->Append(wxT(";"),     (void *)wxT(';'));
  m_cbDelimiter->Append(wxT(","),     (void *)wxT(','));
  m_cbDelimiter->Append(wxT("{Tab}"), (void *)wxT('\t'));
  m_cbDelimiter->Select(0);
  bSizer2->Add(m_cbDelimiter, 0, wxLEFT|wxRIGHT, 5);

  // Quote-Char
  bSizer2->Add(new wxStaticText(this, wxID_ANY,
      _("&Quote-Char:")), 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
  m_cbQuoteChar = new wxChoice(this, ID_CB_QUOTE_CHAR,
      wxDefaultPosition, wxSize(80, -1));
  m_cbQuoteChar->Append(wxT("\""),     (void *)wxT('\"'));
  m_cbQuoteChar->Append(wxT("'"),      (void *)wxT('\''));
  m_cbQuoteChar->Append(wxT("{None}"), (void *)0);
  m_cbQuoteChar->Select(0);
  bSizer2->Add(m_cbQuoteChar, 0, wxLEFT|wxRIGHT, 5);

  // First Line
  bSizer2->Add(new wxStaticText(this, wxID_ANY,
      _("&Start with line:")), 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
  m_spinStart = new wxSpinCtrl(this, ID_SPIN_START_LINE,
      wxT("1"), wxDefaultPosition, wxSize(50, -1),
      wxSP_ARROW_KEYS|wxALIGN_RIGHT, 1, 999, 1);
  bSizer2->Add(m_spinStart, 0, wxLEFT|wxRIGHT, 5);

  bSizer2->AddStretchSpacer(0);
  bSizer2->Add(new wxButton(this, ID_BTN_FILE,
      _("&File...")), 0, wxLEFT|wxRIGHT, 5);

  bSizer1->Add(bSizer2, 0, wxEXPAND|wxALL, 5);

  wxArrayString columns;
  columns.Add(_("Title"));
  columns.Add(_("First Name"));
  columns.Add(_("Middle Name"));
  columns.Add(_("Last Name"));
  columns.Add(_("Nickname"));
  columns.Add(_("Sorted Name"));
  columns.Add(_("Organization"));
  columns.Add(_("Email"));
  for (int i = 0; i < CPhone::NT_MAX; ++i) {
    columns.Add(wxT("Tel") + CPhone::getNumberTypeName(i));
  }
  m_ImportCtrl = new CCsvImportCtrl(this, ID_IMP_CTRL);
  m_ImportCtrl->setChoices(columns);
  bSizer1->Add(new wxStaticText(this, wxID_ANY,
          _("Pre&view (click or right-click header to select target field):")),
          0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT|wxRIGHT, 5);
  bSizer1->Add(m_ImportCtrl, 1, wxEXPAND|wxALL, 5);

  wxBoxSizer *btnBox = new wxBoxSizer(wxHORIZONTAL);
  btnBox->Add(new wxButton(this, wxID_APPLY, _("&Import")), 0, 0);
  btnBox->AddStretchSpacer();
  btnBox->Add(new wxButton(this, wxID_OK, _("Close")), 0, 0);
  bSizer1->Add(btnBox, 0, wxEXPAND|wxALL, 5);

  SetSizerAndFit( bSizer1 );
}

bool CImportCSVDialog::DoValidate()
{
  bool rc = false;

  int itemCount = m_ImportCtrl->GetItemCount();
  wxProgressDialog progress(_("CSV Import"), _("Validating..."),
      itemCount, this, wxPD_APP_MODAL);
  int choice;
  wxString strData;
  m_ImportCtrl->Freeze();
  for (size_t line = 0; line < itemCount; ++line)
  {
    bool invalid = false;
    for (size_t col = 0; col  < m_ImportCtrl->GetColumnCount(); ++col)
    {
      choice  = m_ImportCtrl->getChoice(col);
      if ((choice >= (9 + CPhone::NT_GENERAL)) &&
          (choice <  (9 + CPhone::NT_MAX)))
      {
        strData = m_ImportCtrl->GetItemText(line, col);
        if (strData.Length()) {
          invalid = !CPhone::isPossibleNumber(strData);
          if (invalid) break;
        }
      }
    }
    m_ImportCtrl->setLineInvalid(line, invalid);
    progress.Update(line);
  }
  m_ImportCtrl->Thaw();
  return rc;
}


void CImportCSVDialog::OnImport(wxCommandEvent&)
{
  bool hasChoices = false;
  for (size_t col = 0; col  < m_ImportCtrl->GetColumnCount(); ++col) {
    if (m_ImportCtrl->getChoice(col) != 0) {
      hasChoices = true;
      break;
    }
  }
  if (hasChoices)
  {
    if (!DoValidate()) {
      int rc = wxMessageBox(
          _("Some phone numbers you want to import may be invalid and "
            "have been marked red. Do you want to continue?"),
          wxMessageBoxCaptionStr,  wxYES_NO | wxCENTRE);
      if (rc == wxNO) return;
    }
    int itemCount = m_ImportCtrl->GetItemCount();
    wxProgressDialog progress(_("CSV Import"), _("Importing..."),
        itemCount, this, wxPD_APP_MODAL);
    int choice;
    wxString strData;
    for (size_t line = 0; line < itemCount; ++line)
    {
      {
        CContact c;
        for (size_t col = 0; col  < m_ImportCtrl->GetColumnCount(); ++col)
        {
          choice  = m_ImportCtrl->getChoice(col);
          strData = m_ImportCtrl->GetItemText(line, col);
          switch (choice) {
            case 1: c.setTitle(strData); break;
            case 2: c.setFN(strData); break;
            case 3: c.setMN(strData); break;
            case 4: c.setLN(strData); break;
            case 5: c.setNN(strData); break;
            case 6: c.setSN(strData); break;
            case 7: c.setOrganization(strData); break;
            case 8: c.setEmail(strData); break;
            default:
              if ((choice >= (9 + CPhone::NT_GENERAL)) &&
                  (choice <  (9 + CPhone::NT_MAX))) {
                if (strData.Length()) {
                  CPhone phone((CPhone::TNumberType)(choice-9));
                  phone.setNumber(strData);
                  c.getPhones().push_back(phone);
                }
              }
              break;
          }
        }
        // construct SN field, if empty
        if (c.getSN().IsEmpty())
        {
          wxString strName;
          if (c.getLN().Length())
          {
            strName = c.getLN();
            if (c.getFN().Length()) {
              strName += wxT(", "); strName += c.getFN();
              if (c.getMN().Length()) {
                strName += wxT(" "); strName += c.getMN();
              }
            } else {
              if (c.getMN().Length()) {
                strName += wxT(", "); strName += c.getMN();
              }
            }
            c.setSN(strName);
          }
        }
        wxGetApp().getCurrentAddressbook()->insertUpdateEntry(c);
      }
      progress.Update(line);
    }
  }
}
void CImportCSVDialog::OnOpenFile(wxCommandEvent& e)
{
  wxFileDialog dlg(this, _("Open CSV File"),
      wxEmptyString, wxEmptyString,
      _("CSV Files (*.csv;*.txt)|*.csv;*.txt||"),
      wxFD_OPEN|wxFD_FILE_MUST_EXIST);

  if (dlg.ShowModal() == wxID_OK) {
    m_strPath = dlg.GetPath();
    DoOpenFile();
  }
}

void CImportCSVDialog::OnChoiceChange(wxCommandEvent&)
{
  DoOpenFile();
}

void CImportCSVDialog::OnSpinChange(wxSpinEvent&)
{
  DoOpenFile();
}

void CImportCSVDialog::DoOpenFile()
{
  if (!m_strPath.empty())
  {
    wxFontEncoding fenc = (wxFontEncoding) (int) m_cbEncoding->GetClientData(m_cbEncoding->GetSelection());
    if (fenc == wxFONTENCODING_DEFAULT) fenc = wxFONTENCODING_SYSTEM;
    wxChar sep   = (wxChar) (int) m_cbDelimiter->GetClientData(m_cbDelimiter->GetSelection());
    wxChar quote = (wxChar) (int) m_cbQuoteChar->GetClientData(m_cbQuoteChar->GetSelection());
    int    offs  = m_spinStart->GetValue() - 1;
    wxCSConv conv(fenc);
    m_ImportCtrl->loadFile(m_strPath, sep, (quote != 0), quote, offs, conv);
  }
}

void CImportCSVDialog::OnUpdateImportBtn(wxUpdateUIEvent& e)
{
  e.Enable(m_ImportCtrl->isOk() && (wxGetApp().getCurrentAddressbook() != 0));
}

