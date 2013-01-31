#ifndef CONTACTPAGE_H_INCLUDED
#define CONTACTPAGE_H_INCLUDED

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

#include "glob/contact.h"

class CAddressbookApi;

class CContactListCtrl : public wxListCtrl
{
public:
  enum
  {
    Column_Org,
    Column_LN,
    Column_FN,
    Column_SN,
    Column_Email,
    Column_Phone,
    Column_Count
  };

  CContactListCtrl (wxWindow* parent, wxWindowID id);
  virtual ~CContactListCtrl();

  virtual wxString OnGetItemText(long item, long column) const;
  virtual wxListItemAttr* OnGetItemAttr(long item) const;

  void OnTimer(wxTimerEvent&);
  void OnItemActivated(wxListEvent&);
  void OnRightClick(wxListEvent& e);

  void OnAddAddress(wxCommandEvent&);
  void OnEditAddress(wxCommandEvent&);
  void OnDelAddress(wxCommandEvent&);

  void OnCtxMakeQuickCall(wxCommandEvent&);
  void OnCtxMakeCallDlg(wxCommandEvent&);

  void OnUpdateAdd(wxUpdateUIEvent&);
  void OnUpdateDel(wxUpdateUIEvent&);
  void OnUpdateCallAndEdit(wxUpdateUIEvent&);

  void DelayedUpdate();
  void Update();

  void AddAddress();
  void EditAddress();
  void DelAddress();

  void DoCallDlg();

  const TContactList& getEntries() const { return m_liEntries; }

protected:
  wxTimer           m_Timer;
  TContactList      m_liEntries;
  wxListItemAttr    m_attrEven;
  wxListItemAttr    m_attrOdd;
  int               m_nSortColumn;
  bool              m_bSortAsc;
  CAddressbookApi  *m_pAddressbook;

protected:
  DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------

class CContactPage : public wxPanel
{
  public:
    CContactPage(wxWindow* parent,
                 wxWindowID id = wxID_ANY);
    virtual ~CContactPage();

    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY);

    void OnAddAddress(wxCommandEvent&);
    void OnEditAddress(wxCommandEvent&);
    void OnDelAddress(wxCommandEvent&);
    void OnUpdateDel(wxUpdateUIEvent&);
    void OnUpdateCallAndEdit(wxUpdateUIEvent&);

    void OnImport(wxCommandEvent&);
    void OnExport(wxCommandEvent&);
    void OnUpdateExport(wxUpdateUIEvent&);
    void OnUpdateAddAndImport(wxUpdateUIEvent&);
    void OnChoiceAbook(wxCommandEvent&);
    void OnCall(wxCommandEvent&);

    void OnSearch(wxCommandEvent&);
    void OnSearchUpdate(wxCommandEvent&);
    void OnSearchCancel(wxCommandEvent&);
    void OnTimer(wxTimerEvent&);

    bool DoExport(const wxString& strPath);
    void DoSelectAbook(const CGUID& gidBook);
    void DoSearch();

  protected:
    wxTimer           m_Timer;
    wxChoice*         m_choiceBooks;
    wxSearchCtrl*     m_searchCtrl;
    CContactListCtrl* m_listCtrl;
    wxRegEx           m_RegExNumber;
    wxRegEx           m_RegExNoNumber;

  protected:
    DECLARE_EVENT_TABLE()
};

#endif /* CONTACTPAGE_H_INCLUDED */
