#include "FSA_Sim.h"

using namespace std;

/*
 * Constructor
 */ 
FiniteStatePRNG::FiniteStatePRNG(int machines,
                                 int sts,
                                 int reconT,
                                 uint32_t sd,                    
                                 uint32_t permThresh,
                                 uint32_t permWidth,
                                 int symbolStride,
                                 const char * outfn,
                                 const char * transfn)
    : stage()
{
    numMachines = machines;
    numStates = sts;
    reconThresh = reconT;
    steps = 0;
    deck = 0;
    bitsInDeck = 0;

    outputsFileName = outfn;
    transitionsFileName = transfn;

    //PRNG initialization
    CPURandomSource_MT.seed(sd);
    ctr = {{0,0}};
    key = {{sd}};

    // initialize all start states to be 0
    states = new uint32_t[numMachines];
    for(int i = 0; i < numMachines; i++) {
        states[i] = 0;
    }

    // initialize all parallel stage spots to be 0
    pstageCounter = 0;
    pstage = new uint32_t[numMachines];
    for(int i = 0; i < numMachines; i++) {
        pstage[i] = 0;
    }

    // initialize random drop counter
    randomDrop = 0;
    randomDropCount = 0;
    rotateBy = 0;
    rotateCount = 0;

    // initialize permutation array
    permutations = 0;
    permutationThresh = permThresh;
    permutationWidth = permWidth;
    permutation = new int[numMachines];
    for(int i = 0; i < numMachines; i++)
        permutation[i] = i;
    
    // check to see if we've enabled fixed symbol injection
    // -1 disables this feature
    fixedSymbolStride = symbolStride;
    symbolStrideCounter = 0;
    randomByteCounter = 0;
    holderRandomInt = CPURandomInt();
    
    
    // create transitions
    createTransitions();
    createTransitions2();
}

FiniteStatePRNG::~FiniteStatePRNG() 
{
    //delete &stage;
    //delete states;
    //delete &CPURandomSource;
}


/*                                                                             
 * Converts four bytes from long into a random scaled double using
 * 53 bits from the 64 provided. Scaled meaning [0,1).
 * Method via:
 * http://www0.cs.ucl.ac.uk/staff/d.jones/GoodPracticeRNG.pdf
 */
double FiniteStatePRNG::uniformLongToDouble(uint64_t data) {

    double x;
    // Take upper 52 bits for 1-2
    data = (data >> 12) | 0x3FF0000000000000ULL; 
    *((uint64_t *) &x) = data;

    return x - 1.0L; // subtract 1 for (0,1)     
}

/*
 *
 * Flattened indexing helper method
 *
 */
int FiniteStatePRNG::flatIndex(int x, int y, int z, int x_size, int y_size){

    return (x_size * y_size) * z + x_size * y + x;

}


/*
 *
 */
void FiniteStatePRNG::transitionsToFile(){
 
    char str[100];
    snprintf(str, sizeof(str), "./tables/%d%s%d%s%d%s", 
             numMachines, 
             "Machines_", 
             numStates,
             "States_",
             reconThresh, 
             "Inputs.table");

    FILE *f = fopen(str, "w");
    if (f == NULL)
        {
            printf("Error opening file!\n");
            exit(1);
        }
    
    /* print transition table */
    int machine;
    int state;
    int input;
    for(machine = 0; machine < numMachines; machine++) {
        for(state = 0; state < numStates; state++) {
            for(input = 0; input < NUM_SYMBOLS; input++) { 
                fprintf(f, "%d", transitions[numStates * numMachines * input + machine * numStates + state]);
                if(input < NUM_SYMBOLS - 1) {
                    fprintf(f, ",");
                }
            }
        }
        fprintf(f, "\n");
    }
    
    fclose(f);
}


/*
 * Returns the input symbol with bits SYMBOL_BITS
 * keeps track of how many symbols have been requested
 * allows arbitrary insertion of constant symbols in input stream
 */
