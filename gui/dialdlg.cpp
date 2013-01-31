
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
#include "glob/prefs.h"
#include "pluginloader.h"
#include "resources.h"
#include "mainapp.h"
#include "dialdlg.h"

#include "wx/combo.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI"

// ---------------------------------------------------------------------------

class ListViewComboPopup : public wxListView, public wxComboPopup
{
public:

    virtual void Init()
    {
        m_value = -1;
        m_itemHere = -1; // hot item in list
    }

    virtual bool Create( wxWindow* parent )
    {
        return wxListView::Create(parent,1,
                                  wxPoint(0,0),wxDefaultSize,
                                  wxLC_REPORT|wxLC_VRULES|wxLC_NO_HEADER|
                                  wxLC_SINGLE_SEL|wxSIMPLE_BORDER);
    }

    virtual wxWindow *GetControl() { return this; }

    virtual void SetStringValue( const wxString& s ) {
      int n = wxListView::FindItem(-1, s);
      if (n >= 0 && n < GetItemCount()) {
        m_value = n;
        wxListView::Select(n);
      }
    }
    virtual wxString GetStringValue() const {
        if ( m_value >= 0 ) return wxListView::GetItemText(m_value);
        return wxEmptyString;
    }
    void OnMouseMove(wxMouseEvent& event) {
      int resFlags;
      int itemHere = HitTest(event.GetPosition(), resFlags);
      if ((itemHere >= 0) && (m_itemHere != itemHere)) {
        wxListView::Select(itemHere, true);
        m_itemHere = itemHere;
      }
      event.Skip();
    }
    void OnMouseClick(wxMouseEvent& WXUNUSED(event)) {
        m_value = m_itemHere;
        Dismiss();
    }

protected:

    int m_value;    // current item index
    int m_itemHere; // hot item in popup

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ListViewComboPopup, wxListView)
    EVT_MOTION(ListViewComboPopup::OnMouseMove)
    // NOTE: Left down event is used instead of left up right now
    //       since MSW wxListCtrl doesn't seem to emit left ups
    //       consistently.
    EVT_LEFT_DOWN(ListViewComboPopup::OnMouseClick)
END_EVENT_TABLE()


// CDialDialog ---------------------------------------------------------------

wxArrayString CDialDialog::m_vLastNumbers;

BEGIN_EVENT_TABLE(CDialDialog, wxDialog)
  EVT_TEXT(wxID_ANY, CDialDialog::OnChange)
END_EVENT_TABLE()

