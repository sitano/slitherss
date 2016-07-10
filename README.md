Readme
------

slither.io server in C++ emulating
[native protocol](https://github.com/ClitherProject/Slither.io-Protocol).

_Note: this was mostly experiment and fun reversing slither.io game. Thus some
code or any parts of it may not met real world high quality production standards,
and that was not a goal of this work._

_Note: why c++? I just wanted to play with a language I touched 10 years ago
last time._

_Note: there are a lot of strange decisions inside of slither.io, like i.e. 8ms
simulation tick - I will ignore that._

How to build
------------

This software was developed with _Ubuntu_ and can be easily build under any
_unix_ native environment. It is possible to run it on Windows as well, because
main websockets library is cross-platform, but environment setup under Windows
is out of the scope of this article.

#### Prerequisites

* Boost: `sudo apt-get install libboost-dev libboost-system-dev libboost-thread-dev libboost-program-options-dev`
* Zlib: `sudo apt-get install zlib1g-dev`
* Cmake: `sudo apt-get install cmake`
* Clang: `sudo apt-get install clang`, its better to use latest 3.8, 3.9.
* Valgrind: `sudo apt-get install valgrind`

#### Building

1. `git clone https://github.com/sitano/slitherss`
1. `git submodule update --init`
1. `cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++`
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

SystemTap
---------

    stap -v -d ./bin/slither_server -L 'process("*/slither_server").function("*")'

Code style
----------

Use latest [Google C++ Code Style guide](https://google.github.io/styleguide/cppguide.html)
with lines length 120.

TODO
----

- fix code style
- make performance tests
- fix eating food
- optimize snake intersection
- snake observability tracking

OTHER
-----

- inject googletest / boost testing
- inject benchmark framework (
    https://github.com/google/benchmark,
    https://github.com/DigitalInBlue/Celero,
    https://github.com/nickbruun/hayai)
- fixed point math test
- intersections math test