unsigned int FiniteStatePRNG::APGetNextSymbol()
{

    unsigned int symbol;

    // if we need to insert a constant
    if(fixedSymbolStride >= 0 && symbolStrideCounter == fixedSymbolStride){

        // insert constant
        symbol = 0;
        
        // reset counter
        symbolStrideCounter = 0;

    }else{

        // insert random bits
        symbol = CPURandomBits(SYMBOL_BITS);
        //symbol = CPURandomInt(NUM_SYMBOLS);

        // 
        symbolStrideCounter++;
    }

    return symbol;
}

/*
 * Returns the input symbol with bits SYMBOL_BITS * STRIDE
 */
unsigned int FiniteStatePRNG::APGetNextStridedSymbol()
{

    
    if(SYMBOL_BITS > 32){
        printf("Currently we don't support symbol widths over 32 bits!\n");
        exit(1);
    }
    
    unsigned int symbol = 0;

    for(int i = 0; i < STRIDE; i++){

        symbol = symbol << SYMBOL_BITS;

        symbol = symbol & APGetNextSymbol();
    }

    return symbol;
}

/*
 *
 */
unsigned char FiniteStatePRNG::CPURandomByte()
{


    unsigned char retval;
    /*
    retval = (unsigned char)((holderRandomInt >> (randomByteCounter * 8)) & 0xFF);
    
    randomByteCounter++;
    
    // get new random int if we've exhausted random bytes
    if(randomByteCounter == 4){
        randomByteCounter = 0;
        holderRandomInt = CPURandomInt();
    }
    */
    
    return CPURandomInt() & 0xFF;;
}

/*
 *
 *
 */
unsigned int FiniteStatePRNG::CPURandomInt(unsigned int max) {

    uint32_t value = CPURandomInt();
    
    return (value % max);
}

/*
 *
 *
 */
unsigned int FiniteStatePRNG::CPURandomBits(unsigned int numBits) {

    uint32_t value;
    if(numBits > 8) {
        value = CPURandomInt();
    }else{
        value = CPURandomByte();
    }

    //uint32_t uselessBits = (32 - numBits);
    
    //value = value << uselessBits;
    //value = value >> uselessBits;
    
    return value;
}


/*
 *
 *
 */
unsigned int FiniteStatePRNG::CPURandomInt() {

    // Mersenne Twister
    //int value = (CPURandomSource_MT() % max);
    
    // Philox 
    ctr[0] += 1;
    if(ctr[0] > UINT32_MAX - 10) {
        ctr[1] += 1;
        ctr[0] = 0;
    }

    if(ctr[0] > UINT32_MAX - 10) {
        ctr[1] = 0;
    }

    CBRNG::ctr_type rand = CPURandomSource_PX(ctr, key);
    uint32_t value = rand[0];
    
    return value;
}

/*
 *
 * Shuffles array into random permutation 
 *
 */
void FiniteStatePRNG::shuffle(int *array, int n) {
    int i, j, tmp;

    for (i = n - 1; i > 0; i--) {
        j = CPURandomInt(i + 1);
        tmp = array[j];
        array[j] = array[i];
        array[i] = tmp;
    }
}

/* 
 *
 * Creates an array full of random characters 
 *
 */
uint32_t * FiniteStatePRNG::createRandomSymbolArray(uint32_t length) 
{
    uint32_t * randomChars = (uint32_t *)malloc( sizeof(uint32_t) * ( length ) );
    
    int i;
    for(i = 0; i < length; i++) {
        randomChars[i] = (uint32_t)CPURandomInt(NUM_SYMBOLS);
    }

    return randomChars;
}

/* 
 * Creates a random transition state for each input symbol.
 * Each transition has an equal probability (ala fair dice). 
 */
void FiniteStatePRNG::createRandomTransitions(int machine) {
    
    int state;
    for(state = 0; state < numStates; state++) {
        
        int row[NUM_SYMBOLS];
        
        //initialize row with equal numbers of each state
        int offset = NUM_SYMBOLS / numStates;
        int i,j;
        // e.g. 000000000111111111222222222233333333.....
        for(j = 0; j < numStates; j++) {
            for(i = 0; i < offset; i++) {
                row[j * offset + i] = j;
            }
        }
        
        // Then randomize the row
        shuffle(row, NUM_SYMBOLS);
        
        //store state transitions map with "input" preferred indexing
        for(uint32_t symbol = 0; symbol < NUM_SYMBOLS; symbol++) {
            
            transitions[flatIndex(state, symbol, machine, numStates, NUM_SYMBOLS)] = row[symbol];
        }
    }   
}

