#include "predictor.h"
#include <math.h>

PREDICTOR::PREDICTOR (void)
{
  numbranches = 0;
  ghr = 0;
  for(int i = 0; i < GA_SIZE; i++)
  {
    GA[i] = 0;
  }

}

bool PREDICTOR::GetPrediction (UINT64 PC)
{
  numbranches++;
  output = weights_array[PC & ((1 << ADDRESSBITS)-1)][0][0];
  for(int i = 0; i < GHL; i++)
  {
    if((ghr >> i) & 1)
    {
      output += weights_array[PC & ((1 << ADDRESSBITS)-1)][GA[i]][i + 1];
    }
    else{
       output -= weights_array[PC & ((1 << ADDRESSBITS)-1)][GA[i]][i + 1];
   	}
  }
 
  if(output >= 0)
  	return true;
  else 
  	return false;
}

void PREDICTOR::UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir, bool predDir, UINT64 branchTarget)
{
if (predDir == resolveDir)
  printf("%d\n", output);

 if(abs(output) < theta || (predDir != resolveDir))
 {  
   if(resolveDir == TAKEN)
   {
   	  increment_weights(PC & ((1 << ADDRESSBITS)-1),0); //weights_array[PC & ((1 << HISTORY_LENGTH)-1)][0][0] = weights_array[PC & ((1 << HISTORY_LENGTH)-1)][0][0] + 1;
   }
   else
   {
   	  decrement_weights(PC & ((1 << ADDRESSBITS)-1),0);//weights_array[PC & ((1 << HISTORY_LENGTH)-1)][0][0] = weights_array[PC & ((1 << HISTORY_LENGTH)-1)][0][0] - 1;
   }
   for(int i=0; i < GHL; i++)
   {
   	 if((ghr >> i) & 1)
   	 {
   	   increment_weights(PC & ((1 << ADDRESSBITS)-1),i); //	weights_array[PC & ((1 << HISTORY_LENGTH)-1)][GA[i]][i] = weights_array[PC & ((1 << HISTORY_LENGTH)-1)][GA[i]][i] + 1;
   	 }
   	 else
   	 {
   	   decrement_weights(PC & ((1 << ADDRESSBITS)-1),i);  //	weights_array[PC & ((1 << HISTORY_LENGTH)-1)][GA[i]][i] = weights_array[PC & ((1 << HISTORY_LENGTH)-1)][GA[i]][i] - 1;
   	 }
   }

 }
   ga_update(PC & ((1 << ADDRESSBITS) - 1));// move everything over by one
   ghr_update(resolveDir);

}

void PREDICTOR::init_weightarray (void)
{
 for(int i = 0; i < NUMADDRESSES; i++)
 {
	for(int j = 0; j < NUMADDRESSES; j++)
 	{
 		for(int k = 0; k < TABLESIZE; k++)
 		{
 			weights_array[i][j][k] = 0;
 		}
 	}
 }
}

void PREDICTOR::increment_weights(uint8_t address, uint8_t index)
{
  if (index == 0){
    if (weights_array[address][0][0] < 63)
      weights_array[address][0][0]++;
  }
  else{
    if (weights_array[address][GA[index]][index+1] < 63)
      weights_array[address][GA[index]][index+1]++;
  }
}
void PREDICTOR::decrement_weights(uint8_t address, uint8_t index)
{
  if (index == 0){
    if (weights_array[address][0][0] > -64)
    weights_array[address][0][0]--;
  }
  else{
    if (weights_array[address][GA[index]][index+1] > -64)
      weights_array[address][GA[index]][index+1]--;
  }
}

void PREDICTOR::ga_update(uint8_t addr_bits)
{
	for(int i = GA_SIZE; i >= 1; i--){
		GA[i] = GA[i-1];
	}   
	GA[0] = addr_bits;
}

void PREDICTOR::ghr_update(bool resolveDir){
  ghr <<= 1;
  if (resolveDir == TAKEN)
    ghr++;

 // printf("%llu\n", ghr);
}

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType OPTYPE, bool branchTaken, UINT64 branchTarget){}