
#include "TITAN_bigint.h"

// BigInt ADDITION
int ADD_get_carry(word* dst, word a, word b, unsigned int carry);
int ADD_core(bigint** dst, bigint* x, bigint* y);
int BI_ADD(bigint** dst, bigint* x, bigint* y);

// BigInt SUBTRACTION
int SUB_get_borrow(word* dst, word a, word b, unsigned int borrow);
int SUB_core(bigint** dst, bigint* x, bigint* y);
int BI_SUB(bigint** dst, bigint* x, bigint* y);

// BigInt MULTIPLICATION
int MUL_single_word(word* c, word a, word b);
int MUL_improved_textbook(bigint** dst, bigint* x, bigint* y);
int BI_MUL(bigint **dst, bigint* x, bigint* y);
int BI_MUL_karatsuba(bigint** dst, bigint* x, bigint* y, int flag);

int BI_DIV_2word(word* q, bigint* x, word* b);
int BI_DIVCC(bigint** q, bigint** r, bigint* x, bigint* y);
int BI_DIVC(bigint **q, bigint **r, bigint *x, bigint *y);
int BI_DIV(bigint** q, bigint** r, bigint* x, bigint* y);


int Barrett_RED(bigint** R, bigint* x, bigint* N, bigint* T);