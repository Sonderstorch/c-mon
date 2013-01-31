#ifndef CSVIMPORTDLG_H_INCLUDED
#define CSVIMPORTDLG_H_INCLUDED

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

#include "csvimp.h"

class CImportCSVDialog : public wxDialog
{
public:
  CImportCSVDialog(wxWindow* parent);

protected:
  void OnOpenFile(wxCommandEvent& e);
  void OnImport(wxCommandEvent& e);

  void OnChoiceChange(wxCommandEvent& e);
  void OnSpinChange(wxSpinEvent&);
  void OnUpdateImportBtn(wxUpdateUIEvent&);

  void DoOpenFile();
  bool DoValidate();

protected:
  wxChoice         *m_cbEncoding;
  wxChoice         *m_cbDelimiter;
  wxChoice         *m_cbQuoteChar;
  wxSpinCtrl       *m_spinStart;
  wxString          m_strPath;
  CCsvImportCtrl   *m_ImportCtrl;

protected:
  DECLARE_EVENT_TABLE()
};

#endif /* CSVIMPORTDLG_H_INCLUDED */