/* 
 * Creates a random transition state for each input symbol.
 * Each transition has an equal probability (ala fair dice). 
 *
 */
void FiniteStatePRNG::createRandomTransitions2(int machine) {
    
    int state;
    for(state = 0; state < numStates; state++) {
        
        int row[NUM_SYMBOLS];
        
        //initialize row with equal numbers of each state
        int offset = NUM_SYMBOLS / numStates;
        int i,j;
        for(j = 0; j < numStates; j++) {
            for(i = 0; i < offset; i++) {
                row[j * offset + i] = j;
            }
        }
        
        //randomize row
        shuffle(row, NUM_SYMBOLS);
        
        //store state transitions map with "input" preferred indexing
        for(int symbol = 0; symbol < NUM_SYMBOLS; symbol++) {
            
            transitions2[flatIndex(state, symbol, machine, numStates, NUM_SYMBOLS)] = row[symbol];
        }
    }   
}

/* 
 * 
 * Creates a fully connected fair, transition matrix 
 *
 */
void FiniteStatePRNG::createTransitions() 
{
    
    // The number of transitions is the number of states (rows) times the number
    // of input characters (columns fixed at NUM_SYMBOLS).
    
    if(transitions == NULL)
        transitions = new uint32_t[numMachines * numStates * NUM_SYMBOLS];
    
    // For each state row, fill the columns with an equal number of transitions
    // to each possible state
    // NOTE: how i've implemented this requires that the number of states 
    // equally divides into NUM_SYMBOLS and that states < NUM_SYMBOLS.
    if(numStates > NUM_SYMBOLS) {
        printf("ERROR: number of states is larger than NUM_SYMBOLS\n");
        exit(1);
    }
    if(NUM_SYMBOLS % numStates != 0) {
        printf("ERROR: number of states does not divide into NUM_SYMBOLS\n");
        exit(1);
    }

    // Create transition table for each machine
    int machine;
    for(machine = 0; machine < numMachines; machine++) {
        createRandomTransitions(machine);
    }

    // save transition tables here?

}

/* 
 * 
 * Creates a fully connected fair, transition matrix 
 *
 */
void FiniteStatePRNG::createTransitions2() 
{
    
    // The number of transitions is the number of states (rows) times the number
    // of input characters (columns fixed at NUM_SYMBOLS).
    
    if(transitions2 == NULL)
        transitions2 = new uint32_t[numMachines * numStates * NUM_SYMBOLS];
    
    // For each state row, fill the columns with an equal number of transitions
    // to each possible state
    // NOTE: how i've implemented this requires that the number of states 
    // equally divides into NUM_SYMBOLS and that states < NUM_SYMBOLS.
    if(numStates > NUM_SYMBOLS) {
        printf("ERROR: number of states is larger than NUM_SYMBOLS\n");
        exit(1);
    }
    if(NUM_SYMBOLS % numStates != 0) {
        printf("ERROR: number of states does not divide into NUM_SYMBOLS\n");
        exit(1);
    }

    // Create transition table for each machine
    int machine;
    for(machine = 0; machine < numMachines; machine++) {
        createRandomTransitions2(machine);
    }

    // save transition tables here?

}


/*
 *
 *
 *
 */
