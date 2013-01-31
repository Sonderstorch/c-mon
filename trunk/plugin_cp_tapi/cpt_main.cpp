
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

// $Revision: 43 $ $Date: 2012-10-25 23:30:03 +0200 (Thu, 25 Oct 2012) $

#include "stdwx.h"
#include "plugin/types.h"
#include "plugin/callmonitor.h"

#include "cpt_monitor.h"
#include "cpt_version.h"
#include "cpt_main.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// ----------------------------------------------------------------------------

TPluginDescriptor CCptPlugin::m_Descriptor =
{
    PLUGIN_ID,
    "TAPI",
    "Call provider/monitor for TAPI",
    pluginVER_STRING
};

// ----------------------------------------------------------------------------

CCptPlugin::CCptPlugin(IPluginRegistry *pApp)
  : m_pApplication(pApp)
{
  m_pCallMonitor = new CTapiCallMonitor();
  m_pApplication->registerModule(m_pCallMonitor);
}

CCptPlugin::~CCptPlugin()
{
  m_pApplication->unregisterModule(m_pCallMonitor);
  delete m_pCallMonitor;
}

void CCptPlugin::getInfo(TPluginDescriptor& info)
{
  info = m_Descriptor;
}

wxWindow* CCptPlugin::getGUI(wxWindow* parent)
{
  return reinterpret_cast<CTapiCallMonitor *>(m_pCallMonitor)->getGUI(parent);
}

CCallMonitorApi *CCptPlugin::getCallMonitor()
{
  return m_pCallMonitor;
}

// Factory -------------------------------------------------------------------

IPluginApi* CreatePlugin(IPluginRegistry *pCallback)
{
  CCptPlugin *pPlugin = new CCptPlugin(pCallback);
  return pPlugin;
}
