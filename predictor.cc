#include "predictor.h"
#include <math.h>

PREDICTOR::PREDICTOR (void)
{
  numbranches = 0;
  for (int i = 0; i < GHL; i++){
    ghr[i] = 0;
  }
  for(int i = 0; i < NUMHISTORIES; i++){
    for (int j = 0; j < LHL; j++){
      local_histories[i][j] = 0;
    }
  }
  init_weightarray();
}

bool PREDICTOR::GetPrediction (UINT64 PC)
{
  //COMMENT OUT TO GET RID OF LOOP
  if (lp.get_prediction(PC) == 0){
      usedlp = true;
      return NOTTAKEN;
  }
  usedlp = false;
  //**********//

  numbranches++;
  int perc = get_perceptron_no(PC);
  int history_no = get_history_no(PC);

  output = weights_array[perc][0];
  for (int i = 0; i < GHL; i++){
      output += ghr[i] * weights_array[perc][i + 1];
  }
  for (int i = 0; i < LHL; i++){
    output += local_histories[history_no][i] * weights_array[perc][GHL + 1 + i];
  }
  return (output >= 0);
}

void PREDICTOR::UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir, bool predDir, UINT64 branchTarget)
{
  //COMMENT OUT TO GET RID OF LOOP
  if (branchTarget < PC){
      lp.UpdatePredictor(PC, resolveDir, predDir, usedlp);
    }
  //**********//

  int perc = get_perceptron_no(PC);
  int history_no = get_history_no(PC);

  if (predDir != resolveDir || abs(output) < THETA){
    if (resolveDir == TAKEN){
      if (weights_array[perc][0] < THETA)
        weights_array[perc][0]++;
    }
    else{
      if (weights_array[perc][0] > (-1 * THETA))
        weights_array[perc][0]--;
    }

    for (int i = 0; i < GHL; i++){
        if (ghr[i] == 1){
            if (resolveDir == TAKEN){
                 if (weights_array[perc][i + 1] < THETA)
                   weights_array[perc][i + 1]++;
            }
            else{
                if (weights_array[perc][i + 1] > (-1 * THETA))
                  weights_array[perc][i + 1]--;
            }
        }
        else{
            if (resolveDir == TAKEN){
                if (weights_array[perc][i + 1] > (-1 * THETA))
                  weights_array[perc][i + 1]--;
            }
            else{
                 if (weights_array[perc][i + 1] < THETA)
                  weights_array[perc][i + 1]++;
              }
        }
    }

    for (int i = 0; i < LHL; i++){
        if (local_histories[history_no][i] == 1){
            if (resolveDir == TAKEN){
                 if (weights_array[perc][GHL + i + 1] < THETA)
                   weights_array[perc][GHL + i + 1]++;
            }
            else{
                if (weights_array[perc][GHL + i + 1] > (-1 * THETA))
                  weights_array[perc][GHL + i + 1]--;
            }
        }
        else{
            if (resolveDir == TAKEN){
                if (weights_array[perc][GHL + i + 1] > (-1 * THETA))
                  weights_array[perc][GHL + i + 1]--;
            }
            else{
                 if (weights_array[perc][GHL + i + 1] < THETA)
                  weights_array[perc][GHL + i + 1]++;
              }
        }
    }
  }

  ghr_update(resolveDir);
  lhr_update(resolveDir, history_no);
}

void PREDICTOR::init_weightarray (void)
{
  for(int i = 0; i < NUMPERCEPTRONS; i++){
	 for(int j = 0; j < NUMWEIGHTS; j++){
 			weights_array[i][j] = 0;
 	  }
  }
}

void PREDICTOR::increment_weights(uint8_t perceptron_no, uint8_t index)
{
  if (weights_array[perceptron_no][index] < THETA)
    weights_array[perceptron_no][index]++;
}

void PREDICTOR::decrement_weights(uint8_t perceptron_no, uint8_t index)
{
  if (weights_array[perceptron_no][index] > (-1 * THETA))
    weights_array[perceptron_no][index]--;
}

void PREDICTOR::ghr_update(bool resolveDir){
  for (int i = GHL - 1; i >= 1; i--){
    ghr[i] = ghr[i - 1];
  }
  if (resolveDir)
    ghr[0] = 1;
  else
    ghr[0] = -1;
}

