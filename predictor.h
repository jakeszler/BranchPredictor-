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
#include "utils.h"


#define BPSIZE 4096
#define BANKSIZE 1024
#define MAXSAT 7
#define PREDMSB 4
#define BPINDEXBITS 12
#define TAKEN 1
#define NOTTAKEN 0
#define GHRLENGTH 100
#define NUMBANKS 4


//NOTE competitors are allowed to change anything in this file include the following two defines
//ver2 #define FILTER_UPDATES_USING_BTB     0     //if 1 then the BTB updates are filtered for a given branch if its marker is not btbDYN
//ver2 #define FILTER_PREDICTIONS_USING_BTB 0     //if 1 then static predictions are made (btbANSF=1 -> not-taken, btbATSF=1->taken, btbDYN->use conditional predictor)

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

typedef struct bank_entry{
uint8_t pred_ctr;
uint8_t tag;
uint8_t useful_ctr;
}bank_entry;




class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
 // UINT32  ghr;           // global history register
  //UINT32  *pht;          // pattern history table
  //UINT32  historyLength; // history length
  //UINT32  numPhtEntries; // entries in pht 


 public:

  PREDICTOR(void);
  //NOTE contestants are NOT allowed to use these versions of the functions
//ver2   bool    GetPrediction(UINT64 PC, bool btbANSF, bool btbATSF, bool btbDYN);  
//ver2   void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget, bool btbANSF, bool btbATSF, bool btbDYN);
//ver2   void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);

  // The interface to the functions below CAN NOT be changed
  bool    GetPrediction(UINT64 PC);  
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);

  uint16_t get_bank_index(UINT64 PC, uint8_t bankno, __uint128_t ghr);
  uint8_t get_tag(UINT64 PC, __uint128_t ghr);
  // Contestants can define their own functions below
};

class BasePredictor{
private: 
  uint8_t bp_table[BPSIZE];

public:

BasePredictor(void);

//BASE PREDICTOR METHODS
void init_bp();
void bp_incr(int index);
void bp_decr(int index);
void bp_update(int index, bool resolveDir);
bool bp_getPred(int index);
UINT64 bp_getIndex(UINT64 PC);
};


class Banks{

  Banks(void);

  private:
    bank_entry bank_array[NUMBANKS][BANKSIZE];
    
  public:

  void bank_init();
  bool get_pred(int bank, int entry);
};

class GHR{

  GHR(void);

  private:
  __uint128_t ghr;

  public:
  void ghr_init();
  void ghr_update(bool resolveDir);
  __uint128_t get_ghr();

};

#endif
