
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

// $Revision: 15 $ $Date: 2012-09-23 19:45:41 +0200 (Sun, 23 Sep 2012) $

#include "stdwx.h"

#include "taf.h"
#include "taflog.h"

#include "../cpt_prefs.h"

#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <tchar.h>
#include <string.h>

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

using namespace std;

// ---------------------------------------------------------------------------

#define MIN_TAPI_VER ((2 << 16) | 0)
#define USE_TAPI_VER ((2 << 16) | 2)

#undef LOG_TO_FILE
// #define LOG_TO_FILE 1

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

class TapiErr
{
public:
  TapiErr(LONG result)
  {
    size_t cnt;
    LPTSTR pBuffer = NULL;
    HMODULE hModule = LoadLibrary(_T("TAPIUI.DLL"));
    DWORD len = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE|
                  FORMAT_MESSAGE_IGNORE_INSERTS|
                  FORMAT_MESSAGE_ALLOCATE_BUFFER, hModule,
                  TAPIERROR_FORMATMESSAGE(result),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&pBuffer, 0, NULL);
    // NOTE: We use wxWidgets conversion functions here to avoid
    //       the need to use newer MSVCRT.DLL versions (b/c of the
    //       safe string api)
    const wxCharBuffer msg = wxConvUTF8.cWC2MB(pBuffer, len, &cnt);
    m_msg = msg.data();
    size_t pos = m_msg.rfind('\r');
    if (pos != m_msg.npos) m_msg.erase(pos);
  }
  operator const std::string& () {
    return m_msg;
  }
protected:
  std::string m_msg;
};

// ---------------------------------------------------------------------------

TapiLog::TapiLog()
{
}

TapiLog::~TapiLog()
{
}

TapiLog& TapiLog::operator<<(const std::string& s)
{
  m_s << s;
  return *this;
}

TapiLog& TapiLog::operator<<(int n)
{
  m_s << n;
  return *this;
}

TapiLog& TapiLog::operator<<(unsigned int n)
{
  m_s << n;
  return *this;
}

TapiLog& TapiLog::operator<<(long n)
{
  m_s << n;
  return *this;
}

TapiLog& TapiLog::operator<<(unsigned long n)
{
  m_s << n;
  return *this;
}

TapiLog& TapiLog::operator<< (TapiLog& (*pf)(TapiLog&))
{
  (*pf)(*this);
  return *this;
};

void TapiLog::_e()
{
  static bool bFirst = true;
  if (!bFirst) {
    m_f.open("taf.log", ios_base::out|ios_base::app);
  } else {
    bFirst = false;
    m_f.open("taf.log", ios_base::out|ios_base::trunc);
  }
}

void TapiLog::_x()
{
  m_f.close();
}

TapiLog& endm(TapiLog& log)
{
#ifdef LOG_TO_FILE
  log._e(); log.m_f << log.m_s.str() << "\n"; log._x();
#endif
  wxLogMessage(wxT("%s"), log.m_s.str().c_str());
  log.m_s.str("");
  log.m_s.clear();
  return log;
}

void dump(unsigned char* pBuf, unsigned nLen)
{
  int pos = 0;
  while (pos < nLen) {
    stringstream line;
    size_t i;
    for (i = 0; i < 20; ++i) {
      if (pos+i < nLen) {
        line << " " << hex << setw(2) << setfill('0') << (int)pBuf[pos+i];
      } else {
        line << "   ";
      }
    }
    line << " ";
    for (i = 0; (i < 20) && (pos < nLen); ++i, ++pos) {
      unsigned char c = (unsigned char)pBuf[pos];
      if ((c > 31) && (c < 255) && (c != 127)) {
        line << (char)c;
      } else {
        line << ".";
      }
    }
    TAPILOG << line.str().c_str() << endm;
  }
}

// ---------------------------------------------------------------------------

// Instance
TapiLog TAPILOG;

// ---------------------------------------------------------------------------

void getTapiString(std::string& strOut, void *pStruct, size_t offs, DWORD dwStringFormat)
{
  if (dwStringFormat == STRINGFORMAT_ASCII) {
    strOut = (((const char *)pStruct) + offs);
  }
  if ((dwStringFormat == STRINGFORMAT_DBCS) || (dwStringFormat == STRINGFORMAT_UNICODE)) {
    // NOTE: We use wxWidgets conversion functions here to avoid
    //       the need to use newer MSVCRT.DLL versions (b/c of the
    //       safe string api)
    size_t cnt = 0;
    const wxCharBuffer msg = wxConvUTF8.cWC2MB((wchar_t *)(((const char *)pStruct) + offs), wxNO_LEN, &cnt);
    strOut = msg.data();
  }
}

// ---------------------------------------------------------------------------
// TAPI Call Class
// ---------------------------------------------------------------------------

CTafCall::CTafCall(DWORD id)
  : m_nId(id)
  , m_nState(0)
  , m_pInfo(0)
  , m_nType(TCT_UNKNOWN)
  , m_bReported(false)
{
  TAPILOG << "New call " << m_nId << endm;
  queryInfo();
}

CTafCall::~CTafCall()
{
  TAPILOG << "Call " << m_nId << " deallocated." << endm;
  TAPI::getInstance()->LineDeallocateCall(m_nId);
  if (m_pInfo) free(m_pInfo);
}

