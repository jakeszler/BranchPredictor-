#include "predictor.h"
#include <math.h>

PREDICTOR::PREDICTOR (void)
{

  ghr = 0;

  for(int i = 0; i< GA_SIZE; i++)
  {
    GA[i] = 0;
  }
}

bool PREDICTOR::GetPrediction (UINT64 PC)
{
  output = 0;
  for(int i=0; i < GHL; i++)
  {
    if(ghr & (1<<i))
    {
      output = output + weights_array[PC & ((1 << ADDRESSBITS)-1)][GA[i]][i];
    }
    else
   	{
       output = output - weights_array[PC & ((1 << ADDRESSBITS)-1)][GA[i]][i];
   	}
  }
  if(output >= 0)
  	return true;
  else 
  	return false;
}

void PREDICTOR::UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir, bool predDir, UINT64 branchTarget)
{
 if(abs(output) < theta || (output >= 0) != TAKEN)
 {
   if(TAKEN)
   {
   	  increment_weights(PC & ((1 << ADDRESSBITS)-1),0); //weights_array[PC & ((1 << HISTORY_LENGTH)-1)][0][0] = weights_array[PC & ((1 << HISTORY_LENGTH)-1)][0][0] + 1;
   }
   else
   {
   	  decrement_weights(PC & ((1 << ADDRESSBITS)-1),0);//weights_array[PC & ((1 << HISTORY_LENGTH)-1)][0][0] = weights_array[PC & ((1 << HISTORY_LENGTH)-1)][0][0] - 1;
   }
   for(int i=0; i < GHL; i++)
   {
   	 if(ghr & (1<<i))
   	 {
   	   increment_weights(PC & ((1 << ADDRESSBITS)-1),i); //	weights_array[PC & ((1 << HISTORY_LENGTH)-1)][GA[i]][i] = weights_array[PC & ((1 << HISTORY_LENGTH)-1)][GA[i]][i] + 1;
   	 }
   	 else
   	 {
   	   decrement_weights(PC & ((1 << ADDRESSBITS)-1),i);  //	weights_array[PC & ((1 << HISTORY_LENGTH)-1)][GA[i]][i] = weights_array[PC & ((1 << HISTORY_LENGTH)-1)][GA[i]][i] - 1;
   	 }
   }

 }
   ga_update((1 << ADDRESSBITS)-1);// move everything over by one
   ghr_update(resolveDir);

}

void PREDICTOR::init_weightarray (void)
{
 for(int i = 0; i < TABLESIZE; i++)
 {
	for(int j = 0; j < TABLESIZE; j++)
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
  weights_array[address][GA[index]][index+1]++;
}
void PREDICTOR::decrement_weights(uint8_t address, uint8_t index)
{
  weights_array[address][GA[index]][index+1]--;
}

void PREDICTOR::ga_update(uint8_t address)
{
	for(int i = GA_SIZE; i > 1; i--)
	{
		GA[i] = GA[i-1];
	}   
	GA[0] = address;
}


void PREDICTOR::ghr_update(bool resolveDir){
  ghr <<= 1;
  if (resolveDir == TAKEN)
    ghr++;
}