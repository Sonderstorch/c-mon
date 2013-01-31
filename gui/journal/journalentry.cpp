
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
#include "journalentry.h"
#include "notification.h"
#include "mainapp.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI/JOURNAL"

using namespace std;

// ---------------------------------------------------------------------------
// CJournalEntry
// ---------------------------------------------------------------------------

CJournalEntry::CJournalEntry(unsigned pid, unsigned cid, CJournalEntry::TType type)
  : m_nPid(pid)
  , m_nCid(cid)
  , m_nType(type)
  , m_nState(JS_UNKNOWN)
  , m_nResult(JR_UNKNOWN)
  , m_nDurationInSec(0)
  , m_nDBKey(0)
  , m_bResolvingCaller(false)
  , m_bResolvingCalled(false)
  , m_bDirty(false)
  , m_pNotification(0)
  , m_bHasImage(false)
{
  setCreation(wxDateTime::Now());
  if (type == J_INCOMING) {
    setResult(JR_NOTREACHED);
  }
}

CJournalEntry::~CJournalEntry()
{
  if (m_pNotification) {
    wxGetApp().closeNotification(*this);
  }
}

CJournalEntry& CJournalEntry::operator = (const CJournalEntry& r)
{
  m_nPid              = r.m_nPid;
  m_nCid              = r.m_nCid;
  m_nDBKey            = r.m_nDBKey;
  m_nType             = r.m_nType;
  m_nResult           = r.m_nResult;
  m_nState            = r.m_nState;
  m_dtCreation        = r.m_dtCreation;
  m_dtBegin           = r.m_dtBegin;
  m_nDurationInSec    = r.m_nDurationInSec;
  m_strCallerAddress  = r.m_strCallerAddress;
  m_strCalledAddress  = r.m_strCalledAddress;
  m_strCallerName     = r.m_strCallerName;
  m_strCalledName     = r.m_strCalledName;
  m_bDirty            = r.m_bDirty;
  return *this;
}

void CJournalEntry::setCallerAddress(const wxString& strAddress)
{
  m_bDirty = m_bDirty || (m_strCallerAddress != strAddress);
  m_strCallerAddress = strAddress;
  if (m_pNotification) m_pNotification->update();
}

void CJournalEntry::setCalledAddress(const wxString& strAddress)
{
  m_bDirty = m_bDirty || (m_strCalledAddress != strAddress);
  m_strCalledAddress = strAddress;
  if (m_pNotification) m_pNotification->update();
}

void CJournalEntry::setCallerName(const wxString& strName)
{
  m_bDirty = m_bDirty || (m_strCallerName != strName);
  m_strCallerName = strName;
  if (m_pNotification) m_pNotification->update();
}

void CJournalEntry::setCalledName(const wxString& strName)
{
  m_bDirty = m_bDirty || (m_strCalledName != strName);
  m_strCalledName = strName;
  if (m_pNotification) m_pNotification->update();
}

void CJournalEntry::setResult(TResult result)
{
  m_bDirty = m_bDirty || (m_nResult != result);
  m_nResult = result;
  if (m_pNotification) m_pNotification->update();
}

void CJournalEntry::setCreation(const wxDateTime& dt)
{
  // Avoid silly wxDateTime assertions ... (doh!)
  if (m_dtCreation.IsValid() && !dt.IsValid()) {
    m_bDirty = true;
  } else if (!m_dtCreation.IsValid() && dt.IsValid()) {
    m_bDirty = true;
  } else {
    if (m_dtCreation.IsValid() && dt.IsValid()) {
      m_bDirty = m_bDirty || (m_dtCreation != dt);
    }
  }
  m_dtCreation = dt;
}

void CJournalEntry::setBegin(const wxDateTime& dt)
{
  // Avoid silly wxDateTime assertions ... (doh!)
  if (m_dtBegin.IsValid() && !dt.IsValid()) {
    m_bDirty = true;
  } else if (!m_dtBegin.IsValid() && dt.IsValid()) {
    m_bDirty = true;
  } else {
    if (m_dtBegin.IsValid() && dt.IsValid()) {
      m_bDirty = m_bDirty || (m_dtBegin != dt);
    }
  }
  m_dtBegin = dt;
}