bool CTafCall::queryInfo()
{
# define INFO_SIZE (sizeof(LINECALLINFO) + 512)
  if (m_pInfo == 0) {
    m_pInfo = static_cast<LPLINECALLINFO>(malloc(INFO_SIZE));
    memset(m_pInfo, 0, INFO_SIZE);
    m_pInfo->dwTotalSize = INFO_SIZE;
  }
# undef INFO_SIZE
  DWORD result;
  while (true) {
    result = TAPI::getInstance()->LineGetCallInfo(m_nId, m_pInfo);
    if ((result == LINEERR_STRUCTURETOOSMALL) || (m_pInfo->dwNeededSize > m_pInfo->dwTotalSize)) {
      m_pInfo = static_cast<LPLINECALLINFO>(realloc(m_pInfo, m_pInfo->dwNeededSize));
      m_pInfo->dwTotalSize = m_pInfo->dwNeededSize;
      continue;
    }
    break;
  }
  if (result == 0)
  {
    // dump((unsigned char *)m_pInfo, m_pInfo->dwTotalSize);
    if ( (m_pInfo->dwCallerIDFlags & LINECALLPARTYID_ADDRESS) &&
        ((m_pInfo->dwCallerIDOffset + m_pInfo->dwCallerIDSize) <= m_pInfo->dwUsedSize))
    {
      getTapiString(m_strCallerAddress, m_pInfo, m_pInfo->dwCallerIDOffset, STRINGFORMAT_UNICODE);
    }
    if ( (m_pInfo->dwCallerIDFlags & LINECALLPARTYID_NAME) &&
        ((m_pInfo->dwCallerIDNameOffset + m_pInfo->dwCallerIDNameSize) <= m_pInfo->dwUsedSize))
    {
      getTapiString(m_strCallerName, m_pInfo, m_pInfo->dwCallerIDNameOffset, STRINGFORMAT_UNICODE);
    }
    if ( (m_pInfo->dwCalledIDFlags & LINECALLPARTYID_ADDRESS) &&
        ((m_pInfo->dwCalledIDOffset + m_pInfo->dwCalledIDSize) <= m_pInfo->dwUsedSize))
    {
      getTapiString(m_strCalledAddress, m_pInfo, m_pInfo->dwCalledIDOffset, STRINGFORMAT_UNICODE);
    }
    if ( (m_pInfo->dwCalledIDFlags & LINECALLPARTYID_NAME) &&
        ((m_pInfo->dwCalledIDNameOffset + m_pInfo->dwCalledIDNameSize) <= m_pInfo->dwUsedSize))
    {
      getTapiString(m_strCalledAddress, m_pInfo, m_pInfo->dwCalledIDNameOffset, STRINGFORMAT_UNICODE);
    }
    TAPILOG << "Call (" << m_nId << ")";
    if (!m_strCallerAddress.empty()) TAPILOG << " from " << m_strCallerAddress;
    if (!m_strCallerName.empty())    TAPILOG << " (" << m_strCallerName << ")";
    if (!m_strCalledAddress.empty()) TAPILOG << " to " << m_strCalledAddress;
    if (!m_strCalledName.empty())    TAPILOG << " (" << m_strCalledName << ")";
    TAPILOG << endm;
  }
  else {
    TAPILOG << "lineGetCallInfo() returned " << TapiErr(result) << endm;
  }
  return (result == 0);
}

CTafCall::TTafCallType CTafCall::getType()
{
  return m_nType;
}

DWORD CTafCall::getState()
{
  return m_nState;
}

void CTafCall::handleMessage(const LINEMESSAGE& m)
{
  if (m.dwMessageID == LINE_CALLSTATE) {
    if (m.hDevice == m_nId) {
      //  LINECALLSTATE_IDLE
      //  LINECALLSTATE_OFFERING
      //  LINECALLSTATE_ACCEPTED
      //  LINECALLSTATE_DIALTONE
      //  LINECALLSTATE_DIALING
      //  LINECALLSTATE_RINGBACK
      //  LINECALLSTATE_BUSY
      //  LINECALLSTATE_SPECIALINFO
      //  LINECALLSTATE_CONNECTED
      //  LINECALLSTATE_PROCEEDING
      //  LINECALLSTATE_ONHOLD
      //  LINECALLSTATE_CONFERENCED
      //  LINECALLSTATE_ONHOLDPENDCONF
      //  LINECALLSTATE_ONHOLDPENDTRANSFER
      //  LINECALLSTATE_DISCONNECTED
      switchState(m.dwParam1);
    }
  }
  else if (m.dwMessageID == LINE_CALLINFO) {
    if (m.hDevice == m_nId) {
      // m.dwParam1 -> Bitmask of
      // LINECALLINFOSTATE_OTHER
      // LINECALLINFOSTATE_DEVSPECIFIC
      // LINECALLINFOSTATE_BEARERMODE
      // LINECALLINFOSTATE_RATE
      // LINECALLINFOSTATE_MEDIAMODE
      // LINECALLINFOSTATE_APPSPECIFIC
      // LINECALLINFOSTATE_CALLID
      // LINECALLINFOSTATE_RELATEDCALLID
      // LINECALLINFOSTATE_ORIGIN
      // LINECALLINFOSTATE_REASON
      // LINECALLINFOSTATE_COMPLETIONID
      // LINECALLINFOSTATE_NUMOWNERINCR
      // LINECALLINFOSTATE_NUMOWNERDECR
      // LINECALLINFOSTATE_NUMMONITORS
      // LINECALLINFOSTATE_TRUNK
      // LINECALLINFOSTATE_CALLERID
      // LINECALLINFOSTATE_CALLEDID
      // LINECALLINFOSTATE_CONNECTEDID
      // LINECALLINFOSTATE_REDIRECTIONID
      // LINECALLINFOSTATE_REDIRECTINGID
      // LINECALLINFOSTATE_DISPLAY
      // LINECALLINFOSTATE_USERUSERINFO
      // LINECALLINFOSTATE_HIGHLEVELCOMP
      // LINECALLINFOSTATE_LOWLEVELCOMP
      // LINECALLINFOSTATE_CHARGINGINFO
      // LINECALLINFOSTATE_TERMINAL
      // LINECALLINFOSTATE_DIALPARAMS
      // LINECALLINFOSTATE_MONITORMODES
      // LINECALLINFOSTATE_TREATMENT
      // LINECALLINFOSTATE_QOS
      // LINECALLINFOSTATE_CALLDATA
        queryInfo();
        if (m_bReported) {
          CTafManager::getInstance()->updateCall(*this);
        }
      }
    }
    else if (m.dwMessageID == LINE_REPLY) {
      // Outgoing call, initiated by MakeCall()
      if (m.hDevice == m_nId) {
      }
    }
}

