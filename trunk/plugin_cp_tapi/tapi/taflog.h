#ifndef TAFLOG_H_INCLUDED
#define TAFLOG_H_INCLUDED

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

#include <fstream>
#include <sstream>

class TapiLog
{
public:
  TapiLog();
  virtual ~TapiLog();

  TapiLog& operator<<(const std::string& s);
  TapiLog& operator<<(int n);
  TapiLog& operator<<(unsigned int n);
  TapiLog& operator<<(long n);
  TapiLog& operator<<(unsigned long n);

  // Operator for stream manipulators without argument
  TapiLog& operator<< (TapiLog& (*pf)(TapiLog&));

  friend TapiLog& endm(TapiLog&);

protected:
  void _e();
  void _x();

protected:
  std::ofstream       m_f;
  std::stringstream   m_s;

  friend class endm;
};

extern TapiLog TAPILOG;

#endif /* TAFLOG_H_INCLUDED */
