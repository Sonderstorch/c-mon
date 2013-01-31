
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
#include "contactdlg.h"
#include "picturecontrol.h"
#include "resources.h"

#define wxLOG_COMPONENT "GUI"

// ----------------------------------------------------------------------------
// Phone number UI model
// ----------------------------------------------------------------------------

CPhoneNumberModel::CPhoneNumberModel(TPhoneList& rPhones)
  : wxDataViewVirtualListModel(rPhones.size())
  , m_Phones(rPhones)
{
}

void CPhoneNumberModel::AddItem()
{
  CPhone phone(CPhone::NT_GENERAL);
  phone.setNumber(_("<Enter Number>"));
  m_Phones.push_back(phone);
  RowAppended();
}

void CPhoneNumberModel::DeleteItem(const wxDataViewItem &item)
{
  unsigned int row = GetRow(item);

  if (row < m_Phones.size()) {
    TPhoneList::iterator it = m_Phones.begin();
    it += row;
    m_Phones.erase(it);
    RowDeleted(row);
  }
}

void CPhoneNumberModel::GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
{
  static wxString strVal;

  switch (col)
  {
  case Column_NumberType:
    variant = (long) m_Phones[row].getType();
    break;
  case Column_Number:
    variant = m_Phones[row].getNumber();
    break;
  case Column_Note:
    variant = m_Phones[row].getNote();
    break;
  case Column_Info:
    m_Phones[row].getNumberInfo(strVal);
    variant = strVal;
    break;
  }
}

bool CPhoneNumberModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const
{
  if ((col == Column_Number) && !m_Phones[row].isValid()) {
    attr.SetColour(*wxRED);
    attr.SetItalic(true);
    return true;
  }
  return false;
}

bool CPhoneNumberModel::SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
{
  bool rc = true;

  switch (col)
  {
  case Column_NumberType:
    m_Phones[row].setType((CPhone::TNumberType)variant.GetInteger());
    RowChanged(row);
    break;
  case Column_Number:
    m_Phones[row].setNumber(variant.GetString());
    RowChanged(row);
    break;
  case Column_Note:
    m_Phones[row].setNote(variant.GetString());
    RowChanged(row);
    break;
  default:
    rc = false;
    break;
  }
  return rc;
}

wxString CPhoneNumberModel::GetTitle(unsigned int col) const
{
  return wxEmptyString;
}

// ----------------------------------------------------------------------------
// CContactDialog
// ----------------------------------------------------------------------------

class CPhoneNumberRenderer: public wxDataViewCustomRenderer
{
public:
  CPhoneNumberRenderer();

  bool GetValue( wxVariant &value ) const;
  bool SetValue( const wxVariant &value );

  bool Render(wxRect cell, wxDC *dc, int state);
  wxSize GetSize() const;

  bool HasEditorCtrl() const;
  wxWindow* CreateEditorCtrl( wxWindow *parent, wxRect labelRect,
                              const wxVariant &value );
  bool GetValueFromEditorCtrl( wxWindow* editor, wxVariant &value );

protected:
  wxString m_strValue;

  DECLARE_DYNAMIC_CLASS_NO_COPY(CPhoneNumberRenderer)
};

IMPLEMENT_CLASS(CPhoneNumberRenderer, wxDataViewCustomRenderer)

CPhoneNumberRenderer::CPhoneNumberRenderer()
  : wxDataViewCustomRenderer(wxT("string"), wxDATAVIEW_CELL_EDITABLE)
{
}

bool CPhoneNumberRenderer::SetValue( const wxVariant &value )
{
  m_strValue = value.GetString();
  return true;
}

bool CPhoneNumberRenderer::GetValue( wxVariant &value ) const
{
  value = m_strValue;
  return true;
}

bool CPhoneNumberRenderer::Render(wxRect rect, wxDC *dc, int state)
{
  RenderText( m_strValue, 0, rect, dc, state);
  return true;
}

wxSize CPhoneNumberRenderer::GetSize() const
{
  return wxSize(160, -1);
}

bool CPhoneNumberRenderer::HasEditorCtrl() const
{
  return true;
}

wxWindow* CPhoneNumberRenderer::CreateEditorCtrl(wxWindow *parent,
                                                 wxRect labelRect,
                                                 const wxVariant &value)
{
  wxTextCtrl* ctrl = new wxTextCtrl( parent, wxID_ANY, value,
                                     wxPoint(labelRect.x,labelRect.y),
                                     wxSize(labelRect.width,labelRect.height),
                                     wxTE_PROCESS_ENTER );

  // select the text in the control and place the cursor at the end
  ctrl->SetInsertionPointEnd();
  ctrl->SelectAll();
  return ctrl;
}

