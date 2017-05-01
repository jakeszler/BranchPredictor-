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
#include <stdint.h>
#include "utils.h"


#define NUMPERCEPTRONS 4096
#define INDEXBITS 12
#define GHL 40
#define LHL 20
#define NUMWEIGHTS 61
#define THETA 128
#define NUMHISTORIES 1024
#define HISTORYBITS 10

#define LPSIZE 1024
#define LPINDEXBITS 10
#define LPTAGBITS 16
#define INITAGE 20
#define HIGHCONFIDENCE 3
#define LOWCONFIDENCE 1

#define TAKEN 1
#define NOTTAKEN 0

//NOTE competitors are allowed to change anything in this file include the following two defines
//ver2 #define FILTER_UPDATES_USING_BTB     0     //if 1 then the BTB updates are filtered for a given branch if its marker is not btbDYN
//ver2 #define FILTER_PREDICTIONS_USING_BTB 0     //if 1 then static predictions are made (btbANSF=1 -> not-taken, btbATSF=1->taken, btbDYN->use conditional predictor)

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
typedef struct loop_entry{
  uint16_t loop_count;
  uint16_t iter_count;
  uint16_t tag;
  uint8_t confidence;
  uint16_t age;
  int miss_count;
};

class LoopPredictor{
  private:
    loop_entry loop_table[LPSIZE];

  public:
    LoopPredictor(void);

    void init_lp();
    int get_prediction(UINT64 PC);
    uint16_t get_tag(UINT64 PC);
    uint16_t get_index(UINT64 PC);
    void incr_confidence(uint16_t index);
    void decr_confidence(uint16_t index);
    void incr_age(uint16_t index);
    void decr_age(uint16_t index);
    void UpdatePredictor(UINT64 PC, bool resolveDir, bool predDir, bool usedlp);

};

class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
  // UINT32  ghr;           // global history register
  //UINT32  *pht;          // pattern history table
  //UINT32  historyLength; // history length
  //UINT32  numPhtEntries; // entries in pht 

  //int bank_used[NUMBANKS + 2];
  int16_t weights_array[NUMPERCEPTRONS][GHL + LHL + 1];
  uint8_t index; 
  int64_t output;
  int ghr[GHL];
  int local_histories[NUMHISTORIES][LHL];
  int numbranches;
  bool usedlp;
  LoopPredictor lp;


  void ghr_init();
  void ghr_update(bool resolveDir);
  void lhr_update(bool resolveDir, int history_no);
  void decrement_weights(uint8_t perceptron_no, uint8_t index);
  void increment_weights(uint8_t perceptron_no, uint8_t index);
  int get_perceptron_no(UINT64 PC);
  int get_history_no(UINT64 PC);
  
 // GHR(void);
  //GA(void);



 public:

  PREDICTOR(void);
  void init_weightarray();
  bool GetPrediction (UINT64 PC);
  void UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir, bool predDir, UINT64 branchTarget);
  void TrackOtherInst(UINT64 PC, OpType OPTYPE, bool branchTaken, UINT64 branchTarget);
  //NOTE contestants are NOT allowed to use these versions of the functions
//ver2   bool    GetPrediction(UINT64 PC, bool btbANSF, bool btbATSF, bool btbDYN);  
//ver2   void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget, bool btbANSF, bool btbATSF, bool btbDYN);
//ver2   void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);
  // The interface to the functions below CAN NOT be changed

  // Contestants can define their own functions below
};
#endif