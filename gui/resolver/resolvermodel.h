#ifndef RESOLVERMODEL_H_INCLUDED
#define RESOLVERMODEL_H_INCLUDED

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

#include "resolver/resolver.h"
#include "resolver/rexweb.h"

#include <string>
#include <vector>

// ---

struct TResolveRequest
{
  unsigned    m_nCallId;
  unsigned    m_nUserData;
  wxString    m_strNumberNational;
  wxString    m_strNumberE164;
  bool        m_bOnlineResolvable;

  TResolveRequest(unsigned id = 0)
    : m_nCallId(id)
    , m_nUserData(0)
    , m_bOnlineResolvable(false) {  }
  TResolveRequest(unsigned id,
                  unsigned data,
                  const wxString& strNationalNr,
                  const wxString& strE164,
                  bool bOnlineResolvable)
    : m_nCallId(id)
    , m_nUserData(data)
    , m_strNumberNational(strNationalNr)
    , m_strNumberE164(strE164)
    , m_bOnlineResolvable(bOnlineResolvable)  {  }
};
typedef std::list<TResolveRequest> TRequestList;

// ---

class CDBBackend;

class CResolverModel : public wxThreadHelper
{
public:
  CResolverModel(CDBBackend *pDB);
  ~CResolverModel();

  const std::string& getLastError() const { return m_strLastError; }

  size_t           getEntryCount() const;
  const CResolver *getEntry(size_t idx) const;
  bool             deleteEntry(size_t idx);

  bool             moveUp(size_t idx);
  bool             moveDown(size_t idx);

  bool             toggle(size_t idx);

  void             saveEntries();

  bool             hasEntry(const wxString strName);
  bool             prependEntry(CResolver *pR);
  bool             appendEntry(CResolver *pR);

  void             shutdown();

  // Synchronous resolve (on user request)
  bool resolve(const wxString& strNumber, CContact& resolvedContact);

  // Asynchronous resolve (will post EVT_CMD_RESOLVER_RESPONSE with
  // SetInt(callId), SetExtraLong(userdata) and CContact as payload)
  void resolve(unsigned id,
               const wxString& strNationalNumber,
               const wxString& strE164Number,
               bool bOnlineResolvable,
               unsigned userdata);

protected:
  void clearResolvers();
  void loadResolvers();

protected:
  // Thread stuff
  void processRequest(const TResolveRequest& req);
  virtual wxThread::ExitCode Entry ();

protected:
  bool              m_bTerminate;
  wxCriticalSection m_csRequestList;
  TRequestList      m_RequestList;
  wxSemaphore       m_semaQ;

  wxMutex           m_mtxResolvers;
  TResolverList     m_Resolvers;
  CDBBackend       *m_pDB;
  std::string       m_strLastError;
};

#endif /* RESOLVERMODEL_H_INCLUDED */
