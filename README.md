# APPRNG

## Installation

### Random123 (philox prng)
Please download and install the Philox library from https://www.deshawresearch.com/resources_random123.html

Random123 does not require compilation, but you will need to tell runner/Makefile where the library exists.

### TESTU01 (statistical test battery)
TestU01 must first be compiled into a dynamically linkable library before the AP PRNG simulator can be run against the tests. Follow the steps below to compile TestU01.

`cd stats_tests/TestU01-1.2.3`  
`mkdir build`  
`./configure --prefix=<absolute path to build dir>`  
`make`  
`make install`  

Once TestU01 is built, you must add <exact-path-to>/APPRNG/stats_tests/TestU01-1.2.3/build/lib to your LD_LIBRARY_PATH

### Experiment Runner
To compile the simulator, and the runner program, you should be able to run the makefile in APPRNG/runner. You may have to change the paths in this file (see Random123 above) to get code to successfully compile.

## Contact

Please e-mail jackwadden@gmail.com if you have any build issues.
