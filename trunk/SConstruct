
# This file is part of c'mon, a lightweight telephony monitor.
#
# Copyright (C) 2012 Peter Schaefer-Hutter ("THE AUTHOR")
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>

# ----------------------------------------------------------------------------
# NOTE: This License does NOT permit incorporating this file (or parts or
#       changed versions of it) into proprietary programs that are
#       distributed to third-parties in binary form only.
# ----------------------------------------------------------------------------

# You can contact the original author by email at peter.schaefer@gmx.de
# if you need different license-terms or other modifications to c'mon.

# $Revision: 46 $ $Date: 2012-11-18 18:55:31 +0100 (Sun, 18 Nov 2012) $

import os

AddOption('--build',
          dest='build_kind',
          nargs=1, type='choice',
          action='store',
          choices=['debug','release'],
          default='debug',
          help='kind of build')

AddOption('--compiler',
          dest='compiler',
          nargs=1, type='choice',
          action='store',
          choices=['vc','mingw'],
          default='vc',
          help='compiler to use')

          
if 'ProgramFiles(x86)' in os.environ:
  PROGSPATH = os.environ.get('ProgramFiles(x86)')
else:
  PROGSPATH = os.environ.get('ProgramFiles')

if GetOption('compiler') == "mingw":

  env = Environment( 
    ENV = { 
          'PATH'  : os.environ.get('PATH', ''),
          'TMP'   : os.environ.get('TMP', ''),
          'TEMP'  : os.environ.get('TEMP', ''),
          'WXWIN' : os.environ.get('WXWIN', ''),
          'BOOST' : os.path.join( 'D:/', 'Code', 'boost_1_51_0_mingw' )
          },
    tools = ['mingw', "gch"]
  )

elif GetOption('compiler') == "vc":

  env = Environment(ENV = os.environ)
  env['ENV']['BOOST'] = os.path.join( 'D:/', 'Code', 'boost_1_51_0_msvc' )
#  env = Environment( 
#    ENV = { 
#          'PATH'  : os.environ.get('PATH', ''),
#          'TMP'   : os.environ.get('TMP', ''),
#          'TEMP'  : os.environ.get('TEMP', ''),
#          'WXWIN' : os.environ.get('WXWIN', ''),
#          'BOOST' : os.environ.get('BOOST', os.path.join( 'C:/', 'Boost' )),
#          'VCINSTALLDIR' : os.environ.get('VCINSTALLDIR', ''),
#          'VSINSTALLDIR' : os.environ.get('VSINSTALLDIR', ''),
#          'VS100COMNTOOLS' : os.environ.get('VS100COMNTOOLS', ''),
#          'WindowsSdkDir' : os.environ.get('WindowsSdkDir', '')
#          },
#    tools = ['msvc', 'mslink', 'mslib']
#  )

env.Append(CPPDEFINES = [ 'WIN32', '_WINDOWS' ] )

if GetOption('build_kind') == "debug":

  WXLIBS = [ 'wxbase29ud', 
             'wxbase29ud_net', 
             'wxmsw29ud_core', 
             'wxmsw29ud_aui', 
             'wxmsw29ud_adv', 
             'wxmsw29ud_ribbon', 
             'wxmsw29ud_richtext',
             'wxpngd',
             'wxtiffd', 
             'wxjpegd', 
             'wxzlibd', 
             'wxregexud', 
             'wxexpatd' ]

  env.Append(CPPDEFINES = [ '_DEBUG' ] )
  
  if GetOption('compiler') == "mingw":
    env.Append(CPPFLAGS = [ '-g', '-m32' ] )
  elif GetOption('compiler') == "vc":
    env.Append(CPPFLAGS = [ '/MDd', '/Od', '/Zi', '/Zm116' ] )
    env.Append(CXXFLAGS = [ '/EHsc', '/GR' ] )
    env.Append(LINKFLAGS = [ '/DEBUG' ] )

elif GetOption('build_kind') == "release":

  WXLIBS = [ 'wxbase29u', 
             'wxbase29u_net', 
             'wxmsw29u_core', 
             'wxmsw29u_aui', 
             'wxmsw29u_adv', 
             'wxmsw29u_ribbon', 
             'wxmsw29u_richtext',
             'wxpng',
             'wxtiff', 
             'wxjpeg', 
             'wxzlib', 
             'wxregexu', 
             'wxexpat' ]

  env.Append(CPPDEFINES = [ 'NDEBUG' ] )

  if GetOption('compiler') == "mingw":
    env.Append(CPPFLAGS = [ '-O2' ] )
    env.Append(LINKFLAGS = [ '-s' ] )
  elif GetOption('compiler') == "vc":
    env.Append(CPPFLAGS = [ '/MD', '/Os', '/Zm116' ] )
    env.Append(CXXFLAGS = [ '/EHsc', '/GR' ] )

env_minimal = env.Clone()

# end of minimal environment (common options) ---------------------------------

proto_base = os.path.join( '#third_party', 'protobuf-2.4.1' )
phone_base = os.path.join( '#third_party', 'libphonenumber' )

