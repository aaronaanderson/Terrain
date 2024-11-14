#!/bin/bash

# check if we're in the correct folder
result=${PWD##*/}
if [ $result == "deployment" ]; then
  cd ..
fi

# delete the deployment folder
rm -rf deployment/Linux

# make a new deployment folder
mkdir -p "deployment/Linux/"

# catch if you forgot to enter a version number
if [ $# -eq 0 ]; then
  echo "Error: No version number provided. Version number required in format <MajorVersion>.<MinorVersion>"
  exit 1
fi

# grab version argument, put it in the package name
VERSION="$1""-1"
RELEASENAME="Terrain-""$VERSION""-amd64"

BUILDLOCATION=$(cd deployment && pwd)

echo "Packaging $RELEASENAME..."

# make directory structure

BUILDDIR="deployment/Linux/$RELEASENAME"

mkdir -p "$BUILDDIR/DEBIAN"
# make the VST3 Global directory
mkdir -p "$BUILDDIR/usr/lib/vst3"
mkdir -p "$BUILDDIR/usr/lib/clap"
# The following was commented out by Aaron
# mkdir -p "$BUILDDIR/usr/bin"
# mkdir -p "$BUILDDIR/usr/share/applications/"
# mkdir -p "$BUILDDIR/usr/share/doc/CHON/"
mkdir -p "$BUILDDIR/usr/share/doc/Terrain/"
# mkdir -p "$BUILDDIR/usr/share/pixmaps/"

# copy necessary files over


cd "$Dir"
# objcopy --strip-debug --strip-unneeded bin/CHON "$BUILDDIR/usr/bin/CHON"
# objcopy --strip-debug --strip-unneeded bin/Terrain.vst3 "$BUILDDIR/usr/lib/vst3"
cp -r bin/Terrain.vst3 "$BUILDDIR/usr/lib/vst3"
cp -r bin/Terrain.clap "$BUILDDIR/usr/lib/clap"

# the following line is for app icons
# cp "deployment/icons/CHON.png" "$BUILDDIR/usr/share/pixmaps/CHON.png"

# This is the file that makes an application searchable
# Make .desktop file
# echo "[Desktop Entry]
# Name=CHON
# Comment=Launch CHON
# Exec=CHON
# Icon=/usr/share/pixmaps/CHON.png
# Terminal=false
# Type=Application
# Categories=Audio;Music;Science;
# Name[en_US]=CHON" >>"$BUILDDIR/usr/share/applications/CHON.desktop"

# make Debian control file
echo "Package: Terrain
Architecture: amd64
Section: sound
Priority: optional
Version:$VERSION
Maintainer:Aaron Anderson <aaron.arthur.anderson@gmail.com>
Depends:
Homepage: https://github.com/aaronaanderson/Terrain
Description: This package installs Terrain, an instrument plugin implementing Wave Terrain Synthesis." >>"$BUILDDIR/DEBIAN/control"

# make copyright file
echo "Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: Terrain
Source: https://github.com/aaronaanderson/Terrain

Files: *
Copyright: 2024 Aaron Anderson <aaron.arthur.anderson@gmail.com>
License: GPL-3+
 This program is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later
 version.
 .
 This program is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied
 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the GNU General Public License for more
 details.
 .
 You should have received a copy of the GNU General Public
 License along with this package; if not, write to the Free
 Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301 USA
 .
 On Debian systems, the full text of the GNU General Public
 License version 3 can be found in the file
 '/usr/share/common-licenses/GPL-3'" >>"$BUILDDIR/usr/share/doc/Terrain/copyright"


# Aaron commented this block out - it's for a change log and doesn't actually work right
# DATE="$(date +'%a, %d %b %Y %H:%M:%S %Z')"
# echo "CHON ($VERSION) stable; urgency=high
#   * Many bugfixes and feature additions
#  -- Rodney DuPlessis <rodney@rodneyduplessis.com>  $DATE
#   * Initial Release
#  -- Rodney DuPlessis <rodney@rodneyduplessis.com>  $DATE" >>"$BUILDDIR/usr/share/doc/CHON/changelog.Debian"
# gzip -9 -n "$BUILDDIR/usr/share/doc/CHON/changelog.Debian"
# echo "Packaging .deb at $BUILDLOCATION..."


# package .deb
cd deployment/Linux
fakeroot dpkg -b "$RELEASENAME" "$RELEASENAME.deb"

echo "Packaging Complete!"