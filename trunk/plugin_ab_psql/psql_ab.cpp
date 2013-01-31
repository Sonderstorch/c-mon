
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

// $Revision: 47 $ $Date: 2012-11-18 22:38:42 +0100 (Sun, 18 Nov 2012) $

#include "stdwx.h"
#include "psql_ab.h"
#include "psql_ab_prefs.h"

#include "glob/contact.h"

#include <string>
#include <sstream>

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

using namespace soci;
using namespace std;

// ----------------------------------------------------------------------------

static const int connection_pool_size = 2;

// ----------------------------------------------------------------------------

CPSQLAddressbook::CPSQLAddressbook(CGUID id)
  : m_id(id)
  , m_is_ok(false)
  , m_nSortOrder(C_SORT_SN)
  , m_bSortAsc(true)
  , m_conn_pool(connection_pool_size)
  , m_strName(wxT("PostgreSQL"))
{
  PSQLPREFS::init();

  stringstream ss;

  CPSqlPrefs& prefs = PSQLPREFS::getPrefs();

  ss << "host="       << prefs.getHost()
     << " port="      << prefs.getPort()
     << " dbname="    << prefs.getDatabase()
     << " user="      << prefs.getUsername()
     << " password="  << prefs.getPassword()
     << " options='-c client_encoding=UTF8'";

  try
  {
    for (int i = 0; i < connection_pool_size; ++i) {
      soci::session& session = m_conn_pool.at(i);
      session.open(postgresql, ss.str().c_str());
    }
    m_is_ok = true;
  }
  catch (const soci_error& e) {
    m_strLastError = e.what();
  }
  catch (const std::exception& e) {
    m_strLastError = e.what();
  }
}

CPSQLAddressbook::~CPSQLAddressbook()
{
  PSQLPREFS::free();
}

CGUID CPSQLAddressbook::getID() const
{
  return m_id;
}

unsigned CPSQLAddressbook::getType() const
{
  return (TPLUGIN_TYPE_ADDRESSBOOK|TPLUGIN_ID_AB_PSQL);
}

const wxString& CPSQLAddressbook::getName() const
{
  return m_strName;
}

