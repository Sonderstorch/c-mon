
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
#include "mainapp.h"
#include "notification.h"
#include "resources.h"

#ifdef __WXMSW__
#  include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI"

// ----------------------------------------------------------------------------

#define NOTIFICATION_WIDTH  320
#define NOTIFICATION_HEIGHT 100

// ----------------------------------------------------------------------------
// CNotificationMgr
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CNotificationMgr, wxEvtHandler)
  EVT_TIMER(wxID_ANY, CNotificationMgr::OnTimer)
END_EVENT_TABLE()

CNotificationMgr::CNotificationMgr(CMainApp *pParent)
  : wxEvtHandler()
  , m_pParent(pParent)
{
  m_AnimTimer.SetOwner(this, 1);
  m_bDeletion = false;
}

CNotificationMgr::~CNotificationMgr()
{
}

void CNotificationMgr::notify(CJournalEntry& e)
{
  wxRect screen;
  wxClientDisplayRect(&screen.x, &screen.y, &screen.width, &screen.height);

  if (e.getType() == CJournalEntry::J_INCOMING) {
    CNotificationWnd *pWnd = new CNotificationWnd(e);
    e.assignNotification(pWnd);
    m_Windows.push_back(pWnd);
    pWnd->SetPosition(wxPoint(screen.width - NOTIFICATION_WIDTH, screen.height));
    pWnd->Show();
    if (CalculatePositions()) {
      m_AnimTimer.Start(10);
    }
  }
}

void CNotificationMgr::killWnd(CNotificationWnd *pWnd)
{
  TNotifyList::iterator window = m_Windows.begin();
  for ( ; window != m_Windows.end(); ++window) {
    if (*window == pWnd) {
      (*window)->HideWithEffect(wxSHOW_EFFECT_BLEND);
      (*window)->Destroy();
      delete *window;
      m_Windows.erase(window);
      if (CalculatePositions()) {
        m_AnimTimer.Start(10);
      }
      break;
    }
  }
}

#if 0
void CNotificationMgr::Update()
{
  wxRect screen;
  wxClientDisplayRect(&screen.x, &screen.y, &screen.width, &screen.height);

  const TJournalMap& calls = m_pParent->getActiveCalls();
  
  // Check updates / deletions
  TJournalMap::const_iterator call, cend;
  TNotifyList::iterator window = m_Windows.begin();
  for ( ; window != m_Windows.end(); )
  {
    call = calls.find(TJournalKey((*window)->getCall().getPid(),
                                  (*window)->getCall().getCid()));
    if (call != calls.end())
    {
      // update window
      (*window)->updateCall(*(call->second));
      (*window)->Refresh(false);
//      if (((*window)->getCall().getState() == CJournalEntry::JS_PENDING) &&
//           (call->second->getState() != CJournalEntry::JS_PENDING))
//      {
//        // call left pending state -> erase window
//        (*window)->HideWithEffect(wxSHOW_EFFECT_BLEND);
//      }
//      else {
//        // otherwise update window
//        (*window)->updateCall(*(call->second));
//        (*window)->Refresh(false);
//      }
    }
    else {
      // call has been deleted
      (*window)->HideWithEffect(wxSHOW_EFFECT_BLEND);
      (*window)->Destroy();
      delete *window;
      window = m_Windows.erase(window);
      continue;
    }
    ++window;
  }
  // Check additions
  for (call = calls.begin(), cend = calls.end(); call != cend; ++call) {
    bool bFound = false;
    for (window = m_Windows.begin(); window !=  m_Windows.end(); ++window) {
      TJournalKey key((*window)->getCall().getPid(), (*window)->getCall().getCid());
      if (key == call->first) {
        bFound = true;
        break;
      }
    }
    if (!bFound && (call->second->getType() == CJournalEntry::J_INCOMING)) {
      CNotificationWnd *pWnd = new CNotificationWnd(*(call->second));
      m_Windows.push_back(pWnd);
      pWnd->SetPosition(wxPoint(screen.width - NOTIFICATION_WIDTH, screen.height));
      pWnd->Show();
    }
  }
  if (CalculatePositions()) {
    m_AnimTimer.Start(10);
  }
}
#endif

