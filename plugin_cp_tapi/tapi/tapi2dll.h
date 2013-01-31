#ifndef TAPI2DLL_H_INCLUDED
#define TAPI2DLL_H_INCLUDED

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

#include <tapi.h>

// ---------------------------------------------------------------------------

// TAPI function pointers

typedef LONG (WINAPI *pLineInitializeExA)(LPHLINEAPP lphLineApp, HINSTANCE hInstance, LINECALLBACK lpfnCallback, LPCSTR lpszFriendlyAppName, LPDWORD lpdwNumDevs, LPDWORD lpdwAPIVersion, LPLINEINITIALIZEEXPARAMS lpLineInitializeExParams);
typedef LONG (WINAPI *pLineInitializeExW)(LPHLINEAPP lphLineApp, HINSTANCE hInstance, LINECALLBACK lpfnCallback, LPCWSTR lpszFriendlyAppName, LPDWORD lpdwNumDevs, LPDWORD lpdwAPIVersion, LPLINEINITIALIZEEXPARAMS lpLineInitializeExParams);
typedef LONG (WINAPI *pLineNegotiateAPIVersion)(HLINEAPP hLineApp,DWORD dwDeviceID,DWORD dwAPILowVersion,DWORD dwAPIHighVersion,LPDWORD lpdwAPIVersion,LPLINEEXTENSIONID lpExtensionID);
typedef LONG (WINAPI *pLineGetDevCapsA)(HLINEAPP hLineApp, DWORD dwDeviceID, DWORD dwAPIVersion, DWORD dwExtVersion, LPLINEDEVCAPS lpLineDevCaps);
typedef LONG (WINAPI *pLineGetDevCapsW)(HLINEAPP hLineApp, DWORD dwDeviceID, DWORD dwAPIVersion, DWORD dwExtVersion, LPLINEDEVCAPS lpLineDevCaps);
typedef LONG (WINAPI *pLineGetTranslateCapsA)(HLINEAPP hLineApp, DWORD dwAPIVersion, LPLINETRANSLATECAPS lpTranslateCaps);
typedef LONG (WINAPI *pLineGetTranslateCapsW)(HLINEAPP hLineApp, DWORD dwAPIVersion, LPLINETRANSLATECAPS lpTranslateCaps);
typedef LONG (WINAPI *pLineGetMessage)(HLINEAPP hLineApp,LPLINEMESSAGE lpMessage,DWORD dwTimeout);
typedef LONG (WINAPI *pLineGetIconA)(DWORD dwDeviceID, LPCSTR lpszDeviceClass, LPHICON lphIcon);
typedef LONG (WINAPI *pLineGetIconW)(DWORD dwDeviceID, LPCWSTR lpszDeviceClass, LPHICON lphIcon);
typedef LONG (WINAPI *pLineOpenA)(HLINEAPP hLineApp,DWORD dwDeviceID,LPHLINE lphLine,DWORD dwAPIVersion,DWORD dwExtVersion,DWORD_PTR dwCallbackInstance,DWORD dwPrivileges,DWORD dwMediaModes,LPLINECALLPARAMS const lpCallParams);
typedef LONG (WINAPI *pLineOpenW)(HLINEAPP hLineApp,DWORD dwDeviceID,LPHLINE lphLine,DWORD dwAPIVersion,DWORD dwExtVersion,DWORD_PTR dwCallbackInstance,DWORD dwPrivileges,DWORD dwMediaModes,LPLINECALLPARAMS const lpCallParams);
typedef LONG (WINAPI *pLineGetCallInfoA)(HCALL hCall,LPLINECALLINFO lpCallInfo);
typedef LONG (WINAPI *pLineGetCallInfoW)(HCALL hCall,LPLINECALLINFO lpCallInfo);
typedef LONG (WINAPI *pLineMakeCallA)(HLINE hLine, LPHCALL lphCall, LPCSTR lpszDestAddress, DWORD dwCountryCode, LPLINECALLPARAMS lpCallParams);
typedef LONG (WINAPI *pLineMakeCallW)(HLINE hLine, LPHCALL lphCall, LPCWSTR lpszDestAddress, DWORD dwCountryCode, LPLINECALLPARAMS lpCallParams);
typedef LONG (WINAPI *pLineDeallocateCall)(HCALL hCall);
typedef LONG (WINAPI *pLineClose)(HLINE hLine);
typedef LONG (WINAPI *pLineShutdown)(HLINEAPP hLineApp);

