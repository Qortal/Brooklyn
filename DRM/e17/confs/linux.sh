#!/bin/sh -e
meson -Dwl=true \
 $@ . build
