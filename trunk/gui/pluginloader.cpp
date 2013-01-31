
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
#include "pluginloader.h"
#include "mainapp.h"

#ifdef __WXMSW__
  #include <wx/msw/msvcrt.h>      // redefines the new() operator
#endif

#define wxLOG_COMPONENT "GUI/PLUGINS"

// ---------------------------------------------------------------------------

CPluginLoader GPLUGINLOADER;

// ---------------------------------------------------------------------------

static wxString GSTR_DELETED = _("Deleted");

// ---------------------------------------------------------------------------

class CCallMonitorProxy : public CCallMonitorApi
{
public:
  CCallMonitorProxy(CCallMonitorApi *pPlugin);
  virtual ~CCallMonitorProxy() {};

  void reset();

  virtual CGUID           getID() const;
  virtual unsigned        getType() const;
  virtual const wxString& getName() const;

  virtual void registerCallObserver(ICallObserver *pObserver);
  virtual void deregisterCallObserver(ICallObserver *pObserver);
  virtual void makeCall(const std::string& strNumber);

protected:
  CGUID            m_UUID;
  CCallMonitorApi *m_pPlugin;
  wxMutex          m_mtxPlugin;
};

CCallMonitorProxy::CCallMonitorProxy(CCallMonitorApi *pPlugin)
{
  m_UUID = pPlugin->getID();
  m_pPlugin = pPlugin;
}

void CCallMonitorProxy::reset()
{
  wxMutexLocker lock(m_mtxPlugin);
  m_pPlugin = 0;
}

CGUID CCallMonitorProxy::getID() const
{
  return m_UUID;
}

unsigned CCallMonitorProxy::getType() const
{
  wxMutexLocker lock(const_cast<wxMutex&>(m_mtxPlugin));
  if (m_pPlugin) return m_pPlugin->getType();
  return TPLUGIN_TYPE_CALLPROVIDER;
}

const wxString& CCallMonitorProxy::getName() const
{
  wxMutexLocker lock(const_cast<wxMutex&>(m_mtxPlugin));
  if (m_pPlugin) return m_pPlugin->getName();
  return GSTR_DELETED;
}

void CCallMonitorProxy::registerCallObserver(ICallObserver *pObserver)
{
  wxMutexLocker lock(m_mtxPlugin);
  if (m_pPlugin) m_pPlugin->registerCallObserver(pObserver);
}

void CCallMonitorProxy::deregisterCallObserver(ICallObserver *pObserver)
{
  wxMutexLocker lock(m_mtxPlugin);
  if (m_pPlugin) m_pPlugin->deregisterCallObserver(pObserver);
}

void CCallMonitorProxy::makeCall(const std::string& strNumber)
{
  wxMutexLocker lock(m_mtxPlugin);
  if (m_pPlugin) m_pPlugin->makeCall(strNumber);
}

// ---------------------------------------------------------------------------

class CAddressbookProxy : public CAddressbookApi
{
public:
  CAddressbookProxy(CAddressbookApi *pPlugin);
  virtual ~CAddressbookProxy() {};

  void reset();

  virtual CGUID           getID() const;
  virtual unsigned        getType() const;
  virtual const wxString& getName() const;

  virtual void getEntries(TContactList& list);
  virtual void insertUpdateEntry(CContact& contact);
  virtual void deleteEntry(const CContact& contact);

  virtual void selectSortOrder(TContactSortOrder sf, bool bAsc);

  virtual void setNameAndNumberFilter(const wxString& strName,
                                      const wxString& strNumber);

  virtual bool resolve(const wxString& strNumber, CContact& resolvedContact);

protected:
  CGUID            m_UUID;
  CAddressbookApi *m_pPlugin;
  wxMutex          m_mtxPlugin;
};

CAddressbookProxy::CAddressbookProxy(CAddressbookApi *pPlugin)
{
  m_UUID = pPlugin->getID();
  m_pPlugin = pPlugin;
}

void CAddressbookProxy::reset()
{
  wxMutexLocker lock(m_mtxPlugin);
  m_pPlugin = 0;
}

