
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

// $Revision: 14 $ $Date: 2012-09-21 22:37:23 +0200 (Fri, 21 Sep 2012) $

#include "stdwx.h"

#include "glob/prefs.h"

#include "plugin/types.h"
#include "plugin/addressbook.h"

#include "sqlite_ab.h"
#include "sqlite_ab_gui.h"
#include "sqlite_ab_version.h"
#include "sqlite_ab_main.h"
#include "sqlite_ab_prefs.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// ----------------------------------------------------------------------------

// {0D9BCAB7-6154-4c57-97F2-F6F8920D6D5F}
static const UUID pluginUUID =
{ 0xd9bcab7, 0x6154, 0x4c57, { 0x97, 0xf2, 0xf6, 0xf8, 0x92, 0xd, 0x6d, 0x5f } };

// ----------------------------------------------------------------------------

TPluginDescriptor CSQLiteAddressbookPlugin::m_Descriptor =
{
    (TPLUGIN_TYPE_ADDRESSBOOK|TPLUGIN_ID_AB_SQLITE),
    "SQLite",
    "SQLite data source for address book",
    pluginVER_STRING
};

// Addressbook Interface ------------------------------------------------------

CSQLiteAddressbookPlugin::CSQLiteAddressbookPlugin(IPluginRegistry *pApp)
  : m_pApplication(pApp)
{
  TDbFiles dbFiles;
  SQLITEPREFS::init();
  SQLITEPREFS::getDbFiles(dbFiles);
  if (dbFiles.empty()) {
    // Initial case after update from old version or new installation
    std::string strDBFN = GetGlobalPrefs()->getProfileFileName("pbook", "db");
    CSQLiteAddressbook *pBook =
        new CSQLiteAddressbook(pluginUUID, wxT("SQLite"), strDBFN);
    m_Addressbooks.push_back(pBook);
    m_pApplication->registerModule(pBook);
    saveAddressbookInfo();
  }
  else {
    // Otherwise, we have information in the registry
    TDbFiles::const_iterator it = dbFiles.begin();
    for (; it != dbFiles.end(); ++it) {
      CSQLiteAddressbook *pBook =
          new CSQLiteAddressbook(pluginUUID, it->first, it->second);
      m_Addressbooks.push_back(pBook);
      m_pApplication->registerModule(pBook);
    }
  }
}

CSQLiteAddressbookPlugin::~CSQLiteAddressbookPlugin()
{
  saveAddressbookInfo();
  TAddressbooks::iterator it = m_Addressbooks.begin();
  for (; it != m_Addressbooks.end(); ++it) {
    m_pApplication->unregisterModule(*it);
    delete *it;
  }
  SQLITEPREFS::free();
}

void CSQLiteAddressbookPlugin::getInfo(TPluginDescriptor& info)
{
  info = m_Descriptor;
}

wxWindow* CSQLiteAddressbookPlugin::getGUI(wxWindow* parent)
{
  return new CSQLiteAddressbookGui(this, parent);
}

const TAddressbooks& CSQLiteAddressbookPlugin::getAddressbooks() const
{
  return m_Addressbooks;
}

void CSQLiteAddressbookPlugin::addAddressbook(const wxString& name,
                                              const wxString& path)
{
  UUID id;
  wxASSERT(!addressBookNameExists(name) && !addressBookPathExists(path));
  UuidCreate(&id);
  CSQLiteAddressbook *pBook = new CSQLiteAddressbook(id, name, path);
  m_Addressbooks.push_back(pBook);
  m_pApplication->registerModule(pBook);
  saveAddressbookInfo();
}

bool CSQLiteAddressbookPlugin::addressBookNameExists(const wxString& strName)
{
  wxString name;
  bool rc = false;
  TAddressbooks::const_iterator it = m_Addressbooks.begin();
  for (; it != m_Addressbooks.end(); ++it) {
    name = (*it)->getName();
    if (strName.CmpNoCase(name) == 0) {
      rc = true;
      break;
    }
  }
  return rc;
}

bool CSQLiteAddressbookPlugin::addressBookPathExists(const wxString& strPath)
{
  wxString path;
  bool rc = false;
  TAddressbooks::const_iterator it = m_Addressbooks.begin();
  for (; it != m_Addressbooks.end(); ++it) {
    path = (*it)->getPath();
    if (strPath.CmpNoCase(path) == 0) {
      rc = true;
      break;
    }
  }
  return rc;
}

void CSQLiteAddressbookPlugin::saveAddressbookInfo()
{
  TDbFiles dbFiles;
  wxString name, path;
  TAddressbooks::const_iterator it = m_Addressbooks.begin();
  for (; it != m_Addressbooks.end(); ++it) {
    if (!(*it)->isMarkedForDeletion()) {
      name = (*it)->getName();
      path = (*it)->getPath();
      dbFiles[name] = path;
    }
  }
  SQLITEPREFS::setDbFiles(dbFiles);
}

// Factory -------------------------------------------------------------------

IPluginApi* CreatePlugin(IPluginRegistry *pCallback)
{
  CSQLiteAddressbookPlugin *pPlugin = new CSQLiteAddressbookPlugin(pCallback);
  return pPlugin;
}
