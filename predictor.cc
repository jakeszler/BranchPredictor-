#include "predictor.h"
#include <math.h>

PREDICTOR::PREDICTOR (void)
{
  numbranches = 0;
  bp.init_bp();
  banks.bank_init();
}

bool PREDICTOR::GetPrediction (UINT64 PC)
{
 numbranches++;
 bool foundPred = false;
 altpredno = predno = 0;
 int index = bp.bp_getIndex(PC);
 pred = altpred = bp.bp_getPred(index);

 for (int i = 0; i < NUMBANKS; i++){
    uint16_t entry = get_bank_index(PC, i, ghr.get_ghr());
    uint16_t tag = get_tag(PC, ghr.get_ghr(), i);
    if (banks.tag_match(i, entry, tag)){
      if (foundPred){
        altpred = pred;
        altpredno = predno;
      }
      pred = banks.get_pred(i, entry);
      predno = i + 1;
      foundPred = true;
    }
 } 
return pred; 

}

void PREDICTOR::UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir, bool predDir, UINT64 branchTarget)
{
    if (numbranches % 512000 == 0){
        banks.clearLSBs();
    }
    else if (numbranches % 256000 == 0){
        banks.clearMSBs();
    }

    int entry = get_bank_index(PC, predno - 1, ghr.get_ghr());
    if (pred != altpred && predno > 0){
            //correct, resolve = actual taken or not, pred = predicted taken or not
            if (resolveDir == predDir)
                banks.incr_u_ctr(predno - 1, entry);
            //incorrect
            else
                banks.decr_u_ctr(predno - 1, entry);
    }

    int bp_index = bp.bp_getIndex(PC);
   
    //if overall is correct
    if (resolveDir == predDir){
        if (predno == 0){
            bp.bp_update(bp_index, resolveDir);
        }
        else{
            banks.bank_update(predno - 1, entry, resolveDir);
        }
    }
    //if overall is incorrect
    else{
        banks.bank_update(predno - 1, entry, resolveDir);
        if (predno != NUMBANKS){
            init_update_banks();
            int prob_val = 1 << (NUMBANKS - 1);
            int cuml_prob = 0;
            bool no_zeros = true;
            for (int i = predno; i < NUMBANKS; i++){
                 int bank_entry = get_bank_index(PC, i, ghr.get_ghr());
                 if (banks.get_u_ctr(i, bank_entry) == 0){
                    update_banks[i] = prob_val;
                    cuml_prob += prob_val;
                    prob_val /= 2;
                    no_zeros = false;
                 }
            }
            if (no_zeros){ 
                for (int i = predno; i < NUMBANKS; i++){
                    int bank_entry = get_bank_index(PC, i, ghr.get_ghr());
                    banks.decr_u_ctr(i, bank_entry);
                }
            }
            else{
                int rand_val = rand() % (cuml_prob + 1);
                int running_total = 0;
                int selected_bank = -1;

                for (int i = 0; i < NUMBANKS; i++){
                    if (update_banks[i] != 0){
                        running_total += update_banks[i];
                        if (rand_val <= running_total){
                            selected_bank = i;
                            break;
                        }
                    }
                }
                //allocate
                int allocate_entry = get_bank_index(PC, selected_bank, ghr.get_ghr());
                int allocate_tag = get_tag(PC, ghr.get_ghr(), selected_bank);
                banks.clear_u_ctr(selected_bank, allocate_entry);
                banks.set_pred_ctr(selected_bank, allocate_entry);
                banks.set_tag(selected_bank, allocate_entry, allocate_tag);
            }
        }
    }
    ghr.ghr_update(resolveDir);
}

void PREDICTOR::TrackOtherInst (UINT64 PC, OpType opType, bool taken, UINT64 branchTarget)
{

}

uint16_t PREDICTOR::get_bank_index(UINT64 PC, uint8_t bankno, __uint128_t ghr){
  int numHistoryBits = (int) (pow(2.0, bankno) * 8.0 + .5);
  __uint128_t tempGHR = ghr;
  uint16_t index = PC & ((1 << BANKINDEXBITS) - 1); 
  //tempGHR >>= BANKINDEXBITS;
  //numHistoryBits -= BANKINDEXBITS;

  while (numHistoryBits > 0){
      if (numHistoryBits >= BANKINDEXBITS){
        index ^= tempGHR & ((1 << BANKINDEXBITS) - 1);
        tempGHR >>= BANKINDEXBITS;
        numHistoryBits -= BANKINDEXBITS;
      }
      else{
        index ^= tempGHR & ((1 << numHistoryBits) - 1);
        tempGHR >>= numHistoryBits;
        numHistoryBits -= numHistoryBits;
      }
  }
  return (index % BANKSIZE);
}

