#!/bin/sh
set -e

# Make built-from-source sessions appear in login screen
sudo install @CMAKE_CURRENT_BINARY_DIR@/plasmax11-dev.desktop /usr/share/xsessions/
sudo install @CMAKE_CURRENT_BINARY_DIR@/plasmawayland-dev.desktop /usr/share/wayland-sessions/
install @CMAKE_BINARY_DIR@/prefix.sh @CMAKE_INSTALL_FULL_LIBEXECDIR@/plasma-dev-prefix.sh
install @CMAKE_CURRENT_BINARY_DIR@/startplasma-dev.sh @CMAKE_INSTALL_FULL_LIBEXECDIR@

# Make the system DBus able to see any new DBus files that have been added to
# the built-from-source plasma session which are not yet present in the system
# DBus locations. Because some distros have security policies which prevent the
# use of DBus files in a user's homedir, and even symlinks outside,
# we have to copy the files into a system-owned location.
sudo mkdir -p /opt/kde-dbus-scripts/
sudo cp -r @KDE_INSTALL_FULL_DBUSDIR@/* /opt/kde-dbus-scripts/
if [ ! -f /etc/dbus-1/session-local.conf ]
then
    cat > session-local.conf << EOF
<busconfig>
	<servicedir>/opt/kde-dbus-scripts/services</servicedir>
	<servicedir>/opt/kde-dbus-scripts/system-services</servicedir>
	<includedir>/opt/kde-dbus-scripts/system.d/</includedir>
	<includedir>/opt/kde-dbus-scripts/interfaces/</includedir>
</busconfig>
EOF
    sudo mv session-local.conf /etc/dbus-1/
fi
