#ifndef APITYPES_H_INCLUDED
#define APITYPES_H_INCLUDED

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

// Plug-in type
#define TPLUGIN_TYPE_MASK           0xF0000000
#define TPLUGIN_TYPE_CALLPROVIDER   0x10000000
#define TPLUGIN_TYPE_ADDRESSBOOK    0x20000000

// Plug-in 'serial no'
#define TPLUGIN_ID_MASK             0x0000FFFF

// Plug-ins that are already implemented
#define TPLUGIN_ID_AB_PSQL          0x00000001
#define TPLUGIN_ID_AB_MYSQL         0x00000002
#define TPLUGIN_ID_AB_SQLITE        0x00000003
#define TPLUGIN_ID_AB_RESERVED_2    0x00000004
#define TPLUGIN_ID_AB_RESERVED_3    0x00000005
#define TPLUGIN_ID_AB_RESERVED_4    0x00000006
#define TPLUGIN_ID_AB_RESERVED_5    0x00000007

#define TPLUGIN_ID_CP_TAPI          0x00000010
#define TPLUGIN_ID_CP_FRITZ         0x00000020
#define TPLUGIN_ID_CP_RESERVED_1    0x00000030
#define TPLUGIN_ID_CP_RESERVED_2    0x00000040
#define TPLUGIN_ID_CP_RESERVED_3    0x00000050
#define TPLUGIN_ID_CP_RESERVED_4    0x00000060
#define TPLUGIN_ID_CP_RESERVED_5    0x00000070

typedef struct tag_TPluginDescriptor
{
  unsigned     pluginId;        // = (type | id), must be unique!
  const char  *pszShortName;
  const char  *pszLongName;
  const char  *pszVersion;
} TPluginDescriptor;

#endif /* APITYPES_H_INCLUDED */
