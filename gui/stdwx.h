
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

// $Revision: 46 $ $Date: 2012-11-18 18:55:31 +0100 (Sun, 18 Nov 2012) $

#include "wx/wx.h"
#include "wx/notebook.h"
#include "wx/dialog.h"
#include "wx/sizer.h"
#include "wx/valtext.h"
#include "wx/valgen.h"
#include "wx/listctrl.h"
#include "wx/dir.h"
#include "wx/file.h"
#include "wx/wfstream.h"
#include "wx/txtstrm.h"
#include "wx/textctrl.h"
#include "wx/config.h"
#include "wx/filename.h"
#include "wx/file.h"
#include "wx/taskbar.h"
#include "wx/gbsizer.h"
#include "wx/dynlib.h"
#include "wx/srchctrl.h"
#include "wx/statline.h"
#include "wx/treebook.h"
#include "wx/dataview.h"
#include "wx/spinctrl.h"
#include "wx/progdlg.h"
#include "wx/fontmap.h"
#include "wx/base64.h"
#include "wx/mstream.h"
#include "wx/regex.h"
#include "wx/snglinst.h"
#include "wx/spinctrl.h"
#include "wx/splitter.h"
#include "wx/choicdlg.h"
#include "wx/url.h"
#include "wx/hyperlink.h"
#include "wx/sstream.h"
#include "wx/ribbon/bar.h"
#include "wx/ribbon/page.h"

#include <map>
#include <vector>
#include <list>
#include <sstream>
#include <string>
#include <algorithm>

#ifdef __MSVC__
             // debug memory allocation enhancement
#ifdef _DEBUG
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif

#endif
