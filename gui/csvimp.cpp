
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

// $Revision: 36 $ $Date: 2012-10-04 21:34:20 +0200 (Thu, 04 Oct 2012) $

#include "stdwx.h"
#include "csvimp.h"
#include "resources.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI/CVSIMPORT"

// ----------------------------------------------------------------------------
// CCsvImportStream
// ----------------------------------------------------------------------------

class CCsvImportStream : public wxTextInputStream
{
  public:
    CCsvImportStream(wxInputStream& s,
                      const wxString &sep=wxT(" \t"),
                      const wxMBConv& conv = wxConvAuto())
      : wxTextInputStream(s, sep, conv) {  }
    ~CCsvImportStream() {  }

    wxChar GetCharWithSingleNewline();
};

wxChar CCsvImportStream::GetCharWithSingleNewline()
{
  wxChar c = NextChar();
  if(c != wxEOT) {
    if (EatEOL(c)) {
      c = wxT('\n');
    }
  }
  return (wxChar)(c != wxEOT ? c : 0);
}

// ----------------------------------------------------------------------------
// CCsvImportCtrl
// ----------------------------------------------------------------------------

#define ID_POPUP (1800)

BEGIN_EVENT_TABLE(CCsvImportCtrl, wxListCtrl)
  EVT_LIST_COL_CLICK(wxID_ANY, CCsvImportCtrl::OnColumnClick)
  EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, CCsvImportCtrl::OnColumnRightClick)
  EVT_MENU_RANGE(ID_POPUP, ID_POPUP+50, CCsvImportCtrl::OnColumnPopup)
END_EVENT_TABLE()

// ----

CCsvImportCtrl::CCsvImportCtrl(wxWindow* parent, wxWindowID id)
  : wxListCtrl(parent, id, wxDefaultPosition, wxSize(500, 350),
                  wxLC_REPORT|wxLC_VIRTUAL|wxLC_VRULES|wxLC_SINGLE_SEL)
{
  m_bOk = false;
  m_bQuotedStrings = true;
  m_chSeparator    = wxT(',');
  m_chQuote        = wxT('\"');
  m_attrEven.SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
  m_attrOdd.SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
  m_attrInval.SetBackgroundColour(*wxRED);
  m_line.reserve(500);
  m_selections.reserve(50);
  wxImageList *il = new wxImageList(16, 16, false, 3);
  il->Add(RESOURCES::getBitmap(BMP_BULLETWARN));
  il->Add(RESOURCES::getBitmap(BMP_BULLETADD));
  il->Add(RESOURCES::getBitmap(BMP_BULLETDELETE));
  AssignImageList(il, wxIMAGE_LIST_SMALL);
}

CCsvImportCtrl::~CCsvImportCtrl()
{
}

void CCsvImportCtrl::setChoices(const wxArrayString& choices)
{
  m_choices.Clear();
  m_choices.Add(_("Don't import")); // Always selection index 0
  for (wxArrayString::const_iterator it = choices.begin(); it != choices.end(); ++it)
    m_choices.push_back(*it);
}

void CCsvImportCtrl::setLineInvalid(size_t line, bool invalid)
{
  if (line < m_invalidlines.size()) {
    m_invalidlines[line] = invalid;
    RefreshItem(line);
  }
}


wxString CCsvImportCtrl::OnGetItemText(long item, long column) const
{
  wxString strRC;
  if ((item < m_lines.size()) && (column < m_lines[item].size())) {
    strRC = m_lines[item][column];
  }
  return strRC;
}

int CCsvImportCtrl::OnGetItemImage(long item) const
{
  return -1;
}

wxListItemAttr* CCsvImportCtrl::OnGetItemAttr(long item) const
{
  if (item < m_invalidlines.size() && m_invalidlines[item]) {
    return (wxListItemAttr*) &m_attrInval;
  }
  if ((item % 2) == 0) {
    return (wxListItemAttr*) &m_attrEven;
  } else {
    return (wxListItemAttr*) &m_attrOdd;
  }
}

void CCsvImportCtrl::OnColumnClick(wxListEvent& event)
{
  if (m_choices.empty()) {
    return;
  }
  advanceSelection(event.GetColumn());
}

void CCsvImportCtrl::OnColumnRightClick(wxListEvent& event)
{
  if (m_lines.empty()) {
    return;
  }
  int col = event.GetColumn();
  if (!m_choices.empty()) {
    wxMenu menu;
    menu.SetClientData((void *) col);
    for (size_t i = 0; i < m_choices.size(); ++i) {
      wxMenuItem* pItem = menu.AppendRadioItem(ID_POPUP + i,  m_choices[i]);
      if (col < m_selections.size()) {
        pItem->Check(m_selections[col] == i);
      }
    }
    PopupMenu(&menu);
  }
}