CGUID CAddressbookProxy::getID() const
{
  return m_UUID;
}

unsigned CAddressbookProxy::getType() const
{
  wxMutexLocker lock(const_cast<wxMutex&>(m_mtxPlugin));
  if (m_pPlugin) return m_pPlugin->getType();
  return TPLUGIN_TYPE_ADDRESSBOOK;
}

const wxString& CAddressbookProxy::getName() const
{
  wxMutexLocker lock(const_cast<wxMutex&>(m_mtxPlugin));
  if (m_pPlugin) return m_pPlugin->getName();
  return GSTR_DELETED;
}

void CAddressbookProxy::getEntries(TContactList& list)
{
  wxMutexLocker lock(m_mtxPlugin);
  if (m_pPlugin) m_pPlugin->getEntries(list);
}

void CAddressbookProxy::insertUpdateEntry(CContact& contact)
{
  wxMutexLocker lock(m_mtxPlugin);
  if (m_pPlugin) m_pPlugin->insertUpdateEntry(contact);
}

void CAddressbookProxy::deleteEntry(const CContact& contact)
{
  wxMutexLocker lock(m_mtxPlugin);
  if (m_pPlugin) m_pPlugin->deleteEntry(contact);
}

void CAddressbookProxy::selectSortOrder(TContactSortOrder sf, bool bAsc)
{
  wxMutexLocker lock(m_mtxPlugin);
  if (m_pPlugin) m_pPlugin->selectSortOrder(sf, bAsc);
}

void CAddressbookProxy::setNameAndNumberFilter(const wxString& strName,
                                               const wxString& strNumber)
{
  wxMutexLocker lock(m_mtxPlugin);
  if (m_pPlugin) m_pPlugin->setNameAndNumberFilter(strName, strNumber);
}

bool CAddressbookProxy::resolve(const wxString& strNumber, CContact& resolvedContact)
{
  bool rc = false;
  wxMutexLocker lock(m_mtxPlugin);
  if (m_pPlugin) rc = m_pPlugin->resolve(strNumber, resolvedContact);
  return rc;
}

// ---------------------------------------------------------------------------

CPluginLoader::CPluginLoader()
{
}

CPluginLoader::~CPluginLoader()
{
  free();
}

void CPluginLoader::init()
{
  wxFileName pluginDir(wxGetCwd(), wxT(""));
  pluginDir.AppendDir(wxT("plugins"));

  wxDir dir(pluginDir.GetPath());

  wxLogMessage("Scanning plugins from \"%s\"", dir.GetName());

  if ( !dir.IsOpened() ) return;

  wxString filename;
  bool next = dir.GetFirst(&filename, wxT("hz*.dll"), wxDIR_FILES);
  while (next) {
    wxFileName dllFileName(pluginDir.GetPath(), filename);
    TPluginDescriptor desc;
    TPluginInfo info;
    info.libHandle = new wxDynamicLibrary();
    if (info.libHandle->Load(dllFileName.GetFullPath(), wxDL_DEFAULT/*|wxDL_QUIET*/))
    {
      wxLogMessage("\"%s\": Successfully loaded", dllFileName.GetFullPath());
      wxString symbolName;
#ifdef _MSC_VER
      symbolName = wxT("?CreatePlugin@@YAPAVIPluginApi@@PAVIPluginRegistry@@@Z");
#else
      symbolName = wxT("_Z12CreatePluginP15IPluginRegistry");
#endif
      if (info.libHandle->HasSymbol(symbolName))
      {
        CREATEPLUGIN pfnCreatePlugin = (CREATEPLUGIN)info.libHandle->GetSymbol(symbolName);
        info.plugin = pfnCreatePlugin(this);
        info.plugin->getInfo(desc);
        info.strFilename    = filename;
        info.strName        = wxString::FromAscii(desc.pszShortName);
        info.strDescription = wxString::FromAscii(desc.pszLongName);
        info.strVersion     = wxString::FromAscii(desc.pszVersion);
        info.pluginId       = desc.pluginId;
        // Store plugin info
        m_Plugins.push_back(info);
        // Add locale catalog of plugin
        wxGetApp().getLocale().AddCatalog(dllFileName.GetName());
      }
      else {
        wxLogWarning("\"%s\": No valid plugin", dllFileName.GetFullPath());
        info.libHandle->Unload();
        delete info.libHandle;
      }
    }
    else {
      wxLogWarning("\"%s\": Can't load plugin", dllFileName.GetFullPath());
      delete info.libHandle;
    }
    next = dir.GetNext(&filename);
  }
}