void FiniteStatePRNG::step()
{

    unsigned int logStates = (unsigned int)log2(numStates);
    
    uint32_t symbol = CPURandomInt(NUM_SYMBOLS); // acquire random input

    // for each machine, make a transition
    int machine;
    for(machine = 0; machine < numMachines; machine++) {
        
        uint32_t state = states[machine];
        
        //printf("input: %d\n", input);
        //printf("machine: %d\n", machine);
        //printf("state: %d\n", state);
        
        state = transitions[flatIndex(state, symbol, machine, numStates, NUM_SYMBOLS)];
        
        //printf("next state: %d\n", state);
        stage.push((uint32_t)(state));        
    
        states[machine] = state;
    }

    steps++;
    // if we've reached the reconfiguration threshhold
    // reconfigure the transition tables
    if(steps > reconThresh) {
        //cout << "RECONFIGURED" << endl;
        createTransitions();
        steps = 0;
    }

}

void FiniteStatePRNG::newPermutation() {

    shuffle(permutation, numMachines);
}

void FiniteStatePRNG::newPermutation(int interval) {

    // for all base addresses that start an interval
    for(int i = 0; i < numMachines; i += interval) {
        int len = interval;
        if(i + interval > numMachines)
            len = numMachines - i;
        // shuffle from base address to i (where i = interval or end)
        shuffle((permutation + i), len);
    }
}

/*
 *
 *
 *
 */
void FiniteStatePRNG::stepPermute()
{

    unsigned int logStates = (unsigned int)log2(numStates);
    
    // 
    uint32_t symbol = 0;

    symbol = APGetNextSymbol();
    //symbol = CPURandomInt(NUM_SYMBOLS);
    
    int machine = 0;
    int machine_tmp = 0;
    for(machine_tmp = 0; machine_tmp < numMachines; machine_tmp++) {
        
        // fetch "effective machine" based on one 
        //   level of indirection based on permutation array
        machine = permutation[machine_tmp];
        uint32_t state = states[machine];
        
        //printf("input: %d\n", input);
        //printf("machine: %d\n", machine);
        //printf("state: %d\n", state);
        
        state = transitions[flatIndex(state, symbol, machine, numStates, NUM_SYMBOLS)];
        
        //printf("next state: %d\n", state);
        stage.push((uint32_t)(state));        
    
        states[machine] = state;
    }

    steps++;
    permutations++;

    // if we've reached the reconfiguration threshhold
    // reconfigure the transition tables
    if(steps > reconThresh) {
        //cout << "RECONFIGURED" << endl;
        createTransitions();
        steps = 0;
    }

    // if we've reached the permutation threshold
    // reconfigure the output permutation circuitry
    if(permutations > permutationThresh) {
        //newPermutation();
        newPermutation(permutationWidth);
        permutations = 0;
    }

}

/*
 *
 *
 *
 */
void FiniteStatePRNG::stepIndexed()
{

    unsigned int logStates = (unsigned int)log2(numStates);
    
    // acquire random input with one extra bit
    uint32_t symbol = CPURandomInt(NUM_SYMBOLS * 2); 

    // for each machine, make a transition
    // transitions are indexed based on lowest extra bit
    int machine;
    for(machine = 0; machine < numMachines; machine++) {
        
        uint32_t state = states[machine];
        
        //printf("input: %d\n", input);
        //printf("machine: %d\n", machine);
        //printf("state: %d\n", state);
        if(symbol & 1){
            state = transitions[flatIndex(state, symbol >> 1, machine, numStates, NUM_SYMBOLS)];
        }else{
            state = transitions2[flatIndex(state, symbol >> 1, machine, numStates, NUM_SYMBOLS)];
        }
        //printf("next state: %d\n", state);
        stage.push((uint32_t)(state));        
    
        states[machine] = state;
    }

    steps++;
    // if we've reached the reconfiguration threshhold
    // reconfigure the transition tables
    if(steps > reconThresh) {
        //cout << "RECONFIGURED" << endl;
        createTransitions();
        createTransitions2();
        steps = 0;
    }

}

/*
 *
 *
 *
 */