void CJournalEntry::setDurationInSec(wxLongLong ll)
{
  m_bDirty = m_bDirty || (m_nDurationInSec != ll);
  m_nDurationInSec = ll;
}

void CJournalEntry::setType(TType type)
{
  m_bDirty = m_bDirty || (m_nType != type);
  m_nType = type;
}

void CJournalEntry::setDbKey(long long key)
{
  m_bDirty = m_bDirty || (m_nDBKey != key);
  m_nDBKey = key;
}

void CJournalEntry::setBeginNow()
{
  m_bDirty = true;
  m_dtBegin = wxDateTime::Now();
}

void CJournalEntry::setEndNow()
{
  m_bDirty = true;
  wxTimeSpan span = wxDateTime::Now() - m_dtBegin;
  m_nDurationInSec = span.GetSeconds();
}

void CJournalEntry::setState(CJournalEntry::TState newState)
{
  m_bDirty = m_bDirty || (m_nState != newState);

  if (getType() == J_OUTGOING)
  {
    switch (newState)
    {
    case JS_PENDING:
      setResult(JR_NOTREACHED);
      break;
    case JS_CONNECTED:
      setBeginNow();
      setResult(JR_OK);
      break;
    case JS_DISCONNECTED:
      if (m_dtBegin.IsValid()) {
        setEndNow();
      }
      break;
    case JS_BUSY:
      setResult(JR_BUSY);
      break;
    }
  }
  else if (getType() == J_INCOMING)
  {
    switch (newState)
    {
    case JS_CONNECTED:
      if (m_nState != JS_CONNECTED) {
        setResult(JR_OK);
        setBeginNow();
      }
      break;
    case JS_DISCONNECTED:
      if (m_dtBegin.IsValid()) {
        setEndNow();
      }
      break;
    }
  }
  m_nState = newState;
  if (m_pNotification) m_pNotification->update();
}

#define NOTIFICATION_THUMB_WIDTH  80
#define NOTIFICATION_THUMB_HEIGHT 80

void CJournalEntry::setImage(const wxString& strImageData)
{
  if (strImageData.Length()) {
    wxMemoryBuffer buf = wxBase64Decode(strImageData);
    wxMemoryInputStream stream(buf.GetData(), buf.GetDataLen());
    if (stream.IsOk()) {
      wxImage pic(stream);
      m_bHasImage = pic.IsOk();
      if (m_bHasImage) {
        if ((pic.GetWidth() != NOTIFICATION_THUMB_WIDTH) || (pic.GetHeight() != NOTIFICATION_THUMB_HEIGHT)) {
          double scaleH = (double)NOTIFICATION_THUMB_HEIGHT / (double)pic.GetHeight();
          double scaleW = (double)NOTIFICATION_THUMB_WIDTH / (double)pic.GetWidth();
          double factor = (scaleH < scaleW) ? scaleH : scaleW;
          int newHeight = (int)((double)pic.GetHeight() * factor);
          int newWidth  = (int)((double)pic.GetWidth() * factor);
          pic.Rescale(newWidth, newHeight, wxIMAGE_QUALITY_NORMAL);
        }
        m_bmpImage = wxBitmap(pic);
      }
    }
  }
}

const wxBitmap& CJournalEntry::getImage() const
{
  return m_bmpImage;
}

bool CJournalEntry::hasImage() const
{
  return m_bHasImage;
}

const wxString& CJournalEntry::getStateText() const
{
  switch (m_nState) {
    case JS_PENDING:        return _("pending");
    case JS_CONNECTED:      return _("connected");
    case JS_DISCONNECTED:   return _("disconnected");
    case JS_BUSY:           return _("busy");
    case JS_ONHOLD:         return _("on hold");
  }
  return _("unknown");
}

const wxString& CJournalEntry::getResultText() const
{
  switch (m_nResult) {
    case JR_OK:            return _("ok");
    case JR_BUSY:          return _("busy");
    case JR_NOTREACHED:    return _("not reached");
    case JR_FAILED:        return _("error");
  }
  return _("unknown");
}
