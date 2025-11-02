#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sim_bp.h"
#include "branch_predictor.h"


BranchPredictor::BranchPredictor(bp_params param)
{
    this->param = param;
    //decide the type of predictor based on the inputs
    if (strcmp(param.bp_name, "hybrid") == 0)
    {
        hybrid_predictor = true;
        gshare_predictor = true;
        bimodal_predictor = true;
    }
    else if(strcmp(param.bp_name, "gshare") == 0)
    {
        gshare_predictor = true;
    }
    else
    {
        bimodal_predictor = true;
    }
    create_predictor_table(param);
}

void BranchPredictor::create_predictor_table(bp_params param)
{
    unsigned long index;
    //based on the available predictors,
    //create the predictor table
    if (hybrid_predictor == true)
    {
        //create the chooser table
        hybrid_chooser_table = new uint8_t[param.K];
        //initialize the table with "1"
        for (index=0; index<param.K;index++)
        {
            hybrid_chooser_table[index] = 1;
        }
        mask_hybrid = pow(2,param.K) - 1;
    }
    
    if (gshare_predictor == true)
    {
        //create the chooser table
        gshare_predictor_table = new uint8_t[param.M1];
        //initialize the table with "2"
        for (index=0; index<param.M1;index++)
        {
            gshare_predictor_table[index] = 2;
        }
        mask_gshare = pow(2,param.M1) - 1;
        mask_gshare_bhr = mask_gshare >> (param.M1 - param.N);
    }

    if (bimodal_predictor == true)
    {
        unsigned long size = pow(2,param.M2);
        //create the bimodal predictor table
        bimodal_predictor_table = new uint8_t[size];
        //initialize the table with "2"
        for (index=0; index<size;index++)
        {
            bimodal_predictor_table[index] = 2;
        }
        mask_bimodal = size - 1;
    }

}

uint32_t BranchPredictor::get_table_index(unsigned long pc, uint32_t mask, unsigned long bits)
{
    uint32_t index;
    index = (uint32_t)((pc & (unsigned long)mask) >> bits);
    return index;
}

uint8_t BranchPredictor::clip(uint8_t count_value, int value_to_add)
{   
    int count_value_after_addition;
    // printf("Counter value at start = %d and value to add = %d\n",count_value,value_to_add);
    //add the offset
    count_value_after_addition = count_value + value_to_add;
    // printf("Counter value after addition = %d\n",count_value_after_addition);

    //upper saturate to 3
    if (count_value_after_addition > 3) count_value_after_addition = 3;
    //lower saturate to 0
    else if (count_value_after_addition < 0) count_value_after_addition = 0;
    // printf("Counter value at end = %d\n",count_value_after_addition);
    return (uint8_t)count_value_after_addition;
}

char BranchPredictor::is_correct_prediction(uint8_t counter_value, char actual_outcome)
{
    char predicted_outcome;
    if (counter_value >= 2) predicted_outcome = 't';
    else predicted_outcome = 'n';

    if (predicted_outcome == actual_outcome) return true;
    else return false;
}
void BranchPredictor::update_bimodal(unsigned long pc,char actual_outcome)
{
    uint32_t index;
    uint8_t count_val_to_update_with;
    int val_to_add_to_based_on_outcome;
    bool is_correct_predict;
    index = get_table_index(pc,mask_bimodal,0);
    is_correct_predict = is_correct_prediction(bimodal_predictor_table[index],actual_outcome);

    if (actual_outcome == 't') val_to_add_to_based_on_outcome = 1;
    else val_to_add_to_based_on_outcome = -1;

    if (is_correct_predict == false)
    {
        count_mispredictions += 1;
        // printf("**mispredictions = %u\n",count_mispredictions); 
    }
    //update the counter value
    count_val_to_update_with = clip(bimodal_predictor_table[index],val_to_add_to_based_on_outcome);
    bimodal_predictor_table[index] = count_val_to_update_with;
}

void BranchPredictor::call_predictor(unsigned long pc, char actual_outcome)
{
    count_total_predictions += 1;
    update_bimodal(pc,actual_outcome);
}


void BranchPredictor::print_contents()
{
    if(bimodal_predictor == true)
    {
        printf("FINAL BIMODAL CONTENTS\n");
        unsigned long size = pow(2,param.M2);
        for (unsigned long index=0; index < size; index++)
        {
            printf(" %lu    %d\n",index,bimodal_predictor_table[index]);
        }
    }
    else if(gshare_predictor == true)
    {
        printf("FINAL GSHARE CONTENTS");
    }
    
}