void CPSQLAddressbook::getEntries(TContactList& list)
{
  list.clear();

  if (isOk())
  {
    long long contactkey, phonekey;
    std::string strFN,strMN,strLN,strNN,strSN,strTitle,strOrg,strMail,strPicture;
    indicator i_fn, i_mn, i_ln, i_nn, i_sn, i_title, i_org, i_mail, i_pic;
    int nPhoneType;
    std::string strCanonial, strNote, strCond;
    indicator i_phonetype, i_canonical, i_note;
    soci::session session(m_conn_pool);
    try
    {
      CContact      c;
      stringstream  ss;
      wxString      str;

      ss << "select id,fn,mn,ln,nn,sn,title,org,email,picture from contacts";
      if (!m_strNameFilter.empty()) {
        strCond = " sn ilike '%" + m_strNameFilter + "%' ";
      }
      if (!m_strNumberFilter.empty()) {
        if (strCond.length()) {
          strCond += " and ";
        }
        strCond += " id in (select cref from numbers where canonical like '%" + m_strNumberFilter + "%')";
      }
      if (!strCond.empty()) {
        ss << " where " << strCond;
      }
      ss << " order by ";
      switch (m_nSortOrder) {
      case C_SORT_FN: ss << "fn "; break;
      case C_SORT_LN: ss << "ln "; break;
      case C_SORT_MN: ss << "mn "; break;
      case C_SORT_NN: ss << "nn "; break;
      case C_SORT_SN: ss << "sn "; break;
      case C_SORT_TITLE: ss << "title "; break;
      case C_SORT_ORGANIZATION: ss << "org "; break;
      case C_SORT_EMAIL: ss << "email "; break;
      };
      ss << (m_bSortAsc ? "asc" : "desc");
      statement contacts = (session.prepare << ss.str(),
            into(contactkey),
            into(strFN, i_fn), into(strMN, i_mn),
            into(strLN, i_ln), into(strNN, i_nn),
            into(strSN, i_sn), into(strTitle, i_title),
            into(strOrg, i_org), into(strMail, i_mail),
            into(strPicture, i_pic)
      );
      // Fetch Contacts
      statement phones = (session.prepare
          << "select id,type,canonical,note from numbers where cref = :cref",
            into(phonekey),
            into(nPhoneType, i_phonetype),
            into(strCanonial, i_canonical), into(strNote, i_note),
            use(contactkey)
      );
      contacts.execute();
      while(contacts.fetch())
      {
        c.setDbKey(contactkey);
        str.Clear();
        if (i_fn == i_ok) str = wxString::FromUTF8(strFN.c_str(), strFN.length());
        c.setFN(str);
        str.Clear();
        if (i_mn == i_ok) str = wxString::FromUTF8(strMN.c_str(), strMN.length());
        c.setMN(str);
        str.Clear();
        if (i_ln == i_ok) str = wxString::FromUTF8(strLN.c_str(), strLN.length());
        c.setLN(str);
        str.Clear();
        if (i_nn == i_ok) str = wxString::FromUTF8(strNN.c_str(), strNN.length());
        c.setNN(str);
        str.Clear();
        if (i_sn == i_ok) str = wxString::FromUTF8(strSN.c_str(), strSN.length());
        c.setSN(str);
        str.Clear();
        if (i_title == i_ok) str = wxString::FromUTF8(strTitle.c_str(), strTitle.length());
        c.setTitle(str);
        str.Clear();
        if (i_org == i_ok) str = wxString::FromUTF8(strOrg.c_str(), strOrg.length());
        c.setOrganization(str);
        str.Clear();
        if (i_mail == i_ok) str = wxString::FromUTF8(strMail.c_str(), strMail.length());
        c.setEmail(str);
        str.Clear();
        if (i_pic == i_ok) str = wxString::FromUTF8(strPicture.c_str(), strPicture.length());
        c.setImage(str);
        // Fetch Numbers
        TPhoneList& lPhones = c.getPhones();
        lPhones.clear();
        phones.execute();
        while(phones.fetch()) {
          CPhone phone((CPhone::TNumberType) nPhoneType);
          phone.setDbKey(phonekey);
          if (i_canonical == i_ok) phone.setNumber(wxString::FromAscii(strCanonial.c_str(), strCanonial.length()));
          if (i_note == i_ok) phone.setNote(wxString::FromUTF8(strNote.c_str(), strNote.length()));
          phone.setDirty(false);
          lPhones.push_back(phone);
        }
        c.setDirty(false);
        list.push_back(c);
      }
      phones.clean_up();
      contacts.clean_up();
    }
    catch (const soci_error& e) {
      m_strLastError = e.what();
      wxLogError(wxString::FromUTF8(m_strLastError.data(), m_strLastError.length()));
    }
    catch (const std::exception& e) {
      m_strLastError = e.what();
      wxLogError(wxString::FromUTF8(m_strLastError.data(), m_strLastError.length()));
    }
  }
}

