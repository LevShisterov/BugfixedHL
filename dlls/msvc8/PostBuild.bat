@echo OFF
::
:: Post-build auto-deploy script
:: Create and fill PublishPath.txt file with path to deployment folder
:: I.e. PublishPath.txt should contain one line with a folder path
::

SET targetfile=%~1
SET projectdir=%~2
SET destination=

IF NOT EXIST "%projectdir%\PublishPath.txt" (
	ECHO 	No deployment path specified.
	exit /B 0
)

SET /p destination= <"%projectdir%\PublishPath.txt"
IF "%destination%" == "" (
	ECHO 	No deployment path specified.
	exit /B 0
)

copy /Y "%targetfile%" "%destination%"
