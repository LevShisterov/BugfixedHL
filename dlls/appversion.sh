#!/bin/sh

	# Get old version information from appversion.h
	if [ -e ./appversion.h ]; then
		old_version=$(cat ./appversion.h | grep -i '#define APP_VERSION' | sed -e 's/#define APP_VERSION \(.*\)/\1/i')
		if [ $? -ne 0 ]; then
			old_version=""
		fi
	fi

	# Get information from GIT repository
	git_version=$(git.exe describe --long --tags --dirty --always)
	git_date=$(git.exe log -1 --format=%ci)

	# Process version
	new_version=$(git_version#v)
	new_version=$(git_version/-dirty/+m)
	new_version=$(git_version/-g/+)
	new_version=$(git_version/-/.)

	if [ "$new_version" != "$old_version" ]; then
		echo Updating appversion.h, old version "$old_version", new version "$new_version".

		echo '#ifndef __APPVERSION_H__' > appversion.h
		echo '#define __APPVERSION_H__' >> appversion.h
		echo '' >> appversion.h

		echo -n '#define APP_VERSION ' >> appversion.h
		echo $new_version >> appversion.h

		echo -n '#define APP_VERSION_DATE ' >> appversion.h
		echo $git_date >> appversion.h

		echo '' >> appversion.h
		echo '#endif //__APPVERSION_H__' >> appversion.h
	fi