void CTafCall::switchState(DWORD toState)
{
  m_nState = toState;
  switch (toState)
  {
  case LINECALLSTATE_OFFERING:      // Incoming calls always start with OFFERING
    m_nType = TCT_INCOMING;         // So we can report immediately
    if (!m_bReported) {
      CTafManager::getInstance()->newCall(*this);
      m_bReported = true;
    }
    break;
  case LINECALLSTATE_DIALTONE:      // We get DIALTONE as soon as lifting the hand-set
    m_nType = TCT_OUTGOING;         // so don't report a new call (yet)
    break;
  case LINECALLSTATE_RINGBACK:      // On outgoing calls, some TAPI providers
  case LINECALLSTATE_BUSY:          // skip DIALTONE/RINGBACK states, so
  case LINECALLSTATE_CONNECTED:     // if we get to BUSY or CONNECTED and have
    if (!m_bReported) {             // not reported yet, it must be an outgoing call
      m_nType = TCT_OUTGOING;
      CTafManager::getInstance()->newCall(*this);
      m_bReported = true;
    } else {
      CTafManager::getInstance()->updateCallState(*this);
    }
    break;
  default:
    if (m_bReported) {
      CTafManager::getInstance()->updateCallState(*this);
    }
    break;
  }
}

// ---------------------------------------------------------------------------
// TAPI Line Class
// ---------------------------------------------------------------------------

CTafLine::CTafLine(DWORD idx)
  : m_nIdx(idx)
  , m_nTAPIVersion(0)
  , m_pCaps(0)
  , m_hLine(-1)
{
}

CTafLine::~CTafLine()
{
  close();
  TAPILOG << "TAPI line " << m_nIdx << " destroyed." << endm;
  if (m_pCaps) free(m_pCaps);
}

bool CTafLine::init()
{
  return  negotiateVersion() &&
          queryCaps();
}

bool CTafLine::negotiateVersion()
{
  LINEEXTENSIONID ext;

  ext.dwExtensionID0 = ext.dwExtensionID1 =
  ext.dwExtensionID2 = ext.dwExtensionID3 = 0;

  DWORD result = TAPI::getInstance()->LineNegotiateAPIVersion(
      CTafManager::getInstance()->getTapiHandle(),
      m_nIdx, (DWORD)MIN_TAPI_VER, (DWORD)USE_TAPI_VER,
      &m_nTAPIVersion, &ext);
  bool success = (result == 0);

  if (!success) {
    TAPILOG << "TAPI line " << m_nIdx << " dismissed. "
            << "lineNegotiateAPIVersion() returned " << TapiErr(result)
            << endm;
  }
  return success;
}

bool CTafLine::queryCaps()
{
# define CAPS_SIZE (sizeof(LINEDEVCAPS) + 512)
  if (m_pCaps == 0) {
    m_pCaps = static_cast<LPLINEDEVCAPS>(malloc(CAPS_SIZE));
    memset(m_pCaps, 0, CAPS_SIZE);
    m_pCaps->dwTotalSize = CAPS_SIZE;
  }
# undef CAPS_SIZE

  DWORD result;
  // Get capabilities of TAPI line
  while (true) {
    result = TAPI::getInstance()->LineGetDevCaps(
        CTafManager::getInstance()->getTapiHandle(),
        m_nIdx, m_nTAPIVersion, 0, m_pCaps);
    if ((result == LINEERR_STRUCTURETOOSMALL) || (m_pCaps->dwNeededSize > m_pCaps->dwTotalSize)) {
      m_pCaps = static_cast<LPLINEDEVCAPS>(realloc(m_pCaps, m_pCaps->dwNeededSize));
      m_pCaps->dwTotalSize = m_pCaps->dwNeededSize;
      continue;
    }
    break;
  }
  bool success = (result == 0);
  if (!success) {
    TAPILOG << "TAPI device " << m_nIdx << " dismissed. "
            << "lineGetDevCaps() returned " << TapiErr(result)
            << endm;
  }
  return success;
}

DWORD CTafLine::getIdx()
{
  return m_nIdx;
}

const std::string& CTafLine::getName()
{
  if (m_strName.empty())
  {
    // lazy initialisation
    if ((m_pCaps->dwLineNameOffset + m_pCaps->dwLineNameSize) <= m_pCaps->dwUsedSize)
    {
      getTapiString(m_strName,
          m_pCaps, m_pCaps->dwLineNameOffset,
          m_pCaps->dwStringFormat);
    }
  }
  return m_strName;
}

