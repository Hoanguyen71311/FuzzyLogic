/* Update to Greg Viot's fuzzy system -- DDJ, February 1993, page 94 */
/* By J. Tucker, P. Fraley, and L. Swanson, April 1993 */
/* Update to avoid reading input/output file, avoid dynamic memmory for micro-controller */
/* By Hoa Nguyen, University of Nebraska-Lincoln, July 2017 */
/* This program has been tested 2 inputs, 1 output, 2 antecdents ANDed together and 1 consequence
   For OR, NOT antecedents and consequences refer to
   https://www.mathworks.com/help/fuzzy/foundations-of-fuzzy-logic.html#bp78l70-2
   for more information and modified rule_evaluation function,
   users may also change program as needed to work with more antecedents and consequences */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UPPER_LIMIT                 255
#define NUMBER_OF_INPUT_OUTPUT      3                  // total system input/output
#define TOTAL_NUMBER_OF_MF          21                 // total of membership functions
#define NUMBER_OF_RULE              15                 // number of rules in ruleBase
#define NUMBER_OF_INPUT             2                  // number of inputs for fuzzy logic
#define NUMBER_OF_OUTPUT            1                  // number of outputs for fuzzy logic
#define NUMBER_OF_MF                7                  // number of membership functions for each input/output
#define NUMBER_OF_IF_SIDE           2                  // number of ifSides for each rule
#define NUMBER_OF_THEN_SIDE         1                  // number of thenSides for each rule
#define TOTAL_NUMBER_OF_IF_SIDE     30                 // total number of ifSides for all rules
#define TOTAL_NUMBER_OF_THEN_SIDE   15                 // total number of thenSide for all rules

typedef struct io_type{
  char *name;
  int value;
  struct mf_type *membership_functions;
  struct io_type *next;
}io_type;

typedef struct mf_type{
  char *name;
  int value;
  int point1;
  int point2;
  int slope1;
  int slope2;
  struct mf_type *next;
}mf_type;

typedef struct rule_type{
  struct rule_element_type *if_side;
  struct rule_element_type *then_side;
  struct rule_type *next;
}rule_type;

typedef struct rule_element_type{
  int *value;
  struct rule_element_type *next;
}rule_element_type;


io_type inputOutput[NUMBER_OF_INPUT_OUTPUT];
mf_type mf[TOTAL_NUMBER_OF_MF];
rule_type ruleBase[NUMBER_OF_RULE];
rule_element_type ifSide[TOTAL_NUMBER_OF_IF_SIDE];
rule_element_type thenSide[TOTAL_NUMBER_OF_THEN_SIDE];
/* rules for fuzzy system declared here
   outer array matches with numberOfRule, innter array matches with numberOfInputOutput */
char *rule[NUMBER_OF_RULE][NUMBER_OF_INPUT_OUTPUT] = {{"NL", "ZE", "PL"},
                                                      {"ZE", "NL", "PL"},
                                                      {"NM", "ZE", "PM"},
                                                      {"ZE", "NM", "PM"},
                                                      {"NS", "ZE", "PS"},
                                                      {"ZE", "NS", "PS"},
                                                      {"NS", "PS", "PS"},
                                                      {"ZE", "ZE", "ZE"},
                                                      {"ZE", "PS", "NS"},
                                                      {"PS", "ZE", "NS"},
                                                      {"PS", "NS", "NS"},
                                                      {"ZE", "PM", "NM"},
                                                      {"NM", "ZE", "NM"},
                                                      {"ZE", "PL", "NL"},
                                                      {"PL", "ZE", "NL"}};

/* Name of each membership function. all inputs and outputs membership functions should be normalized
   http://www.drdobbs.com/cpp/fuzzy-logic-in-c/184408940 refer to this website to understand more about
   why name and point arrays should be normalized */
char *name[NUMBER_OF_MF] = {"NL", "NM", "NS", "ZE", "PS", "PM", "PL"}; // match with numberMf
/* Four points input needed to conver to two points two slopes in initialize_system function
   Inner array always has four elements to construct
   membership functions shape such as triangle or Trapezoid
   outer array matches with numberMf
   These numbers from 2d array point are normalized to range 0-255 */
