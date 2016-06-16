# c'mon

Note 2016-06-16: Project has been moved from https://sourceforge.net/projects/cmon/

***

c'mon strives to be a simple and lean phone call monitor (*"no-frills"*) which is expandable by a plugin-architecture. It is primarily designed for the Windows Telephony API (TAPI) but also supports the ubiquitous AVM Fritz!Box routers that come with PBX features. 

However, Fritz!Box support is currently limited to displaying incoming calls, only.

c'mon supports to back-ends for phonebook and journal data:

* File-based on local disk (SQLite)
* Network-based on a (possibly remote) database server (PostgreSQL)

## Release History

### 1.0.2

* Added file-based phonebook provider using SQLite (no PostgeSQL database server instance necessary anymore). 
* Some CSV-import related fixes.

### 1.0.1

* Small packaging fix (missing DLLs), minor translation updates.

### 1.0.0

* The first generally available version of the lean telephony monitor for TAPI and Fritz!Box Phone.