bool CTafLine::open()
{
  if (isOpen()) {
    close();
  }

  DWORD result = TAPI::getInstance()->LineOpen(
      CTafManager::getInstance()->getTapiHandle(),
      m_nIdx, &m_hLine, m_nTAPIVersion, 0, (DWORD_PTR)this,
      LINECALLPRIVILEGE_MONITOR, LINEMEDIAMODE_INTERACTIVEVOICE,
      NULL);

  bool success = (result == 0);

  if (success) {
    TAPILOG << "Opened line '" << getName() << "' successfully." << endm;
  } else {
    TAPILOG << "Failed to open line '" << getName() << "'. "
            << "lineOpen() returned " << TapiErr(result) << endm;
    m_hLine = -1;
  }
  return success;
}

void CTafLine::close()
{
  if (isOpen()) {
    TAPILOG << "Closing line '" << getName() << "'." << endm;
    closeCalls();
    TAPI::getInstance()->LineClose(m_hLine);
    m_hLine = -1;
  }
}

void CTafLine::closeCalls()
{
  TCallMap::iterator it = m_mapCalls.begin(), end = m_mapCalls.end();
  for (; it != end; it++) {
    delete it->second;
  }
  m_mapCalls.clear();
}

DWORD CTafLine::getPermanentId()
{
  return (m_pCaps->dwPermanentLineID);
}

bool CTafLine::isOpen()
{
  return (m_hLine != -1);
}

bool CTafLine::hasFeatures(DWORD features)
{
  return (m_pCaps->dwLineFeatures & features) == features;
}

bool CTafLine::hasMediaModes(DWORD modes)
{
  return (m_pCaps->dwMediaModes & modes) == modes;
}

void CTafLine::makeCall(const std::string& strNumber)
{
  TCHAR number[50];
  HCALL hCall;

  TAPILOG << "MakeCall to " << strNumber << endm;

  if (!isOpen()) {
    TAPILOG << "MakeCall: Line is closed!" << endm;
    return;
  }

#ifdef _UNICODE
  const char *pszNumber = strNumber.c_str();
  mbsrtowcs(number, &pszNumber, sizeof(number)/sizeof(TCHAR), NULL);
#else
  strcpy(number, &strNumber.c_str());
#endif

  DWORD result = TAPI::getInstance()->LineMakeCall(m_hLine, &hCall, number, 0, NULL);
  if (result >= 0) {
    // TODO: new CTafCall object + reporting
  } else {
    TAPILOG << "lineMakeCall() returned (" << result << ") " << TapiErr(result) << endm;
  }
}

void CTafLine::handleMessage(const LINEMESSAGE& m)
{
  switch (m.dwMessageID)
  {
  case LINE_CLOSE:
    close();
    break;
  case LINE_APPNEWCALL:
  {
    CTafCall *pCall = new CTafCall(m.dwParam2);
    m_mapCalls.insert(make_pair(m.dwParam2, pCall));
    break;
  }
  case LINE_CALLINFO:
  case LINE_CALLSTATE:
  case LINE_REPLY:
    {
      TCallMap::iterator call = m_mapCalls.find(m.hDevice);
      if (call != m_mapCalls.end()) {
        call->second->handleMessage(m);
        if (call->second->getState() == LINECALLSTATE_IDLE) {
          CTafManager::getInstance()->deleteCall(*(call->second));
          delete call->second;
          m_mapCalls.erase(call);
        }
      }
      break;
    }
  }
}

// ---------------------------------------------------------------------------
// TAPI Master Class
// ---------------------------------------------------------------------------

CTafManager *CTafManager::m_pInstance = 0;

// --------

CTafManager *CTafManager::getInstance()
{
  if (CTafManager::m_pInstance == 0) {
    CTafManager::m_pInstance = new CTafManager();
    CTafManager::m_pInstance->init();
  }
  return CTafManager::m_pInstance;
}

// --------

DWORD CTafManager::thread(void *arg)
{
  CTafManager *pObj = reinterpret_cast<CTafManager *>(arg);
  pObj->monitor();
  return 0;
}

// --------

CTafManager::~CTafManager()
{
  shutdown();
  delete m_pTAPI;
  m_pInstance = 0;
}

// ---------------------------------------------------------------------------

CTafManager::CTafManager()
  : m_hTAPI(0)
  , m_nTAPIVersion(USE_TAPI_VER)
  , m_hThread(INVALID_HANDLE_VALUE)
  , m_nThreadId(0)
  , m_bRunMonitor(false)
  , m_bMonitorIsRunning(false)
{
  InitializeCriticalSection(&m_csLines);
  m_pTAPI = TAPI::getInstance();
}

// ---------------------------------------------------------------------------

void CTafManager::init(bool bWait /*= true*/)
{
  startMonitor();
  while (bWait && !m_bMonitorIsRunning) {
    ::Sleep(250);
  }
}

bool CTafManager::isRunning()
{
  return m_bMonitorIsRunning;
}

// ---------------------------------------------------------------------------

void CTafManager::shutdown()
{
  if (m_bMonitorIsRunning) {
    m_bRunMonitor = false;
    ::WaitForSingleObject(m_hThread, 10000);
  }
}

// ---------------------------------------------------------------------------

void CTafManager::getLines(TLineList& rLines)
{
  rLines.clear();
  CTafLock((LPCRITICAL_SECTION)&m_csLines);
  rLines.reserve(m_mapLines.size());
  TLineMap::const_iterator it = m_mapLines.begin(), end = m_mapLines.end();
  for (; it != end; ++it) {
    rLines.push_back(TLineInfo(it->second->getName(),
                               it->second->getPermanentId()));
  }
}

// ---------------------------------------------------------------------------

