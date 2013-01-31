#ifndef JOURNALPAGE_H_INCLUDED
#define JOURNALPAGE_H_INCLUDED

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

#include "journal/journalmodel.h"

// ----------------------------------------------------------------------------

class CJournalModel;
class CJournalFilterMgr;

class CJournalFilterListCtrl : public wxListCtrl, public IJournalListener
{
public:
  CJournalFilterListCtrl(CJournalModel& model, wxWindow* parent, wxWindowID id);
  virtual ~CJournalFilterListCtrl();

  virtual wxString OnGetItemText(long item, long column) const;
  virtual int OnGetItemImage(long item) const;
  virtual void OnJournalUpdate();

  void OnFilterSelected(wxListEvent& event);

protected:
  CJournalModel &m_model;

protected:
  DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------

class CJournalListCtrl : public wxListCtrl, public IJournalListener
{
public:
  enum {
    Column_Type,
    Column_Time,
    Column_Name,
    Column_Caller,
    Column_Called,
    Column_Duration,
    Column_Result,
    Column_Count
  };

  CJournalListCtrl(CJournalModel& model, wxWindow* parent, wxWindowID id);
  virtual ~CJournalListCtrl();

  virtual wxString OnGetItemText(long item, long column) const;
  virtual int OnGetItemImage(long item) const;
  virtual wxListItemAttr* OnGetItemAttr(long item) const;

  void OnRightClick(wxListEvent&);
  void OnColumnClick(wxListEvent&);
  void ToggleColumnSort(int col);

  void DoCall();

  void OnCtxReverseLookup(wxCommandEvent&);

  void OnTimer(wxTimerEvent&);

  void DelayedUpdate();
  void Update();

  virtual void OnJournalUpdate();

protected:
  CJournalModel &m_model;
  wxTimer        m_Timer;
  TJournalList   m_liEntries;
  wxListItemAttr m_attrEven;
  wxListItemAttr m_attrOdd;
  int            m_nSortColumn;
  bool           m_bSortAsc;
  wxRegEx        m_RegExNoNumber;

protected:
  DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------

class CJournalPage : public wxPanel
{
  public:
    CJournalPage(CJournalModel& model,
                 wxWindow* parent,
                 wxWindowID id = wxID_ANY);
    virtual ~CJournalPage();

    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY);

    void OnCall(wxCommandEvent& e);
    void OnDialNumber(wxCommandEvent& e);
    void OnUpdateCall(wxUpdateUIEvent&);

    void OnSearch(wxCommandEvent&);
    void OnSearchUpdate(wxCommandEvent&);
    void OnSearchCancel(wxCommandEvent&);
    void OnTimer(wxTimerEvent&);

    void DoSearch();

  protected:
    CJournalModel          &m_model;
    wxSearchCtrl           *m_searchCtrl;
    CJournalListCtrl       *m_listCtrl1;
    CJournalFilterListCtrl *m_listCtrl2;
    wxSplitterWindow       *m_pSplitter;
    wxTimer                 m_Timer;
    wxRegEx                 m_RegExNumber;
    wxRegEx                 m_RegExNoNumber;

  protected:
    DECLARE_EVENT_TABLE()
};

#endif /* JOURNALPAGE_H_INCLUDED */
