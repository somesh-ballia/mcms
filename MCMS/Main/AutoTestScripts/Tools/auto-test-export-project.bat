rem auto-test-export-project.bat
rem backup build forge project to xml file from build forge server
rem works in windows only

echo on

rem Four digit year
set YYYY=%date:~10,4%

rem Two digit month
set MM=%date:~4,2%

rem Two digit day of month
set DD=%date:~7,2%

rem Two digit hour
set HH=%time:~0,2%

rem Two digit minutes
set MN=%time:~3,2%

rem Two digit seconds
set SS=%time:~6,2%

rem Fixing the hour...
rem The hour has no leading digit padding so you need to check
rem if leading char is null. If it is - prepend a zero to 2nd
rem digit of hour

rem Grab the first digit of the time and enclose it in #hashes#
rem This is the only way to check for a blank...
set H1=#%time:~0,1%#

rem Grab the second digit of the time...
set H2=%time:~1,1%

rem If the H1 var has a space between the hashes then you know it's 
rem a single digit time (ie, 1-9) so prepend a zero to the hour
if "%H1%"=="# #" set HH=0%H2%

set FNAME=%YYYY%%MM%%DD%%HH%%MN%%SS%-auto-test-project.xml

cd "C:\Program Files\IBM\Build Forge\"
c:

bfexport.exe -f c:\app\auto-test\backup\%fname% -g -s -C auto-test
copy c:\app\auto-test\backup\%fname% \\accord-fs\ClearCase\BuildForge\auto-test-backup\
copy c:\app\auto-test\backup\%fname% \\f3-darabkin-dt\dev\auto-test\backup