bool CTafManager::initTapi()
{
  LINEINITIALIZEEXPARAMS TAPIParam;
  DWORD                  nTLineCount = 0;

  memset(&TAPIParam, 0, sizeof(TAPIParam));
  TAPIParam.dwTotalSize = TAPIParam.dwUsedSize = sizeof(LINEINITIALIZEEXPARAMS);
  TAPIParam.dwOptions   = LINEINITIALIZEEXOPTION_USEEVENT;

  // Init TAPI
  LONG result;

  do {
    result = m_pTAPI->LineInitializeEx(&m_hTAPI, NULL, NULL,
                                       _T("c'mon TAPI Monitor"),
                                       &nTLineCount, &m_nTAPIVersion,
                                       &TAPIParam);
    if (result == LINEERR_REINIT) ::Sleep(500);
  } while (result == LINEERR_REINIT);

  if (result == 0)
  {
    TAPILOG << "TAPI successfully opened " << nTLineCount << " devices" << endm;
    for (DWORD devIdx = 0; devIdx < nTLineCount; ++devIdx) {
      addLine(devIdx);
    }
    queryLocation();
  }
  else {
    TAPILOG << "lineInitializeEx() returned " << TapiErr(result) << endm;
  }
  return (result == 0);
}

void CTafManager::queryLocation()
{
  char acTextBuffer[1024];

  LPLINETRANSLATECAPS lpTransCaps;
# define INFO_SIZE (sizeof(LINETRANSLATECAPS) + 512)
  lpTransCaps = static_cast<LPLINETRANSLATECAPS>(malloc(INFO_SIZE));
  memset(lpTransCaps, 0, INFO_SIZE);
  lpTransCaps->dwTotalSize = INFO_SIZE;
# undef INFO_SIZE
  DWORD result;
  while (true) {
    result = m_pTAPI->LineGetTranslateCaps(m_hTAPI, m_nTAPIVersion, lpTransCaps);
    if ((result == LINEERR_STRUCTURETOOSMALL) || (lpTransCaps->dwNeededSize > lpTransCaps->dwTotalSize)) {
      lpTransCaps = static_cast<LPLINETRANSLATECAPS>(realloc(lpTransCaps, lpTransCaps->dwNeededSize));
      lpTransCaps->dwTotalSize = lpTransCaps->dwNeededSize;
      continue;
    }
    break;
  }
  if (result == 0) {
    TAPILOG << "Current Location ID      : " << lpTransCaps->dwCurrentLocationID << endm;
    LPLINELOCATIONENTRY lpLocation = (LPLINELOCATIONENTRY)
        (((unsigned char *)lpTransCaps) + lpTransCaps->dwLocationListOffset);
    for (int i = 0; i < lpTransCaps->dwNumLocations; ++i) {
      if (lpLocation->dwPermanentLocationID == lpTransCaps->dwCurrentLocationID) {
        sprintf(acTextBuffer, "%d", lpLocation->dwCountryCode);
        m_strCountryCode = acTextBuffer;
        getTapiString(m_strAreaCode, lpTransCaps, lpLocation->dwCityCodeOffset, STRINGFORMAT_UNICODE);
        getTapiString(m_strOutsideAccess, lpTransCaps, lpLocation->dwLocalAccessCodeOffset, STRINGFORMAT_UNICODE);
        getTapiString(m_strNationalDialingPrefix, lpTransCaps, lpLocation->dwLongDistanceAccessCodeOffset, STRINGFORMAT_UNICODE);
        TAPILOG << "Country Code             : " << m_strCountryCode << endm;
        TAPILOG << "Area Code                : " << m_strAreaCode << endm;
        TAPILOG << "Outside Line Access      : " << m_strOutsideAccess << endm;
        TAPILOG << "National Dialing Prefix  : " << m_strNationalDialingPrefix << endm;
// ??   TAPILOG << "International Prefix     : " << m_strInternationalDialingPrefix << endm;
        break;
      }
      ++lpLocation;
    }
  } else {
      TAPILOG << "lineGetTranslateCaps() returned " << TapiErr(result) << endm;
    }
}

void CTafManager::shutdownTapi()
{
  if (isTapiOk())
  {
    TLineMap::iterator line = m_mapLines.begin(), end = m_mapLines.end();
    for (; line != m_mapLines.end(); line++) {
      delete line->second;
    }
    m_mapLines.clear();
    m_pTAPI->LineShutdown(m_hTAPI);
    m_hTAPI = 0;
  }
}

bool CTafManager::isTapiOk()
{
  return (m_hTAPI != 0);
}

void CTafManager::addLine(DWORD idx)
{
  CTafLock((LPCRITICAL_SECTION)&m_csLines);
  CTafLine *pLine = new CTafLine(idx);
  if (pLine->init() &&
      pLine->hasFeatures(LINEFEATURE_MAKECALL) &&
      pLine->hasMediaModes(LINEMEDIAMODE_INTERACTIVEVOICE))
  {
    TAPILOG << "Accepted line " << pLine->getName() << endm;
    m_mapLines.insert(make_pair(pLine->getPermanentId(), pLine));
    const TPrefLines& vActiveLines = CPTPREFS::getPrefs().getSelectedLines();
    TPrefLines::const_iterator it = find(
        vActiveLines.begin(), vActiveLines.end(), pLine->getPermanentId());
    if (it != vActiveLines.end()) {
      pLine->open();
    }
    notifyObservers(ITafEventHandler::TAF_EVT_LINES_CHANGED);
  }
  else {
    delete pLine;
  }
}

