SIMULATOR=../simulator
TESTU01=../stats_tests/TestU01-1.2.3/build
PHILOX=../../Random123-1.08
TARGET=runner
FLAGS = -g -O3

all:
	g++ ${FLAGS} -o ${TARGET} runner.cpp ${SIMULATOR}/FSA_Sim.cpp -I${PHILOX}/include -I${SIMULATOR} -I${TESTU01}/include -L${TESTU01}/lib -ltestu01 -lprobdist -lmylib -lm -std=c++11

clean:
	rm ${TARGET}
