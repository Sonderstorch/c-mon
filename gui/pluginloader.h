#ifndef PLUGINLOADER_H_INCLUDED
#define PLUGINLOADER_H_INCLUDED

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

#include "plugin/types.h"
#include "plugin/generic.h"
#include "plugin/callmonitor.h"
#include "plugin/addressbook.h"

#include <vector>
#include <map>

// ---------------------------------------------------------------------------

struct TPluginInfo
{
  unsigned          pluginId;
  wxString          strFilename;
  wxString          strName;
  wxString          strDescription;
  wxString          strVersion;
  IPluginApi       *plugin;
  wxDynamicLibrary *libHandle;

  bool isAddressBook() const  { return ((pluginId & TPLUGIN_TYPE_MASK) == TPLUGIN_TYPE_ADDRESSBOOK);  }
  bool isCallProvider() const { return ((pluginId & TPLUGIN_TYPE_MASK) == TPLUGIN_TYPE_CALLPROVIDER);  }
};
typedef std::vector<TPluginInfo> TPluginList;

class CCallMonitorProxy;
class CAddressbookProxy;

typedef std::map<CGUID, CCallMonitorApi *> TCallMonitorMap;
typedef std::map<CGUID, CAddressbookApi *> TAddressbookMap;

// ---------------------------------------------------------------------------

class CPluginLoader : public IPluginRegistry
{
public:
  CPluginLoader();
  virtual ~CPluginLoader();

  void init();
  void free();

  const TPluginList& getPlugins() const { return m_Plugins; }

  CAddressbookApi       *getAddressbook(const CGUID& gid);
  const TAddressbookMap& getAddressbooks() const;

  CCallMonitorApi       *getCallMonitor(const wxString& name);
  void                   getCallMonitorNames(wxArrayString& names) const;
  const TCallMonitorMap& getCallMonitors() const;

protected:
  virtual void registerModule(IPluginModule *p);
  virtual void unregisterModule(IPluginModule *p);

  virtual void notify(unsigned id, unsigned param1, unsigned param2);

protected:
  TPluginList      m_Plugins;
  TCallMonitorMap  m_CallMonitors;
  TAddressbookMap  m_Addressbooks;
};

#endif /* PLUGINLOADER_H_INCLUDED */