void CTafManager::removeLine(DWORD idx)
{
  CTafLock((LPCRITICAL_SECTION)&m_csLines);
  TLineMap::iterator line = m_mapLines.begin();
  for (; line != m_mapLines.end(); line++) {
    if (line->second->getIdx() == idx) {
      delete line->second;
      m_mapLines.erase(line);
      notifyObservers(ITafEventHandler::TAF_EVT_LINES_CHANGED);
      break;
    }
  }
}

void CTafManager::makeCall(const std::string& strNumber)
{
  if (!isTapiOk()) return;

  CTafLock((LPCRITICAL_SECTION)&m_csLines);

  // TODO: make user choose default line for dialling
  TLineMap::iterator it = m_mapLines.begin();
  for (; it != m_mapLines.end(); ++it) {
    if (it->second->isOpen()) {
      it->second->makeCall(strNumber);
      break;
    }
  }
}

void CTafManager::registerObserver(ITafEventHandler *pObserver)
{
  m_Listeners.push_back(pObserver);
}

void CTafManager::deregisterObserver(ITafEventHandler *pObserver)
{
  std::vector<ITafEventHandler*>::iterator it = m_Listeners.begin();
  while (it != m_Listeners.end()) {
    if (*it == pObserver) {
      m_Listeners.erase(it);
      return;
    }
    it++;
  }
}

void CTafManager::notifyObservers(ITafEventHandler::ETAFEVENT e, void *info)
{
  std::vector<ITafEventHandler*>::iterator it = m_Listeners.begin();
  for (; it != m_Listeners.end(); ++it) (*it)->OnTafEvent(e, info);
}

void CTafManager::newCall(const CTafCall& call)
{
  notifyObservers(ITafEventHandler::TAF_EVT_NEW_CALL, (void *)&call);
}

void CTafManager::updateCall(const CTafCall& call)
{
  notifyObservers(ITafEventHandler::TAF_EVT_CALL_INFO_UPDATE, (void *)&call);
}

void CTafManager::updateCallState(const CTafCall& call)
{
  notifyObservers(ITafEventHandler::TAF_EVT_CALL_STATE_UPDATE, (void *)&call);
}

void CTafManager::deleteCall(const CTafCall& call)
{
  notifyObservers(ITafEventHandler::TAF_EVT_CALL_DELETED, (void *)&call);
}

void  CTafManager::startMonitor()
{
  if (m_bMonitorIsRunning) return;

  m_bRunMonitor = true;
  m_hThread = ::CreateThread(NULL, 0,
      (LPTHREAD_START_ROUTINE)&CTafManager::thread, this, 0,
      (LPDWORD)&m_nThreadId);
}

void  CTafManager::stopMonitor()
{
  if (!m_bMonitorIsRunning) return;
  m_bRunMonitor = false;
}

void CTafManager::handleTapiMessages()
{
  LINEMESSAGE tm;

  DWORD result = m_pTAPI->LineGetMessage(m_hTAPI, &tm, 500);
  if (result == 0)
  {
    parseTapiMsg(tm);
    if (tm.dwMessageID == LINE_CREATE) {
      addLine(tm.dwParam1);
    }
    else if (tm.dwMessageID == LINE_REMOVE) {
      removeLine(tm.dwParam1);
    }
    else if (tm.dwMessageID == LINE_LINEDEVSTATE) {
      // TODO
    }
    else {
      CTafLine *pLine = reinterpret_cast<CTafLine *>(tm.dwCallbackInstance);
      if (pLine) pLine->handleMessage(tm);
    }
  }
  else {
    if (result != LINEERR_OPERATIONFAILED) {
      // LINEERR_OPERATIONFAILED means timeout ...
      TAPILOG << "lineGetMessage returned: " << TapiErr(result) << endm;
    }
  }
}

void CTafManager::monitor()
{
  TAPILOG << "TAPI monitor thread started" << endm;

  if (initTapi())
  {
    m_bMonitorIsRunning = true;
    notifyObservers(ITafEventHandler::TAF_EVT_RUNSTATE_CHANGED);
    while (m_bRunMonitor) {
      handleTapiMessages();
    }
    shutdownTapi();
  }
  m_bMonitorIsRunning = false;
  TAPILOG << "TAPI monitor thread stopped" << endm;

  notifyObservers(ITafEventHandler::TAF_EVT_RUNSTATE_CHANGED);
}