void CPluginLoader::free()
{
  for (int i = 0; i < m_Plugins.size(); ++i) {
    wxLogDebug("[PLUGIN] Unloading %s...", m_Plugins[i].strName);
    delete m_Plugins[i].plugin;
    m_Plugins[i].libHandle->Unload();
    delete m_Plugins[i].libHandle;
  }
  m_Plugins.clear();
  TCallMonitorMap::iterator mo = m_CallMonitors.begin();
  for (; mo != m_CallMonitors.end(); ++mo) delete mo->second;
  TAddressbookMap::iterator ab = m_Addressbooks.begin();
  for (; ab != m_Addressbooks.end(); ++ab) delete ab->second;
}

void CPluginLoader::registerModule(IPluginModule *p)
{
  if (p) {
    CGUID id = p->getID();
    if (p->getType() & TPLUGIN_TYPE_CALLPROVIDER) {
      CCallMonitorProxy *proxy = new CCallMonitorProxy(reinterpret_cast<CCallMonitorApi *>(p));
      m_CallMonitors.insert(TCallMonitorMap::value_type(id, proxy));
    }
    else if (p->getType() & TPLUGIN_TYPE_ADDRESSBOOK) {
      CAddressbookProxy *proxy = new CAddressbookProxy(reinterpret_cast<CAddressbookApi *>(p));
      m_Addressbooks.insert(TAddressbookMap::value_type(id, proxy));
    }
  }
}

void CPluginLoader::unregisterModule(IPluginModule *p)
{
  if (p) {
    CGUID id = p->getID();
    if (p->getType() & TPLUGIN_TYPE_CALLPROVIDER) {
      TCallMonitorMap::iterator it = m_CallMonitors.find(id);
      if (it != m_CallMonitors.end()) reinterpret_cast<CCallMonitorProxy *>(it->second)->reset();
    }
    else if (p->getType() & TPLUGIN_TYPE_ADDRESSBOOK) {
      TAddressbookMap::iterator it = m_Addressbooks.find(id);
      if (it != m_Addressbooks.end()) reinterpret_cast<CAddressbookProxy *>(it->second)->reset();
    }
  }
}

void CPluginLoader::notify(unsigned id, unsigned param1, unsigned param2)
{

}

CAddressbookApi *CPluginLoader::getAddressbook(const CGUID& gid)
{
  CAddressbookApi *p = NULL;
  TAddressbookMap::iterator it = m_Addressbooks.find(gid);
  if (it != m_Addressbooks.end()) p = it->second;
  return p;
}

const TAddressbookMap& CPluginLoader::getAddressbooks() const
{
  return m_Addressbooks;
}

CCallMonitorApi *CPluginLoader::getCallMonitor(const wxString& name)
{
  CCallMonitorApi *p = NULL;
  TCallMonitorMap::iterator it = m_CallMonitors.begin();
  for (; it != m_CallMonitors.end(); ++it) {
    if (name.CompareTo(it->second->getName()) != 0) continue;
    p = it->second;
    break;
  }
  return p;
}

void CPluginLoader::getCallMonitorNames(wxArrayString& names) const
{
  TCallMonitorMap::const_iterator it = m_CallMonitors.begin();
  for (; it != m_CallMonitors.end(); ++it) {
    names.Add(it->second->getName());
  }
}

const TCallMonitorMap& CPluginLoader::getCallMonitors() const
{
  return m_CallMonitors;
}
