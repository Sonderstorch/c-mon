
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
#include "resolverpage.h"
#include "resources.h"

#include "resolver/resolvermodel.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI/RESOLVER"

// ----------------------------------------------------------------------------
// Resolver model
// ----------------------------------------------------------------------------

CResolverUIModel::CResolverUIModel(CResolverModel& model)
  : wxDataViewVirtualListModel()
  , m_model(model)
{
  Reset(m_model.getEntryCount());
}

void CResolverUIModel::AddItem()
{
}

void CResolverUIModel::DeleteItem(const wxDataViewItem &item)
{
  unsigned int row = GetRow(item);
  if (m_model.deleteEntry(row)) {
    RowDeleted(row);
  }
}

void CResolverUIModel::GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
{
  const CResolver *pR = m_model.getEntry(row);
  if (pR) {
    switch (col)
    {
    case Column_Type: {
        int bmp = BMP_ABOOK;
        if (pR->getType() == RT_ONLINE_REGEX) {
          bmp = BMP_WORLD;
        }
        variant << RESOURCES::getBitmap(bmp);
      }
      break;
    case Column_Name:
      variant = pR->getName();
      break;
    case Column_Enabled:
      if (pR->isAvailable()) {
        variant = pR->isEnabled() ? _("Enabled") : _("Disabled");
      } else {
        variant = _("Unavailable");
      }
      break;
    }
  }
}

bool CResolverUIModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const
{
  return false;
}

bool CResolverUIModel::SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
{
  return false;
}

wxString CResolverUIModel::GetTitle(unsigned int col) const
{
  return wxEmptyString;
}

void CResolverUIModel::MoveUp(unsigned int row)
{
  if (m_model.moveUp(row)) {
    Reset(m_model.getEntryCount());
  }
}

void CResolverUIModel::MoveDown(unsigned int row)
{
  if (m_model.moveDown(row)) {
    Reset(m_model.getEntryCount());
  }
}

void CResolverUIModel::Toggle(unsigned int row)
{
  if (m_model.toggle(row)) {
    RowChanged(row);
  }
}

bool CResolverUIModel::Append(CResolver *pR)
{
  bool rc = m_model.appendEntry(pR);
  if (rc) {
    RowAppended();
  } else {
    wxMessageBox(wxString::Format(
      _("Inserting the resolver failed. Either a resolver "
        "called '%s' does already exist or a database error happened."),
      pR->getName()), wxMessageBoxCaptionStr, wxOK|wxICON_ERROR);
  }
  return rc;
}

unsigned int CResolverUIModel::GetRowCount()
{
  return m_model.getEntryCount();
}

// --------------------------------------------------------------------------

enum {
  ID_BTN_IMPORT = 1300,
  ID_BTN_DELETE,
  ID_BTN_EXPORT,
  ID_BTN_ONOFF,
  ID_BTN_MOVE_UP,
  ID_BTN_MOVE_DOWN
};

BEGIN_EVENT_TABLE(CResolverPage, wxPanel)
  EVT_BUTTON(ID_BTN_IMPORT, CResolverPage::OnImport)
  EVT_BUTTON(ID_BTN_EXPORT, CResolverPage::OnExport)
  EVT_BUTTON(ID_BTN_DELETE, CResolverPage::OnDelete)
  EVT_BUTTON(ID_BTN_ONOFF, CResolverPage::OnToggle)
  EVT_BUTTON(ID_BTN_MOVE_UP, CResolverPage::OnMoveUp)
  EVT_BUTTON(ID_BTN_MOVE_DOWN, CResolverPage::OnMoveDown)
  EVT_UPDATE_UI_RANGE(ID_BTN_DELETE, ID_BTN_MOVE_DOWN, CResolverPage::OnUpdateButtons)
END_EVENT_TABLE()

