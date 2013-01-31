#ifndef ABOUTPAGE_H_INCLUDED
#define ABOUTPAGE_H_INCLUDED

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

// $Revision: 23 $ $Date: 2012-09-27 22:16:02 +0200 (Thu, 27 Sep 2012) $

class CAboutPage : public wxPanel
{
  public:
    CAboutPage(wxWindow* parent,
               wxWindowID id = wxID_ANY);
    virtual ~CAboutPage();

    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY);

    void OnHyperlink(wxHyperlinkEvent&);

  protected:
    DECLARE_EVENT_TABLE()
};

#endif /* ABOUTPAGE_H_INCLUDED */
