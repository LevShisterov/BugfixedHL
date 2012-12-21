@echo OFF
::
:: Post-build auto-deploy script
:: Create and fill PublishPath.txt file with path to deployment folder
:: I.e. PublishPath.txt should contain one line with a folder path
::

SET targetDir=%~1
SET targetName=%~2
SET targetExt=%~3
SET projectDir=%~4
SET destination=

IF NOT EXIST "%projectDir%\PublishPath.txt" (
	ECHO 	No deployment path specified. Create PublishPath.txt near PostBuild.bat with paths on separate lines for auto deployment.
	exit /B 0
)

FOR /f "tokens=* delims= usebackq" %%a IN ("%projectDir%\PublishPath.txt") DO (
	IF NOT "%%a" == "" (
		copy /Y "%targetDir%%targetName%%targetExt%" "%%a"
		IF EXIST "%targetDir%%targetName%.pdb" (
			copy /Y "%targetDir%%targetName%.pdb" "%%a"
		)
	)
)

IF "%destination%" == "" (
	ECHO 	No deployment path specified.
	exit /B 0
)
