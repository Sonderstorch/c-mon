
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

#include "stdwx.h"
#include "picturecontrol.h"
#include "resources.h"

#define wxLOG_COMPONENT "GUI"

// ----------------------------------------------------------------------------
// Picture Control
// ----------------------------------------------------------------------------

#define THUMBNAIL_WIDTH  125
#define THUMBNAIL_HEIGHT 125

BEGIN_EVENT_TABLE(CPictureControl, wxStaticBitmap)
  EVT_LEFT_DCLICK(CPictureControl::OnDblClick)
END_EVENT_TABLE()

CPictureControl::CPictureControl(wxWindow *parent, wxWindowID id, bool readOnly)
  : wxStaticBitmap(parent, id, RESOURCES::getBitmap(BMP_AVATAR),
      wxDefaultPosition, wxSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT),
      wxBORDER_STATIC)
  , m_bReadOnly(readOnly)
{
  if (!m_bReadOnly) {
    SetToolTip(_("Double-click to change picture"));
  }
}

CPictureControl::~CPictureControl()
{

}

void CPictureControl::AddImage()
{
  wxFileDialog dlg(this, _("Open Picture"),
      wxEmptyString, wxEmptyString,
      _("All Pictures (*.jpg;*.png;*.bmp)|*.jpg;*.png;*.bmp||"),
      wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_PREVIEW);

  if (dlg.ShowModal() == wxID_OK)
  {
    wxImage image;
    if (image.LoadFile(dlg.GetPath()))
    {
      if ((image.GetWidth() != THUMBNAIL_WIDTH) || (image.GetHeight() != THUMBNAIL_HEIGHT)) {
        double scaleH = (double)THUMBNAIL_HEIGHT / (double)image.GetHeight();
        double scaleW = (double)THUMBNAIL_WIDTH / (double)image.GetWidth();
        double factor = (scaleH < scaleW) ? scaleH : scaleW;
        int newHeight = (int)((double)image.GetHeight() * factor);
        int newWidth  = (int)((double)image.GetWidth() * factor);
        image.Rescale(newWidth, newHeight, wxIMAGE_QUALITY_HIGH);
      }
      wxMemoryOutputStream stream;
      if (image.SaveFile(stream, wxBITMAP_TYPE_PNG)) {
        wxStreamBuffer*buf = stream.GetOutputStreamBuffer();
        m_strImage = wxBase64Encode(buf->GetBufferStart(), buf->GetBufferSize());
      }
      SetBitmap(wxBitmap(image));
      GetParent()->Layout();
    }
  }
}

void CPictureControl::RemoveImage()
{
  m_strImage.Clear();
  SetBitmap(RESOURCES::getBitmap(BMP_AVATAR));
  GetParent()->Layout();
}

bool CPictureControl::hasImage()
{
  return !m_strImage.IsEmpty();
}

const wxString& CPictureControl::GetImageData() const
{
  return m_strImage;
}

void CPictureControl::SetImageData(const wxString& strImg)
{
  m_strImage = strImg;
  if (m_strImage.Length()) {
    wxMemoryBuffer buf = wxBase64Decode(m_strImage);
    wxMemoryInputStream stream(buf.GetData(), buf.GetDataLen());
    if (stream.IsOk()) {
      wxImage pic(stream);
      if (pic.IsOk()) {
        SetBitmap(pic);
        GetParent()->Layout();
      }
    }
  }
}

void CPictureControl::OnDblClick(wxMouseEvent& evt)
{
  if (!m_bReadOnly) AddImage();
}
