#!/bin/sh

function fail_on_error {
	if [ $? != 0 ]; then
		echo 'COMMAND FAILED...EXITING'
		exit 1
	fi

}

rm -rf Makefile src/Makefile \
	autom4te.cache .deps configure config.status aclocal.m4 \
	install-sh depcomp missing

echo running aclocal
aclocal
fail_on_error

echo running autoconf 
autoconf
fail_on_error

echo running autoheader
autoheader
fail_on_error

echo running automake
automake -a --add-missing
fail_on_error