void CCsvImportCtrl::OnColumnPopup(wxCommandEvent &event)
{
  int selection = event.GetId() - ID_POPUP;
  if (selection < m_choices.size()) {
    int col = (int) static_cast<wxMenu *>(event.GetEventObject())->GetClientData();
    if (col < m_selections.size()) {
      m_selections[col] = selection;
      updateHeader();
      Refresh();
    }
  }
}

void CCsvImportCtrl::advanceSelection(int col)
{
  if (col < m_selections.size())
  {
    ++m_selections[col];
    if (m_selections[col] >= m_choices.size()) {
      m_selections[col] = 0;
    }
    updateHeader();
    Refresh();
  }
}

void CCsvImportCtrl::updateHeader()
{
  // Update Header
  m_bOk = true;
  wxListItem item;
  for (int i = 0; i < GetColumnCount(); ++i)
  {
    if (!m_choices.empty()) {
      item.SetText(m_choices[m_selections[i]]);
    } else {
      item.SetText(wxString::Format(wxT("%d"), i));
    }
    if (m_selections[i] == 0) {
      item.SetImage(2);
    } else {
      bool bFoundSame = false;
      for (int j = 0; j < GetColumnCount(); ++j) {
        if ((j != i) && (m_selections[i] == m_selections[j])) {
          bFoundSame = true;
          m_bOk = false;
          break;
        }
      }
      item.SetImage(bFoundSame ? 0 : 1);
    }
    SetColumn(i, item);
    SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
  }
}

bool CCsvImportCtrl::loadFile(const wxString& path,
                              wxChar chSeparator,
                              bool bQuoted,
                              wxChar chQuote,
                              int nLineOffset,
                              const wxMBConv& conv)
{
  m_bOk = false;

  wxFileInputStream file(path);
  if (!file.IsOk()) return false;

  m_bQuotedStrings = bQuoted;
  m_chSeparator    = chSeparator;
  m_chQuote        = chQuote;

  wxArrayString columns;
  int max_col = 0;

  m_lines.clear();
  m_invalidlines.clear();
  m_delim_pos.clear();
  SetItemCount(0);
  DeleteAllColumns();

  CCsvImportStream stream(file, wxT(" \t"), conv);
  while (readLine(stream))
  {
    if (nLineOffset <= 0)
    {
      int p1 = 0, p2, col_count = 0;
      for (int i = 0; i < m_delim_pos.size(); ++i) {
        p2 = m_delim_pos[i];
        columns.Add(m_line.Mid(p1, p2 - p1));
        p1 = p2+1;
      }
      col_count = m_delim_pos.size();
      if (p1 < m_line.length()) {
        columns.Add(m_line.Mid(p1));
        ++col_count;
      }
      if (col_count > max_col) {
        max_col = col_count;
      }
      m_lines.push_back(columns);
      m_invalidlines.push_back(false);
      columns.clear();
    }
    else {
      --nLineOffset;
    }
    m_line.clear();
    m_delim_pos.clear();
  }

  if (m_selections.size() < max_col) {
    m_selections.resize(max_col, 0);
  }

  wxListItem item;
  for (int i = 0; i < max_col; ++i) {
    item.SetText(wxString::Format(wxT("%d"), i));
    InsertColumn(i, item);
  }

  updateHeader(); // sets m_bOk
  SetItemCount(m_lines.size());
  Refresh();

  return true;
}

bool CCsvImportCtrl::readLine(CCsvImportStream& stream)
{
  // reading character-wise is simple, but (probably) slow like hell ...

  // TODO: Handle quoted strings correctly
  // (i.e comma within quoted strings, quoted strings in strings, etc.) ...

  bool rc = true;
  bool bInString = false;
  while (1)
  {
    wxChar c = stream.GetCharWithSingleNewline();
    if (c == 0) {
      rc = false;
      break;
    }
    if (m_bQuotedStrings && (c == m_chQuote)) {
      bInString = !bInString;
      continue; // eat quote char
    }
    if (c == wxT('\n')) {
      if (m_bQuotedStrings && bInString) {
        m_line += c;
        continue;
      } else {
        break;
      }
    }
    if ((c == m_chSeparator) && !(m_bQuotedStrings && bInString)) {
      m_delim_pos.push_back(m_line.length());
    }
    m_line += c;
  }
  return rc;
}

int CCsvImportCtrl::getChoice(size_t col)
{
  int rc = 0;
  if (col < GetColumnCount()) {
    rc = m_selections[col];
  }
  return rc;
}
