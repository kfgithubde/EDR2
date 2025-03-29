echo off
if /I "%1%" neq "" goto ParExists
 echo Zielsystem nicht angegeben:
 echo  WX - wX durch CB und wxSmith
 goto done

:ParExists
echo Betriebssystemunabhaengige Komponenten
copy ..\Source\*.* *.*

if /I "%1%" neq "WX" goto NoWX
 echo Copying Files for MINGW GNU-Compiler, CB and wxSmith
 copy ..\Source\\*.* *.*
 copy ..\libs\GNU\*.* *.*
 xcopy /iey ..\Makefiles\GNU\ . 
 goto done

:NoWX
 echo Unbekanntes Zielsystem

:done
 echo Done.
