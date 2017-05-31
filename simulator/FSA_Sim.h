#ifndef FSASIM_H
#define FSASIM_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <random>

#include "Random123/philox.h"

#define NUM_SYMBOLS 256
typedef r123::Philox4x32_R<10> CBRNG;

using namespace std;

class FiniteStatePRNG 
{
    
public:
    double uniformLongToDouble(uint64_t data);
    void byteArrayToFile();
    void transitionsToFile();
    uint32_t CPURandomInt(uint32_t max);
    uint32_t CPURandomInt();
    double CPURandomDouble();
    uint32_t APRandomInt();
    uint32_t APRandomIntFair();
    uint64_t APRandomLong();
    double APRandomDouble();
    void shuffle(int * array, int n);
    void newPermutation();
    void newPermutation(int interval);
    uint32_t * createRandomSymbolArray(uint32_t length);
    void createRandomTransitions(int machine);
    void createRandomTransitions2(int machine);
    void createTransitions();
    void createTransitions2();
    int flatIndex(int x, int y, int z, int x_size, int y_size);
    void step();
    void stepParallel();
    void stepPermute();
    void stepIndexed();
    void printStage();
    uint32_t operator()() {return APRandomInt(); }
    //uint32_t operator()() {return APRandomIntFair(); }
    FiniteStatePRNG(int machines, 
                    int sts, 
                    int reconT,
                    uint32_t sd, 
                    uint32_t permThresh,
                    uint32_t permWidth,
                    int symbolStride,
                    const char * outfn, 
                    const char *  transfn);
    ~FiniteStatePRNG();

private:
    int numMachines;
    int numStates;
    int reconThresh;
    int steps;
    uint64_t deck;
    int bitsInDeck;
    int randomDrop;
    int randomDropCount;
    int rotateBy;
    int rotateCount;
    int * permutation;
    uint32_t permutationThresh;
    int permutations;
    uint32_t permutationWidth;

    int fixedSymbolStride;

    int seed;
    CBRNG::ctr_type ctr;
    CBRNG::key_type key; 

    const char * outputsFileName;
    const char * transitionsFileName;
    uint32_t * transitions;
    uint32_t * transitions2;
    uint32_t * states;
    queue<uint32_t> stage; //bytes with bits that can be converted to output
    uint32_t * pstage;
    int pstageCounter; // number of 32bit integers in pstage
    mt19937 CPURandomSource_MT;
    CBRNG CPURandomSource_PX;
};
#endif
