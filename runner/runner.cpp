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

    cout << "USAGE: <machines> <states> <reconThresh> <permthresh> <permwidth> <seed> <crushtype> <outfile> <tablefile>" << endl;
}

int main (int argc, char * argv[])
{
    if(argc != 10) {
        usage();
        exit(1);
    }
    
    int machines = stoi(argv[1]);
    int states = stoi(argv[2]);
    int reconThresh = stoi(argv[3]);
    int permThresh = stoi(argv[4]);
    int permWidth = stoi(argv[5]);
    int seed = stoi(argv[6]);
    int crushtype = stoi(argv[7]);
    char * outfile = argv[8];
    char * tablefile = argv[9];
    
    engine = new FiniteStatePRNG(machines,
                                 states,
                                 reconThresh,
                                 permThresh,
                                 permWidth,
                                 seed,
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