bool CNotificationMgr::CalculatePositions()
{
  bool bNeedMove = false;

  wxRect screen;
  wxClientDisplayRect(&screen.x, &screen.y, &screen.width, &screen.height);

  wxPoint pos(screen.width - NOTIFICATION_WIDTH, screen.height - NOTIFICATION_HEIGHT);
  TNotifyList::iterator window = m_Windows.begin(), wend = m_Windows.end();
  for ( ; window != wend; window++ )
  {
    (*window)->setTargetPos(pos);
    bNeedMove = bNeedMove || (*window)->needToMove();
    pos.y -= NOTIFICATION_HEIGHT;
  }

  return bNeedMove;
}

void CNotificationMgr::OnTimer(wxTimerEvent& evt)
{
  bool bNeedMove = false;
  TNotifyList::iterator window = m_Windows.begin(), wend = m_Windows.end();
  for ( ; window != wend; window++ ) {
    (*window)->move();
    // (*window)->Refresh();
    bNeedMove = bNeedMove || (*window)->needToMove();
  }
  if (!bNeedMove) {
    m_AnimTimer.Stop();
  }
}

// ----------------------------------------------------------------------------
// CNotificationWnd
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CNotificationWnd, wxTopLevelWindow)
  EVT_CLOSE(CNotificationWnd::OnCloseWindow)
  EVT_PAINT(CNotificationWnd::OnPaint)
  EVT_ERASE_BACKGROUND(CNotificationWnd::OnEraseBackGround)
END_EVENT_TABLE()

CNotificationWnd::CNotificationWnd(const CJournalEntry& call)
  : m_call(call)
{
  Create();
}

CNotificationWnd::~CNotificationWnd()
{
}

void CNotificationWnd::update()
{
  Refresh(false);
}

void CNotificationWnd::setTargetPos(const wxPoint& pos)
{
   m_TargetPos = pos;
}

bool CNotificationWnd::needToMove()
{
  return (GetScreenPosition() != m_TargetPos);
}

void CNotificationWnd::move()
{
  wxPoint pos = GetScreenPosition();

  if (pos != m_TargetPos)
  {
    if (pos.x < m_TargetPos.x) {
      pos.x += 3; if (pos.x > m_TargetPos.x) pos.x = m_TargetPos.x;
    }
    else if (pos.x > m_TargetPos.x) {
      pos.x -= 3; if (pos.x < m_TargetPos.x) pos.x = m_TargetPos.x;
    }
    if (pos.y < m_TargetPos.y) {
      pos.y += 3; if (pos.y > m_TargetPos.y) pos.y = m_TargetPos.y;
    }
    else if (pos.y > m_TargetPos.y) {
      pos.y -= 10; if (pos.y < m_TargetPos.y) pos.y = m_TargetPos.y;
    }
    Move(pos, wxSIZE_ALLOW_MINUS_ONE );
  }
}

bool CNotificationWnd::Create()
{
  wxTopLevelWindow::Create(NULL, wxID_ANY, wxEmptyString,
    wxDefaultPosition, wxSize(NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT),
    wxFRAME_TOOL_WINDOW|wxSTAY_ON_TOP);
  /*
    wxStaticBitmap *left = new wxStaticBitmap(this, wxID_ANY, *m_pLogo);

    wxStaticText *line1 = new wxStaticText(this, wxID_ANY, call.GetCallerAddress());
    line1->SetFont(*m_pFont2);
    wxStaticText *line2 = new wxStaticText(this, wxID_ANY, call.GetCallerName());
    line2->SetFont(*m_pFont1);
    wxStaticText *line3 = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("\x25BA %s"), call.GetCalledAddress()));
    line3->SetFont(*m_pFont3);

    wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *sizer2 = new wxBoxSizer(wxVERTICAL);
    sizer2->Add(line1, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5);
    sizer2->Add(line2, 0, wxLEFT|wxRIGHT|wxEXPAND, 5);
    sizer2->Add(line3, 0, wxLEFT|wxRIGHT|wxEXPAND, 5);
    sizer1->Add(left, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
    sizer1->Add(sizer2, 1, wxEXPAND);

    m_pSizer->Add(sizer1);
  */
  return TRUE;
}