void CPSQLAddressbook::insertUpdateEntry(CContact& c)
{
  if (c.isDirty())
  {
    if (isOk())
    {
      int contactkey    = (int) c.getDbKey();
      std::string strFN = c.getFN().ToUTF8().data();
      std::string strMN = c.getMN().ToUTF8().data();
      std::string strLN = c.getLN().ToUTF8().data();
      std::string strNN = c.getNN().ToUTF8().data();
      std::string strSN = c.getSN().ToUTF8().data();
      std::string strTitle= c.getTitle().ToUTF8().data();
      std::string strOrg = c.getOrganization().ToUTF8().data();
      std::string strMail = c.getEmail().ToUTF8().data();
      std::string strPic = c.getImage().ToAscii().data();
      soci::session session(m_conn_pool);
      try
      {
        session.begin();
        // Save Contact
        if (contactkey == 0) {
          session << "insert into contacts (fn,mn,ln,nn,sn,title,org,email,picture) "
                     "values (:fn,:mn,:ln,:nn,:sn,:title,:org,:email,:pic)",
                     use(strFN), use(strMN), use(strLN), use(strNN),
                     use(strSN), use(strTitle), use(strOrg), use(strMail),
                     use(strPic);
          session << "select currval('contacts_id_seq')", into(contactkey);
          c.setDbKey(contactkey);
        } else {
          session << "update contacts set fn=:fn,mn=:mn,ln=:ln,nn=:nn,"
                     "sn=:sn,title=:title,org=:org,email=:email,picture=:pic "
                     "where id = :id",
                     use(strFN), use(strMN), use(strLN), use(strNN), use(strSN),
                     use(strTitle), use(strOrg), use(strMail), use(strPic),
                     use(contactkey);
        }
        // Save Numbers
        stringstream  ssKeys;
        TPhoneList& lPhones = c.getPhones();
        if (lPhones.size())
        {
          ssKeys << "delete from numbers where cref = :cref and id not in (";
          for (int pidx = 0; pidx < lPhones.size(); ++pidx)
          {
            long long phonekey = lPhones[pidx].getDbKey();
            int phoneType = lPhones[pidx].getType();
            std::string strCanonical = lPhones[pidx].getNumber().ToAscii().data();
            std::string strNote = lPhones[pidx].getNote().ToUTF8().data();
            if (phonekey == 0) {
              session << "insert into numbers (cref,type,canonical,note) "
                         "values (:cref,:type,:can,:note)",
                         use(contactkey), use(phoneType), use(strCanonical),
                         use(strNote);
              session << "select currval('numbers_id_seq')", into(phonekey);
              lPhones[pidx].setDbKey(phonekey);
            } else {
              session << "update numbers set type=:type,canonical=:can,note=:note "
                         "where id = :id",
                         use(phoneType), use(strCanonical), use(strNote),
                         use(phonekey);
            }
            if (pidx > 0) ssKeys << ",";
            ssKeys << lPhones[pidx].getDbKey();
            lPhones[pidx].setDirty(false);
          }
          ssKeys << ")";
          session << ssKeys.str(), use(contactkey);
        }
        else {
          session << "delete from numbers where cref = :cref", use(contactkey);
        }
        session.commit();
        c.setDirty(false);
        // notifyListeners();
      }
      catch (const std::exception& e) {
        m_strLastError = e.what();
        session.rollback();
        wxMessageBox(wxString::FromUTF8(
            m_strLastError.data(), m_strLastError.length()),
            wxT("PSQL Plugin"), wxOK|wxICON_ERROR);
      }
    }
    else {
      c.setDirty(false);
    }
  }
}

void CPSQLAddressbook::deleteEntry(const CContact& c)
{
  if (isOk())
  {
    soci::session session(m_conn_pool);
    int contactkey    = (int) c.getDbKey();
    try
    {
      if (contactkey != 0) {
        session.begin();
        session << "delete from contacts "
                   "where id = :id", use(contactkey);
        session.commit();
      }
    }
    catch (const std::exception& e) {
      m_strLastError = e.what();
      session.rollback();
      wxMessageBox(wxString::FromUTF8(
          m_strLastError.data(), m_strLastError.length()),
          wxT("PSQL Plugin"), wxOK|wxICON_ERROR);
    }
  }
}

void CPSQLAddressbook::selectSortOrder(TContactSortOrder sf, bool bAsc)
{

}

void CPSQLAddressbook::setNameAndNumberFilter(const wxString& strName,
                                              const wxString& strNumber)
{
  m_strNameFilter = strName.ToUTF8().data();
  m_strNumberFilter = strNumber.ToUTF8().data();
}

