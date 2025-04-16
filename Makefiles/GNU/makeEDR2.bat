REM Adapt wxBase and GCCbase to your individual wX Installation
set wxBase="C:\ProgWin32\wx3-1-5Us64bit"
set GCCbinBase="C:\ProgWin32\TDM-GCC-64\bin"
set GCClibBase="C:\ProgWin32\TDM-GCC-64\lib\gcc\x86_64-w64-mingw32\10.3.0"
set FlexBase="C:\ProgWin32\msys64\usr\bin"

mkdir obj

%GCCbinBase%\gcc.exe -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -DwxUSE_UNICODE -O2 -m64 -I%wxBase%\include -I%wxBase%\lib\gcc_lib\mswu -c ButterworthTime.for -o obj\ButterworthTime.o
%GCCbinBase%\gcc.exe -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -DwxUSE_UNICODE -O2 -m64 -I%wxBase%\include -I%wxBase%\lib\gcc_lib\mswu -c EDRplot.f08 -o obj\EDRplot.o
%GCCbinBase%\gcc.exe -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -DwxUSE_UNICODE -O2 -m64 -I%wxBase%\include -I%wxBase%\lib\gcc_lib\mswu -c EDR2App.cpp -o obj\EDR2App.o
%GCCbinBase%\gcc.exe -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -DwxUSE_UNICODE -O2 -m64 -I%wxBase%\include -I%wxBase%\lib\gcc_lib\mswu -c EDR2Main.cpp -o obj\EDR2Main.o
%FlexBase%\flex -oReadCrashData.scanner.cc ReadCrashData.l
%GCCbinBase%\gcc.exe -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -DwxUSE_UNICODE -O2 -m64 -I%wxBase%\include -I%wxBase%\mswu -c ReadCrashData.scanner.cc -o obj\ReadCrashData.scanner.o
%GCCbinBase%\windres.exe -I%wxBase%\include -I%wxBase%\lib\gcc_lib\mswu  -J rc -O coff -i resource.rc -o obj\resource.res
%GCCbinBase%\g++.exe -L%wxBase%\lib\gcc_lib -L%GCClibBase% -o .\EDR2.exe obj\ButterworthTime.o obj\EDRplot.o obj\EDR2App.o obj\EDR2Main.o obj\ReadCrashData.scanner.o obj\resource.res -s -mthreads -m64  libgraph2d.a -lwxmsw31u -lwxpng -lwxjpeg -lwxtiff -lwxzlib -lwxexpat -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32 -lwsock32 -lodbc32 -lshlwapi -lversion -loleacc -luxtheme -lgfortran -mwindows
