
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

// $Revision: 45 $ $Date: 2012-10-28 22:50:28 +0100 (Sun, 28 Oct 2012) $

#include "stdwx.h"
#include "aboutpage.h"
#include "pluginloader.h"
#include "resources.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#include <wx/stdpaths.h>

// ----------------------------------------------------------------------------
// CAboutPage
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CAboutPage, wxPanel)
  EVT_HYPERLINK(wxID_ANY, CAboutPage::OnHyperlink)
END_EVENT_TABLE()

CAboutPage::CAboutPage(wxWindow* parent, wxWindowID id)
  : wxPanel(parent, id, wxDefaultPosition, wxSize(0,0),
      wxCLIP_CHILDREN|wxNO_BORDER|wxTAB_TRAVERSAL)
{
  Create(parent, id);
}

CAboutPage::~CAboutPage()
{
}

bool CAboutPage::Create(wxWindow* parent, wxWindowID id)
{
  wxColour colBackground(20,50,80);

  wxGridBagSizer *sizer = new wxGridBagSizer( 0, 0 );
  sizer->SetCols(6);
  sizer->SetRows(8);
  sizer->AddGrowableCol( 1 );
  sizer->AddGrowableCol( 4 );
  sizer->AddGrowableRow( 6 );
  sizer->SetFlexibleDirection( wxBOTH );
  sizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  wxStaticBitmap *bmp1 = new wxStaticBitmap(this, wxID_ANY, RESOURCES::getBitmap(BMP_LOGO));
  wxStaticBitmap *bmpSheep = new wxStaticBitmap(this, wxID_ANY, RESOURCES::getBitmap(BMP_SHEEP));
  wxStaticText *txt2 = new wxStaticText(this, wxID_ANY, wxT("V 1.1.0\nCopyright \xA9 2012"));
  txt2->SetFont(*RESOURCES::getFont(FONT_ABOUT_SUBTITLE));
  txt2->SetForegroundColour(*wxWHITE);
  wxStaticText *txt3 = new wxStaticText(this, wxID_ANY, _("All Rights Reserved"));
  txt3->SetFont(*RESOURCES::getFont(FONT_ABOUT_TEXT));
  txt3->SetForegroundColour(*wxWHITE);
  // Top Spacer
  sizer->Add(new wxGBSizerItem(15, 50,
      wxGBPosition( 1, 1 ), wxGBSpan( 1, 2 ), wxLEFT|wxRIGHT, 5));
  sizer->Add(new wxGBSizerItem(15, 50,
      wxGBPosition( 1, 5 ), wxGBSpan( 1, 2 ), wxLEFT|wxRIGHT, 5));
  // Headliner with Line
  sizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL),
      wxGBPosition( 1, 3 ), wxGBSpan( 6, 1 ), wxEXPAND, 0);
  sizer->Add(bmp1,
      wxGBPosition( 2, 2 ), wxGBSpan( 2, 1 ), wxALIGN_RIGHT|wxLEFT|wxRIGHT, 5);
  sizer->Add(new wxGBSizerItem(5, 50,
      wxGBPosition( 2, 4 ), wxGBSpan( 1, 1 ), wxLEFT|wxRIGHT, 5));
  wxBoxSizer *pCopySizerV = new wxBoxSizer(wxVERTICAL);
  pCopySizerV->Add(txt2, 0, 0, 0);
  pCopySizerV->Add(bmpSheep, 0, wxTOP, 5);
  sizer->Add(pCopySizerV,
      wxGBPosition( 3, 4 ), wxGBSpan( 1, 1 ), wxALIGN_BOTTOM|wxLEFT, 10);
  sizer->Add(txt3,
      wxGBPosition( 4, 4 ), wxGBSpan( 1, 1 ), wxALIGN_TOP|wxLEFT, 10);
  // Middle Spacer
  sizer->Add(new wxHyperlinkCtrl(this, wxID_ANY,
      _("License of 3rd-party components"), wxT("3rdparty")),
      wxGBPosition( 5, 4 ), wxGBSpan( 1, 1 ), wxALIGN_TOP|wxLEFT, 10);
