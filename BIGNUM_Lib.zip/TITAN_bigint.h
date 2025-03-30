#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "time.h"
#include "config.h"

typedef struct
{
    int sign;
    int wordlen;
    word* N;
}bigint;

void BI_new(bigint** x, int wordlen);
void BI_delete(bigint** x);
void BI_refine(bigint* x);
void BI_assign(bigint** y, bigint* x);

void BI_set_one(bigint** x);
void BI_set_zero(bigint** x);
int Is_Zero(bigint* x);
int Is_One(bigint* x);

//int compare_core(bigint* a, bigint* b);
int BI_compare(bigint* x, bigint* y);

int BI_leftShift(bigint** x, int r);
int BI_rightShift(bigint** x, int r);

int BI_reduction(bigint* x, int r);
int hexStr_to_hexArr(char* hexStr, word* hexArr, int wordlen);
int BI_set_by_array(bigint** x, int sign, const word *a, int wordlen);
int BI_set_by_string(bigint** x, int sign, char* a, int base);

void BI_show_hex(bigint* x);
void BI_show_bin(bigint* x);
void array_rand(word* dst, int wordlen);
void BI_gen_rand(bigint** x, int wordlen);
void array_new_rand(word **const a, const int wordlen);