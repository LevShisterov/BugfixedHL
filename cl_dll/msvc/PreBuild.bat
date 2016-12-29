@ECHO OFF
::
:: Pre-build auto-versioning script
::

SET srcdir=%~1
SET repodir=%~2

SET old_version=""
SET new_version="0.0.0"
SET new_specialbuild=""
SET git_version="v0.0-0"
SET git_date="?"

::
:: Check for git.exe presence
::
git.exe 2>NUL >NUL
SET errlvl=%ERRORLEVEL%

::
:: Read old appversion.h, if present
::
IF EXIST "%srcdir%\appversion.h" (
	FOR /F "usebackq tokens=1,2,3" %%i IN ("%srcdir%\appversion.h") DO (
		IF %%i==#define (
			IF %%j==APP_VERSION SET old_version=%%k
		)
	)
)
SET old_version=%old_version:~1,-1%

::
:: Bail out if git.exe not found
::
IF NOT "%errlvl%" == "1" (
	ECHO Can not locate git.exe. Auto-versioning step will not be performed.

	:: If we doesn't have appversion.h, we need to create it
	IF "%old_version%" == "" GOTO COMPARE
	EXIT /B 0
)

::
:: Generate temp file name
::
:GETTEMPNAME
:: Use current path, current time and random number to create unique file name
SET TMPFILE=git-%CD:~-15%-%RANDOM%-%TIME:~-5%-%RANDOM%
:: Remove bad characters
SET TMPFILE=%TMPFILE:\=%
SET TMPFILE=%TMPFILE:.=%
SET TMPFILE=%TMPFILE:,=%
SET TMPFILE=%TMPFILE: =%
:: Will put in a temporary directory
SET TMPFILE=%TMP%.\%TMPFILE%
IF EXIST "%TMPFILE%" GOTO :GETTEMPNAME

::
:: Get information from GIT repository
::
SET errlvl=0
git.exe -C "%repodir%." describe --long --tags --dirty --always > "%TMPFILE%.tmp1"
IF NOT "%ERRORLEVEL%" == "0" THEN SET errlvl=1
git.exe -C "%repodir%." log -1 --format^=%%ci >> "%TMPFILE%.tmp2"
IF NOT "%ERRORLEVEL%" == "0" THEN SET errlvl=1
IF NOT "%errlvl%" == "0" (
	ECHO git.exe done with errors [%ERRORLEVEL%].
	ECHO Check if you have correct GIT repository at '%repodir%'.
	ECHO Auto-versioning step will not be performed.

	DEL /F /Q "%TMPFILE%.tmp1" 2>NUL
	DEL /F /Q "%TMPFILE%.tmp2" 2>NUL

	:: If we doesn't have appversion.h, we need to create it
	IF "%old_version%" == "" GOTO COMPARE
	EXIT /B 0
)

::
:: Read version and commit date from temp files
::
SET /P git_version=<"%TMPFILE%.tmp1"
SET /P git_date=<"%TMPFILE%.tmp2"

DEL /F /Q "%TMPFILE%.tmp1" 2>NUL
DEL /F /Q "%TMPFILE%.tmp2" 2>NUL

::
:: Detect local modifications
::
SET new_version=%git_version:-dirty=+m%
IF NOT x%new_version%==x%git_version% (
	SET new_specialbuild=modified
) ELSE (
	SET new_specialbuild=
)

::
:: Process version string
::
:: Remove "v" at start, change "-g" hash prefix into "+", "-" before patch into "."
SET new_version=%new_version:~1%
SET new_version=%new_version:-g=+%
SET new_version=%new_version:-=.%

::
:: Check if version has changed
::
:COMPARE
IF NOT "%new_version%"=="%old_version%" GOTO UPDATE
EXIT /B 0

::
:: Update appversion.h
::
:UPDATE
FOR /F "tokens=1,2,3 delims=.+" %%a IN ("%new_version%") DO SET major=%%a&SET minor=%%b&SET patch=%%c

ECHO Updating appversion.h, old version "%old_version%", new version "%new_version%".

ECHO #ifndef __APPVERSION_H__>"%srcdir%\appversion.h"
ECHO #define __APPVERSION_H__>>"%srcdir%\appversion.h"
ECHO.>>"%srcdir%\appversion.h"
ECHO // >>"%srcdir%\appversion.h"
ECHO // This file is generated automatically.>>"%srcdir%\appversion.h"
ECHO // Don't edit it.>>"%srcdir%\appversion.h"
ECHO // >>"%srcdir%\appversion.h"
ECHO.>>"%srcdir%\appversion.h"
ECHO // Version defines>>"%srcdir%\appversion.h"

ECHO #define APP_VERSION "%new_version%">>"%srcdir%\appversion.h"
ECHO #define APP_VERSION_C %major%,%minor%,%patch%,^0>>"%srcdir%\appversion.h"

ECHO.>>"%srcdir%\appversion.h"
ECHO #define APP_VERSION_DATE "%git_date%">>"%srcdir%\appversion.h"

ECHO.>>"%srcdir%\appversion.h"
IF NOT "%new_specialbuild%" == "" (
	ECHO #define APP_VERSION_FLAGS VS_FF_SPECIALBUILD>>"%srcdir%\appversion.h"
	ECHO #define APP_VERSION_SPECIALBUILD "%new_specialbuild%">>"%srcdir%\appversion.h"
) ELSE (
	ECHO #define APP_VERSION_FLAGS 0x0L>>"%srcdir%\appversion.h"
)

ECHO.>>"%srcdir%\appversion.h"
ECHO #endif //__APPVERSION_H__>>"%srcdir%\appversion.h"

::
:: Update last modify time on files that use appversion.h header to force them to recompile
::
COPY /B "%srcdir%\msvc\client.rc"+,, "%srcdir%\msvc\client.rc" >NUL
COPY /B "%srcdir%\hud.cpp"+,, "%srcdir%\hud.cpp" >NUL

EXIT /B 0
