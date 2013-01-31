
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

#include "stdwx.h"
#include "glob/prefs.h"
#include "glob/contact.h"

#include "phonenumbers/phonenumber.pb.h"
#include "phonenumbers/phonenumberutil.h"
#include "phonenumbers/phonenumbermatch.h"
#include "phonenumbers/phonenumbermatcher.h"
#include "phonenumbers/geocoding/phonenumber_offline_geocoder.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

using namespace std;
using namespace i18n;
using namespace phonenumbers;

// ---------------------------------------------------------------------------
// CPhone
// ---------------------------------------------------------------------------

// Static variables

bool                                            CPhone::m_bClassInitialized = false;
i18n::phonenumbers::PhoneNumberUtil            *CPhone::m_PhoneUtil = 0;
i18n::phonenumbers::PhoneNumberOfflineGeocoder *CPhone::m_GeoCoder = 0;
CGlobalPreferences                             *CPhone::m_Prefs = 0;
std::map<std::string, wxString>                 CPhone::m_mapNdd;

// Static Methods ------------------------------------------------------------

wxString CPhone::getNumberTypeName(int i)
{
  switch(i) {
    case CPhone::NT_GENERAL: return _("General");
    case CPhone::NT_CELL:    return _("Mobile");
    case CPhone::NT_WORK:    return _("Work");
    case CPhone::NT_HOME:    return _("Home");
    case CPhone::NT_PAGER:   return _("Pager");
    case CPhone::NT_FAX:     return _("Fax");
  }
  return _("Other");
}

wxString CPhone::getNumberKindName(int i)
{
  switch (i) {
    case CPhone::INTERNAL:            return wxT("INTERNAL");
    case CPhone::NATIONAL:            return wxT("NATIONAL");
    case CPhone::NATIONAL_RESOLVABLE: return wxT("NATIONAL_RESOLVABLE");
    case CPhone::INTERNATIONAL:       return wxT("INTERNATIONAL");
  }
  return wxT("INVALID");
}

void CPhone::initClass()
{
  // init statics
  if (!m_bClassInitialized)
  {
    m_bClassInitialized = true;

    m_PhoneUtil = PhoneNumberUtil::GetInstance();
    m_GeoCoder  = new PhoneNumberOfflineGeocoder();
    m_Prefs     = GetGlobalPrefs();

    std::string ndd;
    std::set<std::string> regions;
    m_PhoneUtil->GetSupportedRegions(&regions);
    std::set<std::string>::const_iterator it = regions.begin();
    for (; it != regions.end(); ++it) {
      m_PhoneUtil->GetNddPrefixForRegion(*it, true, &ndd);
      m_mapNdd.insert(make_pair(*it, wxString::FromUTF8(ndd.c_str())));
    }
  }
}

void CPhone::getSupportedRegions(std::set<std::string> *regions)
{
  initClass();
  std::map<std::string, wxString>::const_iterator it = m_mapNdd.begin();
  for (; it != m_mapNdd.end(); ++it) {
    regions->insert(it->first);
  }
}

bool CPhone::isInternalNumber(const wxString& strNr)
{
  initClass();

  if (strNr.StartsWith(wxT("+")))
    return false;

  CGeneralPrefs& rp = m_Prefs->getPrefs();
  if (rp.isExternalByLAC())
  {
    if (strNr.StartsWith(wxString::FromUTF8(rp.getLAC().c_str()))) {
      return false;
    } else {
      return (rp.isExternalByLen()) &&
             (rp.getInternalNrLen() > 0) &&
             (strNr.Length() <= rp.getInternalNrLen());
    }
  }
  return (rp.isExternalByLen()) &&
         (rp.getInternalNrLen() > 0) &&
         (strNr.Length() <= rp.getInternalNrLen());
}

bool CPhone::isPossibleNumber(const wxString& strNr)
{
  initClass();
  return m_PhoneUtil->IsPossibleNumberForString(
      strNr.ToStdString(), m_Prefs->getPrefs().getCC());
}

