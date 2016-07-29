#ifndef _TRAIN_LINEAR_H
#define _TRAIN_LINEAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include<iostream>
#include "TlibCommon/linear.h"

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
#define INF HUGE_VAL

using namespace std;


static char *line = NULL;
static int max_line_len;

struct feature_node *x_space;
struct parameter param;
struct problem prob;
struct model* model_;
int flag_cross_validation;
int flag_find_C;
int flag_C_specified;
int flag_solver_specified;
int nr_fold;
double bias;






void print_null(const char *s);

void exit_with_help();

void exit_input_error(int line_num);

static char* readline(FILE *input);

int train_model(char* input_file_name, char* model_file_name);

void do_find_parameter_C();

void do_cross_validation();

void parse_command_line(int argc, char **argv, char *input_file_name, char *model_file_name); 

// read in a problem (in libsvm format)
void read_problem(const char *filename);


#ifdef __cplusplus
}
#endif

#endif 