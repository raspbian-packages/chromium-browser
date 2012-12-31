#!/bin/sh
#
# Copyright (C) 2006-2007 Linus Walleij <triad@df.lth.se>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

#set -e

srcdir=`dirname $0`

# Get sources from gphoto2 SVN
WGET=`which wget`
if [ "x$WGET" != "x" ]; then
    wget -O tmpfile http://gphoto.svn.sourceforge.net/viewvc/*checkout*/gphoto/trunk/libgphoto2/camlibs/ptp2/ptp.c
    mv tmpfile ptp.c.gphoto2
    wget -O tmpfile http://gphoto.svn.sourceforge.net/viewvc/*checkout*/gphoto/trunk/libgphoto2/camlibs/ptp2/ptp.h
    mv tmpfile ptp.h.gphoto2
    wget -O tmpfile http://gphoto.svn.sourceforge.net/viewvc/*checkout*/gphoto/trunk/libgphoto2/camlibs/ptp2/ptp-pack.c
    mv tmpfile ptp-pack.c.gphoto2
    wget -O tmpfile http://gphoto.svn.sourceforge.net/viewvc/*checkout*/gphoto/trunk/libgphoto2/camlibs/ptp2/library.c
    mv tmpfile library.c.gphoto2
    wget -O tmpfile http://gphoto.svn.sourceforge.net/viewvc/*checkout*/gphoto/trunk/libgphoto2/camlibs/ptp2/music-players.h
    mv tmpfile music-players.h.gphoto2
else
    echo "Could not sync to gphoto2. No WGET."
fi

echo "Finished!"

