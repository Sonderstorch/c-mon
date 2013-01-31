
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

// $Revision: 42 $ $Date: 2012-10-23 23:15:58 +0200 (Tue, 23 Oct 2012) $

#include "stdwx.h"
#include "resources.h"

#include "bitmaps.h"    // auto generated

#include <cassert>

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI"

// -----------

wxBitmap *LoadMemoryBitmap(const unsigned char* data, int length, wxBitmapType type = wxBITMAP_TYPE_PNG)
{
  wxMemoryInputStream stream(data, length);
  return new wxBitmap( wxImage( stream, type, -1 ), -1 );
}

#define MEMPNG(a) LoadMemoryBitmap(a, sizeof a)

// -----------

wxFont    *RESOURCES::m_pFont[FONT_COUNT];
wxBitmap  *RESOURCES::m_pBmp[BMP_COUNT];
wxBrush   *RESOURCES::m_pBrush[BRUSH_COUNT];
wxColour   RESOURCES::m_Colour[COL_COUNT];

void RESOURCES::init()
{
  wxImage::AddHandler(new wxPNGHandler);

  m_pFont[FONT_NOTIFICATION]        = new wxFont(11, wxFONTFAMILY_SWISS,
      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Calibri"));
  m_pFont[FONT_NOTIFICATION_BOLD]   = new wxFont(12, wxFONTFAMILY_SWISS,
      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Calibri"));
  m_pFont[FONT_NOTIFICATION_SMALL]  = new wxFont(9, wxFONTFAMILY_SWISS,
      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Calibri"));
  m_pFont[FONT_NOTIFICATION_VERYSMALL]  = new wxFont( 8, wxFONTFAMILY_SWISS,
      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Calibri"));
  m_pFont[FONT_ABOUT_TITLE]         = new wxFont(24, wxFONTFAMILY_SWISS,
      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
  m_pFont[FONT_ABOUT_SUBTITLE]      = new wxFont(11, wxFONTFAMILY_SWISS,
      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
  m_pFont[FONT_ABOUT_TEXT]          = new wxFont(9, wxFONTFAMILY_SWISS,
      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  m_pFont[FONT_ABOUT_SUBSCRIPT]     = new wxFont(8, wxFONTFAMILY_SWISS,
      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

  m_pBrush[BRUSH_NOTIFICATION_BGR] = new wxBrush(
      wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

  m_Colour[COL_NOTIFICATION_BGR] =
      wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK);
  m_Colour[COL_NOTIFICATION_TEXT] =
      wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT);

  m_pBmp[BMP_LOGO]              = MEMPNG(logo_png);
  m_pBmp[BMP_CALL_ALERT]        = MEMPNG(appbar_alert_png);
  m_pBmp[BMP_CALL_INCOMING]     = MEMPNG(appbar_door_enter_png);
  m_pBmp[BMP_CALL_OUTGOING]     = MEMPNG(appbar_door_leave_png);
  m_pBmp[BMP_CALL_ABSENCE]      = MEMPNG(appbar_suitcase_png);
  m_pBmp[BMP_FOLDER]            = MEMPNG(appbar_folder_open_png);
  m_pBmp[BMP_WRENCH]            = MEMPNG(wrench_png);
  m_pBmp[BMP_COG]               = MEMPNG(cog_png);
  m_pBmp[BMP_VCARD]             = MEMPNG(vcard_png);
  m_pBmp[BMP_CLOCK]             = MEMPNG(time_png);
  m_pBmp[BMP_PLUGIN]            = MEMPNG(plugin_png);
  m_pBmp[BMP_ABOOK]             = MEMPNG(book_addresses_png);
  m_pBmp[BMP_WORLD]             = MEMPNG(world_png);
  m_pBmp[BMP_ZOOM]              = MEMPNG(zoom_png);
  m_pBmp[BMP_PHONE]             = MEMPNG(telephone_png);
  m_pBmp[BMP_TB_JOURNAL]        = MEMPNG(appbar_clock_big_png);
  m_pBmp[BMP_TB_CONTACTS]       = MEMPNG(appbar_user_big_png);
  m_pBmp[BMP_TB_SETTINGS]       = MEMPNG(appbar_tools_big_png);
  m_pBmp[BMP_TB_ABOUT]          = MEMPNG(appbar_information_circle_big_png);
  m_pBmp[BMP_TB_LOG]            = MEMPNG(appbar_journal_big_png);
  m_pBmp[BMP_USER_ADD]          = MEMPNG(appbar_user_add_png);
  m_pBmp[BMP_USER]              = MEMPNG(appbar_user_png);
  m_pBmp[BMP_USER_DEL]          = MEMPNG(appbar_user_delete_png);
  m_pBmp[BMP_AVATAR]            = MEMPNG(avatar_male_light_on_gray_96x96_png);
  m_pBmp[BMP_PLUS]              = MEMPNG(appbar_add_png);
  m_pBmp[BMP_MINUS]             = MEMPNG(appbar_minus_png);
  m_pBmp[BMP_IMPORT]            = MEMPNG(appbar_inbox_in_png);
  m_pBmp[BMP_EXPORT]            = MEMPNG(appbar_inbox_out_png);
  m_pBmp[BMP_HANDSET]           = MEMPNG(appbar_phone_png);
  m_pBmp[BMP_DIAL]              = MEMPNG(appbar_dial_png);
  m_pBmp[BMP_BULLETDOWN]        = MEMPNG(bullet_arrow_down_png);
  m_pBmp[BMP_BULLETUP]          = MEMPNG(bullet_arrow_up_png);
  m_pBmp[BMP_BULLETWARN]        = MEMPNG(bullet_error_png);
  m_pBmp[BMP_BULLETADD]         = MEMPNG(bullet_add_png);
  m_pBmp[BMP_BULLETDELETE]      = MEMPNG(bullet_delete_png);
  m_pBmp[BMP_PHONE_BIG]         = MEMPNG(phone_big_png);
  m_pBmp[BMP_PAYPAL]            = MEMPNG(paypal_png);
  m_pBmp[BMP_SHEEP]             = MEMPNG(sheepleap_logo_small_png);
}

void RESOURCES::free()
{
  for (int i = 0; i < FONT_COUNT; ++i) delete m_pFont[i];
  for (int i = 0; i < BMP_COUNT; ++i) delete m_pBmp[i];
  for (int i = 0; i < BRUSH_COUNT; ++i) delete m_pBrush[i];
}

wxFont *RESOURCES::getFont(size_t idx)
{
  assert(idx < FONT_COUNT);
  return m_pFont[idx];
}

const wxBitmap& RESOURCES::getBitmap(size_t idx)
{
  assert(idx < BMP_COUNT);
  return *(m_pBmp[idx]);
}

wxBrush *RESOURCES::getBrush(size_t idx)
{
  assert(idx < BRUSH_COUNT);
  return m_pBrush[idx];
}

wxColour RESOURCES::getColour(size_t idx)
{
  assert(idx < COL_COUNT);
  return m_Colour[idx];
}