void FiniteStatePRNG::stepParallel()
{

    unsigned int logStates = (unsigned int)log2(numStates);
    
    uint32_t symbol = CPURandomInt(NUM_SYMBOLS); // acquire random input
    uint32_t machine;
    for(machine = 0; machine < numMachines; machine++) {
        
        uint32_t state = states[machine];
        
        //printf("input: %d\n", input);
        //printf("machine: %d\n", machine);
        //printf("state: %d\n", state);
        
        state = transitions[flatIndex(state, symbol, machine, numStates, NUM_SYMBOLS)];
        
        //printf("next state: %d\n", state);
        // store value into stage belonging to that machine
        uint32_t tmp = pstage[machine] << logStates;
        tmp |= (uint32_t)state;
        pstage[machine] = tmp;

        states[machine] = state;
    }

    steps++;
    // if we've reached the reconfiguration threshhold
    // reconfigure the transition tables
    if(steps > reconThresh) {
        //cout << "RECONFIGURED" << endl;
        createTransitions();
        steps = 0;
    }

}

uint32_t rotl32a (uint32_t x, uint32_t n)
{
    return (x<<n) | (x>>(32-n));
}

/*
 *
 *
 *
 */
uint32_t FiniteStatePRNG::APRandomInt() {

    //we need at least 32 bits in the stage
    uint32_t requiredBits = 32;
    uint64_t logStates = (uint64_t)log2(numStates);

    //we need at least 32 bits in the deck
    while(bitsInDeck < requiredBits) {

        // while we don't have enough randomness, build more
        while(logStates * stage.size() < requiredBits) {
            //step();
            stepPermute();
            //stepIndexed();
        }
        
        // pop a value off the stage
        deck = deck << logStates;
        deck |= (uint32_t)(stage.front());
        
        bitsInDeck += logStates;
                
        stage.pop();
    }

    //build 32bit value from outputs
    uint32_t result = deck;

    // reset deck
    bitsInDeck = 0;
    deck = 0;

    return result;
    //return rotl32a(result, randomDrop);
    //return CPURandomSource();
}

/*
 *
 *
 *
 */
uint32_t FiniteStatePRNG::APRandomIntFair() {
    
    uint32_t result;

    if(pstageCounter == 0) {
        
        //we need at least 1 integer in the pstage
        uint32_t requiredBits = 32;
        uint64_t logStates = (uint64_t)log2(numStates);
        
        // while we don't have enough randomness, build more
        for(int step = 0; step * logStates < requiredBits; step++) {
            stepParallel();
            
        }
        
        // now we have numMachines integers in the pstage
        pstageCounter = numMachines;
        
    }
    
    //cout << pstageCounter << endl;

    result = pstage[pstageCounter - 1];
    pstage[pstageCounter - 1] = 0;
    pstageCounter--;

    return result;
    //return CPURandomSource();
}

/*
 *
 *
 *
 */
uint64_t FiniteStatePRNG::APRandomLong() {

    //we need at least 32 bits in the stage
    uint32_t requiredBits = 64;
    bitsInDeck =0;
    uint64_t logStates = (uint64_t)log2(numStates);

    // while we don't have enough randomness, build more
    while(logStates * stage.size() < requiredBits)
        step();

    //we need at least 64 bits in the deck
    //note that overflow is OK here
    while(bitsInDeck < requiredBits) {
        // pop a value off the stage
        deck = deck << logStates;
        deck |= (uint32_t)(stage.front());
        stage.pop();

        bitsInDeck += logStates;
    }

    //build 64bit value from outputs
    uint64_t result = deck;
    bitsInDeck = 0;
    deck = 0;
    return result;
    //return CPURandomSource();
}

/*
 *
 *
 *
 */
double FiniteStatePRNG::APRandomDouble() {

    return uniformLongToDouble(APRandomLong());
}

/*
 *
 *
 *
 */
double FiniteStatePRNG::CPURandomDouble() {

    uint64_t result = ((uint64_t)CPURandomInt()) << 32;
    result = result | ((uint64_t)CPURandomInt());
    return uniformLongToDouble(result);
}

/*
 *
 *
 *
 */
void FiniteStatePRNG::printStage() {
    
    cout << "STAGE SIZE: " << stage.size() << endl;
    cout << "STAGE:" << endl;
    while(!stage.empty()) {
        cout << (int)stage.front() << endl;
        stage.pop();
    }
}

/*
int main(){

    printf("HELLO WORLD!\n");
}
*/
