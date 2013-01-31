
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

#include "tapi2dll.h"

#include <tchar.h>

// Statics -------------------------------------------------------------------

TAPI *TAPI::m_pInstance = 0;

TAPI* TAPI::getInstance()
{
  if (TAPI::m_pInstance == 0) {
    TAPI::m_pInstance = new TAPI();
  }
  return TAPI::m_pInstance;
}

TAPI::~TAPI()
{
  ::FreeLibrary(m_hDll);
  m_pInstance = 0;
}

// ---------------------------------------------------------------------------

TAPI::TAPI()
  : m_lineInitializeExA(0)
  , m_lineInitializeExW(0)
  , m_lineNegotiateAPIVersion(0)
  , m_lineGetDevCapsA(0)
  , m_lineGetDevCapsW(0)
  , m_lineGetTranslateCapsA(0)
  , m_lineGetTranslateCapsW(0)
  , m_lineShutdown(0)
  , m_lineGetMessage(0)
  , m_lineOpenA(0)
  , m_lineOpenW(0)
  , m_lineClose(0)
  , m_lineGetCallInfoA(0)
  , m_lineGetCallInfoW(0)
  , m_lineDeallocateCall(0)
{
  m_hDll = ::LoadLibrary(_T("tapi32.dll"));
  if (m_hDll) {
    m_lineInitializeExA       = (pLineInitializeExA)::GetProcAddress(m_hDll, "lineInitializeExA");
    m_lineInitializeExW       = (pLineInitializeExW)::GetProcAddress(m_hDll, "lineInitializeExW");
    m_lineNegotiateAPIVersion = (pLineNegotiateAPIVersion)::GetProcAddress(m_hDll, "lineNegotiateAPIVersion");
    m_lineGetDevCapsA         = (pLineGetDevCapsA)::GetProcAddress(m_hDll, "lineGetDevCapsA");
    m_lineGetDevCapsW         = (pLineGetDevCapsW)::GetProcAddress(m_hDll, "lineGetDevCapsW");
    m_lineGetTranslateCapsA   = (pLineGetTranslateCapsA)::GetProcAddress(m_hDll, "lineGetTranslateCapsA");
    m_lineGetTranslateCapsW   = (pLineGetTranslateCapsW)::GetProcAddress(m_hDll, "lineGetTranslateCapsW");
    m_lineShutdown            = (pLineShutdown)::GetProcAddress(m_hDll, "lineShutdown");
    m_lineGetMessage          = (pLineGetMessage)::GetProcAddress(m_hDll, "lineGetMessage");
    m_lineOpenA               = (pLineOpenA)::GetProcAddress(m_hDll, "lineOpenA");
    m_lineOpenW               = (pLineOpenW)::GetProcAddress(m_hDll, "lineOpenW");
    m_lineGetCallInfoA        = (pLineGetCallInfoA)::GetProcAddress(m_hDll, "lineGetCallInfoA");
    m_lineGetCallInfoW        = (pLineGetCallInfoW)::GetProcAddress(m_hDll, "lineGetCallInfoW");
    m_lineDeallocateCall      = (pLineDeallocateCall)::GetProcAddress(m_hDll, "lineDeallocateCall");
    m_lineClose               = (pLineClose)::GetProcAddress(m_hDll, "lineClose");
    m_lineGetIconA            = (pLineGetIconA)::GetProcAddress(m_hDll, "lineGetIconA");
    m_lineGetIconW            = (pLineGetIconW)::GetProcAddress(m_hDll, "lineGetIconW");
    m_lineMakeCallA           = (pLineMakeCallA)::GetProcAddress(m_hDll, "lineMakeCallA");
    m_lineMakeCallW           = (pLineMakeCallW)::GetProcAddress(m_hDll, "lineMakeCallW");
  }
}

LONG TAPI::LineInitializeEx(LPHLINEAPP lphLineApp, HINSTANCE hInstance, LINECALLBACK lpfnCallback,
                            LPCTSTR lpszFriendlyAppName, LPDWORD lpdwNumDevs, LPDWORD lpdwAPIVersion, 
                            LPLINEINITIALIZEEXPARAMS lpLineInitializeExParams)
{
  LONG rc = -1;
#ifdef UNICODE
  if (m_lineInitializeExW) {
    rc = m_lineInitializeExW(lphLineApp, hInstance, lpfnCallback, lpszFriendlyAppName, 
                             lpdwNumDevs, lpdwAPIVersion, lpLineInitializeExParams);
  }
#else
  if (m_lineInitializeExA) {
    rc = m_lineInitializeExA(lphLineApp, hInstance, lpfnCallback, lpszFriendlyAppName, 
                             lpdwNumDevs, lpdwAPIVersion, lpLineInitializeExParams);
  }
#endif
  return rc;
}

LONG TAPI::LineNegotiateAPIVersion(HLINEAPP hLineApp,DWORD dwDeviceID,DWORD dwAPILowVersion,DWORD dwAPIHighVersion,LPDWORD lpdwAPIVersion,LPLINEEXTENSIONID lpExtensionID)
{
  LONG rc = -1;
  if (m_lineNegotiateAPIVersion) {
    rc = m_lineNegotiateAPIVersion(hLineApp, dwDeviceID,
                                   dwAPILowVersion, dwAPIHighVersion,
                                   lpdwAPIVersion, lpExtensionID);
  }
  return rc;
}

