
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
#include "csvfile.h"

#define wxLOG_COMPONENT "GUI/CVSIMPORT"

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

bool CCsvImportStream::readRecord()
{
  bool rc = true;
  bool bInString = false;
  while (1)
  {
    wxChar c = GetCharWithSingleNewline();
    if (c == 0) {
      rc = false;
      break;
    }
    if (c == m_chQuote) bInString = !bInString;
    if (c == wxT('\n')) {
      if (m_bQuotedStrings && bInString) {
        m_line += c;
        continue;
      } else {
        break;
      }
    }
    if ((c == m_chSeparator) && !bInString) {
      m_delim_pos.push_back(m_line.length());
    }
    m_line += c;
  }
  return rc;
}


bool loadFile(const wxString& path,
              wxChar chSeparator = wxT(';'),
              bool bQuoted = true,
              wxChar chQuote = wxT('"'),
              int nLineOffset = 0,
              const wxMBConv& conv = wxConvAuto());

bool isOk() { return m_bOk; }
