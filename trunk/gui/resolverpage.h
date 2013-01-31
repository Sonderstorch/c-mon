#ifndef RESOLVERPAGE_H_INCLUDED
#define RESOLVERPAGE_H_INCLUDED

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

#include "resolver/resolver.h"
#include "resolver/resolvermodel.h"

// ----------------------------------------------------------------------------
// Resolver UI model
// ----------------------------------------------------------------------------

class CResolverUIModel : public wxDataViewVirtualListModel
{
public:
  enum
  {
    Column_Type,
    Column_Enabled,
    Column_Name,
    Column_Count
  };

  CResolverUIModel(CResolverModel& model);

  void AddItem();
  void DeleteItem(const wxDataViewItem &item);

  void MoveUp(unsigned int row);
  void MoveDown(unsigned int row);
  void Toggle(unsigned int row);

  bool Append(CResolver *pR);

  wxString GetTitle(unsigned int col) const;

  virtual unsigned int GetColumnCount() const {
    return Column_Count;
  }

  virtual wxString GetColumnType(unsigned int col) const
  {
    if (col == Column_Type) {
      return wxT("integer");
    }
    if (col == Column_Enabled) {
      return wxT("bool");
    }
    return wxT("string");
  }

  virtual unsigned int GetRowCount();
  virtual void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const;
  virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const;
  virtual bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col);

protected:
  CResolverModel&    m_model;
};

// ----------------------------------------------------------------------------
// Resolver Pref Page
// ----------------------------------------------------------------------------

class CResolverPage : public wxPanel
{
public:
  CResolverPage(CResolverModel& model, wxWindow *parent, wxWindowID id = wxID_ANY);
  virtual ~CResolverPage();

  void OnImport(wxCommandEvent&);
  void OnExport(wxCommandEvent&);
  void OnToggle(wxCommandEvent&);
  void OnDelete(wxCommandEvent&);

  void OnMoveUp(wxCommandEvent&);
  void OnMoveDown(wxCommandEvent&);

  void OnUpdateButtons(wxUpdateUIEvent&);

protected:
  wxDataViewCtrl    *m_dvResolvers;
  CResolverUIModel  *m_pUIModel;
  CResolverModel&    m_model;

protected:
  DECLARE_EVENT_TABLE()
};

#endif /* RESOLVERPAGE_H_INCLUDED */
