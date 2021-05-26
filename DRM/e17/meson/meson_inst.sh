#!/bin/sh

for x in "$@" ; do
	chown root "$DESTDIR/$x"
	chmod a=rx,u+xs "$DESTDIR/$x"
done
