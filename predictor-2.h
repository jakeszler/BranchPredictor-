///////////////////////////////////////////////////////////////////////
////  Copyright 2015 Samsung Austin Semiconductor, LLC.                //
/////////////////////////////////////////////////////////////////////////
//
#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include "utils.h"


#define TABLESIZE 101
#define GA_SIZE 100
#define GHL 100
#define HISTORY_LENGTH 8
#define theta 128

#define TAKEN 1
#define NOTTAKEN 0

//NOTE competitors are allowed to change anything in this file include the following two defines
//ver2 #define FILTER_UPDATES_USING_BTB     0     //if 1 then the BTB updates are filtered for a given branch if its marker is not btbDYN
//ver2 #define FILTER_PREDICTIONS_USING_BTB 0     //if 1 then static predictions are made (btbANSF=1 -> not-taken, btbATSF=1->taken, btbDYN->use conditional predictor)

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
  // UINT32  ghr;           // global history register
  //UINT32  *pht;          // pattern history table
  //UINT32  historyLength; // history length
  //UINT32  numPhtEntries; // entries in pht 

  //int bank_used[NUMBANKS + 2];
  uint8_t GA[GA_SIZE];
  uint8_t address;
  uint8_t weights_array[TABLESIZE][TABLESIZE][TABLESIZE];
  uint8_t index; 
  int output;
  
  __uint128_t ghr;
  void ghr_init();
  void ghr_update(bool resolveDir);
  void ga_update(uint8_t address);
  void decrement_weights(uint8_t address, uint8_t index);
  void increment_weights(uint8_t address, uint8_t index);
  bool GetPrediction (UINT64 PC);
  void UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir, bool predDir, UINT64 branchTarget);

 // GHR(void);
  //GA(void);



 public:

  PREDICTOR(void);
  void init_weightarray();
  //NOTE contestants are NOT allowed to use these versions of the functions
//ver2   bool    GetPrediction(UINT64 PC, bool btbANSF, bool btbATSF, bool btbDYN);  
//ver2   void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget, bool btbANSF, bool btbATSF, bool btbDYN);
//ver2   void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);
  // The interface to the functions below CAN NOT be changed

  // Contestants can define their own functions below
};
#endif