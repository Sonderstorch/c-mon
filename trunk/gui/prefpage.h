#ifndef PREFPAGE_H_INCLUDED
#define PREFPAGE_H_INCLUDED

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

class CPrefsPage : public wxScrolled<wxPanel>
{
public:
  CPrefsPage( wxWindow *parent, wxWindowID id = wxID_ANY);
  virtual ~CPrefsPage();

protected:
  void OnChange(wxCommandEvent&);
  void OnChangeSpin(wxSpinEvent&);

  void OnUpdateDialrules(wxUpdateUIEvent&);
  void OnUpdateLocalNrLen(wxUpdateUIEvent&);
  void OnUpdateJournalAge(wxUpdateUIEvent&);
  void OnUpdateJournalCount(wxUpdateUIEvent&);
  void OnTimer(wxTimerEvent&);

protected:
  void DoChange();

protected:
  wxChoice*       m_choiceCC;               // Country Code                        (49)
  wxTextCtrl*     m_edtAC;                  // Area Code                         (7326)

  wxCheckBox*     m_chkNrStart;
  wxCheckBox*     m_chkNrLength;
  wxCheckBox*     m_chkLocalNrLength;
  wxCheckBox*     m_chkJournalAge;
  wxCheckBox*     m_chkJournalCount;
  wxSpinCtrl*     m_spinJournalAge;
  wxSpinCtrl*     m_spinJournalCount;
  wxTextCtrl*     m_edtLAC;
  wxTextCtrl*     m_edtNrLength;
  wxTextCtrl*     m_edtLocalNrLength;

  wxTimer         m_Timer;

  struct UIData {
    wxString strCC;
    wxString strAC;
    bool     bAddACIfShortLen;
    int      nLocalNrMaxLen;
    wxString strLAC;
    bool     bExtNrByLAC;
    bool     bExtNrByLen;
    int      nIntNrLen;
    int      nNrChangeMode;
    bool     bDelJournalByAge;
    int      nMaxJournalAge;
    bool     bDelJournalByCount;
    int      nMaxJournalCount;
    void     fromPrefs();
    void     toPrefs();
  } m_Data;

  bool            m_bInitialUpdate;

protected:
  DECLARE_EVENT_TABLE()
};

#endif /* PREFPAGE_H_INCLUDED */
