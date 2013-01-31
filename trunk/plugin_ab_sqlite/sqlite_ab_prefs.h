#ifndef SQLITE_AB_PREFS_H_INCLUDED
#define SQLITE_AB_PREFS_H_INCLUDED

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
#include <map>

class IPrefChangeListener
{
  public:
    IPrefChangeListener() {  }
    virtual ~IPrefChangeListener() {  }

    virtual void OnPrefChange() = 0;
};

// --------------------------------------------------------------------

typedef std::map<wxString, wxString> TDbFiles;

// --------------------------------------------------------------------

class SQLITEPREFS
{
  public:
    static void init();
    static void free();

    static void registerListener(IPrefChangeListener *);
    static void unregisterListener(IPrefChangeListener *l);
    static void notify();

    static void getDbFiles(TDbFiles& list);
    static void setDbFiles(const TDbFiles& list);

  private:
    static void readDatabases();
    static void writeDatabases();

  private:
    static wxConfig                         *m_pConfig;
    static std::vector<IPrefChangeListener*> m_Listeners;
    static TDbFiles                          m_DbFiles;

    SQLITEPREFS() {};
    ~SQLITEPREFS() {};
};

#endif /* SQLITE_AB_PREFS_H_INCLUDED */
