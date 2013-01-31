
; This file is part of c'mon, a lightweight telephony monitor.
;
; Copyright (C) 2012 Peter Schaefer-Hutter ("THE AUTHOR")
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http:;www.gnu.org/licenses/>

; ----------------------------------------------------------------------------
; NOTE: This License does NOT permit incorporating this file (or parts or
;       changed versions of it) into proprietary programs that are
;       distributed to third-parties in binary form only.
; ----------------------------------------------------------------------------

; You can contact the original author by email at peter.schaefer@gmx.de
; if you need different license-terms or other modifications to c'mon.

; $Revision: 45 $ $Date: 2012-10-28 22:50:28 +0100 (Sun, 28 Oct 2012) $

[Setup]
AppId={{75A58CEE-4F84-4F03-BA75-A7D4D6C986F0}
AppName=c'mon
AppVersion=1.0.2
AppPublisher=sheepleap Software
AppPublisherURL=https://sites.google.com/site/cmontelephony/
AppSupportURL=https://sites.google.com/site/cmontelephony/
AppUpdatesURL=https://sites.google.com/site/cmontelephony/
DefaultDirName={pf}\c'mon
DefaultGroupName=c'mon
AllowNoIcons=yes
OutputBaseFilename=cmon_setup
OutputDir=.
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\cmon.exe

[Types]
Name: "standard"; Description: "Standard"; Flags: iscustom

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"

[Components]
Name: "main";             Description: "Main Files";                  Types: standard; Languages: en; Flags: checkablealone 
Name: "main\pb";          Description: "Phonebooks";                                   Languages: en
Name: "main\pb\sqlite";   Description: "Phonebook using SQLite";      Types: standard; Languages: en
Name: "main\pb\postgres"; Description: "Phonebook using PostgreSQL";                   Languages: en
Name: "main\cm";          Description: "Call monitors";                                Languages: en
Name: "main\cm\tapi";     Description: "Call monitor for TAPI";       Types: standard; Languages: en

Name: "main";             Description: "Hauptprogramm";               Types: standard; Languages: de; Flags: checkablealone
Name: "main\pb";          Description: "Telefonbücher";                                Languages: de
Name: "main\pb\sqlite";   Description: "Telefonbuch SQLite";          Types: standard; Languages: de
Name: "main\pb\postgres"; Description: "Telefonbuch PostgreSQL";                       Languages: de
Name: "main\cm";          Description: "Anrufmonitore";                                Languages: de
Name: "main\cm\tapi";     Description: "Anrufmonitor für TAPI";       Types: standard; Languages: de
Name: "main\cm\fritz";    Description: "Anrufmonitor für Fritz!Box";  Types: standard; Languages: de

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
; Core
Source: "..\bin\release\mingw\cmon.exe"; DestDir: "{app}"; Components: main; Flags: ignoreversion
Source: "..\bin\release\mingw\hzshared.dll"; DestDir: "{app}"; Components: main; Flags: ignoreversion
Source: "..\bin\release\mingw\icudt49.dll"; DestDir: "{app}"; Components: main; Flags: ignoreversion
Source: "..\bin\release\mingw\icuin49.dll"; DestDir: "{app}"; Components: main; Flags: ignoreversion
Source: "..\bin\release\mingw\icuuc49.dll"; DestDir: "{app}"; Components: main; Flags: ignoreversion
Source: "..\bin\release\mingw\soci_core.dll"; DestDir: "{app}"; Components: main; Flags: ignoreversion
Source: "..\bin\release\mingw\soci_sqlite.dll"; DestDir: "{app}"; Components: main; Flags: ignoreversion
; SQLite Plugin
Source: "..\bin\release\mingw\plugins\hzabsqlt.dll"; DestDir: "{app}\plugins"; Components: main\pb\sqlite; Flags: ignoreversion
Source: "..\i18n\de\hzabsqlt.mo"; DestDir: "{app}\de"; Components: main\pb\sqlite; Flags: ignoreversion
; Postgres Plugin
Source: "..\bin\release\mingw\plugins\hzabpsql.dll"; DestDir: "{app}\plugins"; Components: main\pb\postgres; Flags: ignoreversion
Source: "..\bin\release\mingw\soci_postgresql.dll"; DestDir: "{app}"; Components: main\pb\postgres; Flags: ignoreversion
Source: "..\i18n\de\hzabpsql.mo"; DestDir: "{app}\de"; Components: main\pb\postgres; Flags: ignoreversion
; TAPI Plugin
Source: "..\bin\release\mingw\plugins\hzcptapi.dll"; DestDir: "{app}\plugins"; Components: main\cm\tapi; Flags: ignoreversion
Source: "..\i18n\de\hzcptapi.mo"; DestDir: "{app}\de"; Components: main\cm\tapi; Flags: ignoreversion
; FRITZ Plugin
Source: "..\bin\release\mingw\plugins\hzcpfrtz.dll"; DestDir: "{app}\plugins"; Components: main\cm\fritz; Flags: ignoreversion
Source: "..\i18n\de\hzcpfrtz.mo"; DestDir: "{app}\de"; Components: main\cm\fritz; Flags: ignoreversion
; MinGW32 Runtime
Source: "distrib\mingw32\*.dll"; DestDir: "{app}"; Components: main; Flags: ignoreversion
; wxWidgets Runtime
Source: "distrib\wxwidgets\*.dll"; DestDir: "{app}"; Components: main; Flags: ignoreversion
; i18n
Source: "..\i18n\de\gui.mo"; DestDir: "{app}\de"; Components: main; Flags: ignoreversion
Source: "..\i18n\de\hzshared.mo"; DestDir: "{app}\de"; Components: main; Flags: ignoreversion
Source: "..\i18n\de\wxstd.mo"; DestDir: "{app}\de"; Components: main; Flags: ignoreversion
; Licenses
Source: "distrib\licenses\*"; DestDir: "{app}\licenses"; Components: main; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\c'mon"; Filename: "{app}\cmon.exe"
Name: "{group}\Visit the c'mon Website"; Filename: "https://sites.google.com/site/cmontelephony/"; Languages: en
Name: "{group}\Follow c'mon on Google+"; Filename: "https://plus.google.com/105093189409072811300/"; Languages: en
Name: "{group}\c'mon im Web"; Filename: "https://sites.google.com/site/cmontelephony/"; Languages: de
Name: "{group}\c'mon auf Google+ folgen"; Filename: "https://plus.google.com/105093189409072811300/"; Languages: de
Name: "{commonstartup}\c'mon"; Filename: "{app}\cmon.exe"
Name: "{commondesktop}\c'mon"; Filename: "{app}\cmon.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\c'mon"; Filename: "{app}\cmon.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\cmon.exe"; Description: "{cm:LaunchProgram,c'mon}"; Flags: nowait postinstall skipifsilent

