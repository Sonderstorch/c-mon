
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
#include "log/filelog.h"

#include <time.h>

CFileLog::CFileLog(const wxString& strFileName, size_t maxFileSize, size_t maxFileCount)
  : m_FileName(strFileName)
  , m_nFileSize(maxFileSize)
  , m_nFileCount(maxFileCount)
{
  if (m_FileName.FileExists()) {
    rotateFiles();
  }
  openFile();
}

CFileLog::~CFileLog()
{
  closeFile();
}

void CFileLog::openFile()
{
  m_File.Create(m_FileName.GetFullPath(), true,
    wxPOSIX_USER_WRITE|wxPOSIX_USER_READ|
    wxPOSIX_GROUP_WRITE|wxPOSIX_GROUP_READ|
    wxPOSIX_OTHERS_READ);
}

void CFileLog::closeFile()
{
  if (m_File.IsOpened()) {
    m_File.Close();
  }
}

void CFileLog::rotateFiles()
{
  wxString   e1, e2;
  wxFileName f1, f2;
  f1 = m_FileName; f2 = m_FileName;
  for (int i = m_nFileCount; i > 1; --i) {
    f1.ClearExt(); e1 = wxString::Format(wxT("%03d.log"), i-1); f1.SetExt(e1);
    f2.ClearExt(); e2 = wxString::Format(wxT("%03d.log"), i);   f2.SetExt(e2);
    if (wxFileExists(f1.GetFullPath())) {
      wxRenameFile(f1.GetFullPath(), f2.GetFullPath(), true);
    }
  }
  closeFile();
  wxRenameFile(m_FileName.GetFullPath(), f1.GetFullPath(), true);
  openFile();
}

void CFileLog::DoLogRecord(wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info)
{
  tm *stm = gmtime(&info.timestamp);
  const char *pszLevel = "";
  switch (level) {
    case wxLOG_FatalError: pszLevel = "FATAL"; break;
    case wxLOG_Error     : pszLevel = "ERROR"; break;
    case wxLOG_Warning   : pszLevel = "WARN";  break;
    case wxLOG_Message   : pszLevel = "MSG";   break;
    case wxLOG_Status    : pszLevel = "STAT";  break;
    case wxLOG_Info      : pszLevel = "INFO";  break;
    case wxLOG_Debug     : pszLevel = "DEBUG"; break;
    case wxLOG_Trace     : pszLevel = "TRACE"; break;
  }
  m_strLine = wxString::Format(wxT("%02d:%02d:%02d %-5s [%-20s] %s\n"),
      stm->tm_hour, stm->tm_min, stm->tm_sec, pszLevel, info.component, msg);
  m_File.Write(m_strLine);
  if (m_File.Tell() > m_nFileSize) {
    rotateFiles();
  }
}
