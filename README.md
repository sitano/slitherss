Readme
------

slither.io server in C++ emulating
[native protocol](https://github.com/sitano/Slither.io-Protocol).

_Note: this was mostly an experiment and fun reversing slither.io game. Thus, some
code or parts of it may not met real world high quality standards, and that
was not a goal of this work._

_Note: why c++? I just wanted to play with a language I've touched 10 years ago
last time._

_Note: there are a lot of strange decisions inside of slither.io, like i.e. 8ms
simulation tick - I will ignore that._

![anim](https://cloud.githubusercontent.com/assets/564610/16897363/2b0b032a-4baf-11e6-9bf5-5668d1518b6d.gif)

How to build
------------

This software was developed with _Ubuntu_ and can be easily build under any
_unix_ native environment. It is possible to run it on Windows as well, because
main websockets library is cross-platform, but environment setup under Windows
is out of the scope of this article.

#### Prerequisites

* Boost: `sudo apt-get install libboost-dev libboost-system-dev libboost-thread-dev libboost-program-options-dev`
* Zlib: `sudo apt-get install zlib1g-dev`
* CMake env: `sudo apt-get install cmake gcc g++ clang clang-3.5 cppcheck valgrind git`, its better to use latest 3.8, 3.9.

#### Building

1. `git clone https://github.com/sitano/slitherss`
1. `git submodule update --init`
1. `cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=Debug .`
1. `make -j`

_Note: Make sure boost lib was built using the same compiler or at least
compatible ABI._

How to play
-----------

1. Run the server using something like `./bin/slither_server --bots 50 --avg_len 50`.
   Built-in bots are supported and you can choose average snakes length on the start.
1. Force your client to connect to opened server port (127.0.0.1:8080) using
   either [debug tool, pressing 'c'](https://github.com/sitano/Slither.io-Protocol/blob/master/slither_debug.js),
   or inject force command right into the console of running web site in a browser:
   `window.bso = { ip: "127.0.0.1", po: 8080 }; window.forcing = true; window.want_play = true;`.

Valgrind
--------

CPU

    valgrind --tool=callgrind ./bin/slither_server --bots=500 --min_len=50

Memory

    valgrind --track-origins=yes -v ./bin/slither_server

VGDB

    $ valgrind --vgdb=yes --vgdb-error=0 ./bin/slither_server --bots 5000
    gdb: target remote | vgdb

SystemTap
---------

    stap -v -d ./bin/slither_server -L 'process("*/slither_server").function("*")'

Vagrant
-------

There is vagrant configuration with all dependencies required for development.

    vagrant up
    vagrant ssh

Code style
----------

Use latest [Google C++ Code Style guide](https://google.github.io/styleguide/cppguide.html)
with lines length 120.

TODO
----

- force died snake disconnect 
- fix eating food
- optimize snake intersection
- snake observability tracking
- make performance tests

OTHER
-----

- inject googletest / boost testing
- inject benchmark framework (
    https://github.com/google/benchmark,
    https://github.com/DigitalInBlue/Celero,
    https://github.com/nickbruun/hayai)
- fixed point math test
- intersections math test