LONG TAPI::LineGetDevCaps(HLINEAPP hLineApp, DWORD dwDeviceID, DWORD dwAPIVersion,
                          DWORD dwExtVersion, LPLINEDEVCAPS lpLineDevCaps)
{
  LONG rc = -1;
#ifdef UNICODE
  if (m_lineGetDevCapsW) {
    rc = m_lineGetDevCapsW(hLineApp, dwDeviceID, dwAPIVersion, dwExtVersion, lpLineDevCaps);
  }
#else
  if (m_lineGetDevCapsA) {
    rc = m_lineGetDevCapsA(hLineApp, dwDeviceID, dwAPIVersion, dwExtVersion, lpLineDevCaps);
  }
#endif
  return rc;
}

LONG TAPI::LineGetTranslateCaps(HLINEAPP hLineApp, DWORD dwAPIVersion, LPLINETRANSLATECAPS lpTranslateCaps)
{
  LONG rc = -1;
#ifdef UNICODE
  if (m_lineGetTranslateCapsW) {
    rc = m_lineGetTranslateCapsW( hLineApp, dwAPIVersion, lpTranslateCaps);
  }
#else
  if (m_lineGetTranslateCapsA) {
    rc = m_lineGetTranslateCapsA( hLineApp, dwAPIVersion, lpTranslateCaps);
  }
#endif
  return rc;
}

LONG TAPI::LineOpen(HLINEAPP hLineApp, DWORD dwDeviceID, LPHLINE lphLine, DWORD dwAPIVersion, DWORD dwExtVersion, DWORD_PTR dwCallbackInstance, DWORD dwPrivileges, DWORD dwMediaModes, LPLINECALLPARAMS const lpCallParams)
{
  LONG rc = -1;
#ifdef UNICODE
  if (m_lineOpenW) {
    rc = m_lineOpenW(hLineApp, dwDeviceID, lphLine, dwAPIVersion, dwExtVersion, dwCallbackInstance, dwPrivileges, dwMediaModes, lpCallParams);
  }
#else
  if (m_lineOpenA) {
    rc = m_lineOpenA(hLineApp, dwDeviceID, lphLine, dwAPIVersion, dwExtVersion, dwCallbackInstance, dwPrivileges, dwMediaModes, lpCallParams);
  }
#endif
  return rc;
}

LONG TAPI::LineMakeCall(HLINE hLine, LPHCALL lphCall, LPCTSTR lpszDestAddress, DWORD dwCountryCode, LPLINECALLPARAMS lpCallParams)
{
  LONG rc = -1;
#ifdef UNICODE
  if (m_lineMakeCallW) {
    rc = m_lineMakeCallW(hLine, lphCall, lpszDestAddress, dwCountryCode, lpCallParams);
  }
#else
  if (m_lineMakeCallA) {
    rc = m_lineMakeCallA(hLine, lphCall, lpszDestAddress, dwCountryCode, lpCallParams);
  }
#endif
  return rc;
}

LONG TAPI::LineGetMessage(HLINEAPP hLineApp,LPLINEMESSAGE lpMessage,DWORD dwTimeout)
{
  LONG rc = -1;
  if (m_lineGetMessage) {
    rc = m_lineGetMessage(hLineApp, lpMessage, dwTimeout);
  }
  return rc;
}

LONG TAPI::LineClose(HLINE hLine)
{
  LONG rc = -1;
  if (m_lineClose) {
    rc = m_lineClose(hLine);
  }
  return rc;
}

LONG TAPI::LineDeallocateCall(HCALL hCall)
{
  LONG rc = -1;
  if (m_lineDeallocateCall) {
    rc = m_lineDeallocateCall(hCall);
  }
  return rc;
}

LONG TAPI::LineGetCallInfo(HCALL hCall, LPLINECALLINFO lpCallInfo)
{
  LONG rc = -1;
#ifdef UNICODE
  if (m_lineGetCallInfoW) {
    rc = m_lineGetCallInfoW(hCall, lpCallInfo);
  }
#else
  if (m_lineGetCallInfoA) {
    rc = m_lineGetCallInfoA(hCall, lpCallInfo);
  }
#endif
  return rc;
}

LONG TAPI::LineShutdown(HLINEAPP hLineApp)
{
  LONG rc = -1;
  if (m_lineShutdown) {
    rc = m_lineShutdown(hLineApp);
  }
  return rc;
}

LONG TAPI::LineGetIcon(DWORD dwDeviceID, LPCTSTR lpszDeviceClass, LPHICON lphIcon)
{
  LONG rc = -1;
#ifdef UNICODE
  if (m_lineGetIconW) {
    rc = m_lineGetIconW(dwDeviceID, lpszDeviceClass, lphIcon);
  }
#else
  if (m_lineGetIconA) {
    rc = m_lineGetIconA(dwDeviceID, lpszDeviceClass, lphIcon);
  }
#endif
  return rc;
}
