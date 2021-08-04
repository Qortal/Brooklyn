#!/bin/sh

for x in "$@" ; do
	chown nobody "$DESTDIR/$x"
	chmod a=rx,u+xs "$DESTDIR/$x"
done
