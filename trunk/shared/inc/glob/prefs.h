#ifndef GLOBALPREFS_H_INCLUDED
#define GLOBALPREFS_H_INCLUDED

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

// $Revision: 47 $ $Date: 2012-11-18 22:38:42 +0100 (Sun, 18 Nov 2012) $

#include <string>

#include "glob/hzshared.h"  // API definitions
#include "glob/uuid.h"

// --------------------------------------------------------------------

enum EEXTNR_CHANGE_MODE {
  PREF_EXTNR_CHANGE_MODE_NOCHANGE,
  PREF_EXTNR_CHANGE_MODE_ADD_PREFIX,
  PREF_EXTNR_CHANGE_MODE_DEL_PREFIX
};

class CGlobalPreferences;

class SHARED_API CGeneralPrefs
{
public:
  CGeneralPrefs();

  bool startHidden() const                        { return m_bStartHidden;  }
  void enableHiddenStart(bool val)                { m_bStartHidden = val;  }

  int getSelectedPage() const                     { return m_nSelectedPage; }
  void setSelectedPage(int p)                     { m_nSelectedPage = p;    }

  const CGUID& getSelectedABook() const           { return m_uuidAB; }
  void setSelectedABook(const CGUID& val)         { m_uuidAB = val;  }

  const wxString& getDefaultCallProvider() const   { return m_strCP; }
  void setDefaultCallProvider(const wxString& val) { m_strCP = val;  }

  bool addACIfShortLen() const                    { return m_bAddACIfShortLen; }
  void setACIfShortLen(bool val)                  { m_bAddACIfShortLen = val;  }
  int  getLocalNrMaxLen() const                   { return m_nLocalNrMaxLen; }
  void setLocalNrMaxLen(int val)                  { m_nLocalNrMaxLen = val;  }

  bool isExternalByLAC() const                    { return m_bExtNrByLAC; }
  void setExternalByLAC(bool val)                 { m_bExtNrByLAC = val;  }
  bool isExternalByLen() const                    { return m_bExtNrByLength; }
  void setExternalByLen(bool val)                 { m_bExtNrByLength = val;  }
  int  getInternalNrLen() const                   { return m_nIntNrLength; }
  void setInternalNrLen(int val)                  { m_nIntNrLength = val;  }

  int getExternalNrChangeMode() const          { return m_nExtNrChangeMode; }
  void setExternalNrChangeMode(int val)        { m_nExtNrChangeMode = val;  }

  bool getDelJournalByAge() const              { return m_bDelJournalByAge; }
  int  getMaxJournalAge() const                { return m_nMaxJournalAge; }
  bool getDelJournalByCount() const            { return m_bDelJournalByCount; }
  int  getMaxJournalCount() const              { return m_nMaxJournalCount; }

  void setDelJournalByAge(bool val)            { m_bDelJournalByAge = val; }
  void setMaxJournalAge(int val)               { m_nMaxJournalAge = val; }
  void setDelJournalByCount(bool val)          { m_bDelJournalByCount = val; }
  void setMaxJournalCount(int val)             { m_nMaxJournalCount = val; }

  const std::string& getCC() const                { return m_strCC;    }
  void setCC(const std::string& val)              { m_strCC = val;     }
  const std::string& getAC() const                { return m_strAC;    }
  void setAC(const std::string& val)              { m_strAC = val;     }
  const std::string& getLAC() const               { return m_strLAC;   }
  void setLAC(const std::string& val)             { m_strLAC = val;    }

  bool operator==(const CGeneralPrefs& r) const {
    return (m_strCP               == r.m_strCP)                &&
           (m_uuidAB              == r.m_uuidAB)               &&
           (m_strCC               == r.m_strCC)                &&
           (m_strAC               == r.m_strAC)                &&
           (m_bAddACIfShortLen    == r.m_bAddACIfShortLen)     &&
           (m_nLocalNrMaxLen      == r.m_nLocalNrMaxLen)       &&
           (m_strLAC              == r.m_strLAC)               &&
           (m_bExtNrByLAC         == r.m_bExtNrByLAC)          &&
           (m_bExtNrByLength      == r.m_bExtNrByLength)       &&
           (m_nIntNrLength        == r.m_nIntNrLength)         &&
           (m_nExtNrChangeMode    == r.m_nExtNrChangeMode)     &&
           (m_bDelJournalByAge    == r.m_bDelJournalByAge)     &&
           (m_nMaxJournalAge      == r.m_nMaxJournalAge)       &&
           (m_bDelJournalByCount  == r.m_bDelJournalByCount)   &&
           (m_nMaxJournalCount    == r.m_nMaxJournalCount);
  }

protected:
  void load(wxConfig *);
  void save(wxConfig *);

protected:
  bool        m_bStartHidden;
  int         m_nSelectedPage;

  wxString    m_strCP;   // Default  Callprovider
  CGUID       m_uuidAB;  // Selected Addressbook

  std::string m_strCC;   // Country Code
  std::string m_strAC;   // Area Code
  bool        m_bAddACIfShortLen;
  int         m_nLocalNrMaxLen;
  std::string m_strLAC;  // Line Access Code
  bool        m_bExtNrByLAC;
  bool        m_bExtNrByLength;
  int         m_nIntNrLength;
  int         m_nExtNrChangeMode;
  bool        m_bDelJournalByAge;
  int         m_nMaxJournalAge;
  bool        m_bDelJournalByCount;
  int         m_nMaxJournalCount;

  friend class CGlobalPreferences;
};

// ----------------------------------------------------------------------------

class SHARED_API CGlobalPreferences
{
  public:
    CGlobalPreferences();
    ~CGlobalPreferences();

    void readUILayout(wxString&);
    void writeUILayout(const wxString&);

    bool readFrameSize(wxRect& result);
    void writeFrameSize(const wxRect&);

    CGeneralPrefs& getPrefs();
    void           readPrefs();
    void           savePrefs();

    static std::string getProfileFileName(const std::string& fn,
                                          const std::string ext);
    static std::string getDBFileName();
    static wxString getLogFileName();

  private:
    wxConfig       *m_pConfig;
    CGeneralPrefs   m_GeneralPrefs;

  private:
    CGlobalPreferences(const CGlobalPreferences&);
    CGlobalPreferences& operator =(const CGlobalPreferences&);
};

// ----------------------------------------------------------------------------

CGlobalPreferences SHARED_API * GetGlobalPrefs();

#endif /* GLOBALPREFS_H_INCLUDED */
