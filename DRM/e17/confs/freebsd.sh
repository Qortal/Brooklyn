#!/bin/sh -e
meson -Dsystemd=false -Delput=false -Ddevice-udev=false -Dgesture-recognition=false \
 $@ . build
