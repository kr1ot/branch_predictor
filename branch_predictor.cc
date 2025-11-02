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
        unsigned long size = pow(2,param.M1);
        //create the chooser table
        gshare_predictor_table = new uint8_t[size];
        //initialize the table with "2"
        for (index=0; index<size;index++)
        {
            gshare_predictor_table[index] = 2;
        }
        mask_gshare = size - 1;
        mask_gshare_bhr = mask_gshare >> (param.M1 - param.N);
        //get only the m-n bits
        mask_gshare = pow(2,(param.M1 - param.N)) - 1;
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

uint32_t BranchPredictor::get_table_index(unsigned long pc, uint32_t mask, unsigned long shift_pc_bits)
{
    uint32_t index;
    index = (uint32_t)((pc >> shift_pc_bits) & mask);
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
    }
    //update the counter value
    count_val_to_update_with = clip(bimodal_predictor_table[index],val_to_add_to_based_on_outcome);
    bimodal_predictor_table[index] = count_val_to_update_with;
}

void BranchPredictor::update_gshare(unsigned long pc,char actual_outcome)
{
    uint32_t pc_bhr_val;
    uint32_t index_from_pc;
    uint32_t index;
    uint8_t count_val_to_update_with;
    int val_to_add_to_based_on_outcome;
    bool is_correct_predict;
    uint8_t outcome_to_store_in_bhr;

    //get only the bits from pc that will be XORed to bhr
    pc_bhr_val = get_table_index(pc,mask_gshare_bhr,(param.M1 - param.N));
    //XOR pc and BHR value
    pc_bhr_val = pc_bhr_val ^ bhr_reg;
    //get the remaining m-n bits from pc 
    index_from_pc = get_table_index(pc,mask_gshare,0);
    //calculate the final index
    index = (pc_bhr_val << (param.M1 - param.N)) | index_from_pc;
    is_correct_predict = is_correct_prediction(gshare_predictor_table[index],actual_outcome);
    
    if (is_correct_predict == false)
    {
        count_mispredictions += 1;
    }

    if (actual_outcome == 't') 
    {
        val_to_add_to_based_on_outcome = 1;
        outcome_to_store_in_bhr = 1;
    }
    else 
    {
        val_to_add_to_based_on_outcome = -1;
        outcome_to_store_in_bhr = 0;
    }


    //update the counter value
    count_val_to_update_with = clip(gshare_predictor_table[index],val_to_add_to_based_on_outcome);
    gshare_predictor_table[index] = count_val_to_update_with;

    //update the bhr register
    //outcome is right shifted into bhr
    bhr_reg = (bhr_reg >> 1) | (outcome_to_store_in_bhr << (param.N - 1));
    
}


void BranchPredictor::call_predictor(unsigned long pc, char actual_outcome)
{
    count_total_predictions += 1;
    if (bimodal_predictor == true) update_bimodal(pc,actual_outcome);
    if (gshare_predictor == true) update_gshare(pc,actual_outcome);
    //if (hybrid_predictor == true) update_hybrid(pc,actual_outcome);
}


void BranchPredictor::print_contents()
{
    if(gshare_predictor == true)
    {
        printf("FINAL GSHARE CONTENTS\n");
        unsigned long size = pow(2,param.M1);
        for (unsigned long index=0; index < size; index++)
        {
            printf(" %lu    %d\n",index,gshare_predictor_table[index]);
        }
    }

    if(bimodal_predictor == true)
    {
        printf("FINAL BIMODAL CONTENTS\n");
        unsigned long size = pow(2,param.M2);
        for (unsigned long index=0; index < size; index++)
        {
            printf(" %lu    %d\n",index,bimodal_predictor_table[index]);
        }
    }
    
}