// TAPI32.DLL C++ Wrapper

class TAPI
{
public:
  static TAPI *getInstance();
  virtual ~TAPI();

  LONG LineInitializeEx(LPHLINEAPP lphLineApp, HINSTANCE hInstance, LINECALLBACK lpfnCallback, LPCTSTR lpszFriendlyAppName, LPDWORD lpdwNumDevs, LPDWORD lpdwAPIVersion, LPLINEINITIALIZEEXPARAMS lpLineInitializeExParams);
  LONG LineNegotiateAPIVersion(HLINEAPP hLineApp,DWORD dwDeviceID,DWORD dwAPILowVersion,DWORD dwAPIHighVersion,LPDWORD lpdwAPIVersion,LPLINEEXTENSIONID lpExtensionID);
  LONG LineGetDevCaps(HLINEAPP hLineApp, DWORD dwDeviceID, DWORD dwAPIVersion, DWORD dwExtVersion, LPLINEDEVCAPS lpLineDevCaps);
  LONG LineGetTranslateCaps(HLINEAPP hLineApp, DWORD dwAPIVersion, LPLINETRANSLATECAPS lpTranslateCaps);
  LONG LineGetMessage(HLINEAPP hLineApp,LPLINEMESSAGE lpMessage,DWORD dwTimeout);
  LONG LineGetIcon(DWORD dwDeviceID, LPCTSTR lpszDeviceClass, LPHICON lphIcon);
  LONG LineOpen(HLINEAPP hLineApp, DWORD dwDeviceID, LPHLINE lphLine, DWORD dwAPIVersion, DWORD dwExtVersion, DWORD_PTR dwCallbackInstance, DWORD dwPrivileges, DWORD dwMediaModes, LPLINECALLPARAMS const lpCallParams);
  LONG LineClose(HLINE hLine);
  LONG LineShutdown(HLINEAPP hLineApp);
  LONG LineGetCallInfo(HCALL hCall, LPLINECALLINFO lpCallInfo);
  LONG LineMakeCall(HLINE hLine, LPHCALL lphCall, LPCTSTR lpszDestAddress, DWORD dwCountryCode, LPLINECALLPARAMS lpCallParams);
  LONG LineDeallocateCall(HCALL hCall);

protected:
  TAPI();

protected:
  static TAPI *m_pInstance;

public:
  HINSTANCE                 m_hDll;
  pLineInitializeExA        m_lineInitializeExA;
  pLineInitializeExW        m_lineInitializeExW;
  pLineNegotiateAPIVersion  m_lineNegotiateAPIVersion;
  pLineGetDevCapsA          m_lineGetDevCapsA;
  pLineGetDevCapsW          m_lineGetDevCapsW;
  pLineGetTranslateCapsA    m_lineGetTranslateCapsA;
  pLineGetTranslateCapsW    m_lineGetTranslateCapsW;
  pLineGetMessage           m_lineGetMessage;
  pLineOpenA                m_lineOpenA;
  pLineOpenW                m_lineOpenW;
  pLineClose                m_lineClose;
  pLineShutdown             m_lineShutdown;
  pLineGetCallInfoA         m_lineGetCallInfoA;
  pLineGetCallInfoW         m_lineGetCallInfoW;
  pLineDeallocateCall       m_lineDeallocateCall;
  pLineGetIconA             m_lineGetIconA;
  pLineGetIconW             m_lineGetIconW;
  pLineMakeCallA            m_lineMakeCallA;
  pLineMakeCallW            m_lineMakeCallW;
};

#endif /* TAPI2DLL_H_INCLUDED */