bool CPSQLAddressbook::resolve(const wxString& strNumber,
                               CContact& resolvedContact)
{
  bool rc = false;
  if (isOk())
  {
    CPhone phoneNr(CPhone::NT_GENERAL);
    phoneNr.setNumber(strNumber);
    stringstream  ss;
    if (!phoneNr.isInternal()) {
      std::string toSearch = strNumber.Right(6).ToUTF8().data();
      ss << "select cref, canonical from numbers where "
            "regexp_replace(canonical, '[^[:digit:]]', '', 'g') like '%" << toSearch << "%'";
    } else {
      std::string toSearch = strNumber.ToUTF8().data();
      ss << "select cref, canonical from numbers where "
            "regexp_replace(canonical, '[^[:digit:]]', '', 'g') = '" << toSearch << "'";
    }
    soci::session session(m_conn_pool);
    try
    {
      int contactref; std::string canonical;
      indicator i_ref, i_canonical;
      statement phones = ( session.prepare << ss.str(),
                            into(contactref, i_ref),
                            into(canonical, i_canonical)
      );
      phones.execute();
      while(!rc && phones.fetch()) {
        rc = phoneNr.isMatch(canonical);
        wxLogMessage(wxT("Found %s -> %smatch"),
            wxString::From8BitData(canonical.c_str()),
            !rc ? wxT("no ") : wxEmptyString);
      }
      if (rc)
      {
        wxString  str;
        std::string strFN,strMN,strLN,strNN,strSN,strTitle,strOrg,strMail,strPicture;
        indicator i_fn, i_mn, i_ln, i_nn, i_sn, i_title, i_org, i_mail, i_pic;
        session << "select fn,mn,ln,nn,sn,title,org,email,picture from contacts where id = :cref",
              into(strFN, i_fn), into(strMN, i_mn),
              into(strLN, i_ln), into(strNN, i_nn),
              into(strSN, i_sn), into(strTitle, i_title),
              into(strOrg, i_org), into(strMail, i_mail),
              into(strPicture, i_pic), use(contactref);
        resolvedContact.setDbKey(contactref);
        str.Clear();
        if (i_fn == i_ok) str = wxString::FromUTF8(strFN.c_str(), strFN.length());
        resolvedContact.setFN(str);
        str.Clear();
        if (i_mn == i_ok) str = wxString::FromUTF8(strMN.c_str(), strMN.length());
        resolvedContact.setMN(str);
        str.Clear();
        if (i_ln == i_ok) str = wxString::FromUTF8(strLN.c_str(), strLN.length());
        resolvedContact.setLN(str);
        str.Clear();
        if (i_nn == i_ok) str = wxString::FromUTF8(strNN.c_str(), strNN.length());
        resolvedContact.setNN(str);
        str.Clear();
        if (i_sn == i_ok) str = wxString::FromUTF8(strSN.c_str(), strSN.length());
        resolvedContact.setSN(str);
        str.Clear();
        if (i_title == i_ok) str = wxString::FromUTF8(strTitle.c_str(), strTitle.length());
        resolvedContact.setTitle(str);
        str.Clear();
        if (i_org == i_ok) str = wxString::FromUTF8(strOrg.c_str(), strOrg.length());
        resolvedContact.setOrganization(str);
        str.Clear();
        if (i_mail == i_ok) str = wxString::FromUTF8(strMail.c_str(), strMail.length());
        resolvedContact.setEmail(str);
        str.Clear();
        if (i_pic == i_ok) str = wxString::FromUTF8(strPicture.c_str(), strPicture.length());
        resolvedContact.setImage(str);
        wxLogMessage(wxT("Resolved to: %s"), resolvedContact.getSN());
      }
    }
    catch (const std::exception& e) {
      m_strLastError = e.what();
      wxLogError(wxString::FromUTF8(m_strLastError.data(), m_strLastError.length()));
    }
  }
  return rc;
}
