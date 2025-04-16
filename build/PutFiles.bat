echo off
if /I "%1%" neq "" goto ParExists
 echo Zielsystem nicht angegeben:
 echo  wX - wX durch CB und wxSmith
 goto done

:ParExists

if /I "%1%" neq "WX" goto NoWX
 echo Copying Files for 64bit wX for Windows
 copy edr2.exe ..\bin\edr2.exe
 copy EDR2config.xml ..\bin\EDR2config.xml
 copy EDR2config_FullTemplate.xml ..\bin\EDR2config_FullTemplate.xml
 copy EDR2UserInit-de.xml ..\bin\EDR2UserInit-de.xml
 copy EDR2-de.mo ..\bin\de\EDR2.mo
 copy EDR2UserInit-de.xml ..\bin\EDR2UserInit-fr.xml
 copy EDR2-fr.mo ..\bin\fr\EDR2.mo
 goto done

:NoWX
 echo Unbekanntes Zielsystem

:done
 echo Done.


