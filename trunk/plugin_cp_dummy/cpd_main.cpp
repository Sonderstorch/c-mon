
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

// $Revision: 44 $ $Date: 2012-10-25 23:31:06 +0200 (Thu, 25 Oct 2012) $

#include "stdwx.h"
#include "plugin/types.h"
#include "plugin/generic.h"
#include "plugin/callmonitor.h"

#include "cpd_gui.h"
#include "cpd_impl.h"
#include "cpd_main.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

// ----------------------------------------------------------------------------

TPluginDescriptor CCpDummyPlugin::m_Descriptor =
{
    PLUGIN_ID,
    "DUMMY",
    "Dummy call provider/monitor for debugging",
    "1.0.0"
};

// ----------------------------------------------------------------------------

CCpDummyPlugin::CCpDummyPlugin(IPluginRegistry *pApp)
  : m_pApplication(pApp)
{
  m_pCallMonitor = new CDummyCallMonitor();
  m_pApplication->registerModule(m_pCallMonitor);
}

CCpDummyPlugin::~CCpDummyPlugin()
{
  m_pApplication->unregisterModule(m_pCallMonitor);
  delete m_pCallMonitor;
}

void CCpDummyPlugin::getInfo(TPluginDescriptor& info)
{
  info = m_Descriptor;
}

wxWindow* CCpDummyPlugin::getGUI(wxWindow* parent)
{
  return new CCpDummyGui(parent, m_pCallMonitor, 1500);
}

CCallMonitorApi *CCpDummyPlugin::getCallMonitor()
{
  return m_pCallMonitor;
}

// Factory -------------------------------------------------------------------

IPluginApi* CreatePlugin(IPluginRegistry *pCallback)
{
  CCpDummyPlugin *pPlugin = new CCpDummyPlugin(pCallback);
  return pPlugin;
}
