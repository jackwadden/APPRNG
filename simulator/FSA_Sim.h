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

//#define NUM_SYMBOLS 256 // number of symbols in the alphabet (MUST BE 2^(SYMBOL_BITS * STRIDE))
//#define SYMBOL_BITS 8 // bit width of each individual symbol
//#define STRIDE 1 // symbols considered per transition
typedef r123::Philox4x32_R<10> CBRNG;

using namespace std;

class FiniteStatePRNG 
{
    
public:
    double uniformLongToDouble(uint64_t data);
    void byteArrayToFile();
    void transitionsToFile();
    uint32_t CPURandomInt(uint32_t max);
    uint32_t CPURandomBits(uint32_t numBits);
    uint32_t CPURandomInt();
    unsigned char CPURandomByte();
    double CPURandomDouble();
    uint32_t APRandomInt();
    uint32_t APGetNextStridedSymbol();
    uint32_t APGetNextSymbol();
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
                    uint32_t stride,
                    uint32_t bitsPerSymbol,
                    int fixedSymbolStride,
                    const char * outfn, 
                    const char *  transfn);
    ~FiniteStatePRNG();

private:
    //
    uint32_t NUM_SYMBOLS;
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

    // Variables added to support strided computation
    uint32_t stride;
    uint32_t bitsPerSymbol;
    
    // Variables added to support byte striding
    int fixedSymbolStride;
    int symbolStrideCounter;
    unsigned int holderRandomInt;
    unsigned int randomByteCounter;
    
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