bool CPhone::parseAndFormat(const wxString& val, wxString& formattedNr)
{
  initClass();

  bool rc;
  if (isInternalNumber(val))
  {
    formattedNr = val;
    rc = val.IsNumber();
  }
  else
  {
    PhoneNumber phoneNr;
    std::string strCC        = m_Prefs->getPrefs().getCC();
    std::string strAC        = m_Prefs->getPrefs().getAC();
    bool        bAddAC       = m_Prefs->getPrefs().addACIfShortLen();
    int         nLocalMaxLen = m_Prefs->getPrefs().getLocalNrMaxLen();
    PhoneNumberUtil::ErrorType err = m_PhoneUtil->ParseAndKeepRawInput(
        val.ToUTF8().data(), strCC, &phoneNr);
    if (err == PhoneNumberUtil::NO_PARSING_ERROR)
    {
      std::string number;
      if (bAddAC && !strAC.empty() && (val.Length() < nLocalMaxLen))
      {
        // When we have an AreaCode set in the preferences and the number
        // entered is too short to contain an area code then add it and
        // parse/format again.
        // NOTE: Because Area Codes are ambiguous this can produce unexpected
        //       results or simply not work at all.
        std::string nsn;
        m_PhoneUtil->GetNationalSignificantNumber(phoneNr, &nsn);
        number  = strAC;
        number += nsn;
        err = m_PhoneUtil->Parse(number, strCC, &phoneNr);
        if (err == PhoneNumberUtil::NO_PARSING_ERROR) {
          m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::INTERNATIONAL, &number);
          formattedNr = wxString::FromUTF8(number.c_str());
          rc = true;
        } else {
          formattedNr = val;
          rc = false;
        }
      }
      else {
        m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::INTERNATIONAL, &number);
        formattedNr = wxString::FromUTF8(number.c_str());
        rc = true;
      }
    }
    else {
      formattedNr = val;
      rc = false;
    }
  }
  return rc;
}

CPhone::TNumberKind CPhone::parseAndFormatForResolver(const wxString& val,
                                                      wxString& nationalNr,
                                                      wxString& e164Nr)
{
  initClass();

  TNumberKind rc = INVALID;
  nationalNr = val;
  e164Nr = val;

  if (isInternalNumber(val)) {
    return INTERNAL;
  }

  PhoneNumber phoneNr;
  std::string strCC = m_Prefs->getPrefs().getCC();
   PhoneNumberUtil::ErrorType err = m_PhoneUtil->ParseAndKeepRawInput(
      val.ToUTF8().data(), strCC, &phoneNr);
  if (err != PhoneNumberUtil::NO_PARSING_ERROR)
      return INVALID;

  if (phoneNr.has_country_code() &&
      (phoneNr.country_code_source() != PhoneNumber_CountryCodeSource_FROM_DEFAULT_COUNTRY))
  {
    if (phoneNr.country_code() != m_PhoneUtil->GetCountryCodeForRegion(strCC)) {
      // Phone Number has an IDD other than current country, so format for international dialing
      std::string number;
      m_PhoneUtil->FormatInOriginalFormat(phoneNr, strCC, &number);
      nationalNr = wxString::FromUTF8(number.c_str());
      m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::E164, &number);
      e164Nr = wxString::FromUTF8(number.c_str());
      return INTERNATIONAL;
    }
    // Phone Number has own IDD, so format national for resolver
    std::string number;
    m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::NATIONAL, &number);
    nationalNr = wxString::FromUTF8(number.c_str());
    m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::E164, &number);
    e164Nr = wxString::FromUTF8(number.c_str());
    return NATIONAL_RESOLVABLE;
  }

  // Seems to be a national number

  std::string number;
  wxString strNDD = m_mapNdd[strCC];
  if (strNDD.empty()) {
    std::string number;
    m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::NATIONAL, &number);
    nationalNr = wxString::FromUTF8(number.c_str());
    m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::E164, &number);
    e164Nr = wxString::FromUTF8(number.c_str());
    return NATIONAL_RESOLVABLE;
  }

  if (val.StartsWith(strNDD)) { // probably with area code
    m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::NATIONAL, &number);
    nationalNr = wxString::FromUTF8(number.c_str());
    m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::E164, &number);
    e164Nr = wxString::FromUTF8(number.c_str());
    return NATIONAL_RESOLVABLE;
  }

  // probably local number without area code, so return a formatted
  // number with added area code if one is set in the preferences
  std::string strAC = m_Prefs->getPrefs().getAC();
  bool bAddAC       = m_Prefs->getPrefs().addACIfShortLen();
  int nLocalMaxLen  = m_Prefs->getPrefs().getLocalNrMaxLen();
  if (bAddAC && !strAC.empty() && (val.Length() < nLocalMaxLen))
  {
    std::string nsn;
    m_PhoneUtil->GetNationalSignificantNumber(phoneNr, &nsn);
    number  = strAC; number += nsn;
    err = m_PhoneUtil->Parse(number, strCC, &phoneNr);
    if (err == PhoneNumberUtil::NO_PARSING_ERROR) {
      m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::NATIONAL, &number);
      nationalNr = wxString::FromUTF8(number.c_str());
      m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::E164, &number);
      e164Nr = wxString::FromUTF8(number.c_str());
      return NATIONAL_RESOLVABLE;
    }
  }

  m_PhoneUtil->Format(phoneNr, PhoneNumberUtil::E164, &number);
  e164Nr = wxString::FromUTF8(number.c_str());
  return NATIONAL;
}