uint16_t PREDICTOR::get_tag(UINT64 PC, __uint128_t ghr, int bankno){
  int numHistoryBits = (int) (pow(13.0/8.0, bankno) * 10.0 + .5);
  __uint128_t tempGHR = ghr;
  //if (numbranches == 10)
    //printf("%llu\n", ghr);
  UINT64 index = PC & ((1 << BANKINDEXBITS) - 1);
 // if (numbranches == 10)
 //   printf("index: %d\n", index);
 // tempGHR >>= BANKINDEXBITS;
  //numHistoryBits -= BANKINDEXBITS;

  while (numHistoryBits > 0){
      if (numHistoryBits >= BANKINDEXBITS){
        index ^= tempGHR & ((1 << BANKINDEXBITS) - 1);
    //    if (numbranches == 10)
  //  printf("index: %d\n", index);
        tempGHR >>= BANKINDEXBITS;
        numHistoryBits -= BANKINDEXBITS;
    //    if (numbranches == 1)
   // printf("%d\n", index);
      }
      else{
        index ^= tempGHR & ((1 << numHistoryBits) - 1);
     //   if (numbranches == 10)
   // printf("index: %d\n", index);
        tempGHR >>= numHistoryBits;
        numHistoryBits -= numHistoryBits;
   //     if (numbranches == 1)
   // printf("%d\n", index);
      }
  }
 // index >>= BANKINDEXBITS;
  return (index % 256);
}

void PREDICTOR::init_update_banks(){
    for (int i = 0; i < NUMBANKS; i++){
        update_banks[i] = 0;
    }
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
BasePredictor::BasePredictor(void){}

void BasePredictor::init_bp(){
  for (int i = 0; i < BPSIZE; i++)
    bp_table[i] = 4;
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
Banks::Banks(void){};

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

bool Banks::tag_match(int bank, int entry, int tag){
  return (bank_array[bank][entry].tag == tag);
}

void Banks::set_tag(int bankno, int entry, int tag){
    bank_array[bankno][entry].tag = tag;
}

void Banks::incr_pred_ctr(int bank, int entry){
  if (bank_array[bank][entry].pred_ctr < MAXSAT)
    bank_array[bank][entry].pred_ctr++;
}
  
void Banks::decr_pred_ctr(int bank, int entry){
  if (bank_array[bank][entry].pred_ctr > 0)
    bank_array[bank][entry].pred_ctr--;
}

void Banks::set_pred_ctr(int bank, int entry){
    bank_array[bank][entry].pred_ctr = 4;
}

void Banks::incr_u_ctr(int bank, int entry){
  if (bank_array[bank][entry].useful_ctr < UMAX)
    bank_array[bank][entry].useful_ctr++;
}
  
void Banks::decr_u_ctr(int bank, int entry){
  if (bank_array[bank][entry].useful_ctr > 0)
    bank_array[bank][entry].useful_ctr--;
}

void Banks::clear_u_ctr(int bank, int entry){
    bank_array[bank][entry].useful_ctr = 0;
}

int Banks::get_u_ctr(int bank, int entry){
    return bank_array[bank][entry].useful_ctr;
}

void Banks::bank_update(int bankno, int entry, bool resolveDir){
  if (resolveDir == TAKEN)
    incr_pred_ctr(bankno, entry);
  else
    decr_pred_ctr(bankno, entry);
}

void Banks::clearMSBs(){
    for (int i = 0; i < NUMBANKS; i++){
        for (int j = 0; j < BANKSIZE; j++){
            bank_array[i][j].useful_ctr &= 1;
        }
    }
}

void Banks::clearLSBs(){
    for (int i = 0; i < NUMBANKS; i++){
        for (int j = 0; j < BANKSIZE; j++){
            bank_array[i][j].useful_ctr &=  2;
        }
    }
}

int Banks::get_tag(int bank, int entry){
  return bank_array[bank][entry].tag;
}

