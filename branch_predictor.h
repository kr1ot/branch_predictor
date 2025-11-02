#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "sim_bp.h"

//class that encases all the branch predictors
class BranchPredictor{
    //member variables of class
    private:
        //2d arrays
        uint8_t *bimodal_predictor_table = nullptr;
        uint8_t *gshare_predictor_table = nullptr;
        uint8_t *hybrid_chooser_table = nullptr;

        //bhr register for gshare
        unsigned long bhr_reg=0;
        //define masks for all the predictors to get index for tables
        uint32_t mask_bimodal=0;
        uint32_t mask_gshare_bhr=0; //mask for bhr
        uint32_t mask_gshare=0; //mask for gshare
        uint32_t mask_hybrid=0;

        //flags that indicate the type of predictor
        bool bimodal_predictor = false;
        bool gshare_predictor = false;
        bool hybrid_predictor = false;

        bp_params param;
        
    //member functions for class 
    public:
        uint32_t count_total_predictions;
        uint32_t count_mispredictions;

        //constructor for passing the values from main sim class
        BranchPredictor(bp_params);
        //create the predictor tables
        void create_predictor_table(bp_params);
        //initialize the predictor table
        void initialize_predictor_table();

        //clip the values of the counter
        uint8_t clip(uint8_t, int);
        uint32_t get_table_index(unsigned long, uint32_t, unsigned long);

        //functions to update the predictors
        uint32_t get_bimodal_index(unsigned long);
        void update_bimodal(char,uint32_t);
        uint32_t get_gshare_index(unsigned long);
        void update_gshare(char,uint32_t);
        
        void update_hybrid(unsigned long, char);

        //main function that performs prediction
        //inputs are pc and actual outcome
        void call_predictor(unsigned long pc,char);

        char is_correct_prediction(uint8_t, char);
        
        void print_contents();

};

#endif