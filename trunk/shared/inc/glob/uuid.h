#ifndef UUID_H_INCLUDED
#define UUID_H_INCLUDED

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

// $Revision: 40 $ $Date: 2012-10-18 23:19:11 +0200 (Thu, 18 Oct 2012) $

#include <rpc.h>

#include "glob/hzshared.h"  // API definitions

class SHARED_API CGUID
{
public:
  CGUID();
  CGUID(const UUID& u);
  CGUID(const CGUID& u);
  CGUID(const wxString& str);
  ~CGUID() { };

  void fromString(const wxString& str);
  void toString(wxString& str);

  void operator= (const CGUID& u);
  bool operator< (const CGUID& u) const;
  bool operator== (const CGUID& u) const;

protected:
  UUID m_UUID;
};

#endif /* UUID_H_INCLUDED */