CDialDialog::CDialDialog(wxWindow* parent, const CContact *pContact)
  : wxDialog(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
             wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
  , m_cbPhones(0)
  , m_btnOk(0)
{
  wxArrayString choices;

  if (pContact) {
    SetTitle(wxString::Format(wxT("%s - %s"), _("Make Call"), pContact->getSN()));
  }
  else {
    SetTitle(_("Make Call"));
  }

  wxGetApp().getPluginLoader().getCallMonitorNames(choices);
  m_strCallProvider = wxGetApp().getPrefs().getDefaultCallProvider();
  bool bFound = false;
  for (int i = 0; i < choices.size(); ++i) {
    if (choices[i] == m_strCallProvider) {
      bFound = true;
      break;
    }
  }
  if (!bFound && choices.size()) m_strCallProvider = choices[0];

  wxBoxSizer* bSizerH = new wxBoxSizer( wxHORIZONTAL );

  bSizerH->Add(new wxStaticBitmap(this, wxID_ANY,
                    RESOURCES::getBitmap(BMP_PHONE_BIG)),
      0, wxALIGN_TOP|wxALIGN_LEFT|wxALL, 10);

  m_cbPhones = new wxComboCtrl(this, wxID_ANY, wxEmptyString,
      wxDefaultPosition, wxSize(220, -1));
  m_cbPhones->UseAltPopupWindow();
  m_cbPhones->SetPopupMinWidth(300);
  m_cbPhones->SetPopupMaxHeight(200);

  ListViewComboPopup *pPopup = new ListViewComboPopup();
  m_cbPhones->SetPopupControl(pPopup);

  pPopup->InsertColumn(1, _T("Number"), wxLIST_FORMAT_RIGHT, 100);
  pPopup->InsertColumn(2, _T("Type"),   wxLIST_FORMAT_LEFT,   80);
  pPopup->InsertColumn(3, _T("Note"),   wxLIST_FORMAT_LEFT,  120);

  if (pContact) {
    int itemIdx; wxString strNumber;
    const TPhoneList& rPhones = pContact->getConstPhones();
    for (size_t i = 0; i < rPhones.size(); ++i) {
      rPhones[i].getCallableNumber(strNumber);
      if (i == 0) m_strNumber = strNumber;
      itemIdx = pPopup->InsertItem(pPopup->GetItemCount(), strNumber);
      pPopup->SetItem(itemIdx, 1, CPhone::getNumberTypeName(rPhones[i].getType()));
      pPopup->SetItem(itemIdx, 2, rPhones[i].getNote());
    }
  }
  else {
    for (size_t i = 0; i < m_vLastNumbers.size(); ++i) {
      pPopup->InsertItem(pPopup->GetItemCount(), m_vLastNumbers[i]);
    }
    if (!m_vLastNumbers.IsEmpty()) {
      m_strNumber = m_vLastNumbers[0];
    }
  }

  wxBoxSizer* bSizerV1 = new wxBoxSizer( wxVERTICAL );
  bSizerV1->Add(new wxStaticText(this, wxID_ANY, _("&Number:")),
      0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 10);
  bSizerV1->Add(m_cbPhones, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

  bSizerV1->Add(new wxStaticText(this, wxID_ANY, _("&Call using:")),
      0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 10);
  bSizerV1->Add(new wxChoice(this, wxID_ANY, wxDefaultPosition,
                    wxSize(220, -1), choices, 0,
                    wxGenericValidator(&m_strCallProvider)),
      0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
  bSizerH->Add(bSizerV1, 1, wxEXPAND|wxBOTTOM, 10);

  wxBoxSizer* bSizerV2 = new wxBoxSizer( wxVERTICAL );
  m_btnOk = new wxButton(this, wxID_OK);
  m_btnOk->SetDefault();
  bSizerV2->Add(m_btnOk, 0, wxALL, 5);
  bSizerV2->Add(new wxButton(this, wxID_CANCEL), 0, wxALL, 5);
  bSizerH->Add(bSizerV2, 0, wxALL, 5);

  SetSizerAndFit( bSizerH );

  m_btnOk->Enable(m_cbPhones->GetValue().Length() > 0);

  m_cbPhones->SetValue(m_strNumber);  // Set value last, since it may trigger
                                      // EVT_TEXT which calls OnChange
                                      // where we need an initialized m_btnOk
  CenterOnParent();
}

void CDialDialog::SetNumber(const wxString& strNumber)
{
  CPhone phone(CPhone::NT_GENERAL);
  phone.setNumber(strNumber);
  phone.getCallableNumber(m_strNumber);
  m_cbPhones->SetValue(m_strNumber);
}

void CDialDialog::OnChange(wxCommandEvent&)
{
  if (m_btnOk) m_btnOk->Enable(m_cbPhones->GetValue().Length() > 0);
}

void CDialDialog::EndModal(int retCode)
{
  if (retCode == wxID_OK)
  {
    m_strNumber = m_cbPhones->GetValue();
    m_strNumber.Trim(true); m_strNumber.Trim(false);
    bool bFound = false;
    for (int i = 0; !bFound && (i < m_vLastNumbers.size()); ++i) {
      bFound = m_strNumber == m_vLastNumbers[i];
    }
    if (!bFound) {
      m_vLastNumbers.Insert(m_strNumber, 0);
    }
    if (m_vLastNumbers.size() > 20) {
      m_vLastNumbers.pop_back();
    }
  }
  wxDialog::EndModal(retCode);
}
