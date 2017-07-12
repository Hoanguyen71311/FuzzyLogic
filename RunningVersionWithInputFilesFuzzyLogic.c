/* Update to Greg Viot's fuzzy system -- DDJ, February 1993, page 94 */
/* By J. Tucker, P. Fraley, and L. Swanson, April 1993 */
/* By Hoa Nguyen, University of Nebraska Lincoln, July 2017 */
/* This program has been tested 2 inputs, 1 output, 2 antecdents ANDed together and 1 consequence
   For OR, NOT antecedents and consequences refer to
   https://www.mathworks.com/help/fuzzy/foundations-of-fuzzy-logic.html#bp78l70-2
   for more information and modified rule_evaluation function,
   users may also change program as needed to work with more antecedents and consequences */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct io_type *System_Inputs;           /* anchor inputs NEW */
struct io_type *System_Output;           /* anchor output NEW */
#define MAXNAME 10
#define UPPER_LIMIT 255
struct io_type{
  char name[MAXNAME];
  int value;
  struct mf_type *membership_functions;
  struct io_type *next;
};
struct mf_type{
  char name[MAXNAME];
  int value;
  int point1;
  int point2;
  float slope1;
  float slope2;
  struct mf_type *next;
};
struct rule_type{
  struct rule_element_type *if_side;
  struct rule_element_type *then_side;
  struct rule_type *next;
};
struct rule_element_type{
  int *value;
  struct rule_element_type *next;
};
struct rule_type *Rule_Base;
struct rule_type *Rule_Base;
float max(float a, float b);
float min(float a, float b);
void fuzzification();
void rule_evaluation();
void defuzzification();
void compute_degree_of_membership(struct mf_type *mf,int input);
int compute_area_of_trapezoid(struct mf_type *mf);
void initialize_system();
void put_system_outputs();
void get_system_inputs(int input1,int input2);
int main(){
  initialize_system();                  /* Read input files, NEW */
  get_system_inputs(60,125);            // provide input here
  fuzzification();
  rule_evaluation();
  defuzzification();
  put_system_outputs();                 /* print all data, NEW */
}                                        /* END MAIN */
void fuzzification(){
  struct io_type *si;
  struct mf_type *mf;
  for(si=System_Inputs;si!=NULL;si=si->next)
  for(mf=si->membership_functions;mf!=NULL;mf=mf->next)
  compute_degree_of_membership(mf,si->value);
}                                        /* END FUZZIFICATION */
void rule_evaluation(){
  struct rule_type *rule;
  struct rule_element_type *ip;    /* if ptr */
  struct rule_element_type *tp;    /* then ptr */
  int strength;
  int nomatch=0;                   /* NEW, test some rules */
  for(rule=Rule_Base;rule!=NULL;rule=rule->next){
    strength=UPPER_LIMIT;
    for(ip=rule->if_side;ip!=NULL;ip=ip->next){
      strength=min(strength,*(ip->value));
    }
    for(tp=rule->then_side;tp!=NULL;tp=tp->next){
      {  *(tp->value)=max(strength,*(tp->value));      /* NEW */
        if(strength>0)nomatch=1;                      /* NEW */
      }
    }                                            /* NEW */
  }
  if(nomatch==0)printf("NO MATCHING RULES FOUND!\n"); /* NEW */
}                                        /* END RULE EVALUATION */
void defuzzification(){
  struct io_type *so;
  struct mf_type *mf;
  int sum_of_products;
  int sum_of_areas;
  int area, centroid;
  for(so=System_Output;so!=NULL;so=so->next){
    sum_of_products=0;
    sum_of_areas=0;
    for(mf=so->membership_functions;mf!=NULL;mf=mf->next){
      area=compute_area_of_trapezoid(mf);
      centroid=mf->point1+(mf->point2-mf->point1)/2;
      sum_of_products+=area*centroid;
      sum_of_areas+=area;
    }
    if(sum_of_areas==0){                                    /* NEW */
      printf("Sum of Areas = 0, will cause div error\n"); /* NEW */
      printf("Sum of Products= %d\n",sum_of_products);    /* NEW */
      so->value=0;                                        /* NEW */
      return;                                             /* NEW */
    }                                                      /* NEW */
    so->value=sum_of_products/sum_of_areas;
  }
}                                        /* END DEFUZZIFICATION */
void compute_degree_of_membership(struct mf_type *mf, int input){
  int delta_1, delta_2;
  delta_1=input - mf->point1;
  delta_2=mf->point2 - input;
  if((delta_1<=0)||(delta_2<=0))mf->value=0;
  else{
    mf->value=min((mf->slope1*delta_1),(mf->slope2*delta_2));
    mf->value=min(mf->value,UPPER_LIMIT);
  }
}                                        /* END DEGREE OF MEMBERSHIP */
int compute_area_of_trapezoid(struct mf_type *mf){
  int run_1,run_2,area,top;
  int base;
  base=mf->point2 - mf->point1;
  run_1=mf->value / mf->slope1;
  run_2=mf->value / mf->slope2;
  top=base - run_1 - run_2;
  area=mf->value*(base+top)/2;
  return area;
}                                        /* END AREA OF TRAPEZOID */
void initialize_system(){                      /* NEW FUNCTION INITIALIZE */
  int a, b, c, d, x;
  char buff[10],buff1[4],buff2[4];
  static char filename1[]="in1.txt";  /* "angles" filename */
  static char filename2[]="in2.txt";  /* "velocities" filename */
  static char filename3[]="out1.txt"; /* "forces" filename */
  FILE *fp;
  struct io_type *outptr;
  struct mf_type *top_mf;
  struct mf_type *mfptr;
  struct io_type *ioptr;
  struct rule_type *ruleptr;
  struct rule_element_type *ifptr;
  struct rule_element_type *thenptr;
  ioptr=NULL;
  ruleptr=NULL;
  ifptr=NULL;
  thenptr=NULL;
  /* READ THE FIRST FUZZY SET (ANTECEDENT); INITIALIZE STRUCTURES */
  if((fp=fopen(filename1,"r"))==NULL){   /* open "angles" file */

  printf("ERROR- Unable to open data file named %s.\n",filename1);
  exit(0);
}
ioptr=(struct io_type *)calloc(1,sizeof(struct io_type));
System_Inputs=ioptr;                  /* Anchor to top of inputs */
x=fscanf(fp,"%s",buff);               /* from 1st line, get set's name */
sprintf(ioptr->name,"%s",buff);       /* into struct io_type.name */
mfptr=NULL;
while((x=fscanf(fp,"%s %d %d %d %d",buff,&a,&b,&c,&d))!=EOF){/* get line */

  if(mfptr==NULL){                    /* first time thru only */
    mfptr=(struct mf_type *)calloc(1,sizeof(struct mf_type));
    top_mf=mfptr;
    ioptr->membership_functions=mfptr;
  }
  else{
    for(mfptr=top_mf;mfptr->next;mfptr=mfptr->next); /* spin to last */
    mfptr->next=(struct mf_type *)calloc(1,sizeof(struct mf_type));
    mfptr=mfptr->next;
  }
  sprintf(mfptr->name,"%s",buff);    /* membership name, NL, ZE, etc */
  mfptr->point1=a;                   /* left x axis value */
  mfptr->point2=d;                   /* right x axis value */
  printf("a = %d\n", a);
  printf("b = %d\n", b);
  printf("c = %d\n", c);
  printf("d = %d\n", d);
    mfptr->slope1=UPPER_LIMIT/(b-a);     /* left slope */
    mfptr->slope2=UPPER_LIMIT/(d-c);     /* right slope */
}
close(fp);                            /* close "angles" file */
/* READ THE SECOND FUZZY SET (ANTECEDENT); INITIALIZE STRUCTURES */
if((fp=fopen(filename2,"r"))==NULL){   /* open "velocity" file */

printf("ERROR- Unable to open data file named %s.\n",filename2);

exit(0);
}
ioptr->next=(struct io_type *)calloc(1,sizeof(struct io_type));
ioptr=ioptr->next;
x=fscanf(fp,"%s",buff);               /* from 1st line, get set's name */
sprintf(ioptr->name,"%s",buff);       /* into struct io_type.name */
mfptr=NULL;
while((x=fscanf(fp,"%s %d %d %d %d",buff,&a,&b,&c,&d))!=EOF){/* get line */
  if(mfptr==NULL){                    /* first time thru only */
    mfptr=(struct mf_type *)calloc(1,sizeof(struct mf_type));
    top_mf=mfptr;
    ioptr->membership_functions=mfptr;
  }
  else{
    for(mfptr=top_mf;mfptr->next;mfptr=mfptr->next); /* spin to last */
    mfptr->next=(struct mf_type *)calloc(1,sizeof(struct mf_type));
    mfptr=mfptr->next;
  }
  sprintf(mfptr->name,"%s",buff);    /* membership name, NL, ZE, etc */
  mfptr->point1=a;                   /* left x axis value */
  mfptr->point2=d;                   /* right x axis value */
  if(b-a>0) mfptr->slope1=UPPER_LIMIT/(b-a);     /* left slope */
  else{
    printf("Error in input file %s, membership element %s.\n",
    filename2,buff);
    exit(1);
  }
  if(d-c>0) mfptr->slope2=UPPER_LIMIT/(d-c);     /* right slope */
  else{
    printf("Error in input file %s, membership element %s.\n",
    filename2,buff);
    exit(1);
  }
}
close(fp);                            /* close "velocity" file */
/* READ THE THIRD FUZZY SET (CONSEQUENCE); INITIALIZE STRUCTURES */
if((fp=fopen(filename3,"r"))==NULL)   /* open "force" file */
{
  printf("ERROR- Unable to open data file named %s.\n",filename3);
  exit(0);
}
ioptr=(struct io_type *)calloc(1,sizeof(struct io_type));
System_Output=ioptr;                  /* Anchor output structure */
x=fscanf(fp,"%s",buff);               /* from 1st line, get set's name */
sprintf(ioptr->name,"%s",buff);       /* into struct io_type.name */
mfptr=NULL;
while((x=fscanf(fp,"%s %d %d %d %d",buff,&a,&b,&c,&d))!=EOF){/* get line */
  if(mfptr==NULL){                    /* first time thru */
    mfptr=(struct mf_type *)calloc(1,sizeof(struct mf_type));
    top_mf=mfptr;
    ioptr->membership_functions=mfptr;
  }
  else{
    for(mfptr=top_mf;mfptr->next;mfptr=mfptr->next);
    mfptr->next=(struct mf_type *)calloc(1,sizeof(struct mf_type));
    mfptr=mfptr->next;
  }
  sprintf(mfptr->name,"%s",buff);    /* membership name, NL, ZE, etc */
  mfptr->point1=a;                   /* left x axis value */
  mfptr->point2=d;                   /* right x axis value */
  if(b-a>0) mfptr->slope1=UPPER_LIMIT/(b-a);     /* left slope */
  else{
    printf("Error in input file %s, membership element %s.\n",
    filename3,buff);
    exit(1);
  }
  if(d-c>0) mfptr->slope2=UPPER_LIMIT/(d-c);     /* right slope */
  else{
    printf("Error in input file %s, membership element %s.\n",
    filename3,buff);
    exit(1);
  }
}
close(fp);                            /* close "force" file */
/* READ RULES FILE; INITIALIZE STRUCTURES */
ioptr=NULL;
outptr=NULL;
if((fp=fopen("rules.txt","r"))==NULL){     /* open rules file */
  printf("ERROR- Unable to open data file named %s.\n","rules");
  exit(0);
}
ruleptr=(struct rule_type *)calloc(1,sizeof(struct rule_type));
if(ioptr==NULL)Rule_Base=ruleptr;     /* first time thru, anchor */
while((x=fscanf(fp,"%s %s %s",buff,buff1,buff2))!=EOF){ /* get a line */
  ioptr=System_Inputs;               /* points to angle */
  for(mfptr=ioptr->membership_functions;mfptr!=NULL;mfptr=mfptr->next){
    if((strcmp(mfptr->name,buff))==0){
      ifptr=(struct rule_element_type *)
      calloc(1,sizeof(struct rule_element_type));
      ruleptr->if_side=ifptr;      /* points to angle */
      ifptr->value=&mfptr->value;  /* needs address here */
      ifptr->next=(struct rule_element_type *)
      calloc(1,sizeof(struct rule_element_type));
      ifptr=ifptr->next;
      break;                       /* match found */
    }
  }
  ioptr=ioptr->next;                 /* points to velocity */
  for(mfptr=ioptr->membership_functions;mfptr!=NULL;mfptr=mfptr->next){
    if((strcmp(mfptr->name,buff1))==0){
      ifptr->value=&mfptr->value;  /* needs address here */
      break;                       /* match found */
    }
  }
  if(outptr==NULL)outptr=System_Output;/* point then stuff to output */
  for(mfptr=outptr->membership_functions;mfptr!=NULL;mfptr=mfptr->next){
    if((strcmp(mfptr->name,buff2))==0){
      thenptr=(struct rule_element_type *)
      calloc(1,sizeof(struct rule_element_type));
      ruleptr->then_side=thenptr;
      thenptr->value=&mfptr->value; /* needs address here */
      break;                        /* match found */
    }
  }
  ruleptr->next=(struct rule_type *)calloc(1,sizeof(struct rule_type));
  ruleptr=ruleptr->next;
}                                     /* END WHILE READING RULES FILE */
close(fp);                            /* close "rules" file */
}                                        /* END INITIALIZE */
void put_system_outputs(){                     /* NEW */
  struct io_type *ioptr;
  struct mf_type *mfptr;
  struct rule_type *ruleptr;
  struct rule_element_type *ifptr;
  struct rule_element_type *thenptr;
  int cnt=1;
  for(ioptr=System_Inputs;ioptr!=NULL;ioptr=ioptr->next){
    printf("%s: Value= %d\n",ioptr->name,ioptr->value);
    for(mfptr=ioptr->membership_functions;mfptr!=NULL;mfptr=mfptr->next){
      printf("  %s: Value %d Left %d Right %d\n",
      mfptr->name,mfptr->value,mfptr->point1,mfptr->point2);
    }
    printf("\n");
  }
  for(ioptr=System_Output;ioptr!=NULL;ioptr=ioptr->next){
    printf("%s: Value= %d\n",ioptr->name,ioptr->value);
    for(mfptr=ioptr->membership_functions;mfptr!=NULL;mfptr=mfptr->next){
      printf("  %s: Value %d Left %d Right %d\n",
      mfptr->name,mfptr->value,mfptr->point1,mfptr->point2);
    }
  }
  /* print values pointed to by rule_type (if & then) */
  printf("\n");
  for(ruleptr=Rule_Base;ruleptr->next!=NULL;ruleptr=ruleptr->next){
    printf("Rule #%d:",cnt++);
    for(ifptr=ruleptr->if_side;ifptr!=NULL;ifptr=ifptr->next)
    printf("  %d",*(ifptr->value));
    for(thenptr=ruleptr->then_side;thenptr!=NULL;thenptr=thenptr->next)
    printf("  %d\n",*(thenptr->value));
  }
  printf("\n");
}                                        /* END PUT SYSTEM OUTPUTS */
void get_system_inputs(int input1,int input2){         /* NEW */

  struct io_type *ioptr;
  ioptr=System_Inputs;
  ioptr->value=input1;
  ioptr=ioptr->next;
  ioptr->value=input2;
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
