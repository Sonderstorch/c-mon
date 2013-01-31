@ECHO OFF
REM
REM Compile STEP
REM
for %%f in (i18n\de\*.po) do tools\gnuwin32\bin\msgfmt -o i18n\de\%%~nf.mo i18n\de\%%~nf.po
pause