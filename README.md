# Cap

Cap is the shell for the snippet codes.
See Wiki: https://github.com/narupo/cap/wiki

# Build

How to build of cap.

## UNIX && Windows

    $ git clone https://github.com/narupo/cap
    $ cd cap
    $ make init && make
    $ cd build
    $ ./cap -h

In Windows, Recommended using TDM-GCC

# Install

## UNIX

Copy binary and library into file sysytem.

    $ cp build/cap /usr/local/bin
    $ cp build/libpad.so /usr/local/lib

## Windows

Copy binary and library into same directory and set PATH.

# How to use

You should be set home.

    $ cap home /my/snippet/directory

Show your snippet codes.

    $ cap cat mysnippet.code

And run shell.

    $ cap sh

