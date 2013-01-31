#ifndef ADDRESSBOOK_H_INCLUDED
#define ADDRESSBOOK_H_INCLUDED

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

#ifdef BUILDING_PLUGIN
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __declspec(dllimport)
#endif

#include <vector>

#include "plugin/types.h"
#include "plugin/generic.h"

#include "glob/contact.h"

// Addressbook Interface -----------------------------------------------------

enum TContactSortOrder {
  C_SORT_FN,
  C_SORT_LN,
  C_SORT_MN,
  C_SORT_NN,
  C_SORT_SN,
  C_SORT_TITLE,
  C_SORT_ORGANIZATION,
  C_SORT_EMAIL
};

class CAddressbookApi : public IPluginModule
{
public:
  virtual ~CAddressbookApi() {};

  virtual void getEntries(TContactList& list) = 0;
  virtual void insertUpdateEntry(CContact& contact) = 0;
  virtual void deleteEntry(const CContact& contact) = 0;

  virtual void selectSortOrder(TContactSortOrder sf, bool bAsc) = 0;

  virtual void setNameAndNumberFilter(const wxString& strName,
                                      const wxString& strNumber) = 0;

  virtual bool resolve(const wxString& strNumber, CContact& resolvedContact) = 0;

protected:
  CAddressbookApi()  {};
};

// Addressbook Plugin Interface ----------------------------------------------

class CAddressbookPlugin : public IPluginApi
{
public:
  virtual ~CAddressbookPlugin() {};

protected:
  CAddressbookPlugin()  {};
};

#endif /* ADDRESSBOOK_H_INCLUDED */