int point[NUMBER_OF_MF][4] = { {0,    31,   31,   63},
                               {31,   63,   63,   95},
                               {63,   95,   95,   127},
                               {95,   127,  127,  159},
                               {127,  159,  159,  191},
                               {159,  191,  191,  223},
                               {191,  223,  223,  255}};
float max(float a, float b);
float min(float a, float b);
/* all needed functions are declared here */
void initialize_system();
void fuzzification();
void rule_evaluation();
void defuzzification();
void compute_degree_of_membership(mf_type *mf,int input);
int compute_area_of_trapezoid(mf_type *mf);
void initialize_system();
void put_system_outputs();
void get_system_inputs(int input1,int input2);
void compute_degree_of_membership(mf_type *mf,int input);


int main(){
  int angle[2] = {60, 125};
  int velocity[2] = {125, 230};
  for(int i = 0; i < 2; i++){
    initialize_system();
    get_system_inputs(angle[i],velocity[i]);// this function is used for reading input for fuzzy logic, should be normalized to 0-255 range
    fuzzification();
    rule_evaluation();
    defuzzification();
    put_system_outputs();
  }
}

void fuzzification(){
  int k = 0;
  for(int i = 0; i < NUMBER_OF_INPUT; i++)
  {
    for(int j = 0; j < NUMBER_OF_MF; j++)
    {
      compute_degree_of_membership(&mf[k],inputOutput[i].value);
      k++;
    }
  }
}

void rule_evaluation(){
  int a = 0;
  int b = 0;
  int strength;
  int nomatch=0;                   /* NEW, test some rules */
  for(int i = 0; i < NUMBER_OF_RULE; i++)
  {
    strength=UPPER_LIMIT;
    for(int j = 0; j < NUMBER_OF_IF_SIDE; j++)
    {
      strength=min(strength,*(ifSide[a].value));
      a++;
    }
    for(int k = 0; k < NUMBER_OF_THEN_SIDE; k++)
    {
        *(thenSide[b].value)=max(strength,*(thenSide[b].value));      /* NEW */
        if(strength>0)nomatch=1;                      /* NEW */
        b++;
    }
  }
  if(nomatch==0)printf("NO MATCHING RULES FOUND!\n"); /* NEW */
}

void defuzzification(){
  int forOutputMf = TOTAL_NUMBER_OF_MF - NUMBER_OF_MF;
  int sum_of_products;
  int sum_of_areas;
  int area, centroid;
    sum_of_products=0;
    sum_of_areas=0;
    for(int i = forOutputMf; i < TOTAL_NUMBER_OF_MF; i++){
      area=compute_area_of_trapezoid(&mf[i]);
      centroid=mf[i].point1+(mf[i].point2-mf[i].point1)/2;
      sum_of_products+=area*centroid;
      sum_of_areas+=area;
    }
    if(sum_of_areas==0){                                    /* NEW */
      printf("Sum of Areas = 0, will cause div error\n"); /* NEW */
      printf("Sum of Products= %d\n",sum_of_products);    /* NEW */
      inputOutput[2].value=0;                                        /* NEW */
      return;                                             /* NEW */
    }                                                      /* NEW */
    inputOutput[2].value=sum_of_products/sum_of_areas;
}

void compute_degree_of_membership(mf_type *mf, int input){
  int delta_1, delta_2;
  delta_1=input - mf->point1;
  delta_2=mf->point2 - input;
  if((delta_1<=0)||(delta_2<=0))mf->value=0;
  else{
    mf->value=min((mf->slope1*delta_1),(mf->slope2*delta_2));
    mf->value=min(mf->value,UPPER_LIMIT);
    //printf("testing = %d\n", mf->value);
  }
}

int compute_area_of_trapezoid(mf_type *mf){
  int run_1,run_2,area,top;
  int base;
  base=mf->point2 - mf->point1;
  run_1=mf->value / mf->slope1;
  run_2=mf->value / mf->slope2;
  top=base - run_1 - run_2;
  area=mf->value*(base+top)/2;
  //printf("area = %d\n", area);
  return area;
}                                        /* END AREA OF TRAPEZOID */