void PREDICTOR::lhr_update(bool resolveDir, int history_no){
  for (int i = LHL - 1; i >= 1; i--){
    local_histories[history_no][i] = local_histories[history_no][i] ;
  }
  if (resolveDir)
    local_histories[history_no][0] = 1;
  else
    local_histories[history_no][0] = -1;
}

int PREDICTOR::get_perceptron_no(UINT64 PC){
  return (PC & ((1 << INDEXBITS) -1 ));
}

int PREDICTOR::get_history_no(UINT64 PC){
  return (PC & ((1 << HISTORYBITS) - 1));
}

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType OPTYPE, bool branchTaken, UINT64 branchTarget){}

int LoopPredictor::get_prediction(UINT64 PC){
  int index = get_index(PC);
  int tag = get_tag(PC);

 /* if (loop_table[index].tag != tag)
    return -1;*/

  if (loop_table[index].confidence == HIGHCONFIDENCE 
      && loop_table[index].loop_count > 0
      && loop_table[index].iter_count == loop_table[index].loop_count
      && loop_table[index].miss_count < 1000
      && loop_table[index].tag == tag)
     return NOTTAKEN;
  else
    return -1;
}

void LoopPredictor::UpdatePredictor(UINT64 PC, bool resolveDir, bool predDir, bool usedlp){
  int tag = get_tag(PC);
  int index = get_index(PC);

  //if (usedlp && predDir != resolveDir)
    //printf("Index: %d, Tag: %d, Stored Tag: %d, Loop count: %d, Iter Count: %d, confidence: %d, Age:%d\n", index,
    //tag, loop_table[index].tag,  loop_table[index].loop_count,  loop_table[index].iter_count,  loop_table[index].confidence, loop_table[index].age);
  if (loop_table[index].tag == tag){
    //correct
    if (predDir == resolveDir){
      //took branch
     // loop_table[index].miss_count = 0;
      if (resolveDir == TAKEN){
        loop_table[index].iter_count++;
        if (loop_table[index].iter_count > loop_table[index].loop_count && 
            loop_table[index].confidence <= LOWCONFIDENCE)
          loop_table[index].loop_count = loop_table[index].iter_count;
      }
      //did not take
      else{
        incr_confidence(index);
        incr_age(index);
        loop_table[index].iter_count = 0;
      }
    }
    //incorrect
    else{
      loop_table[index].miss_count++;
      if (resolveDir == TAKEN){
        loop_table[index].iter_count++;
      }
      else{
        if (loop_table[index].confidence <= LOWCONFIDENCE){
          loop_table[index].loop_count = loop_table[index].iter_count;
        }
        loop_table[index].iter_count = 0;
      }
      decr_age(index);
      decr_confidence(index);
    }
  }
  else{
    if(loop_table[index].age == 0 && resolveDir == TAKEN){
        loop_table[index].tag = tag;
        loop_table[index].age = INITAGE;
        loop_table[index].iter_count = 1;
        loop_table[index].loop_count = 1;
        loop_table[index].confidence = 0;
        loop_table[index].miss_count = 0;
    } 
    else
      decr_age(index);
  }
}
uint16_t LoopPredictor::get_index(UINT64 PC){
  return (PC & ((1 << LPINDEXBITS) - 1));
}

uint16_t LoopPredictor::get_tag(UINT64 PC){
  return (PC >> LPINDEXBITS) & ((1 << LPTAGBITS) - 1);
}

void LoopPredictor::init_lp(){
  for (int i = 0; i < LPSIZE; i++){
    loop_table[i].iter_count = 0;
    loop_table[i].loop_count = 0;
    loop_table[i].confidence = 0;
    loop_table[i].tag = 0;
    loop_table[i].age = 0;
    loop_table[i].miss_count = 0;
  }
}

void LoopPredictor::incr_confidence(uint16_t index){
  if (loop_table[index].confidence < HIGHCONFIDENCE)
    loop_table[index].confidence++;
}
void LoopPredictor::decr_confidence(uint16_t index){
  if (loop_table[index].confidence > 0)
    loop_table[index].confidence--;
}
void LoopPredictor::incr_age(uint16_t index){
  if (loop_table[index].age < INITAGE)
    loop_table[index].age++;
}
void LoopPredictor::decr_age(uint16_t index){
  if (loop_table[index].age > 0)
    loop_table[index].age--;
}

LoopPredictor::LoopPredictor(void){};