
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
#include "settingspage.h"
#include "prefpage.h"
#include "resolverpage.h"
#include "pluginloader.h"
#include "resources.h"
#include "mainapp.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI"

// ----------------------------------------------------------------------------

class CPluginOverviewPage : public wxPanel
{
public:
  CPluginOverviewPage( wxWindow *parent, wxWindowID id = wxID_ANY);
  virtual ~CPluginOverviewPage();

protected:
  DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CPluginOverviewPage, wxPanel)
END_EVENT_TABLE()

CPluginOverviewPage::CPluginOverviewPage(wxWindow* parent, wxWindowID id)
  : wxPanel( parent, id, wxDefaultPosition, wxDefaultSize,
             wxTAB_TRAVERSAL | wxNO_BORDER )
{
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

  wxStaticText *txt = new wxStaticText(this, wxID_ANY, _("Plugins"));
  txt->SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_BOLD));
  sizer->Add(txt, 0, wxLEFT|wxRIGHT|wxTOP, 5);
  sizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
      wxDefaultSize, wxHORIZONTAL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

  sizer->Add(new wxStaticText(this, wxID_ANY,
      _("Installed plug-ins are listed below. Please click on the "
        "corresponding tree-item to change a plugin's settings.")),
      wxSizerFlags(0).Expand().Border(wxTOP|wxBOTTOM, 10));
  wxListCtrl *list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition,
                                    wxSize(350, 160), wxLC_REPORT);
  list->InsertColumn(1, _("Plugin"), wxLIST_FORMAT_LEFT, 120);
  list->InsertColumn(2, _("Description"), wxLIST_FORMAT_LEFT, 250);
  list->InsertColumn(3, _("Version"), wxLIST_FORMAT_LEFT, 80);
  list->InsertColumn(4, _("File"), wxLIST_FORMAT_LEFT, 120);

  const TPluginList& plugins = wxGetApp().getPluginLoader().getPlugins();
  for (int i = 0; i < plugins.size(); ++i) {
    int idx = list->InsertItem(0, plugins[i].strName);
    list->SetItem(idx, 1, plugins[i].strDescription);
    list->SetItem(idx, 2, plugins[i].strVersion);
    list->SetItem(idx, 3, plugins[i].strFilename);
  }
  sizer->Add(list, wxSizerFlags(0).Expand().Border(wxTOP, 5));

  SetSizerAndFit( sizer );
}

CPluginOverviewPage::~CPluginOverviewPage()
{
}

// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CSettingsPage, wxTreebook)
END_EVENT_TABLE()

CSettingsPage::CSettingsPage(CResolverModel& model, wxWindow* parent, wxWindowID id)
  : wxTreebook(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxBK_DEFAULT)
{
  wxImageList *il = new wxImageList(16, 16, false, 5);
  il->Add(RESOURCES::getBitmap(BMP_COG));
  il->Add(RESOURCES::getBitmap(BMP_PLUGIN));
  il->Add(RESOURCES::getBitmap(BMP_PHONE));
  il->Add(RESOURCES::getBitmap(BMP_ABOOK));
  il->Add(RESOURCES::getBitmap(BMP_ZOOM));
  AssignImageList(il);

  AddPage(new CPrefsPage(this), _("General"), true, 0);
  AddPage(new CResolverPage(model, this), _("Resolvers"), false, 4);
  AddPage(new CPluginOverviewPage(this), _("Plugins"), false, 1);
  const TPluginList& plugins = wxGetApp().getPluginLoader().getPlugins();
  for (int i = 0; i < plugins.size(); ++i) {
    wxWindow *pGui = plugins[i].plugin->getGUI(this);
    if (pGui) {
      int img = (plugins[i].isCallProvider()) ? 2 :
                (plugins[i].isAddressBook()) ? 3 : 0;
      AddSubPage(pGui, plugins[i].strName, false, img);
    }
  }

  ExpandNode(2);  // Expand plug-in node, so the Layout()-call
                  // below considers the extra width
  GetTreeCtrl()->Layout();
}

CSettingsPage::~CSettingsPage()
{
}
