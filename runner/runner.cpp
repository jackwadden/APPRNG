#include "FSA_Sim.h"
#include <string.h>
 
extern "C"
{
#include "unif01.h"
#include "swrite.h"
#include "bbattery.h"
#include "gdef.h"
#include "sres.h"
#include "sknuth.h"
#include "sspectral.h"
#include "statcoll.h"
#include "gofw.h"
}
 
FiniteStatePRNG * engine;

uint32_t rnd_function() 
{
    return (*engine)();
}
 
void usage() {

    cout << "USAGE: <machines> <states> <reconThresh> <permthresh> <permwidth> <stride> <bits per symbol> <seed> <crushtype 1|2|3> <fixed symbol stride (-1 to disable)> <pipelined 0|1>" << endl;
}

int main (int argc, char * argv[])
{
    if(argc != 12) {
        usage();
        exit(1);
    }
    
    int machines = stoi(argv[1]);
    int states = stoi(argv[2]);
    int reconThresh = stoi(argv[3]);
    int permThresh = stoi(argv[4]);
    int permWidth = stoi(argv[5]);
    uint32_t stride = stoi(argv[6]);
    uint32_t bitsPerSymbol = stoi(argv[7]);

    int seed = stoi(argv[8]);
    int crushtype = stoi(argv[9]);
    int fixedSymbolStride = stoi(argv[10]);
    uint32_t concurrentExec = stoi(argv[11]);
    //char * outfile = argv[8];
    //char * tablefile = argv[9];
    char const * outfile = "out.txt"; //added const for c++11 support
    char const * tablefile = "table.txt";//added const for c++11 support
    cout <<"Sup yo";
    // Configuration output
    printf("Starting APPRNG simulation with the following configuration:\n");
    printf("Machines: %d\n", machines);
    printf("States per machine: %d\n", states);
    printf("Reconfiguration Threshold: %d\n", reconThresh);
    printf("Permutation Threshold: %d\n", permThresh);
    printf("Permutation width (bits): %d\n", permWidth);
    printf("Symbols per strided symbol: %d\n", stride);
    printf("Bits per symbol: %d\n", bitsPerSymbol);
    printf("PRNG seed: %d\n", seed);
    printf("Crush type (1 small|2 medium|3 big): %d\n", crushtype);
    printf("Fixed symbol stride (-1 is disabled): %d\n", fixedSymbolStride);
    printf("Pipelined (0|1): %d\n", concurrentExec);
    
    engine = new FiniteStatePRNG(machines,
                                 states,
                                 reconThresh,
                                 seed,
                                 permThresh,
                                 permWidth,
                                 stride,
                                 bitsPerSymbol,
                                 fixedSymbolStride,
                                 concurrentExec,
                                 outfile,
                                 tablefile);
        
    unif01_Gen *gen;
    //swrite_Basic = FALSE;
    
    char tmp_type[80];
    gen = unif01_CreateExternGenBits(strcat(argv[5]," AP_PRNG"), rnd_function);

    long N;
    long n;
    int r;
    int d;
    int t;

    // CRUSH
    //N =  5,  n = 10000000,  r =  0,   d = 100000,   t = 10;
    //sknuth_MaxOft (gen, NULL,
    //               N, n, r, d, t);

    //N = 10,  n = 10000000,  r =  0,   d = 100000,   t =  5;
    //sknuth_MaxOft (gen, NULL,
    //               N, n, r, d, t);

    // BIGCRUSH
    /*
    N = 30,  n = 10000000,  r =  0,   d = 100000,   t =  16;
    sknuth_MaxOft (gen, NULL,
                   N, n, r, d, t);
    */
    int k, s;
    //N = 100000,  r = 0, s = 3, k = 14;
    /*
    N = 100000,  r = 27, s = 3, k = 14;
    sspectral_Fourier3(gen, NULL,
                       N, k, r, s);
    */
    
    if(crushtype == 1) {
        bbattery_SmallCrush (gen);
    }else if(crushtype == 2) {
        bbattery_Crush (gen);
    }else if (crushtype == 3){
        bbattery_BigCrush (gen);
    }else {
        cout << "UNRECOGNIZED CRUSH TYPE" << endl;
        exit(1);
    }
    

    unif01_DeleteExternGenBits (gen);
    
    return 0;
}
