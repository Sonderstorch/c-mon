
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
#include "glob/contact.h"
#include "resolver/rexweb.h"

#define wxLOG_COMPONENT "GUI/REGEXRESOLVER"

// Statics

CRegExWebResolver::CRegExWebResolver()
  : CResolver(RT_ONLINE_REGEX)
{
}

CRegExWebResolver::~CRegExWebResolver()
{
}

bool CRegExWebResolver::resolve(const wxString& strNumber,
                                CContact& resolvedContact)
{
  bool rc = false;

  if (!isEnabled() || !m_RegEx.IsValid()) return false;

  wxString strResolvedName;

  wxURL session;
  // wxURL::SetDefaultProxy(wxT("proxy:port"));
  wxString strURL = m_strURL;
  strURL.Replace(wxT("{Nr}"), strNumber);

  if (session.SetURL(strURL) == wxURL_NOERR)
  {
    session.GetProtocol().SetTimeout(5);
//    ((wxHTTP&)session.GetProtocol()).SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
    wxInputStream *httpData = session.GetInputStream();
    if (session.GetError() == wxURL_NOERR)
    {
      wxString strResult;
      wxStringOutputStream out_stream(&strResult);
      httpData->Read(out_stream);
      if (!strResult.IsEmpty())
      {
#ifdef _DEBUG
        wxFile txtFile(wxT("web_result.txt"), wxFile::write);
        txtFile.Write(strResult.c_str().AsChar(), strResult.size());
#endif
        if (m_RegEx.Matches(strResult)) {
          strResolvedName = m_RegEx.GetMatch(strResult, 1);
          strResolvedName.Replace(wxT("&amp;"), wxT("&"), true);
          strResolvedName.Replace(wxT("&lt;"), wxT("<"), true);
          strResolvedName.Replace(wxT("&gt;"), wxT(">"), true);
          strResolvedName.Replace(wxT("&quot;"), wxT("\""), true);
          strResolvedName.Replace(wxT("&nbsp;"), wxT(" "), true);
          strResolvedName.Replace(wxT("&auml;"), wxT("\xE4"), true);
          strResolvedName.Replace(wxT("&ouml;"), wxT("\xF6"), true);
          strResolvedName.Replace(wxT("&uuml;"), wxT("\xFC"), true);
          strResolvedName.Replace(wxT("&Auml;"), wxT("\xC4"), true);
          strResolvedName.Replace(wxT("&Ouml;"), wxT("\xD6"), true);
          strResolvedName.Replace(wxT("&Uuml;"), wxT("\xDC"), true);
          strResolvedName.Replace(wxT("&szlig;"), wxT("\xDF"), true);
          resolvedContact.setSN(strResolvedName);
          rc = true;
        }
        else {
          wxLogError(wxString::Format(wxT("%s: RegEx did not match"), m_strName));
        }
      }
      else {
        wxLogError(wxString::Format(wxT("%s: Result is empty"), m_strName));
      }
    }
    else {
      wxLogError(wxString::Format(wxT("%s: GetInputStream returned %d"), m_strName, session.GetError()));
    }
    wxDELETE(httpData);
  }
  else {
    wxLogError(wxString::Format(wxT("%s: SetURL returned %d : %s"),
        m_strName, session.GetError(), m_strURL));
  }
  return rc;
}

const wxString& CRegExWebResolver::getURL() const
{
  return m_strURL;
}

void CRegExWebResolver::setURL(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strURL != val);
  m_strURL = val;
}

const wxString& CRegExWebResolver::getRegEx() const
{
  return m_strRegEx;
}

void CRegExWebResolver::setRegEx(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strRegEx != val);
  m_strRegEx = val;
  m_RegEx.Compile(m_strRegEx, wxRE_ADVANCED);
  m_bEnabled = m_RegEx.IsValid();
}

bool CRegExWebResolver::saveToFile(const wxString& strPath) const
{
  wxFileOutputStream file(strPath);
  if (!file.IsOk()) return false;

  wxTextOutputStream tos(file, wxEOL_DOS, wxCSConv(wxFONTENCODING_UTF8));

  wxString strLine = wxT("REGEXRESOLVER\n");
  tos.WriteString(strLine);
  tos.Write32(0x01);  // Version
  tos.PutChar(wxT('\n'));
  strLine = wxString::Format(wxT("%s\n"), m_strName);
  tos.WriteString(strLine);
  strLine = wxString::Format(wxT("%s\n"), m_strURL);
  tos.WriteString(strLine);
  strLine = wxString::Format(wxT("%s\n"), m_strRegEx);
  tos.WriteString(strLine);

  return true;
}

bool CRegExWebResolver::loadFromFile(const wxString& strPath)
{
  wxFileInputStream file(strPath);
  if (!file.IsOk()) return false;

  wxTextInputStream tis(file, wxT(" \t"), wxCSConv(wxFONTENCODING_UTF8));

  if (!file.CanRead()) return false;
  wxString strLine = tis.ReadLine();
  if (strLine != wxT("REGEXRESOLVER")) return false;

  if (!file.CanRead()) return false;
  wxUint32 ver = tis.Read32();
  if (ver != 0x01) return false;

  if (!file.CanRead()) return false;
  wxString strName = tis.ReadLine();

  if (!file.CanRead()) return false;
  wxString strURL = tis.ReadLine();

  if (!file.CanRead()) return false;
  wxString strRegEx = tis.ReadLine();

  if (strName.IsEmpty() || strURL.IsEmpty() || strRegEx.IsEmpty()) return false;

  setName(strName);
  setURL(strURL);
  setRegEx(strRegEx);

  return m_RegEx.IsValid();
}
