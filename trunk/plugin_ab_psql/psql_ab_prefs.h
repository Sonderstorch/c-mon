#ifndef PSQL_AB_PREFS_H_INCLUDED
#define PSQL_AB_PREFS_H_INCLUDED

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

#include <vector>

class IPrefChangeListener
{
  public:
    IPrefChangeListener() {  }
    virtual ~IPrefChangeListener() {  }

    virtual void OnPrefChange() = 0;
};

// --------------------------------------------------------------------

class PSQLPREFS;
class CPSQLAddressbookGui;

class CPSqlPrefs
{
public:
  CPSqlPrefs();

  const wxString& getHost() const             { return m_strHost;   }
  unsigned        getPort() const             { return m_nHostPort; }
  bool            useOSUser() const           { return m_bUseOSUser;  }

  const wxString& getDatabase() const             { return m_strDatabase;  }
  const wxString& getUsername() const             { return m_strUsername;  }
  const wxString& getPassword() const             { return m_strPassword;  }

  bool operator ==(const CPSqlPrefs& r) {
    return (m_strHost      == r.m_strHost)      &&
           (m_nHostPort    == r.m_nHostPort)    &&
           (m_strDatabase  == r.m_strDatabase)  &&
           (m_bUseOSUser   == r.m_bUseOSUser)   &&
           (m_strUsername  == r.m_strUsername)  &&
           (m_strPassword  == r.m_strPassword);
  }

protected:
  wxString m_strHost;
  unsigned m_nHostPort;
  wxString m_strDatabase;
  bool     m_bUseOSUser;
  wxString m_strUsername;
  wxString m_strPassword;

  friend class PSQLPREFS;
  friend class CPSQLAddressbookGui;
};

// --------------------------------------------------------------------

class PSQLPREFS
{
  public:
    static void init();
    static void free();

    static CPSqlPrefs&  getPrefs();
    static void         readPrefs();
    static void         savePrefs();

    static void registerListener(IPrefChangeListener *);
    static void unregisterListener(IPrefChangeListener *l);
    static void notify();

  private:
    static CPSqlPrefs                        m_Prefs;
    static wxConfig                         *m_pConfig;
    static std::vector<IPrefChangeListener*> m_Listeners;

    PSQLPREFS() {};
    ~PSQLPREFS() {};
};

#endif /* PSQL_AB_PREFS_H_INCLUDED */