CResolverPage::CResolverPage(CResolverModel& model, wxWindow* parent, wxWindowID id)
  : wxPanel( parent, id, wxDefaultPosition, wxDefaultSize,
             wxTAB_TRAVERSAL|wxNO_BORDER )
  , m_model(model)
{
  m_pUIModel = new CResolverUIModel(m_model);

  wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxStaticText *txt = new wxStaticText(this, wxID_ANY, _("Number Resolvers"));
  txt->SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_BOLD));
  bSizer1->Add(txt, 0, wxLEFT|wxRIGHT|wxTOP, 5);
  bSizer1->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
      wxDefaultSize, wxHORIZONTAL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  bSizer1->Add(new wxStaticText(this, wxID_ANY,
      _("The following sources (local address-books or Internet servers) are queried \n"
        "in sequence to obtain a name for an incoming number:")), 0,
      wxEXPAND|wxLEFT|wxRIGHT, 5);

  wxBoxSizer* bSizer2 = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* bSizer5 = new wxBoxSizer(wxVERTICAL);

  m_dvResolvers = new wxDataViewCtrl(this, wxID_ANY,
      wxDefaultPosition, wxSize(-1, 300),
      wxDV_SINGLE|wxDV_HORIZ_RULES|wxDV_VERT_RULES|wxDV_ROW_LINES);
  m_dvResolvers->AssociateModel(m_pUIModel);
  m_pUIModel->DecRef();
  wxDataViewColumn *col01 = new wxDataViewColumn(wxEmptyString,
      new wxDataViewBitmapRenderer(wxT("wxBitmap"), wxDATAVIEW_CELL_INERT,
          wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL),
      CResolverUIModel::Column_Type, 32, wxALIGN_CENTER);
  wxDataViewColumn *col02 = new wxDataViewColumn(_("Name"),
      new wxDataViewTextRenderer(),
      CResolverUIModel::Column_Name, 200, wxALIGN_LEFT);
  wxDataViewColumn *col03 = new wxDataViewColumn(_("State"),
      new wxDataViewTextRenderer(wxT("string"), wxDATAVIEW_CELL_INERT,
          wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL),
      CResolverUIModel::Column_Enabled, 80);
  m_dvResolvers->AppendColumn(col01);
  m_dvResolvers->AppendColumn(col02);
  m_dvResolvers->AppendColumn(col03);
  bSizer5->Add( m_dvResolvers, 1, wxALL|wxEXPAND, 5 );

  wxBoxSizer* bSizer4 = new wxBoxSizer(wxHORIZONTAL);
  bSizer4->AddStretchSpacer();
  bSizer4->Add(new wxStaticBitmap(this,
      wxID_ANY, RESOURCES::getBitmap(BMP_WORLD)));
  bSizer4->Add(new wxStaticText(this, wxID_ANY, _("NOTE:")), 0, wxLEFT, 10);
  bSizer4->Add(new wxStaticText(this,
      wxID_ANY, _("Resolvers with this icon use public services in the Internet.\n"
                  "The telephone-number to be resolved will be transmitted to\n"
                  "foreign servers. If you don't want this to happen, disable\n"
                  "those resolvers.")), 0, wxLEFT, 5);
  bSizer5->Add( bSizer4, 0, wxALL, 5 );
  bSizer2->Add( bSizer5, 1, wxEXPAND, 0 );

  wxBoxSizer* bSizer3 = new wxBoxSizer(wxVERTICAL);
  bSizer3->Add(new wxButton(this, ID_BTN_MOVE_UP, _("&Up")), 0,
      wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
  bSizer3->Add(new wxButton(this, ID_BTN_MOVE_DOWN, _("D&own")), 0,
      wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
  bSizer3->AddSpacer(25);
  bSizer3->Add(new wxButton(this, ID_BTN_ONOFF, _("En&able")), 0,
      wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
  bSizer3->AddSpacer(25);
  bSizer3->Add(new wxButton(this, ID_BTN_DELETE, _("&Delete")), 0,
      wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
  bSizer3->AddSpacer(25);
  bSizer3->Add(new wxButton(this, ID_BTN_IMPORT, _("&Import...")), 0,
      wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
  bSizer3->Add(new wxButton(this, ID_BTN_EXPORT, _("&Export...")), 0,
      wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
  bSizer2->Add( bSizer3, 0, wxALL, 5 );
  bSizer1->Add( bSizer2, 0, wxALL|wxEXPAND, 5 );

  SetSizerAndFit( bSizer1 );
}

CResolverPage::~CResolverPage()
{
}

void CResolverPage::OnImport(wxCommandEvent&)
{
  wxFileDialog dlg(this, _("Open Resolver File"),
      wxEmptyString, wxEmptyString,
      _("CRE Files (*.cre)|*.cre||"),
      wxFD_OPEN|wxFD_FILE_MUST_EXIST);

  if (dlg.ShowModal() == wxID_OK) {
    CRegExWebResolver *pR = new CRegExWebResolver();
    if (pR->loadFromFile(dlg.GetPath())) {
      if (!m_pUIModel->Append(pR)) {
        delete pR;
      }
    } else {
      wxMessageBox(_("An error occurred while loading the file."),
          wxMessageBoxCaptionStr, wxOK|wxICON_ERROR);
      delete pR;
    }
  }
}

void CResolverPage::OnExport(wxCommandEvent&)
{
  wxFileDialog dlg(this, _("Save Resolver File"),
      wxEmptyString, wxEmptyString,
      _("CRE Files (*.cre)|*.cre||"),
      wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

  if (dlg.ShowModal() == wxID_OK) {
    const CResolver *pR = m_model.getEntry(
        m_pUIModel->GetRow(m_dvResolvers->GetSelection()));
    if (pR) {
      if (!pR->saveToFile(dlg.GetPath())) {
        wxMessageBox(_("An error occurred while writing the file."),
            wxMessageBoxCaptionStr, wxOK|wxICON_ERROR);
      }
    }
  }
}

void CResolverPage::OnDelete(wxCommandEvent&)
{
  if (m_dvResolvers->GetSelectedItemsCount() > 0) {
    m_pUIModel->DeleteItem(m_dvResolvers->GetSelection());
    m_dvResolvers->SetFocus();
  }
}

void CResolverPage::OnMoveUp(wxCommandEvent&)
{
  if (m_dvResolvers->GetSelectedItemsCount() > 0) {
    unsigned sel = m_pUIModel->GetRow(m_dvResolvers->GetSelection());
    m_pUIModel->MoveUp(sel);
    if (sel > 0) --sel;
    m_dvResolvers->Select(m_pUIModel->GetItem(sel));
    m_dvResolvers->SetFocus();
  }
}

void CResolverPage::OnMoveDown(wxCommandEvent&)
{
  if (m_dvResolvers->GetSelectedItemsCount() > 0) {
    unsigned sel = m_pUIModel->GetRow(m_dvResolvers->GetSelection());
    m_pUIModel->MoveDown(sel);
    if (sel < m_pUIModel->GetCount() - 1) ++sel;
    m_dvResolvers->Select(m_pUIModel->GetItem(sel));
    m_dvResolvers->SetFocus();
  }
}

void CResolverPage::OnToggle(wxCommandEvent&)
{
  if (m_dvResolvers->GetSelectedItemsCount() > 0) {
    unsigned sel = m_pUIModel->GetRow(m_dvResolvers->GetSelection());
    m_pUIModel->Toggle(sel);
    m_dvResolvers->SetFocus();
  }
}


void CResolverPage::OnUpdateButtons(wxUpdateUIEvent& e)
{
  bool bEnable = false;
  if ((e.GetId() == ID_BTN_DELETE) || (e.GetId() == ID_BTN_EXPORT)) {
    if (m_dvResolvers->GetSelectedItemsCount() > 0) {
      // Can only delete/export regex resolvers or orphaned addressbooks
      const CResolver *pR = m_model.getEntry(
          m_pUIModel->GetRow(m_dvResolvers->GetSelection()));
      if (pR) {
        bEnable = (!pR->isAvailable() || (pR->getType() == RT_ONLINE_REGEX));
      }
    }
  }
  else if (e.GetId() == ID_BTN_MOVE_UP) {
    bEnable = (m_pUIModel->GetRowCount() > 1) &&
              (m_dvResolvers->GetSelectedItemsCount() > 0) &&
              (m_pUIModel->GetRow(m_dvResolvers->GetSelection()) > 0);
  }
  else if (e.GetId() == ID_BTN_MOVE_DOWN) {
    bEnable = (m_pUIModel->GetRowCount() > 1) &&
              (m_dvResolvers->GetSelectedItemsCount() > 0) &&
              (m_pUIModel->GetRow(m_dvResolvers->GetSelection()) < m_pUIModel->GetRowCount()-1);;
  }
  else if (e.GetId() == ID_BTN_ONOFF) {
    if (m_dvResolvers->GetSelectedItemsCount() > 0) {
      const CResolver *pR = m_model.getEntry(
          m_pUIModel->GetRow(m_dvResolvers->GetSelection()));
      bEnable = pR->isAvailable();
      e.SetText(pR->isEnabled() ? _("Dis&able") : _("En&able"));
    }
  }
  e.Enable(bEnable);
}
