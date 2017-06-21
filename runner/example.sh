#!/bin/bash
#USAGE: <machines> <states> <reconThresh> <permthresh> <permwidth> <symbol stride> <bits per symbol> <seed> <crushtype 1|2|3> <fixedSymbolStride (-1 to disable)>
MACHINES=571
STATES=8
RECON_T=1000000
PERM_T=1000000
PERM_W=32
STRIDE=1
SYMBOL_BITS=8
SEED=7
CRUSH=1
FIXED_SYMBOL_STRIDE=-1

./runner ${MACHINES} ${STATES} ${RECON_T} ${PERM_T} ${PERM_W} ${STRIDE} ${SYMBOL_BITS} ${SEED} ${CRUSH} ${FIXED_SYMBOL_STRIDE}

