#!/bin/sh -e
# make sure you run it from toplevel source dir
if test -d ./po && test -f ./po/POTFILES-UPDATE.sh; then
  find src/bin src/modules -name '*.[ch]' | sort > ./po/POTFILES.in
else
  echo "Please run this from the toplevel source directory."
  exit 77
fi