bool CPhone::numberFromText(const wxString& strText, wxString& result)
{
  initClass();

  bool rc = false;
  std::string number;
  PhoneNumberMatch match;
  PhoneNumberMatcher matcher(strText.ToUTF8().data(),  m_Prefs->getPrefs().getCC());
  if (matcher.HasNext()) {
    matcher.Next(&match);
    m_PhoneUtil->Format(match.number(), PhoneNumberUtil::E164, &number);
    result = wxString::FromUTF8(number.c_str());
    rc = true;
  }
  return rc;
}

// Methods -------------------------------------------------------------------

CPhone::CPhone(CPhone::TNumberType type)
  : m_nDBKey(0)
  , m_bDirty(true)
  , m_Type(type)
  , m_bValid(false)
{
  initClass();
  m_PhoneNr = new PhoneNumber();
}

CPhone::CPhone(const CPhone& ref)
{
  copyFrom(ref);
}

void CPhone::copyFrom(const CPhone& ref)
{
  m_nDBKey  = ref.m_nDBKey;
  m_bDirty  = ref.m_bDirty;
  m_Type    = ref.m_Type;
  m_Number  = ref.m_Number;
  m_Note    = ref.m_Note;
  m_bValid  = ref.m_bValid;
  // Deep Copy
  m_PhoneNr = new PhoneNumber(*ref.m_PhoneNr);
}

CPhone::~CPhone()
{
  delete m_PhoneNr;
}

void CPhone::setType(CPhone::TNumberType val)
{
  m_bDirty = m_bDirty || (m_Type != val);
  m_Type = val;
}

void CPhone::setNumber(const wxString& val)
{
  m_bDirty = m_bDirty || (m_Number != val);
  m_Number = val;
  m_Number.Trim(false); m_Number.Trim(true);
  if (isInternal()) {
    m_bValid = m_Number.IsNumber();
  }
  else {
    PhoneNumberUtil::ErrorType err = m_PhoneUtil->Parse(
        m_Number.ToUTF8().data(), m_Prefs->getPrefs().getCC(),
        m_PhoneNr);
    m_bValid = (err == PhoneNumberUtil::NO_PARSING_ERROR);
  }
}

void CPhone::setNote(const wxString& val)
{
  m_bDirty = m_bDirty || (m_Note != val);
  m_Note = val;
}

void CPhone::setDbKey(long long key)
{
  m_bDirty = m_bDirty || (m_nDBKey != key);
  m_nDBKey = key;
}

void CPhone::getNumberInfo(wxString& rInfo) const
{
  if (!m_bValid) {
    rInfo = wxT("");
  }
  else if (isInternal()) {
    rInfo = _("Internal");
  }
  else if (m_PhoneNr->IsInitialized()) {
    std::string desc = m_GeoCoder->GetDescriptionForNumber(*m_PhoneNr,
        icu::Locale::getDefault());
    rInfo = wxString::FromUTF8(desc.c_str());
  }
  else {
    rInfo = wxT("");
  }
}

void CPhone::getFormattedNumber(wxString& rNumber) const
{
  if (!m_bValid || isInternal()) {
    rNumber = m_Number;
  }
  else if (m_PhoneNr->IsInitialized()) {
    std::string number;
    m_PhoneUtil->Format(*m_PhoneNr, PhoneNumberUtil::INTERNATIONAL, &number);
    rNumber = wxString::FromUTF8(number.c_str());
  }
  else {
    rNumber = wxT("");
  }
}

void CPhone::getCallableNumber(wxString& rNumber) const
{
  if (!m_bValid || isInternal()) {
    rNumber = m_Number;
  }
  else if (m_PhoneNr->IsInitialized())
  {
    wxString wxNumber;
    std::string number;
    m_PhoneUtil->FormatOutOfCountryCallingNumber(*m_PhoneNr, m_Prefs->getPrefs().getCC(), &number);
    wxNumber = wxString::FromUTF8(number.c_str(), number.length());
    int chMode = m_Prefs->getPrefs().getExternalNrChangeMode();
    wxString strLineAccess = wxString::FromUTF8(
        m_Prefs->getPrefs().getLAC().c_str(),
        m_Prefs->getPrefs().getLAC().length());
    if (chMode == PREF_EXTNR_CHANGE_MODE_ADD_PREFIX) {
      rNumber = strLineAccess; rNumber += wxNumber;
    } else if (chMode == PREF_EXTNR_CHANGE_MODE_DEL_PREFIX) {
      if (!wxNumber.StartsWith(strLineAccess, &rNumber)) {
        rNumber = wxNumber;
      }
    } else {
      rNumber = wxNumber;
    }
  }
  else {
    rNumber = wxT("");
  }
}