void CNotificationWnd::OnCloseWindow(wxCloseEvent& WXUNUSED(evt))
{
  this->Destroy();
}

void CNotificationWnd::OnPaint(wxPaintEvent&)
{
  wxPaintDC  dc(this);
  wxMemoryDC mdc(&dc);

  wxRect rect = GetClientRect();
  int height  = rect.GetHeight();
  int width   = rect.GetWidth();

  // Create & Select bitmap into memory dc to draw upon
  wxBitmap bmpBgr(width, height, dc);
  mdc.SelectObject(bmpBgr);

  // Erase background
  mdc.SetPen(*wxTRANSPARENT_PEN);
  mdc.SetBrush(*RESOURCES::getBrush(BRUSH_NOTIFICATION_BGR));
  mdc.DrawRectangle(0, 0, width, height);

  // Draw info
  wxString strTxt;
  int      xpos, ypos = 2;

  mdc.SetTextForeground(RESOURCES::getColour(COL_NOTIFICATION_TEXT));

  mdc.SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_SMALL));
  strTxt = _("Call from");
  mdc.DrawText(strTxt, 5, ypos);
  wxSize szTxt =  mdc.GetTextExtent(strTxt);
  xpos = szTxt.GetWidth() + 20;

  mdc.SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_BOLD));
  strTxt = m_call.getCallerAddress();
  mdc.DrawText(strTxt, xpos, ypos);
  szTxt = mdc.GetTextExtent(strTxt);
  ypos += szTxt.GetHeight();

  mdc.SetFont(*RESOURCES::getFont(FONT_NOTIFICATION));
  strTxt = m_call.getCallerName();
  mdc.DrawText(strTxt, xpos, ypos);
  szTxt = mdc.GetTextExtent(strTxt);
  ypos += szTxt.GetHeight() + 10;

  mdc.SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_SMALL));
  strTxt = _("Call to");
  mdc.DrawText(strTxt, 5, ypos);
  mdc.SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_BOLD));
  strTxt = m_call.getCalledAddress();
  mdc.DrawText(strTxt, xpos, ypos);
  szTxt = mdc.GetTextExtent(strTxt);
  ypos += szTxt.GetHeight();

  mdc.SetFont(*RESOURCES::getFont(FONT_NOTIFICATION));
  mdc.DrawText(m_call.getCalledName(), xpos, ypos);

  mdc.SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_VERYSMALL));
  wxPoint pos = GetScreenPosition();
  strTxt = m_call.getStateText();
  szTxt = mdc.GetTextExtent(strTxt);
  mdc.DrawText(strTxt, width - szTxt.x - 2, height - szTxt.y - 2);

  if (m_call.hasImage()) {
    mdc.DrawBitmap(m_call.getImage(), NOTIFICATION_WIDTH - m_call.getImage().GetWidth() - 4, 2);
  }

  /*
  mdc.SetFont(*RESOURCES::getFont(FONT_NOTIFICATION_VERYSMALL));
  wxPoint pos = GetScreenPosition();
  strTxt = wxString::Format(wxT("%3d:%3d -> %3d:%3d: %s"),
      pos.x, pos.y, m_TargetPos.x, m_TargetPos.y,
      needToMove() ? wxT("yes") : wxT("no"));
  szTxt = mdc.GetTextExtent(strTxt);
  mdc.DrawText(strTxt, width - szTxt.x - 2, height - szTxt.y - 2);
  */

  // Blit result
  dc.Blit(0, 0, width, height, &mdc, 0, 0, wxCOPY, false);
  mdc.SelectObject(wxNullBitmap);
}

void CNotificationWnd::OnEraseBackGround(wxEraseEvent& evt)
{
  /* do nth. */
}
