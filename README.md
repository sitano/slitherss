TODO
----

- fix code style
- make performance tests
- fix eating food
- optimize snake intersection
- snake observability tracking

OTHER
-----

- inject googletest
- inject benchmark framework
- fixed point math test
- intersections math test

How to build
------------

### Ubuntu

#### Prerequisites

In case automatic script didn't work:

* Boost:

  `sudo apt-get install libboost-dev libboost-system-dev libboost-thread-dev libboost-program-options-dev`

* Zlib:

  `sudo apt-get install zlib1g-dev`

* Cmake:

  `sudo apt-get install cmake`

* Clang:

  `sudo apt-get install clang`, its better to use latest 3.8, 3.9.

* Valgrind:

  `sudo apt-get install valgrind`

#### Building

1. `cmake .`
1. `make -j`

Code style
----------

Use latest [Google C++ Code Style guide](https://google.github.io/styleguide/cppguide.html).