bool CPhone::isMatch(const std::string& nr)
{
  bool rc = false;
  if (m_bValid) {
    if (isInternal()) {
      rc = m_Number.IsSameAs(nr.c_str());
    }
    else if (m_PhoneNr->IsInitialized())
    {
      PhoneNumberUtil::MatchType match = m_PhoneUtil->
          IsNumberMatchWithOneString(*m_PhoneNr, nr);
      rc = ((match == PhoneNumberUtil::EXACT_MATCH) ||
            (match == PhoneNumberUtil::NSN_MATCH));
    }
  }
  return rc;
}

// Other informational members -----------------------------------------------

bool CPhone::isValid() const
{
  return m_bValid;
}

bool CPhone::isInternal() const
{
  return isInternalNumber(m_Number);
}

// ---------------------------------------------------------------------------
// CContact
// ---------------------------------------------------------------------------

CContact& CContact::operator =(const CContact& rContact)
{
  m_nDBKey   = rContact.m_nDBKey;
  m_bDirty   = rContact.m_bDirty;
  m_strFN    = rContact.m_strFN;
  m_strMN    = rContact.m_strMN;
  m_strLN    = rContact.m_strLN;
  m_strNN    = rContact.m_strNN;
  m_strSN    = rContact.m_strSN;
  m_strTitle = rContact.m_strTitle;
  m_strImage = rContact.m_strImage;
  m_Phones   = rContact.m_Phones;
  return *this;
}

CContact& CContact::operator =(const CContactDialogData& d)
{
  setFN(d.strFN);
  setMN(d.strMN);
  setLN(d.strLN);
  setNN(d.strNN);
  setSN(d.strSN);
  setTitle(d.strTitle);
  setOrganization(d.strOrganization);
  setEmail(d.strEmail);
  setImage(d.strImage);
  TPhoneList& rPhones = getPhones();
  m_bDirty = m_bDirty || (rPhones.size() != d.vPhones.size());
  rPhones.clear();
  TPhoneList::const_iterator it = d.vPhones.begin();
  for (; it != d.vPhones.end(); ++it) {
    rPhones.push_back(*it);
  }
  return *this;
}

CContactDialogData& CContactDialogData::operator = (const CContact& c)
{
  strFN           = c.getFN();
  strMN           = c.getMN();
  strLN           = c.getLN();
  strNN           = c.getNN();
  strSN           = c.getSN();
  strTitle        = c.getTitle();
  strOrganization = c.getOrganization();
  strEmail        = c.getEmail();
  strImage        = c.getImage();
  vPhones.clear();
  TPhoneList::const_iterator it = c.getConstPhones().begin();
  for (; it != c.getConstPhones().end(); ++it) {
    vPhones.push_back(*it);
  }
  return *this;
}

// ---------------------------------------------------------------------------

CContact::CContact()
  : m_nDBKey(0)
  , m_bDirty(false)
{
}

CContact::~CContact()
{
}

// ---------------------------------------------------------------------------

void CContact::setFN(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strFN != val);
  m_strFN = val;
}

void CContact::setMN(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strMN != val);
  m_strMN = val;
}

void CContact::setLN(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strLN != val);
  m_strLN = val;
}

void CContact::setNN(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strNN != val);
  m_strNN = val;
}

void CContact::setSN(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strSN != val);
  m_strSN = val;
}

void CContact::setTitle(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strTitle != val);
  m_strTitle = val;
}

void CContact::setOrganization(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strOrganization != val);
  m_strOrganization = val;
}

void CContact::setEmail(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strEmail != val);
  m_strEmail = val;
}

void CContact::setImage(const wxString& val)
{
  m_bDirty = m_bDirty || (m_strImage != val);
  m_strImage = val;
}

void CContact::setDbKey(long long key)
{
  m_bDirty = m_bDirty || (m_nDBKey != key);
  m_nDBKey = key;
}

bool CContact::isDirty()
{
  if (!m_bDirty) {
    for (TPhoneList::const_iterator it = m_Phones.begin(); it < m_Phones.end(); ++it) {
      if (it->isDirty()) {
        m_bDirty = true;
        break;
      }
    }
  }
  return m_bDirty;
}