env.Append(CPPPATH = [ '#.',
                       os.path.join(env['ENV']['BOOST']),
                       os.path.join('#shared', 'inc'),
                       os.path.join(phone_base, 'cpp', 'src'),
                       os.path.join(proto_base, 'src'),
                       os.path.join('#third_party', 'soci', 'core'),
                       os.path.join('#third_party', 'soci', 'backends', 'sqlite3' ),
                       os.path.join('#third_party', 'soci', 'backends', 'postgresql' ),
                       os.path.join('#third_party', 'sqlite3' ) ] )
env.Append(LIBPATH = [ os.path.join('#bin', GetOption('build_kind'), GetOption('compiler'), 'shared' ),
                       os.path.join('#bin', GetOption('build_kind'), GetOption('compiler'), 'third_party' ),
                       os.path.join(env['ENV']['BOOST'], 'stage', 'lib' ) ] )

env.Append(CPPDEFINES = [ '_UNICODE', 'UNICODE', '__WXMSW__', 'WXUSINGDLL', 'SOCI_DLL' ] )

if GetOption('compiler') == "mingw":
    
    env.Append(CPPDEFINES = [ 'HAVE_W32API_H' ] )
    env.Append(CPPFLAGS = [ '-mthreads', '-pipe', '-fmessage-length=0', '-Wl,-subsystem,windows'] )
    env.Append(CXXFLAGS = [ '-Wno-ctor-dtor-privacy' ] )
    env.Append(LIBPATH = os.path.join(env['ENV']['WXWIN'], 'lib', 'gcc_dll') )
    env.Append(LINKFLAGS =  [ '-mwindows' ] )

    if GetOption('build_kind') == "debug":

      env.Append(CPPPATH = [ os.path.join(env['ENV']['WXWIN'], 'lib', 'gcc_dll', 'mswud'),
                             os.path.join( env['ENV']['WXWIN'], 'include') ] )
      env.Append(CPPDEFINES = [ '__WXDEBUG__' ] )

    elif GetOption('build_kind') == "release":

      env.Append(CPPPATH = [ os.path.join(env['ENV']['WXWIN'], 'lib', 'gcc_dll', 'mswu'),
                             os.path.join( env['ENV']['WXWIN'], 'include') ] )

if GetOption('compiler') == "vc":

    env.Append(LIBPATH = os.path.join(env['ENV']['WXWIN'], 'lib', 'vc_dll') )

    if GetOption('build_kind') == "debug":

      env.Append(CPPPATH = [ os.path.join(env['ENV']['WXWIN'], 'lib', 'vc_dll', 'mswud'),
                             os.path.join( env['ENV']['WXWIN'], 'include') ] )
      env.Append(CPPDEFINES = [ '__WXDEBUG__' ] )

    elif GetOption('build_kind') == "release":

      env.Append(CPPPATH = [ os.path.join(env['ENV']['WXWIN'], 'lib', 'vc_dll', 'mswu'),
                             os.path.join( env['ENV']['WXWIN'], 'include') ] )

# Targets

SConscript('third_party/SConscript', 
  variant_dir=os.path.join('bin', GetOption('build_kind'), GetOption('compiler'), 'third_party'),  
  duplicate = 0,
  exports = { 'env' : env_minimal } )

SConscript('shared/SConscript', 
  variant_dir=os.path.join('bin', GetOption('build_kind'), GetOption('compiler'), 'shared'),
  duplicate = 0,
  exports = { 'env' : env, 'WXLIBS' : WXLIBS } )

SConscript('gui/SConscript', 
  variant_dir=os.path.join('bin', GetOption('build_kind'), GetOption('compiler'), 'gui'),  
  duplicate = 0,
  exports = { 'env' : env, 'WXLIBS' : WXLIBS } )

SConscript('plugin_ab_psql/SConscript', 
  variant_dir=os.path.join('bin', GetOption('build_kind'), GetOption('compiler'), 'plugin_ab_psql'),
  duplicate = 0,
  exports = { 'env' : env, 'WXLIBS' : WXLIBS } )

SConscript('plugin_ab_sqlite/SConscript', 
  variant_dir=os.path.join('bin', GetOption('build_kind'), GetOption('compiler'), 'plugin_ab_sqlite'),
  duplicate = 0,
  exports = { 'env' : env, 'WXLIBS' : WXLIBS } )

SConscript('plugin_cp_tapi/SConscript', 
  variant_dir=os.path.join('bin', GetOption('build_kind'), GetOption('compiler'), 'plugin_cp_tapi'),  
  duplicate = 0,
  exports = { 'env' : env, 'WXLIBS' : WXLIBS } )

SConscript('plugin_cp_fritz/SConscript', 
  variant_dir=os.path.join('bin', GetOption('build_kind'), GetOption('compiler'), 'plugin_cp_fritz'),  
  duplicate = 0,
  exports = { 'env' : env, 'WXLIBS' : WXLIBS } )

SConscript('plugin_cp_dummy/SConscript', 
  variant_dir=os.path.join('bin', GetOption('build_kind'), GetOption('compiler'), 'plugin_cp_dummy'),  
  duplicate = 0,
  exports = { 'env' : env, 'WXLIBS' : WXLIBS } )
