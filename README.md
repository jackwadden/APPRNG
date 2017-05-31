# APPRNG

## Installation

TestU01 must first be compiled into a dynamically linkable library before the AP PRNG simulator can be run against the tests. Follow the steps below to compile TestU01.

`cd stats_tests/TestU01-1.2.3`
`mkdir build`
`./configure --prefix=<absolute path to build dir>`
`make`
`make install`

Once TestU01 is built, you must add APPRNG/stats_tests/TestU01-1.2.3/build/lib to your LD_LIBRARY_PATH

To compile the simulator, and the runner program, you should be able to run the makefile in APPRNG/runner

Please e-mail jackwadden@gmail.com if you have any build issues.
