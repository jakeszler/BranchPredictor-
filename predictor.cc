#include "predictor.h"
#include <math.h>

PREDICTOR::PREDICTOR (void)
{
  numbranches = 0;
  for (int i = 0; i < GHL; i++){
    ghr[i] = 0;
  }
  init_weightarray();
}

bool PREDICTOR::GetPrediction (UINT64 PC)
{
  numbranches++;
  int perc = get_perceptron_no(PC);
  output = weights_array[perc][0];
  for (int i = 0; i < GHL; i++){
      output += ghr[i] * weights_array[perc][i + 1];
    }
  return (output >= 0);
}

void PREDICTOR::UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir, bool predDir, UINT64 branchTarget)
{
    
  int perc = get_perceptron_no(PC);

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
  }
  ghr_update(resolveDir);
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

int PREDICTOR::get_perceptron_no(UINT64 PC){
  return (PC & ((1 << INDEXBITS) -1 ));
}

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType OPTYPE, bool branchTaken, UINT64 branchTarget){}