void CTafManager::parseTapiMsg(LINEMESSAGE m)
{
  if (m.dwMessageID == LINE_CREATE) {
    // m.dwParam1  => idDevice
    TAPILOG << "LINE_CREATE (" << m.dwParam1 << ")" << endm;
  }
  else if (m.dwMessageID == LINE_REMOVE) {
    // m.dwParam1  => hDeviceID
    TAPILOG << "LINE_REMOVE (" << m.dwParam1 << ")" << endm;
  }
  else if (m.dwMessageID == LINE_CLOSE) {
    // m.hDevice  => hLine
    TAPILOG << "LINE_CLOSE (" << m.hDevice << ")" << endm;
  }
  else if (m.dwMessageID == LINE_APPNEWCALL) {
    // m.hDevice  => hLine
    // m.dwParam1 => dwAddressId
    // m.dwParam2 => hCall
    // m.dwParam3 => CallPrivilige
    TAPILOG << "LINE_APPNEWCALL (" << m.dwParam2 << ") ";
    switch (m.dwParam3) {
      case LINECALLPRIVILEGE_NONE   : TAPILOG << "NONE"; break;
      case LINECALLPRIVILEGE_MONITOR: TAPILOG << "MONITOR"; break;
      case LINECALLPRIVILEGE_OWNER  : TAPILOG << "OWNER"; break;
      default: TAPILOG << m.dwParam3 << " ??"; break;
    }
    TAPILOG << endm;
  }
  else if (m.dwMessageID == LINE_CALLSTATE) {
    // m.hDevice  => hCall
    // m.dwParam1 => CallState
    // m.dwParam2 => CallStateDetail
    // m.dwParam3 => CallPrivilige
    TAPILOG << "LINE_CALLSTATE (" << m.hDevice << ") ";
    switch (m.dwParam1) {
      case LINECALLSTATE_IDLE              : TAPILOG << "IDLE"; break;
      case LINECALLSTATE_OFFERING          : TAPILOG << "OFFERING"; break;
      case LINECALLSTATE_ACCEPTED          : TAPILOG << "ACCEPTED"; break;
      case LINECALLSTATE_DIALTONE          : TAPILOG << "DIALTONE"; break;
      case LINECALLSTATE_DIALING           : TAPILOG << "DIALING"; break;
      case LINECALLSTATE_RINGBACK          : TAPILOG << "RINGBACK"; break;
      case LINECALLSTATE_BUSY              : TAPILOG << "BUSY"; break;
      case LINECALLSTATE_SPECIALINFO       : TAPILOG << "SPECIALINFO"; break;
      case LINECALLSTATE_CONNECTED         : TAPILOG << "CONNECTED"; break;
      case LINECALLSTATE_PROCEEDING        : TAPILOG << "PROCEEDING"; break;
      case LINECALLSTATE_ONHOLD            : TAPILOG << "ONHOLD"; break;
      case LINECALLSTATE_CONFERENCED       : TAPILOG << "CONFERENCED"; break;
      case LINECALLSTATE_ONHOLDPENDCONF    : TAPILOG << "ONHOLDPENDCONF"; break;
      case LINECALLSTATE_ONHOLDPENDTRANSFER: TAPILOG << "ONHOLDPENDTRANSFER"; break;
      case LINECALLSTATE_DISCONNECTED      : TAPILOG << "DISCONNECTED"; break;
      default                              : TAPILOG << m.dwParam1 << " ??"; break;
    }
    TAPILOG << " (" << m.dwParam2 << ") ";
    switch (m.dwParam3) {
      case LINECALLPRIVILEGE_NONE     : TAPILOG << "NONE"; break;
      case LINECALLPRIVILEGE_MONITOR  : TAPILOG << "MONITOR"; break;
      case LINECALLPRIVILEGE_OWNER    : TAPILOG << "OWNER"; break;
      default                         : TAPILOG << "--"; break;
    }
    TAPILOG << endm;
  }
  else if (m.dwMessageID == LINE_CALLINFO) {
    // m.hDevice  => hCall
    // m.dwParam1 => CallInfoState
    // m.dwParam2 => 0
    // m.dwParam3 => 0
    TAPILOG << "LINE_CALLINFO (" << m.hDevice << ") ";
    if (m.dwParam1 & LINECALLINFOSTATE_OTHER        ) { TAPILOG << "OTHER "; }
    if (m.dwParam1 & LINECALLINFOSTATE_DEVSPECIFIC  ) { TAPILOG << "DEVSPECIFIC "; }
    if (m.dwParam1 & LINECALLINFOSTATE_BEARERMODE   ) { TAPILOG << "BEARERMODE "; }
    if (m.dwParam1 & LINECALLINFOSTATE_RATE         ) { TAPILOG << "RATE "; }
    if (m.dwParam1 & LINECALLINFOSTATE_MEDIAMODE    ) { TAPILOG << "MEDIAMODE "; }
    if (m.dwParam1 & LINECALLINFOSTATE_APPSPECIFIC  ) { TAPILOG << "APPSPECIFIC "; }
    if (m.dwParam1 & LINECALLINFOSTATE_CALLID       ) { TAPILOG << "CALLID "; }
    if (m.dwParam1 & LINECALLINFOSTATE_RELATEDCALLID) { TAPILOG << "RELATEDCALLID "; }
    if (m.dwParam1 & LINECALLINFOSTATE_ORIGIN       ) { TAPILOG << "ORIGIN "; }
    if (m.dwParam1 & LINECALLINFOSTATE_REASON       ) { TAPILOG << "REASON "; }
    if (m.dwParam1 & LINECALLINFOSTATE_COMPLETIONID ) { TAPILOG << "COMPLETIONID "; }
    if (m.dwParam1 & LINECALLINFOSTATE_NUMOWNERINCR ) { TAPILOG << "NUMOWNERINCR "; }
    if (m.dwParam1 & LINECALLINFOSTATE_NUMOWNERDECR ) { TAPILOG << "NUMOWNERDECR "; }
    if (m.dwParam1 & LINECALLINFOSTATE_NUMMONITORS  ) { TAPILOG << "NUMMONITORS "; }
    if (m.dwParam1 & LINECALLINFOSTATE_TRUNK        ) { TAPILOG << "TRUNK "; }
    if (m.dwParam1 & LINECALLINFOSTATE_CALLERID     ) { TAPILOG << "CALLERID "; }
    if (m.dwParam1 & LINECALLINFOSTATE_CALLEDID     ) { TAPILOG << "CALLEDID "; }
    if (m.dwParam1 & LINECALLINFOSTATE_CONNECTEDID  ) { TAPILOG << "CONNECTEDID "; }
    if (m.dwParam1 & LINECALLINFOSTATE_REDIRECTIONID) { TAPILOG << "REDIRECTIONID "; }
    if (m.dwParam1 & LINECALLINFOSTATE_REDIRECTINGID) { TAPILOG << "REDIRECTINGID "; }
    if (m.dwParam1 & LINECALLINFOSTATE_DISPLAY      ) { TAPILOG << "DISPLAY "; }
    if (m.dwParam1 & LINECALLINFOSTATE_USERUSERINFO ) { TAPILOG << "USERUSERINFO "; }
    if (m.dwParam1 & LINECALLINFOSTATE_HIGHLEVELCOMP) { TAPILOG << "HIGHLEVELCOMP "; }
    if (m.dwParam1 & LINECALLINFOSTATE_LOWLEVELCOMP ) { TAPILOG << "LOWLEVELCOMP "; }
    if (m.dwParam1 & LINECALLINFOSTATE_CHARGINGINFO ) { TAPILOG << "CHARGINGINFO "; }
    if (m.dwParam1 & LINECALLINFOSTATE_TERMINAL     ) { TAPILOG << "TERMINAL "; }
    if (m.dwParam1 & LINECALLINFOSTATE_DIALPARAMS   ) { TAPILOG << "DIALPARAMS "; }
    if (m.dwParam1 & LINECALLINFOSTATE_MONITORMODES ) { TAPILOG << "MONITORMODES "; }
    if (m.dwParam1 & LINECALLINFOSTATE_TREATMENT    ) { TAPILOG << "TREATMENT "; }
    if (m.dwParam1 & LINECALLINFOSTATE_QOS          ) { TAPILOG << "QOS "; }
    if (m.dwParam1 & LINECALLINFOSTATE_CALLDATA     ) { TAPILOG << "CALLDATA "; }
    TAPILOG << endm;
  }
  else if (m.dwMessageID == LINE_LINEDEVSTATE) {
    // m.hDevice  => hLine
    // m.dwParam1 => DeviceState
    // m.dwParam2 => DeviceStateDetail1
    // m.dwParam3 => DeviceStateDetail2
    TAPILOG << "LINEDEVSTATE (" << m.hDevice << ") ";
    if (m.dwParam1 & LINEDEVSTATE_OTHER           ) { TAPILOG << "OTHER "; }
    if (m.dwParam1 & LINEDEVSTATE_RINGING         ) { TAPILOG << "RINGING "; }
    if (m.dwParam1 & LINEDEVSTATE_CONNECTED       ) { TAPILOG << "CONNECTED "; }
    if (m.dwParam1 & LINEDEVSTATE_DISCONNECTED    ) { TAPILOG << "DISCONNECTED "; }
    if (m.dwParam1 & LINEDEVSTATE_MSGWAITON       ) { TAPILOG << "MSGWAITON "; }
    if (m.dwParam1 & LINEDEVSTATE_MSGWAITOFF      ) { TAPILOG << "MSGWAITOFF "; }
    if (m.dwParam1 & LINEDEVSTATE_INSERVICE       ) { TAPILOG << "INSERVICE "; }
    if (m.dwParam1 & LINEDEVSTATE_OUTOFSERVICE    ) { TAPILOG << "OUTOFSERVICE "; }
    if (m.dwParam1 & LINEDEVSTATE_MAINTENANCE     ) { TAPILOG << "MAINTENANCE "; }
    if (m.dwParam1 & LINEDEVSTATE_OPEN            ) { TAPILOG << "OPEN "; }
    if (m.dwParam1 & LINEDEVSTATE_CLOSE           ) { TAPILOG << "CLOSE "; }
    if (m.dwParam1 & LINEDEVSTATE_NUMCALLS        ) { TAPILOG << "NUMCALLS "; }
    if (m.dwParam1 & LINEDEVSTATE_NUMCOMPLETIONS  ) { TAPILOG << "NUMCOMPLETIONS "; }
    if (m.dwParam1 & LINEDEVSTATE_TERMINALS       ) { TAPILOG << "TERMINALS "; }
    if (m.dwParam1 & LINEDEVSTATE_ROAMMODE        ) { TAPILOG << "ROAMMODE "; }
    if (m.dwParam1 & LINEDEVSTATE_BATTERY         ) { TAPILOG << "BATTERY "; }
    if (m.dwParam1 & LINEDEVSTATE_SIGNAL          ) { TAPILOG << "SIGNAL "; }
    if (m.dwParam1 & LINEDEVSTATE_DEVSPECIFIC     ) { TAPILOG << "DEVSPECIFIC "; }
    if (m.dwParam1 & LINEDEVSTATE_REINIT          ) { TAPILOG << "REINIT "; }
    if (m.dwParam1 & LINEDEVSTATE_LOCK            ) { TAPILOG << "LOCK "; }
    if (m.dwParam1 & LINEDEVSTATE_CAPSCHANGE      ) { TAPILOG << "CAPSCHANGE "; }
    if (m.dwParam1 & LINEDEVSTATE_CONFIGCHANGE    ) { TAPILOG << "CONFIGCHANGE "; }
    if (m.dwParam1 & LINEDEVSTATE_TRANSLATECHANGE ) { TAPILOG << "TRANSLATECHANGE "; }
    if (m.dwParam1 & LINEDEVSTATE_COMPLCANCEL     ) { TAPILOG << "COMPLCANCEL "; }
    if (m.dwParam1 & LINEDEVSTATE_REMOVED         ) { TAPILOG << "REMOVED "; }
    TAPILOG << " " << m.dwParam2 << "/" << m.dwParam3 << endm;
  }
  else {
    TAPILOG << "TAPI: Got message "
            << m.dwMessageID << " ("
            << m.dwParam1 << "/" << m.dwParam2 << "/" << m.dwParam3
            << ") on device " << m.hDevice
            << endm;
  }
}