bool CPhoneNumberRenderer::GetValueFromEditorCtrl(wxWindow* editor,
                                                  wxVariant &value)
{
  wxString strResult, strValue = ((wxTextCtrl*)editor)->GetValue();
  strValue.Trim(true); strValue.Trim(false);
  CPhone::parseAndFormat(strValue, strResult);
  value = strResult;
  return true;
}

// ----------------------------------------------------------------------------
// CContactDialog
// ----------------------------------------------------------------------------

enum {
  CMD_ADD_PHONE = 1200,
  CMD_DEL_PHONE
};

// Detail Page 1 --------------------------------------------------------------

BEGIN_EVENT_TABLE(CContactDialog::DetailPage1, wxPanel)
  EVT_BUTTON(CMD_ADD_PHONE, CContactDialog::DetailPage1::OnAddPhone)
  EVT_BUTTON(CMD_DEL_PHONE, CContactDialog::DetailPage1::OnDelPhone)
  EVT_UPDATE_UI(CMD_DEL_PHONE, CContactDialog::DetailPage1::OnUpdateDelPhone)
END_EVENT_TABLE()

CContactDialog::DetailPage1::DetailPage1(wxWindow* parent, TPhoneList& phones,  wxWindowID id)
  : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize,
            wxCLIP_CHILDREN|wxNO_BORDER|wxTAB_TRAVERSAL)
{
  m_pModel = new CPhoneNumberModel(phones);
  Create(parent, id);
}

CContactDialog::DetailPage1::~DetailPage1()
{
}

bool CContactDialog::DetailPage1::Create(wxWindow* parent, wxWindowID id)
{
  wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );

  m_dvPhones = new wxDataViewCtrl(this, wxID_ANY,
      wxDefaultPosition, wxSize(-1, 200),
      wxDV_MULTIPLE|wxDV_HORIZ_RULES|wxDV_VERT_RULES|wxDV_ROW_LINES);
  m_dvPhones->AssociateModel(m_pModel);
  wxArrayString choices;
  for (int i = 0; i < CPhone::NT_MAX; ++i) {
    choices.Add(CPhone::getNumberTypeName(i));
  }
  wxDataViewColumn *col01 = new wxDataViewColumn(_("Type"),
      new wxDataViewChoiceByIndexRenderer(choices),
      CPhoneNumberModel::Column_NumberType, 120, wxALIGN_LEFT);
  wxDataViewColumn *col02 = new wxDataViewColumn(_("Number"),
      new CPhoneNumberRenderer(),
      CPhoneNumberModel::Column_Number, 150, wxALIGN_RIGHT);
  wxDataViewColumn *col03 = new wxDataViewColumn(_("Info"),
      new wxDataViewTextRenderer(wxT("string"), wxDATAVIEW_CELL_INERT),
      CPhoneNumberModel::Column_Info, 200, wxALIGN_LEFT);
  wxDataViewColumn *col04 = new wxDataViewColumn(_("Note"),
      new wxDataViewTextRenderer(wxT("string"), wxDATAVIEW_CELL_EDITABLE),
      CPhoneNumberModel::Column_Note, 200, wxALIGN_LEFT);
  m_dvPhones->AppendColumn(col01);
  m_dvPhones->AppendColumn(col02);
  m_dvPhones->AppendColumn(col03);
  m_dvPhones->AppendColumn(col04);
  bSizer1->Add( m_dvPhones, 1, wxALL|wxEXPAND, 5 );

  wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );

  wxButton *btn = new wxButton( this, CMD_ADD_PHONE, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxBU_NOTEXT);
  btn->SetBitmap(RESOURCES::getBitmap(BMP_PLUS));
  btn->SetToolTip(_("Add new phone"));
  bSizer2->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

  btn = new wxButton( this, CMD_DEL_PHONE, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxBU_NOTEXT);
  btn->SetBitmap(RESOURCES::getBitmap(BMP_MINUS));
  btn->SetToolTip(_("Delete phone"));
  bSizer2->Add(btn, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

  bSizer1->Add( bSizer2, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );

  SetSizerAndFit( bSizer1 );

  return true;
}

void CContactDialog::DetailPage1::OnAddPhone(wxCommandEvent& e)
{
  m_pModel->AddItem();
  int row = m_pModel->GetRowCount();
  if (row) {
    wxDataViewItem item = m_pModel->GetItem(row-1);
    m_dvPhones->EnsureVisible(item);
    m_dvPhones->EditItem(item, m_dvPhones->GetColumn(1));
  }
}

