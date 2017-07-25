# **APPRNG**
## **Installation**

### **I.Download Random123 (philox prng)**

Please download and install the Philox library from https://www.deshawresearch.com/resources_random123.html
Random123 does not require compilation, but you will need to tell the runner/Makefile where the library exists. Place Random123 in the same directory location as APPRNG in order for APPRNG to be able to access Random123’s resources. 

### **II. Configure TESTU01 (statistical test battery)**

TestU01 must first be compiled into a dynamically linkable library before the AP PRNG simulator can be run against the tests. Follow the steps below to compile TestU01(Assuming that you are in the APPRNG directory).
```
cd stats_tests
unzip TestU01.zip
cd TestU01-1.2.3
mkdir build
./configure --prefix=<absolute path to build dir>
make
make install
```
Once TestU01 is built, you must add <exact-path-to>/APPRNG/stats_tests/TestU01-1.2.3/build/lib to your LD_LIBRARY_PATH in the command-line. Do this with the following line:
```
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<full_path_here_to_build/lib_here>
```

### **III. Experiment Runner**

To compile the simulator, and the runner program, you should be able to run the makefile in APPRNG/runner. You may have to change the paths in this file (see Random123 above) to get code to successfully compile.

#### How to compile and run the APPRNG code
In the command-line, use the following command, when located in the APPRNG/runner directory 
```
$ make
```
After the above command is run, an executable is created, which is named “runner”. In order to execute this executable, use the following command with each of the command-line arguments you wish to give to runner.cpp.
``` 
$ ./runner Arg1 Arg2 Arg3...
```


## Contact

Please e-mail jackwadden@gmail.com if you have any build issues.
