#ifndef RESOLVER_H_INCLUDED
#define RESOLVER_H_INCLUDED

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

// $Revision: 10 $ $Date: 2012-09-21 21:35:29 +0200 (Fri, 21 Sep 2012) $

#include <list>

class CContact;

enum TResolverType {
  RT_ADDRESSBOOK,
  RT_ONLINE_REGEX
};

class CResolver
{
public:
  CResolver(TResolverType type);
  virtual ~CResolver();

  TResolverType getType() const { return m_Type; }

  virtual bool isAvailable() const = 0;

  bool isEnabled() const { return m_bEnabled; }
  void enable(bool bEnable = true);

  const wxString& getName() const;
  void setName(const wxString&);

  int getPosition() const;
  void setPosition(int);

  bool isDirty() const             { return m_bDirty; }
  void setDirty(bool flag = false) { m_bDirty = flag; }

  virtual bool resolve(const wxString& strNumber, CContact& resolvedContact) = 0;

  virtual bool saveToFile(const wxString& strPath) const;
  virtual bool loadFromFile(const wxString& strPath);

  bool operator < (const CResolver& a) { return m_nPosition < a.getPosition(); }

protected:
  bool          m_bEnabled;
  bool          m_bDirty;
  int           m_nPosition;
  wxString      m_strName;
  TResolverType m_Type;
};
typedef std::list<CResolver *> TResolverList;

#endif /* RESOLVER_H_INCLUDED */