void CContactDialog::DetailPage1::OnDelPhone(wxCommandEvent& e)
{
  wxDataViewItemArray selections;
  m_dvPhones->GetSelections(selections);
  if (selections.size()) {
    for (int i = selections.size() - 1; i >= 0; --i) {
      m_pModel->DeleteItem(selections[i]);
    }
  }
}

void CContactDialog::DetailPage1::OnUpdateDelPhone(wxUpdateUIEvent& e)
{
  e.Enable(m_dvPhones->HasSelection());
}

// ----------------------------------------------------------------------------
// Main Dialog
// ----------------------------------------------------------------------------

enum {
  ID_TXT_FN = 1100, // First name
  ID_TXT_MN,        // Middle name
  ID_TXT_LN,        // Last name
  ID_TXT_TITLE,     // Title
  ID_CB_SN,         // Sorted name
  ID_TXT_NN,        // Nick name
  ID_TXT_ORG,       // Organization
  ID_TXT_MAIL,      // Email
  ID_PICTURE,
  ID_PICTURE_ADD,
  ID_PICTURE_DEL
};

BEGIN_EVENT_TABLE(CContactDialog, wxDialog)
  EVT_TEXT(ID_TXT_FN, CContactDialog::OnNameChanged)
  EVT_TEXT(ID_TXT_MN, CContactDialog::OnNameChanged)
  EVT_TEXT(ID_TXT_LN, CContactDialog::OnNameChanged)
  EVT_UPDATE_UI(wxID_OK, CContactDialog::OnUpdateOk)
  EVT_BUTTON(ID_PICTURE_ADD, CContactDialog::OnAddPicture)
  EVT_BUTTON(ID_PICTURE_DEL, CContactDialog::OnDelPicture)
  EVT_UPDATE_UI(ID_PICTURE_DEL, CContactDialog::OnUpdateDelPicture)
END_EVENT_TABLE()

