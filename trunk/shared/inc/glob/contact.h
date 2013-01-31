#ifndef CONTACT_H_INCLUDED
#define CONTACT_H_INCLUDED

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

#include <vector>
#include <set>
#include <map>
#include <string>

// ---------------------------------------------------------------------------

#include "glob/hzshared.h"  // API definitions

// libphonenumber ------------------------------------------------------------

namespace i18n {
  namespace phonenumbers {
    class PhoneNumberUtil;              // forwards
    class PhoneNumberOfflineGeocoder;
    class PhoneNumber;
  }
}

class CGlobalPreferences;

// ---------------------------------------------------------------------------

class SHARED_API CPhone
{
public:
  enum TNumberType {
    NT_GENERAL,
    NT_CELL,
    NT_WORK,
    NT_HOME,
    NT_PAGER,
    NT_FAX,
    NT_OTHER,
    NT_MAX
  };
  enum TNumberKind {
    INTERNAL,
    NATIONAL,
    NATIONAL_RESOLVABLE,
    INTERNATIONAL,
    INVALID
  };

  CPhone(CPhone::TNumberType type);
  CPhone(const CPhone& ref);
  ~CPhone();

  // Statics
  static wxString getNumberTypeName(int i);
  static wxString getNumberKindName(int i);
  static void getSupportedRegions(std::set<std::string> *regions);
  static bool parseAndFormat(const wxString& strInput, wxString& formattedNr);
  static TNumberKind parseAndFormatForResolver(const wxString& strInput,
                                               wxString& nationalNr,
                                               wxString& e164Nr);
  static bool isInternalNumber(const wxString& strNr);
  static bool isPossibleNumber(const wxString& strNr);
  static bool numberFromText(const wxString& strText, wxString& strNumber);

  // Informational members
  bool isInternal() const;
  bool isValid() const;
  bool isMatch(const std::string& nr);

  // Getter / Setters
  void setType(CPhone::TNumberType);
  void setNumber(const wxString&);
  void setNote(const wxString&);

  CPhone::TNumberType getType() const { return m_Type;   }
  const wxString& getNumber() const   { return m_Number; }
  void getFormattedNumber(wxString&) const;
  void getCallableNumber(wxString&) const;
  const wxString& getNote() const     { return m_Note; }

  void getNumberInfo(wxString& rInfo) const;

  bool isDirty() const                { return m_bDirty; }
  void setDirty(bool flag = false)    { m_bDirty = flag; }

  void setDbKey(long long key);
  long long getDbKey() const   { return m_nDBKey; }

  CPhone& operator = (const CPhone& ref) {
    copyFrom(ref);
    return *this;
  }

protected:
  static void initClass();

  void copyFrom(const CPhone& ref);

protected:
  long long            m_nDBKey;
  bool                 m_bDirty;
  CPhone::TNumberType  m_Type;
  wxString             m_Number;
  wxString             m_Note;
  bool                 m_bValid;

  i18n::phonenumbers::PhoneNumber                       *m_PhoneNr;

  static i18n::phonenumbers::PhoneNumberUtil            *m_PhoneUtil;
  static i18n::phonenumbers::PhoneNumberOfflineGeocoder *m_GeoCoder;
  static CGlobalPreferences                             *m_Prefs;
  static std::map<std::string, wxString>                 m_mapNdd;
  static bool                                            m_bClassInitialized;

private:
  CPhone();
};
typedef std::vector<CPhone> TPhoneList;

// ---------------------------------------------------------------------------

class CContact;

struct SHARED_API CContactDialogData {
  wxString    strFN;
  wxString    strMN;
  wxString    strLN;
  wxString    strNN;
  wxString    strSN;
  wxString    strTitle;
  wxString    strOrganization;
  wxString    strEmail;
  wxString    strImage;
  TPhoneList  vPhones;
  CContactDialogData& operator = (const CContact& c);
};

// ---------------------------------------------------------------------------

class SHARED_API CContact
{
public:
  CContact();
  virtual ~CContact();

  void setFN(const wxString& val);  // First name
  void setMN(const wxString& val);  // Middle name(s)
  void setLN(const wxString& val);  // Last name
  void setNN(const wxString& val);  // Nick name
  void setSN(const wxString& val);  // Sorted/Displayed name
  void setTitle(const wxString& val);
  void setOrganization(const wxString& val);
  void setEmail(const wxString& val);
  void setImage(const wxString& val);

  const wxString& getFN() const     { return m_strFN; }
  const wxString& getMN() const     { return m_strMN; }
  const wxString& getLN() const     { return m_strLN; }
  const wxString& getNN() const     { return m_strNN; }
  const wxString& getSN() const     { return m_strSN; }
  const wxString& getTitle() const  { return m_strTitle; }
  const wxString& getOrganization() const { return m_strOrganization; }
  const wxString& getEmail() const  { return m_strEmail; }
  const wxString& getImage() const  { return m_strImage; }

  void setDbKey(long long key);
  long long getDbKey() const   { return m_nDBKey; }

  const TPhoneList& getConstPhones() const { return m_Phones; }
  TPhoneList& getPhones() { return m_Phones; }

  bool isDirty();
  void setDirty(bool flag = false) { m_bDirty = flag; }

  CContact& operator = (const CContact& rContact);
  CContact& operator = (const CContactDialogData&);

protected:
  long long   m_nDBKey;
  bool        m_bDirty;

  wxString    m_strFN;
  wxString    m_strMN;
  wxString    m_strLN;
  wxString    m_strNN;
  wxString    m_strSN;
  wxString    m_strTitle;
  wxString    m_strOrganization;
  wxString    m_strEmail;
  wxString    m_strImage;

  TPhoneList  m_Phones;
};
typedef std::vector<CContact> TContactList;

#endif /* CONTACT_H_INCLUDED */

