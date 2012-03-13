#!/bin/sh

	# Get old version information from appversion.h
	if [ -e ./appversion.h ]; then
		oldver=$(cat ./appversion.h | grep -i '#define APP_VERSION_STRD' | sed -e 's/#define APP_VERSION_STRD \(.*\)/\1/i')
		if [ $? -ne 0 ]; then
			oldver=""
		fi
		oldmod=$(cat ./appversion.h | grep -i '#define APP_VERSION_SPECIALBUILD' | sed -e 's/#define APP_VERSION_SPECIALBUILD \(.*\)/\1/i')
		if [ $? -ne 0 ]; then
			oldmod=""
		fi
	fi

	# Get major, minor and maitenance information from version.h
	major=$(cat ./version.h | grep -i '#define VERSION_MAJOR' | sed -e 's/#define VERSION_MAJOR \(.*\)/\1/i')
	if [ $? -ne 0 -o "$major" = "" ]; then
		major=0
	fi
	minor=$(cat ./version.h | grep -i '#define VERSION_MINOR' | sed -e 's/#define VERSION_MINOR \(.*\)/\1/i')
	if [ $? -ne 0 -o "$minor" = "" ]; then
		minor=0
	fi
	maintenance=$(cat ./version.h | grep -i '#define VERSION_MAITENANCE' | sed -e 's/#define VERSION_MAITENANCE \(.*\)/\1/i')
	if [ $? -ne 0 -o "$maintenance" = "" ]; then
		maintenance=
	fi

	# Get revision and modification status from local SVN copy
	revision=$(svnversion -n ../ | sed -e 's/\([0-9]\+:\)\?\([0-9]\+\).*/\2/')
	if [ $? -ne 0 -o "$revision" = "" ]; then
		revision=0
	fi
	modifications=$(svnversion -n ../ | sed -e 's/\([0-9]\+:\)\?[0-9]\+.*\(M\).*/\2/')
	if [ $? -eq 0 -a "$modifications" = "M" ]; then
		modifications='"'modified'"'
	else
		modifications=""
	fi

	# Construct version string
	if [ "$maitenance" = "" ]; then
		version='"'$major.$minor.$revision'"'
	else
		version='"'$major.$minor.$maitenance.$revision'"'
	fi

	if [ "$version" != "$oldver" -o "$modifications" != "$oldmod" ]; then
		echo Old version is $oldver $oldmod and new one is $version $modifications
		echo Going to update appversion.h

		echo '#ifndef __APPVERSION_H__' > appversion.h
		echo '#define __APPVERSION_H__' >> appversion.h
		echo '' >> appversion.h

		echo -n '#define APP_VERSION_STRD ' >> appversion.h
		echo $version >> appversion.h

		if [ "$modifications" != "" ]; then
			echo -n '#define APP_VERSION_SPECIALBUILD ' >> appversion.h
			echo $modifications >> appversion.h
		fi
		echo '' >> appversion.h

		echo '#endif //__APPVERSION_H__' >> appversion.h
	fi
