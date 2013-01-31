#ifndef CSVIMP_H
#define CSVIMP_H

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

#include <vector>

class CCsvImportStream;

class CCsvImportCtrl : public wxListCtrl
{
public:
  CCsvImportCtrl (wxWindow* parent, wxWindowID id);
  virtual ~CCsvImportCtrl();

  virtual wxString OnGetItemText(long item, long column) const;
  virtual int OnGetItemImage(long item) const;
  virtual wxListItemAttr* OnGetItemAttr(long item) const;

  void OnColumnClick(wxListEvent& event);
  void OnColumnRightClick(wxListEvent& event);
  void OnColumnPopup(wxCommandEvent &event);

  void setChoices(const wxArrayString& choices);
  int  getChoice(size_t col);

  bool loadFile(const wxString& path,
                wxChar chSeparator = wxT(';'),
                bool bQuoted = true,
                wxChar chQuote = wxT('"'),
                int nLineOffset = 0,
                const wxMBConv& conv = wxConvAuto());

  bool isOk() { return m_bOk; }

  void setLineInvalid(size_t line, bool invalid = true);

protected:
  bool readLine(CCsvImportStream& stream);
  void advanceSelection(int col);
  void updateHeader();

protected:
  wxArrayString               m_choices;
  std::vector<wxArrayString>  m_lines;
  std::vector<bool>           m_invalidlines;
  std::vector<int>            m_selections;
  wxChar                      m_chSeparator;
  wxChar                      m_chQuote;
  bool                        m_bQuotedStrings;
  bool                        m_bOk;
  wxString                    m_line;
  std::vector<int>            m_delim_pos;

  wxListItemAttr  m_attrEven;
  wxListItemAttr  m_attrOdd;
  wxListItemAttr  m_attrInval;

protected:
  DECLARE_EVENT_TABLE()
};

#endif // CSVIMP_H