CContactDialog::CContactDialog(wxWindow* parent, CContact& rContact)
  : wxDialog(parent, wxID_ANY, _("Contact"), wxDefaultPosition,
             wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
  wxBoxSizer* bSizer10 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* bSizer12 = new wxBoxSizer(wxHORIZONTAL);

  wxGridBagSizer* gbSizer3 = new wxGridBagSizer(0, 0);
  gbSizer3->SetFlexibleDirection(wxBOTH);
  gbSizer3->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  gbSizer3->Add(new wxStaticText(this, wxID_ANY, _("Title"),
      wxDefaultPosition, wxDefaultSize, 0), wxGBPosition(0, 0), wxGBSpan(1, 1),
      wxALIGN_BOTTOM | wxLEFT, 5);

  gbSizer3->Add(new wxStaticText(this, wxID_ANY, _("First Name"),
      wxDefaultPosition, wxDefaultSize, 0), wxGBPosition(0, 1), wxGBSpan(1, 1),
      wxALIGN_BOTTOM | wxLEFT, 5);

  gbSizer3->Add(new wxStaticText(this, wxID_ANY, _("Middle Name"),
      wxDefaultPosition, wxDefaultSize, 0), wxGBPosition(0, 2), wxGBSpan(1, 1),
      wxALIGN_BOTTOM | wxLEFT, 5);

  gbSizer3->Add(new wxStaticText(this, wxID_ANY, _("Last Name"),
      wxDefaultPosition, wxDefaultSize, 0), wxGBPosition(0, 3), wxGBSpan(1, 1),
      wxALIGN_BOTTOM | wxLEFT, 5);

  m_txtTitle = new wxTextCtrl(this, ID_TXT_TITLE, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, wxTE_LEFT,
      wxTextValidator(wxFILTER_NONE, &m_data.strTitle));
  gbSizer3->Add(m_txtTitle, wxGBPosition(1, 0), wxGBSpan(1, 1), wxLEFT|wxRIGHT|wxBOTTOM, 5);

  m_txtFN = new wxTextCtrl(this, ID_TXT_FN, wxEmptyString,
      wxDefaultPosition, wxSize(150, -1), wxTE_LEFT,
      wxTextValidator(wxFILTER_NONE, &m_data.strFN));
  gbSizer3->Add(m_txtFN, wxGBPosition(1, 1), wxGBSpan(1, 1), wxLEFT|wxRIGHT|wxBOTTOM, 5);

  m_txtMN = new wxTextCtrl(this, ID_TXT_MN, wxEmptyString,
      wxDefaultPosition, wxSize(150, -1), wxTE_LEFT,
      wxTextValidator(wxFILTER_NONE, &m_data.strMN));
  gbSizer3->Add(m_txtMN, wxGBPosition(1, 2), wxGBSpan(1, 1), wxLEFT|wxRIGHT|wxBOTTOM, 5);

  m_txtLN = new wxTextCtrl(this, ID_TXT_LN, wxEmptyString,
      wxDefaultPosition, wxSize(150, -1), wxTE_LEFT,
      wxTextValidator(wxFILTER_NONE, &m_data.strLN));
  gbSizer3->Add(m_txtLN, wxGBPosition(1, 3), wxGBSpan(1, 1), wxLEFT|wxRIGHT|wxBOTTOM, 5);

  gbSizer3->Add(new wxStaticText(this, wxID_ANY, _("Organization"),
      wxDefaultPosition, wxDefaultSize, 0), wxGBPosition(2, 0), wxGBSpan(1, 1),
      wxALIGN_BOTTOM | wxLEFT, 5);

  gbSizer3->Add(new wxStaticText(this, wxID_ANY, _("Email Address"),
      wxDefaultPosition, wxDefaultSize, 0), wxGBPosition(2, 1), wxGBSpan(1, 1),
      wxALIGN_BOTTOM | wxLEFT, 5);

  gbSizer3->Add(new wxStaticText(this, wxID_ANY, _("Nickname"),
      wxDefaultPosition, wxDefaultSize, 0), wxGBPosition(2, 2), wxGBSpan(1, 1),
      wxALIGN_BOTTOM | wxLEFT, 5);

  m_txtOrg = new wxTextCtrl(this, ID_TXT_ORG, wxEmptyString,
      wxDefaultPosition, wxSize(150, -1), wxTE_LEFT,
      wxTextValidator(wxFILTER_NONE, &m_data.strOrganization));
  gbSizer3->Add(m_txtOrg, wxGBPosition(3, 0), wxGBSpan(1, 1), wxLEFT|wxRIGHT|wxBOTTOM, 5);

  m_txtMail = new wxTextCtrl(this, ID_TXT_MAIL, wxEmptyString,
      wxDefaultPosition, wxSize(150, -1), wxTE_LEFT,
      wxTextValidator(wxFILTER_NONE, &m_data.strEmail));
  gbSizer3->Add(m_txtMail, wxGBPosition(3, 1), wxGBSpan(1, 1), wxLEFT|wxRIGHT|wxBOTTOM, 5);

  m_txtNN = new wxTextCtrl(this, ID_TXT_NN, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, wxTE_LEFT,
      wxTextValidator(wxFILTER_NONE, &m_data.strNN));
  gbSizer3->Add(m_txtNN, wxGBPosition(3, 2), wxGBSpan(1, 1), wxLEFT|wxRIGHT|wxBOTTOM, 5);

  gbSizer3->Add(new wxStaticText(this, wxID_ANY, _("Displayed as"),
      wxDefaultPosition, wxDefaultSize, 0), wxGBPosition(4, 0), wxGBSpan(1, 1),
      wxALIGN_BOTTOM | wxLEFT, 5);

  m_cbSN = new wxComboBox(this, ID_CB_SN, wxEmptyString, wxDefaultPosition,
      wxSize(200, -1), 0, NULL, wxCB_DROPDOWN,
      wxTextValidator(wxFILTER_NONE, &m_data.strSN));
  gbSizer3->Add(m_cbSN, wxGBPosition(5, 0), wxGBSpan(1, 2), wxLEFT|wxRIGHT|wxBOTTOM, 5);

  bSizer12->Add(gbSizer3, 1, wxALL, 5);

  wxBoxSizer *vSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hSizer = new wxBoxSizer(wxHORIZONTAL);
  m_bmpPhoto = new CPictureControl(this, ID_PICTURE, false);
  vSizer->Add(m_bmpPhoto, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxRIGHT, 5);
  wxButton *btn = new wxButton(this, ID_PICTURE_ADD, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxBU_NOTEXT);
  btn->SetBitmap(RESOURCES::getBitmap(BMP_PLUS));
  btn->SetToolTip(_("Add Picture"));
  hSizer->Add(btn, 0, wxALIGN_LEFT|wxALL, 1);
  btn = new wxButton(this, ID_PICTURE_DEL, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxBU_NOTEXT);
  btn->SetBitmap(RESOURCES::getBitmap(BMP_MINUS));
  btn->SetToolTip(_("Delete Picture"));
  hSizer->Add(btn, 0, wxALIGN_LEFT|wxALL, 1);
  vSizer->Add(hSizer, 0, wxEXPAND, 0);

  bSizer12->Add(vSizer, 0, wxALIGN_LEFT|wxALIGN_TOP, 0);

  bSizer10->Add(bSizer12, 0, wxEXPAND, 5);

  SetData(rContact);    // must be after picture (SetImageData)
                        // and before detail page (m_data.vPhones)

  wxBoxSizer* bSizer11 = new wxBoxSizer(wxVERTICAL);
  m_nbDetails = new wxNotebook(this, wxID_ANY,
      wxDefaultPosition, wxDefaultSize,
      wxBK_DEFAULT);
  m_nbDetails->AddPage(
      new DetailPage1(m_nbDetails, m_data.vPhones),
      _("Phones"), true);
  bSizer11->Add(m_nbDetails, 1, wxEXPAND, 0);
  bSizer10->Add(bSizer11, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  wxStdDialogButtonSizer *btnBox = new wxStdDialogButtonSizer();
  btnBox->AddButton(new wxButton(this, wxID_OK));
  btnBox->AddButton(new wxButton(this, wxID_CANCEL));
  btnBox->Realize();
  bSizer10->Add(btnBox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  btnBox->GetAffirmativeButton()->Enable(false);

  SetSizerAndFit(bSizer10);

  TransferDataToWindow();
}

void CContactDialog::updateSN()
{
  wxString strFN       = m_txtFN->GetValue();
  wxString strMN       = m_txtMN->GetValue();
  wxString strLN       = m_txtLN->GetValue();
  m_Names.clear();
  if (strFN.Length() || strMN.Length() || strLN.Length())
  {
    wxString strName;
    if (strLN.Length()) {
      strName = strLN;
      if (strFN.Length()) {
        strName += wxT(", "); strName += strFN;
        if (strMN.Length()) {
          strName += wxT(" "); strName += strMN;
        }
        m_Names.Add(strName);
      }
      else {
        strName = strLN;
        if (strMN.Length()) {
          strName += wxT(", "); strName += strMN;
        }
        m_Names.Add(strName);
      }
    }
    if (strFN.Length()) {
      strName = strFN;
      if (strMN.Length()) {
        strName += wxT(" "); strName += strMN;
      }
      if (strLN.Length()) {
        strName += wxT(" ");  strName += strLN;
      }
      m_Names.Add(strName);
    }
  }
}

bool CContactDialog::findSN(const wxString& s)
{
  for (int i = 0; i < m_Names.size(); ++i) {
    if (m_Names[i] == s) return true;
  }
  return false;
}

void CContactDialog::OnNameChanged(wxCommandEvent&)
{
  wxString strOldValue = m_cbSN->GetValue();
  bool bSNUnaltered = findSN(strOldValue);
  updateSN();
  m_cbSN->Clear();
  if (m_Names.GetCount() > 0) {
    m_cbSN->Append(m_Names);
    if (!strOldValue.Length() || bSNUnaltered) {
      m_cbSN->ChangeValue(m_Names[0]);
    }
  }
  if (!bSNUnaltered && strOldValue.Length()) {
    m_cbSN->ChangeValue(strOldValue);
  }
}

void CContactDialog::OnUpdateOk(wxUpdateUIEvent& event)
{
  wxString strFN = m_txtFN->GetValue();
  wxString strLN = m_txtLN->GetValue();
  event.Enable( (strFN.Length() > 0) || (strLN.Length() > 0) );
}

void CContactDialog::OnAddPicture(wxCommandEvent&)
{
  m_bmpPhoto->AddImage();
}

void CContactDialog::OnDelPicture(wxCommandEvent&)
{
  m_bmpPhoto->RemoveImage();
}

void CContactDialog::OnUpdateDelPicture(wxUpdateUIEvent& e)
{
  e.Enable(m_bmpPhoto->hasImage());
}

void CContactDialog::EndModal(int retCode)
{
  if (retCode == wxID_OK) {
    m_data.strImage = m_bmpPhoto->GetImageData();
  }
  wxDialog::EndModal(retCode);
}

void CContactDialog::SetData(const CContact& c)
{
  m_data = c;
  m_bmpPhoto->SetImageData(m_data.strImage);
}

void CContactDialog::GetData(CContact& c)
{
  c = m_data;
}
