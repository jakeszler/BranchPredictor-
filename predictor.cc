#include "predictor.h"
#include <math.h>

PREDICTOR::PREDICTOR (void)
{
  

}


bool PREDICTOR::GetPrediction (UINT64 PC)
{
 return true; 
}

void PREDICTOR::UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir, bool predDir, UINT64 branchTarget)
{


}

void PREDICTOR::TrackOtherInst (UINT64 PC, OpType opType, bool taken, UINT64 branchTarget)
{


}

uint16_t PREDICTOR::get_bank_index(UINT64 PC, uint8_t bankno, __uint128_t ghr){
  int numHistoryBits = (int) (pow(13.0/8.0, bankno) * 10.0 + .5);
  __uint128_t tempGHR = ghr;
  uint16_t index = tempGHR & ((1 << 10) - 1); 
  tempGHR >>= 10;
  numHistoryBits -= 10;

  while (numHistoryBits > 0){
      if (numHistoryBits >= 10){
        index ^= tempGHR & ((1 << 10) - 1);
        tempGHR >>= 10;
        numHistoryBits -= 10;
      }
      else{
        index ^= tempGHR & ((1 << numHistoryBits) - 1);
        tempGHR >>= numHistoryBits;
      }
  }
  return (index % BANKSIZE);
}

uint8_t PREDICTOR::get_tag(UINT64 PC, __uint128_t ghr, int bankno){
  int numHistoryBits = (int) (pow(13.0/8.0, bankno) * 10.0 + .5);
  __uint128_t result = PC * (ghr & ((1 << numHistoryBits) - 1));
  return (result % 256);
}


//Global History Methods
GHR::GHR(void){
  ghr = 0;
}

void GHR::ghr_update(bool resolveDir){
  ghr <<= 1;
  if (resolveDir == TAKEN)
    ghr++;
}

 __uint128_t GHR::get_ghr(){
  return ghr;
 };

//BASE PREDICTOR METHODS


void BasePredictor::init_bp(){
  for (int i = 0; i < BPSIZE; i++)
    bp_table[i] = 3;
}

void BasePredictor::bp_incr(int index){
  if (bp_table[index] < MAXSAT)
    bp_table[index]++;
}

void BasePredictor::bp_decr(int index){
  if (bp_table[index] > 0)
    bp_table[index]--;
}

void BasePredictor::bp_update(int index, bool resolveDir){
  if (resolveDir == TAKEN)
    bp_incr(index);
  else
    bp_decr(index);
}

bool BasePredictor::bp_getPred(int index){
  return (bp_table[index] & PREDMSB);
}

UINT64 BasePredictor::bp_getIndex(UINT64 PC){
    return (PC & ((1<<BPINDEXBITS) - 1));
}


//bank methods 
void Banks::bank_init(){  
  for (int i = 0; i < NUMBANKS; i++){
    for (int j = 0; j < BANKSIZE; j++){
      bank_array[i][j].pred_ctr = 3;
      bank_array[i][j].tag = 0;
      bank_array[i][j].useful_ctr = 0;
    }
  }
}

bool Banks::get_pred(int bank, int entry){
  return (bank_array[bank][entry].pred_ctr & PREDMSB);
}

