
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

#include "stdwx.h"
#include "plugin/types.h"
#include "plugin/generic.h"
#include "plugin/addressbook.h"

#include "psql_ab.h"
#include "psql_ab_gui.h"
#include "psql_ab_version.h"
#include "psql_ab_main.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// ----------------------------------------------------------------------------

// {7B6A10ED-C0B0-4e3e-BC7B-2EE60FD6143E}
static const UUID pluginUUID =
{ 0x7b6a10ed, 0xc0b0, 0x4e3e, { 0xbc, 0x7b, 0x2e, 0xe6, 0xf, 0xd6, 0x14, 0x3e } };

static const CGUID GPLUGIN_GID(pluginUUID);

// ----------------------------------------------------------------------------

TPluginDescriptor CPSQLAddressbookPlugin::m_Descriptor =
{
    (TPLUGIN_TYPE_ADDRESSBOOK|TPLUGIN_ID_AB_PSQL),
    "PostgreSQL",
    "PostgreSQL data source for address book",
    pluginVER_STRING
};

// Addressbook Interface ------------------------------------------------------

CPSQLAddressbookPlugin::CPSQLAddressbookPlugin(IPluginRegistry *pApp)
  : m_pApplication(pApp)
{
  m_pAddressbook = new CPSQLAddressbook(GPLUGIN_GID);
  m_pApplication->registerModule(m_pAddressbook);
}

CPSQLAddressbookPlugin::~CPSQLAddressbookPlugin()
{
  m_pApplication->unregisterModule(m_pAddressbook);
  delete m_pAddressbook;
}

void CPSQLAddressbookPlugin::getInfo(TPluginDescriptor& info)
{
  info = m_Descriptor;
}

wxWindow* CPSQLAddressbookPlugin::getGUI(wxWindow* parent)
{
  return new CPSQLAddressbookGui(parent);
}

// Factory -------------------------------------------------------------------

IPluginApi* CreatePlugin(IPluginRegistry *pCallback)
{
  CPSQLAddressbookPlugin *pPlugin = new CPSQLAddressbookPlugin(pCallback);
  return pPlugin;
}
