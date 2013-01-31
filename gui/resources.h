#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED

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

enum {
  FONT_NOTIFICATION,
  FONT_NOTIFICATION_BOLD,
  FONT_NOTIFICATION_SMALL,
  FONT_NOTIFICATION_VERYSMALL,
  FONT_ABOUT_TITLE,
  FONT_ABOUT_SUBTITLE,
  FONT_ABOUT_TEXT,
  FONT_ABOUT_SUBSCRIPT,
  // <--- insert here
  FONT_COUNT
};

enum {
  BMP_LOGO,
  BMP_CALL_ALERT,
  BMP_CALL_INCOMING,
  BMP_CALL_OUTGOING,
  BMP_CALL_ABSENCE,
  BMP_FOLDER,
  BMP_WRENCH,
  BMP_COG,
  BMP_VCARD,
  BMP_CLOCK,
  BMP_PLUGIN,
  BMP_PHONE,
  BMP_ABOOK,
  BMP_WORLD,
  BMP_ZOOM,
  BMP_TB_JOURNAL,
  BMP_TB_CONTACTS,
  BMP_TB_SETTINGS,
  BMP_TB_ABOUT,
  BMP_TB_LOG,
  BMP_HANDSET,
  BMP_DIAL,
  BMP_USER,
  BMP_USER_ADD,
  BMP_USER_DEL,
  BMP_PLUS,
  BMP_MINUS,
  BMP_AVATAR,
  BMP_IMPORT,
  BMP_EXPORT,
  BMP_BULLETUP,
  BMP_BULLETDOWN,
  BMP_BULLETWARN,
  BMP_BULLETADD,
  BMP_BULLETDELETE,
  BMP_PHONE_BIG,
  BMP_PAYPAL,
  BMP_SHEEP,
    // <--- insert here
  BMP_COUNT
};

enum {
  COL_NOTIFICATION_BGR,
  COL_NOTIFICATION_TEXT,
  // <--- insert here
  COL_COUNT
};


enum {
  BRUSH_NOTIFICATION_BGR,
  // <--- insert here
  BRUSH_COUNT
};

class RESOURCES
{
public:
  static void init();
  static void free();

  static wxFont *getFont(size_t idx);
  static const wxBitmap& getBitmap(size_t idx);
  static wxColour getColour(size_t idx);
  static wxBrush *getBrush(size_t idx);

protected:
  static wxFont    *m_pFont[FONT_COUNT];
  static wxBitmap  *m_pBmp[BMP_COUNT];
  static wxBrush   *m_pBrush[BRUSH_COUNT];
  static wxColour   m_Colour[COL_COUNT];

private:
  RESOURCES()     {  };
  ~RESOURCES()    {  };


};

#endif /* RESOURCES_H_ */