void initialize_system(){                      /* NEW FUNCTION INITIALIZE */
  int forOutputMf = TOTAL_NUMBER_OF_MF - NUMBER_OF_MF;
  int a, b, c, d, x;
  int k = 0;
  int l = 0;
  /* name of system input/output */
  inputOutput[0].name = "Angle";
  inputOutput[1].name = "Velocity";
  inputOutput[2].name = "Force";
  for(int i = 0; i < NUMBER_OF_INPUT_OUTPUT; i++){
    for(int j = 0; j < NUMBER_OF_MF; j++){
      mf[k].name = name[j];
      mf[k].point1=point[j][0];                   /* left x axis value */
      b = point[j][1];
      c = point[j][2];
      mf[k].point2=point[j][3];                   /* right x axis value */
      mf[k].slope1=UPPER_LIMIT/(b-point[j][0]);     /* left slope */
      mf[k].slope2=UPPER_LIMIT/(point[j][3]-c);     /* right slope */
      k++;
    }
  }
  /* READ RULES FILE; INITIALIZE STRUCTURES */
  for (int i = 0; i < NUMBER_OF_RULE; i++){
    for(int j = 0; j < NUMBER_OF_MF; j++)
    {
      if((strcmp(mf[j].name,rule[i][0]))==0)
      {
        ruleBase[i].if_side=ifSide;
        ifSide[l].value=&mf[j].value;  /* needs address here */
        l++;
        break;                           /* match found */
      }
    }
    for(int h = NUMBER_OF_MF; h < (2*NUMBER_OF_MF); h++)
    {
      if((strcmp(mf[h].name,rule[i][1]))==0)
      {
        ifSide[l].value= &mf[h].value;  /* needs address here */
        l++;
        break;                       /* match found */
      }
    }
    for(int g = forOutputMf; g < TOTAL_NUMBER_OF_MF; g++)
    {
      if((strcmp(mf[g].name,rule[i][2]))==0)
      {
        ruleBase[i].then_side=thenSide;
        thenSide[i].value=&mf[g].value; /* needs address here */
        break;                        /* match found */
      }
    }
  }                                     /* END WHILE READING RULES FILE */
}                                        /* END INITIALIZE */

void put_system_outputs(){                     /* NEW */
  int forOutputMf = TOTAL_NUMBER_OF_MF - NUMBER_OF_MF;
  int a = 0;
  for(int i = 0; i < NUMBER_OF_INPUT; i++)
  {
    printf("%s: Value = %d\n",inputOutput[i].name,inputOutput[i].value);
    for(int j = 0; j < NUMBER_OF_MF; j++)
    {
      printf("  %s: Value %d Left %d Right %d\n",
      mf[a].name,mf[a].value,mf[a].point1,mf[a].point2);
      a++;
    }
    printf("\n");
  }
  for(int i = NUMBER_OF_INPUT; i < NUMBER_OF_INPUT_OUTPUT; i++){
    printf("%s: Value= %d\n",inputOutput[i].name,inputOutput[i].value);
    for(int j = forOutputMf; j < TOTAL_NUMBER_OF_MF; j++){
      printf("  %s: Value %d Left %d Right %d\n",
      mf[j].name,mf[j].value,mf[j].point1,mf[j].point2);
    }
  }
  /* print values pointed to by rule_type (if & then) */
  printf("\n");
  int b = 0;
  int c = 0;
  for(int i = 0; i < NUMBER_OF_RULE; i++){
    printf("Rule #%d:",(i+1));
    for(int j = 0; j < NUMBER_OF_IF_SIDE; j++)
    {
      printf("  %d",*(ifSide[b].value));
      b++;
    }
    for(int k = 0; k < NUMBER_OF_THEN_SIDE; k++){
      printf("  %d\n",*(thenSide[c].value));
      c++;
    }
  }
  printf("\n");
}                                        /* END PUT SYSTEM OUTPUTS */

void get_system_inputs(int input1,int input2){         /* NEW */
  inputOutput[0].value = input1;
  inputOutput[1].value = input2;
}                                        /* END GET SYSTEM INPUTS */

float max(float a, float b){
  float max;
  if(a > b){
    max = a;
  }
  else {
    max = b;
  }
  return max;
}

float min(float a, float b){
  float min;
  if(a > b){
    min = b;
  }
  else {
    min = a;
  }
  return min;
}
