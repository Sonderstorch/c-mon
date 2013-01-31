#ifndef JOURNALENTRY_H_INCLUDED
#define JOURNALENTRY_H_INCLUDED

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

class CJournalModel;
class CNotificationWnd;

class CJournalEntry
{
public:
  enum TType {
    J_UNKNOWN,
    J_INCOMING,
    J_OUTGOING
  };
  enum TState {
    JS_UNKNOWN,
    JS_PENDING,
    JS_CONNECTED,
    JS_DISCONNECTED,
    JS_BUSY,
    JS_ONHOLD
  };
  enum TResult {
    JR_UNKNOWN,
    JR_OK,
    JR_BUSY,
    JR_NOTREACHED,
    JR_FAILED
  };
public:
  CJournalEntry(unsigned pid = 0, unsigned cid = 0,
                CJournalEntry::TType type = J_UNKNOWN);
  virtual ~CJournalEntry();

  void setCallerAddress(const wxString&);
  void setCalledAddress(const wxString&);
  void setCallerName(const wxString&);
  void setCalledName(const wxString&);

  const wxString& getCallerAddress() const  { return m_strCallerAddress; }
  const wxString& getCalledAddress() const  { return m_strCalledAddress; }
  const wxString& getCallerName() const     { return m_strCallerName; }
  const wxString& getCalledName() const     { return m_strCalledName; }

  void setState(TState state);
  TState getState() const          { return m_nState; }
  const wxString& getStateText() const;

  void setResult(TResult result);
  TResult getResult() const       { return m_nResult; }
  const wxString& getResultText() const;

  void setCreation(const wxDateTime& dt);
  wxDateTime getCreation() const         { return m_dtCreation; }

  void setBeginNow();
  void setBegin(const wxDateTime& dt);
  wxDateTime getBegin() const            { return m_dtBegin; }

  void setEndNow();

  void setDurationInSec(wxLongLong ll);
  wxLongLong getDurationInSec() const    { return m_nDurationInSec; }

  void  setType(TType type);
  TType getType() const     { return m_nType; }

  void setDbKey(long long key);
  long long getDbKey() const   { return m_nDBKey; }

  bool isDirty() const { return m_bDirty; }
  void setDirty(bool flag = false) { m_bDirty = flag; }

  bool isResolvingCaller() const { return m_bResolvingCaller; }
  void setResolvingCaller() { m_bResolvingCaller = true; }

  bool isResolvingCalled() const { return m_bResolvingCalled; }
  void setResolvingCalled() { m_bResolvingCalled = true; }

  void setPid(unsigned val) { m_nPid = val; }
  unsigned getPid() const { return m_nPid; }
  void setCid(unsigned val) { m_nCid = val; }
  unsigned getCid() const { return m_nCid; }

  void assignNotification(CNotificationWnd *pWnd) { m_pNotification = pWnd; }
  CNotificationWnd *getNotification() const       { return m_pNotification; }

  bool hasImage() const;
  void setImage(const wxString& strImageData);
  const wxBitmap& getImage() const;

  CJournalEntry& operator = (const CJournalEntry&);

public:
  // non-persisted members
  unsigned    m_nPid;       // Call-Provider Id
  unsigned    m_nCid;       // Call Id

protected:
  long long         m_nDBKey;
  bool              m_bDirty;
  bool              m_bResolvingCaller;
  bool              m_bResolvingCalled;
  TType             m_nType;
  TResult           m_nResult;
  TState            m_nState;
  wxDateTime        m_dtCreation;
  wxDateTime        m_dtBegin;
  wxLongLong        m_nDurationInSec;
  wxString          m_strCallerAddress;
  wxString          m_strCalledAddress;
  wxString          m_strCallerName;
  wxString          m_strCalledName;
  wxBitmap          m_bmpImage;
  bool              m_bHasImage;
  CNotificationWnd *m_pNotification;

  friend class CJournalModel;
};
typedef std::vector<CJournalEntry> TJournalList;

#endif /* JOURNALENTRY_H_INCLUDED */
