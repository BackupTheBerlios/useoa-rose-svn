#!/bin/sh -x
#autoreconf --install -Iconfig
/bin/rm -rf autom4te.cache
aclocal  --install --force -Iconfig 
echo "checking for GNU libtool..."
OS="$(uname -s)"
libtoolize=libtoolize
if [ "x$OS" = "xDarwin" ]; then
    if [ -n "$(type glibtoolize)" ]; then
        echo "Found glibtoolize... $(which glibtoolize)"
        libtoolize=$(which glibtoolize)
    elif [ -n "$(type glibtoolize)" ]; then
        echo "checking if $(which libtoolize) is GNU libtoolize..."
        libtoolize --version
        if [ $? -ne 0 ]; then
            echo "Error: $(which libtoolize) is not GNU libtoolize!"
            exit 1
        else
            echo "Found glibtoolize... $(which libtoolize)"
            libtoolize=$(which libtoolize)
        fi
    fi
fi
$libtoolize --force --copy --ltdl --automake
autoconf
autoheader
automake --add-missing

