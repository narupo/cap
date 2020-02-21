# Cap

Cap is shell for programmer's snippet codes.

# Build

How to build of cap.

## UNIX

    $ git clone https://github.com/narupo/cap
    $ cd cap
    $ make init && make
    $ ./build/cap -h


## Windows

Using MinGW's make (recommend TDM-GCC).

    $ git clone https://github.com/narupo/cap
    $ cd cap
    $ mingw32-make init && mingw32-make
    $ ./build/cap -h   

# How to use

You should be set home.

    $ cap home /my/snippet/directory

Show your snippet codes.

    $ cap cat mysnippet.code

And run shell.

    $ cap sh
