@ECHO OFF
REM
REM INIT STEP
REM
if exist i18n\de\gui.po (
  tools\gnuwin32\bin\xgettext -j --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\gui.po gui\*.cpp gui\journal\*.cpp gui\resolver\*.cpp gui\db\*.cpp
) else (
  tools\gnuwin32\bin\xgettext --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\gui.po gui\*.cpp gui\journal\*.cpp gui\resolver\*.cpp gui\db\*.cpp
)
if exist i18n\de\hzcptapi.po (
  tools\gnuwin32\bin\xgettext -j --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzcptapi.po plugin_cp_tapi\*.cpp
) else (
  tools\gnuwin32\bin\xgettext --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzcptapi.po plugin_cp_tapi\*.cpp
)
if exist i18n\de\hzcpfrtz.po (
  tools\gnuwin32\bin\xgettext -j --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzcpfrtz.po plugin_cp_fritz\*.cpp
) else (
  tools\gnuwin32\bin\xgettext --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzcpfrtz.po plugin_cp_fritz\*.cpp
)
if exist i18n\de\hzabpsql.po (
  tools\gnuwin32\bin\xgettext -j --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzabpsql.po plugin_ab_psql\*.cpp
) else (
  tools\gnuwin32\bin\xgettext --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzabpsql.po plugin_ab_psql\*.cpp
)
if exist i18n\de\hzabsqlt.po (
  tools\gnuwin32\bin\xgettext -j --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzabsqlt.po plugin_ab_sqlite\*.cpp
) else (
  tools\gnuwin32\bin\xgettext --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzabsqlt.po plugin_ab_sqlite\*.cpp
)
if exist i18n\de\hzshared.po (
  tools\gnuwin32\bin\xgettext -j --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzshared.po shared\src\*.cpp
) else (
  tools\gnuwin32\bin\xgettext --sort-output --foreign-user --omit-header --no-location --from-code=UTF-8 -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o i18n\de\hzshared.po shared\src\*.cpp
)