#ifndef INTERNAL_BUILD
  wxBoxSizer *pDonationSizerV = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *pDonationSizerH = new wxBoxSizer(wxHORIZONTAL);
  wxStaticBitmap *bmpDonate = new wxStaticBitmap(this, wxID_ANY,
      RESOURCES::getBitmap(BMP_PAYPAL));
  wxStaticText *txt5 = new wxStaticText(this, wxID_ANY,
      _("c'mon is free of charge,\nbut please support the development\n"
        "of c'mon with a small donation.\nThank you!"));
  txt5->SetFont(*RESOURCES::getFont(FONT_ABOUT_SUBSCRIPT));
  txt5->SetForegroundColour(*wxWHITE);
  pDonationSizerH->Add(txt5, 0, wxLEFT|wxALIGN_TOP, 10);
  pDonationSizerH->Add(bmpDonate, 0, wxLEFT|wxALIGN_TOP, 5);
  pDonationSizerV->Add(pDonationSizerH, 0, wxEXPAND, 0);
  pDonationSizerV->Add(new wxHyperlinkCtrl(this, wxID_ANY,
      _("Visit PayPal to donate (secure connection)"),
      wxT("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=YPMY3UEXF6TAA")),
      0, wxLEFT|wxEXPAND, 10);
  wxStaticText *txt6 = new wxStaticText(this, wxID_ANY,
      _("Give feedback, follow the development and get support:"));
  txt6->SetFont(*RESOURCES::getFont(FONT_ABOUT_SUBSCRIPT));
  txt6->SetForegroundColour(*wxWHITE);
  pDonationSizerV->Add(txt6, 0, wxLEFT|wxTOP|wxALIGN_TOP, 10);
  pDonationSizerV->Add(new wxHyperlinkCtrl(this, wxID_ANY,
      _("Follow c'mon on Google+"),
      wxT("https://plus.google.com/105093189409072811300/")),
      0, wxLEFT|wxEXPAND, 10);
  pDonationSizerV->Add(new wxHyperlinkCtrl(this, wxID_ANY,
      _("Visit the c'mon website"),
      wxT("https://sites.google.com/site/cmontelephony/")),
      0, wxLEFT|wxEXPAND, 10);

  sizer->Add(new wxGBSizerItem(-1, 50,
      wxGBPosition( 5, 1 ), wxGBSpan( 1, 2 ), wxLEFT|wxRIGHT|wxEXPAND, 5));
  sizer->Add(pDonationSizerV,
      wxGBPosition( 6, 4 ), wxGBSpan( 1, 1 ), wxALIGN_TOP|wxEXPAND|wxTOP, 15);
#else
  sizer->Add(new wxGBSizerItem(5, 50,
      wxGBPosition( 5, 1 ), wxGBSpan( 1, 4 ), wxLEFT|wxRIGHT, 5));
#endif
  // Bottom Info
#if 0
  sizer->Add(txt4,
      wxGBPosition( 6, 2 ), wxGBSpan( 1, 4 ), wxALIGN_TOP|wxLEFT|wxRIGHT, 5);
#endif
  // Bottom Spacer
  sizer->Add(new wxGBSizerItem(-1, 100,
      wxGBPosition( 7, 1 ), wxGBSpan( 1, 4 ), wxLEFT|wxRIGHT|wxEXPAND, 5));

  SetBackgroundColour(colBackground);

  SetSizerAndFit(sizer);

  return true;
}

void CAboutPage::OnHyperlink(wxHyperlinkEvent& e)
{
  wxString url = e.GetURL();
  if (url == wxT("3rdparty")) {
    wxFileName fn(wxStandardPaths::Get().GetResourcesDir(), wxEmptyString);
    fn.AppendDir(wxT("licenses")); fn.SetFullName(wxT("licenses.html"));
    url = wxT("file:///"); url.Append(fn.GetFullPath(wxPATH_UNIX));
  }
  wxLaunchDefaultBrowser(url);
}
