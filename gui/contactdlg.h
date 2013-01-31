#ifndef CONTACTDLG_H_INCLUDED
#define CONTACTDLG_H_INCLUDED

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

#include "glob/contact.h"

// ----------------------------------------------------------------------------
// Phone number UI model
// ----------------------------------------------------------------------------

class CPhoneNumberModel : public wxDataViewVirtualListModel
{
public:
  enum
  {
    Column_NumberType,
    Column_Number,
    Column_Info,
    Column_Note,
    Column_Count
  };

  CPhoneNumberModel(TPhoneList& rPhones);

  void AddItem();
  void DeleteItem(const wxDataViewItem &item);

  wxString GetTitle(unsigned int col) const;

  const TPhoneList& GetPhones() const
  {
    return m_Phones;
  }

  virtual unsigned int GetColumnCount() const
  {
    return Column_Count;
  }

  virtual wxString GetColumnType(unsigned int col) const
  {
    if (col == Column_NumberType) {
      return wxT("integer");
    }
    return wxT("string");
  }

  virtual unsigned int GetRowCount()
  {
    return m_Phones.size();
  }

  virtual void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const;
  virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const;
  virtual bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col);

private:
  TPhoneList&  m_Phones;
};

// ----------------------------------------------------------------------------

class CPictureControl;

class CContactDialog : public wxDialog
{
  class CPhoneListCtrl;

  class DetailPage1 : public wxPanel
  {
  public:
    DetailPage1(wxWindow* parent, TPhoneList& phones, wxWindowID id = wxID_ANY);
    virtual ~DetailPage1();

    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY);

    void OnAddPhone(wxCommandEvent&);
    void OnDelPhone(wxCommandEvent&);
    void OnUpdateDelPhone(wxUpdateUIEvent&);

  protected:
    CPhoneNumberModel *m_pModel;
    wxDataViewCtrl    *m_dvPhones;

  protected:
    DECLARE_EVENT_TABLE()
  };

  // -------------

public:
  CContactDialog(wxWindow* parent, CContact& rContact);

  void OnNameChanged(wxCommandEvent&);
  void OnUpdateOk(wxUpdateUIEvent&);

  void OnAddPicture(wxCommandEvent&);
  void OnDelPicture(wxCommandEvent&);
  void OnUpdateDelPicture(wxUpdateUIEvent&);

  virtual void EndModal(int retCode);

  void GetData(CContact& c);

protected:
  void SetData(const CContact& c);
  bool findSN(const wxString&);
  void updateSN();

protected:
  CContactDialogData  m_data;

  wxTextCtrl*         m_txtFN;
  wxTextCtrl*         m_txtMN;
  wxTextCtrl*         m_txtLN;
  wxTextCtrl*         m_txtTitle;
  wxTextCtrl*         m_txtNN;
  wxComboBox*         m_cbSN;
  wxTextCtrl*         m_txtOrg;
  wxTextCtrl*         m_txtMail;
  CPictureControl*    m_bmpPhoto;
  wxNotebook*         m_nbDetails;
  wxArrayString       m_Names;

protected:
  DECLARE_EVENT_TABLE()
};

#endif /* CONTACTDLG_H_INCLUDED */
