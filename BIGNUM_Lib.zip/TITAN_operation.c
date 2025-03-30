
#include "TITAN_operation.h"

/*======================ADDITION======================*/

// get new carry + word단위 덧셈
int ADD_get_carry(word* dst, word a, word b, unsigned int carry) // dst = a + b + carry (mod2^32) , newcarray
{
    unsigned int new_carry = 0;

    (*dst) = a + b; // dst = a+b mod W

    if((*dst) < a) 
    {
        new_carry = 1;
    }

    (*dst) = (*dst) + carry;

    if((*dst) < carry)
    {
        new_carry = new_carry + 1;
    }

    return new_carry;
}

int ADD_core(bigint** dst, bigint* x, bigint* y)   // wordlen(x) >= wordlen(y), dst = x + y
{
    int carry = 0;
    int n = x->wordlen;
    int m = y->wordlen;
    bigint* y_temp = NULL;
    
    BI_new(dst, n+1);
    
    if(n == m) BI_assign(&y_temp, y);
    else if (n > m)
    {
        BI_new(&y_temp, n);
        for(int i=0; i<m; i++) y_temp->N[i]=y->N[i];
        //memcpy(y_temp->N, y->N, m);
        for(int i=m; i<n; i++)
        {
            y_temp->N[i] = 0;
        }
    }
    else
    {
        BI_delete(&y_temp);
        return 0;
    }

    for (int j=0; j<n; j++)
    {
        carry = ADD_get_carry(&((*dst)->N[j]), x->N[j], y_temp->N[j], carry);  // dst[j] = a[j] + b[j] 그대로 한값
    }
    (*dst)->N[n] = carry;    // carry값이 1일때, C의 wordlen = n+1
    
    BI_refine(*dst);
    BI_delete(&y_temp);

    return 1;
}

int BI_ADD(bigint** dst, bigint* x, bigint* y)
{

    bigint* temp = NULL;

    // 부호가 다르면, sub으로 보내준다.
    if(x->sign==NONNEGATIVE && y->sign==NEGATIVE)
    {
        BI_assign(&temp, y);
        temp->sign=NONNEGATIVE;    // 절댓값 |b|
        BI_SUB(dst,x,temp);              // A - |B|
    }
    else if(x->sign==NEGATIVE && y->sign==NONNEGATIVE)
    {
        BI_assign(&temp, x);
        temp->sign=NONNEGATIVE;   //절댓값 |a|
        BI_SUB(dst,y,temp);              // B - |A|
    }

    // 최종 덧셈 부분 : c = a+b , wordlen 비교해서 큰값을 왼쪽에 배치하기
    else if(x->wordlen >= y->wordlen)
    {
        ADD_core(dst, x, y);
        (*dst)->sign = x->sign;
    }
    else
    {
        ADD_core(dst, y, x);
        (*dst)->sign = x->sign; 
    }
    
    BI_delete(&temp);
    return 1;

}


/*====================================================*/
/*====================SUBTRACTION=====================*/

// get new borrow + word단위 뺄셈
int SUB_get_borrow(word* dst, word a, word b, unsigned int borrow)
{
    unsigned int new_borrow =0;
    (*dst) = a - borrow;

    if (a < borrow)   new_borrow += 1;
    if((*dst) < b)   new_borrow = new_borrow + 1;

    (*dst) = (*dst) - b;

    return new_borrow;
}

int SUB_core(bigint** dst, bigint* x, bigint* y) // n>m
{
    int borrow ;
    int n = x->wordlen;
    int m = y->wordlen;

    BI_new(dst, n);

    // y_temp는 x와 크기가 같은 y값을 가진 bigint
    bigint* y_temp = NULL;
    if(n == m){
        BI_assign(&y_temp, y);
    }
    else if(n>m){ 
        BI_new(&y_temp, n); // y_tmep의 wordlen = n
        y_temp->sign = y->sign; // y_temp의 sign = y의 sign
        for(int i=0; i<m; i++)
        {
            y_temp->N[i]= y->N[i]; // y_temp[N]값 = y[N]값
        }
        for(int j=m; j<n; j++)
        {
            y_temp->N[j]=0; // y_temp의 남은 값은 모두 0으로 넣어주기
        }

    }

    
    // borrow 0으로 초기화
    borrow=0;

    for(int j=0; j<n; j++)
    {
        borrow = SUB_get_borrow(&((*dst)->N[j]), x->N[j], y_temp->N[j], borrow);

    }
    BI_delete(&y_temp);
    BI_refine(*dst);
    return 1;
}

int BI_SUB(bigint** dst, bigint* x, bigint* y)
{   

    //부호가 같다면-
    if (x->sign == y->sign)
    {   
        // 모두 양수일때
        if(x->sign==NONNEGATIVE)
        {
            //x>y -> 7-3
            if(BI_compare(x,y)==1)
            {
                SUB_core(dst, x, y);
                (*dst)->sign = NONNEGATIVE;
                return 1;
            }
            //x<y -> 3-7
            else if(BI_compare(x,y)==-1)
            {
                SUB_core(dst, y, x);
                (*dst)->sign = NEGATIVE;
                return 1;
            }
        }
        // 모두 음수일때
        else{
            // x>y -> (-3)-(-5)= 5-3 = 2
            if(BI_compare(x,y)==1)          
            {
                SUB_core(dst, y, x);
                (*dst)->sign = NONNEGATIVE;
                return 1;
            }
            
            // x<y -> (-5)-(-3) = -2
            else if(BI_compare(x,y)==-1)
            {
                SUB_core(dst, x, y);
                (*dst)->sign = NEGATIVE;
                return 1;
            }
        }
    }
    //부호가 다르다면-
    else{

        
        //ADD는 부호가 갈을 떄로 바꿔줘야함.
        if ((x->wordlen) < (y->wordlen)){
            ADD_core(dst, y, x);
        }
        else{
            ADD_core(dst, x, y);
        }
        
        
        // x>|y| -> 5-(-3) = 8

        if(BI_compare(x,y)==1){
            (*dst)->sign = NONNEGATIVE; 
            return 1;
        }              
        // x<|y| -> (-5)-3 = -8
        else if(BI_compare(x,y)==-1){
            
            (*dst)->sign = NEGATIVE;    
            return 1; 
        }        
    }
}



/*====================================================*/
/*===================MULTIPLICATION===================*/
int MUL_single_word(word* c, const word a, const word b)
{
    word a0, a1;    // a0 <- 하위 16비트, a1 <- 상위 16비트
    word b0, b1;
    word t0, t1, temp;
    //int w = sizeof(word);   // word : unsigned int -> w = 4
    memset(c, 0, sizeof(c));

    a0 = a % (0x1<<(WORDBITS/2));
    a1 = (a >> (WORDBITS/2));  // a >> (sizeof(word)*(sizeof(byte)/2)

    b0 = b % (0x1<<(WORDBITS/2));
    b1 = (b >> (WORDBITS/2));
       
    t0 = a0 * b1;
    t1 = a1 * b0;

    t0 = (t0 + t1) % ((unsigned long long)0x1<<WORDBITS);  // t0 + t1 (mod 2^w)
    t1 = (t0 < t1); // t1 = {0,1} : (t0 >2^w)일 경우, mod연산으로 날아간 2^w 를 나중에 더해주기 위함

    c[0] = a0 * b0;
    c[1] = a1 * b1;
     
    temp = c[0];
    c[0] = (c[0] + (t0 << (WORDBITS/2))) % ((unsigned long long)0x1<<WORDBITS);
    c[1] = c[1] + (t1<<(WORDBITS/2)) + (t0>>(WORDBITS/2)) + (c[0]<temp);   // temp = {0,1} : 위의 t1과 같은 역할

    return 1;
}

int MUL_improved_textbook(bigint** dst, bigint* x, bigint* y)
{
    bigint* X = NULL;
    bigint* Y = NULL;  // new x, y
    BI_assign(&X, x);
    BI_assign(&Y, y);
    int wordLen;

    if((X->wordlen%2)!=0)
    {
        wordLen = X->wordlen + 1;
        X->N = (word*)realloc(X->N, sizeof(word)*wordLen);
        X->N[wordLen-1] = 0;
        X->wordlen = wordLen;
    }
    if((Y->wordlen%2)!=0)
    {
        wordLen = Y->wordlen + 1;
        Y->N = (word*)realloc(Y->N, sizeof(word)*wordLen);
        Y->N[wordLen-1] = 0;
        Y->wordlen = wordLen;
    }

    int n = X->wordlen;
    int m = Y->wordlen;

    bigint* t0 = NULL;
    bigint* t1 = NULL;
    bigint* T = NULL;
    bigint* TEMP = NULL;
    word temp[2];

    BI_new(&t0, n);
    BI_new(&t1, n + 1);

    BI_set_zero(&TEMP);

    for(int i=0; i<m; i++)
    {
        for(int k=0; k<n/2; k++)
        {
            MUL_single_word(temp, X->N[2*k], Y->N[i]);
            t0->N[2*k] = temp[0];
            t0->N[2*k+1] = temp[1];
            
            memset(temp, 0, sizeof(temp));

            MUL_single_word(temp, X->N[2*k+1], Y->N[i]);
            t1->N[2*k+1] = temp[0];
            t1->N[2*(k+1)] = temp[1];
        }
        
        BI_ADD(&T, t0, t1);
        
        BI_leftShift(&T, i*WORDBITS);
        
        //printf("%08x", T->N[0]);
        BI_ADD(dst, T, TEMP);
        BI_assign(&TEMP, (*dst));
        //memcpy((*dst)->N, TEMP->N, sizeof(word)*(TEMP->wordlen));
    }

    BI_refine((*dst));
    BI_delete(&X);
    BI_delete(&Y);
    BI_delete(&T);
    BI_delete(&t0);
    BI_delete(&t1);
    BI_delete(&TEMP);

    return 1;
}

int BI_MUL(bigint **dst, bigint* x, bigint* y)
{
    //case: x=0 or y=0
    if ( Is_Zero(x) == 1 || Is_Zero(y) == 1 ){
        BI_set_zero(dst);
        return 1;
    }

    //case: |x| = 1
    if ( Is_One(x) == 1 ){
        BI_assign(dst, y);
        return 1;
    }

    //case: y = +1
    if ( Is_One(y) == 1 ){
        BI_assign(dst, x);
        return 1;
    }
    
    bigint* minus = NULL;
    BI_assign(&minus, x);
    minus->sign = NONNEGATIVE;

    //case: x = -1
    if( Is_One(minus) == 1 )
    {
        BI_assign(dst, y);
        (*dst)->sign = 1 - (y->sign);
        // y->sign = NONNEGATIVE : 1-0 = 1(NEGATIVE)
        // y->sign = NEGATIVE : 1-1 = 0(NONNEGATIVE)
        BI_delete(&minus);
        return 1;
    }

    BI_assign(&minus, y);
    minus->sign = NONNEGATIVE;

    //case y = -1
    if( Is_One(minus) == 1 )
    {
        BI_assign(dst, x);
        (*dst)->sign = 1 - (x->sign);
        // y->sign = NONNEGATIVE : 1-0 = 1(NEGATIVE)
        // y->sign = NEGATIVE : 1-1 = 0(NONNEGATIVE)
        BI_delete(&minus);
        return 1;
    }

    BI_delete(&minus);

#if KARATSUBA
    bigint* x_temp = NULL, y_temp = NULL;
    BI_assign(&x_temp, x);
    BI_assign(&y_temp, y);
    x_temp->sign = NONNEGATIVE;
    y_temp->sign = NONNEGATIVE;
    printf("\n[KARATSUBA]\n");
    if(!BI_MUL_karatsuba(dst, x_temp, y_temp, KARATSUBA_FLAG)) return 0;

    if((x->sign)-(y->sign) == 0) // x,y의 부호가 같을 때 (1-1 = 0, 0-0 = 0)
    {
        (*dst)->sign = NONNEGATIVE;
        return 1;
    }
    else    // x,y의 부호가 다를 때 (1-0 = 1, 0-1 = -1)
    {
        (*dst)->sign = NEGATIVE;
        return 1;
    }
#endif

    if(!MUL_improved_textbook(dst, x, y)) return 0;

    if((x->sign)-(y->sign) == 0) // x,y의 부호가 같을 때 (1-1 = 0, 0-0 = 0)
    {
        (*dst)->sign = NONNEGATIVE;
        return 1;
    }
    else    // x,y의 부호가 다를 때 (1-0 = 1, 0-1 = -1)
    {
        (*dst)->sign = NEGATIVE;
        return 1;
    }
    
    //return 0;
}

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))


// 2-3개짜리 워드단위로 쪼개면서 

int BI_MUL_karatsuba(bigint** dst, bigint* x, bigint* y, int flag)
{
    if((x->sign=NEGATIVE) || (y->sign=NEGATIVE))
    {
        printf("karatsuba input error!\n");
        return 0;
    }

    int n = x->wordlen, m = y->wordlen;

    if (flag >= min(n, m))
    {
        return BI_MUL(dst, x, y);
    } 
    bigint *a1=NULL, *a0=NULL;
    bigint *b1=NULL, *b0=NULL;
    bigint *t1=NULL, *t0=NULL;
    bigint *c=NULL, *c_temp=NULL;
    bigint *s1=NULL, *s0=NULL, *s=NULL, *s_temp=NULL;
    bigint *z=NULL;

    int l,lw;
    
    /* l, lw */
    l = (max(n,m)+1)>>1;    // l=max(n,m)/2
    lw = l*WORDBITS;        // lw=bits of l words

    /* a1, a0 */
    // a1=x, a1>>lw
    BI_assign(&a1,x);
    BI_rightShift(&a1,lw); 
       

    /* a0 = x mod 2^lw */ 
    BI_set_by_array(&a0, NONNEGATIVE, x->N,  l);     // NON_NEGATIVE
    BI_refine(a0);
    
    /* b1, b0 */
    // b1=y, b1>>lw
    BI_assign(&b1,y);
    BI_rightShift(&b1,lw);
    


    //b0 modulo ???????????????????
    /* b0 = (b mod 2^lw) */
    BI_set_by_array(&b0, NONNEGATIVE, y->N, l);     // NON_NEGATIVE
    BI_refine(b0);
    //====================
    //====================

    // t1 = sign * MUL_karatsuba(|a1|,|b1|)

    /* t1, to */
    BI_MUL_karatsuba(&t1, a1, b1, flag);
    BI_MUL_karatsuba(&t0, a0, b0, flag);
    
    /* c=(t1<<2lw) + t0 */

    BI_assign(&c_temp, t1);
    BI_leftShift(&c_temp, 2*lw);
    
    BI_ADD(&c, c_temp, t0);

    /* s1=a0-a1,s0=b1-b0 */
    BI_SUB(&s1, a0, a1);
    BI_SUB(&s0, b1, b0);
    
    BI_new(&s_temp, 1);

    // s_temp부호 = sign = (-1)*sign(S1)^sign(S2)
    int sign;
    if((s1->sign ^ s0->sign)==1)  sign = -1;
    else    sign = 1;

    //s_temp = sign * MUL_karatsuba(|s1|,|s0|)
    s1->sign = NONNEGATIVE;
    s0->sign = NONNEGATIVE;
    
    BI_MUL_karatsuba(&s_temp, s1, s0, flag);
    s_temp->sign = sign;
    // s = add(s_temp, t1) , s= add(s, t0)
    BI_ADD(&s, s_temp, t1);
    BI_assign(&s_temp, s);
    BI_ADD(&s, s_temp, t0);
    
    // s<-s<<lw
    BI_leftShift(&s, lw);
    // c<-add(c,s)

    BI_ADD(&z, c, s);
    BI_assign(dst, z);
    
    BI_delete(&a1); BI_delete(&a0);
    BI_delete(&b1); BI_delete(&b0);
    BI_delete(&t1); BI_delete(&t0);
    BI_delete(&c);  BI_delete(&c_temp);
    BI_delete(&s1); BI_delete(&s0);
    BI_delete(&s);  BI_delete(&s_temp);
    BI_delete(&z);

    return 1;
}


/*====================================================*/
/*===================DIVISION=========================*/

int BI_DIV_2word(word* q, bigint* x, word* b)
{

    // (Q, R) <- (0, A1)
    word Q;
    Q = 0;

    word r;
    r = x->N[1]; // R <- A1

    word B;
    B = (*b);

    for (int j= WORDBITS-1; j>=0; j--)
    {   
        if (r>=(1<<(WORDBITS-1)))
        {
            // q <- q + 2 ^ j
            Q = Q + (1<<j);
            // r <-  2r + aj - b , aj = x->N[0]의 오른쪽->왼쪽으로 비트값
            //printf("\nN[0]= %08x", x->N[0]);
            r = (2*r) + ((x->N[0] >> j)& 0x1) - B;
        }
        else
        {
            // r <- 2r + aj
            r = (2*r)  + ((x->N[0] >> j) & 0x1);

            if (r >= B)
            {
                // (q, r) <- (q + 2^j, r-b)
                Q = Q +(1<<j);
                r  = r - B;
            }
        }
    }
    (*q) = Q;
    return 1;
}


/*
    DIVCC(A, B)
*/
int BI_DIVCC(bigint** q, bigint** r, bigint* x, bigint* y)
{
    // 예외처리
    if( x == NULL || y == NULL ) {
        return 0;
    }

    if( x->sign == NEGATIVE || y->sign == NEGATIVE ) {
        return 0;
    }

    int n = x->wordlen;
    int m = y->wordlen;

    bigint* bq = NULL;
    
    bigint* A_temp = NULL;
    bigint* R_temp = NULL;

    // Q : [0,w] 
    BI_new(q,1);


    // 시작!
    if (n==m) {
        (*q)->N[0] = x->N[m-1] / y->N[m-1];
    }
    else if (n==(m+1)) {
        if(x->N[m] == y->N[m-1]){
            // Q <- W-1
            (*q)->N[0]= 0xffffffff;
        }
        else {

            BI_new(&A_temp, 2);

            A_temp->N[1] = x->N[m];
            A_temp->N[0] = x->N[m-1];

            BI_DIV_2word(&((*q)->N[0]), A_temp, &(y->N[m-1]));
        }
    }
    else{
        // 예외
        return 0;
    }

    // R <- A - BQ 
    BI_MUL(&bq, y, *q); // flag??????


    BI_SUB(r, x, bq);

    // ok


    // while(r<0)
    while((*r)->sign==NEGATIVE){ 
        // q<-q-1
        (*q)->N[0] = (*q)->N[0] - 1;

        // r<-r+y 
        // >>  R_temp<-r+y, r<-R_temp
        BI_ADD(&R_temp, *r, y);
        BI_assign(r, R_temp);

    }

    // delete
    BI_delete(&bq);
    BI_delete(&A_temp);
    BI_delete(&R_temp);

    return 1;

}


/*DIVC(A,B)*/
int BI_DIVC(bigint **q, bigint **r, bigint *x, bigint *y)
{
    bigint* x_temp = NULL;
    bigint* y_temp = NULL;

    BI_assign(&x_temp, x);
    BI_assign(&y_temp, y);

    if( x == NULL || y == NULL ) {
        return 0;
    }

    if( x->sign == NEGATIVE || y->sign == NEGATIVE ) {
        return 0;
    }

    int e = x->wordlen;
    int f = y->wordlen;
    if ((x->N[e-1])>(y->N[f-1])){
        return 0;
    }

    // A = BQ + R -> Q=0, R = A
    if (BI_compare(x,y)==-1){
        (*q)->N[0] = 0;
        BI_assign(r, x);
        BI_refine((*r));
        
    }

    int k=0;

    // k값 계산하기
    int m=y->wordlen;
    word y_m_1 = y->N[m-1];
    while( (( y_m_1 << k) & 0x80000000) != 0x80000000 ) // 1이 되는 k 구하기
        k++;


    bigint* R_temp = NULL;
    
    // Q : [0,w]
    BI_new(q,1);


    // A', B' = 2^k A, 2^k B
    BI_leftShift(&x_temp, k); // x = A'
    BI_leftShift(&y_temp, k); // y = B'
    
    
    // Q', R' = DIVCC(A', B')
    BI_DIVCC(q, r, x_temp, y_temp);
    

    // Q <- Q' :코드없음. 그대로,
    // R <- 2^(-k)R'
    BI_rightShift(r, k);
  
    BI_delete(&R_temp);
    BI_delete(&x_temp);
    BI_delete(&y_temp);
    return 1;
}



// Multi-Precision Long Division //
/* input 
   | x : x_(n-1)W^(n-1) + x_(n-2)W^(n-2) + ... + x_0
   | y : y_(m-1)W^(m-1) + y_(m-2)W^(m-2) + ... + y_0
   output
   | q : q_(n-1)W^(n-1) + q_(n-2)W^(n-2) + ... + q_0
   | r : r < y

   -> X = QY + R   */
int BI_DIV(bigint** q, bigint** r, bigint* x, bigint* y)
{
    if (x == NULL || y == NULL) {
        return 0;
    }

    //추가된 부분 -> y는 음수나 0이 될 수 없음!
    if (y->sign == NEGATIVE || Is_Zero(y) == 1) {
        return 0;
    }

    if(BI_compare(x, y) == -1)
    {
        BI_set_zero(q);
        BI_assign(r, x);
        return 1;
    }
    
    int n = x->wordlen;

    bigint *TEMP = NULL;
    bigint *q_temp = NULL;
    bigint *r_temp = NULL;
 
    BI_new(q, n);
    BI_set_zero(&TEMP);
    
    for(int i =(n-1); i>=0; i--)
    {
        BI_leftShift(&TEMP, WORDBITS);  
        
        TEMP->N[0] = x->N[i];

        BI_refine(TEMP);
        
        BI_DIVC(&q_temp, &r_temp, TEMP, y);
        BI_show_hex(q_temp);
        BI_assign(&TEMP, r_temp);
        
        (*q)->N[i] = q_temp->N[0];
    }
    
    if(x->sign==NEGATIVE)
    {
        // q = -q - 1
        BI_assign(&q_temp, (*q));
        q_temp->sign = NEGATIVE;
        bigint *one = NULL;
        BI_set_one(&one);

        BI_SUB(q, q_temp, one);

        // r = b-r
        BI_SUB(r, y, TEMP);
        BI_delete(&one);
    }
    else    // x가 양수인 경우
    {
        BI_assign(r, TEMP);
        BI_refine((*q));
    }

    BI_delete(&TEMP);
    BI_delete(&q_temp);
    BI_delete(&r_temp);

    return 1;
}

// Barrett Reduction //
/* input 
   | x : x_(2n-1)W^(2n-1) + x_(2n-2)W^(2n-2) + ... + x_0
   | N : y_(n-1)W^(n-1) + y_(n-2)W^(n-2) + ... + y_0
   | T : W^(2n) // N
   output
   | R : r_(n-1)W^(n-1) + r_(n-2)W^(n-2) + ... + r_0

   -> R = x mod N   */
int Barrett_RED(bigint** R, bigint* x, bigint* N, bigint* T)
{
    int n = N->wordlen;

    if(x->wordlen > 2*n)
    {
        printf("\nBarrett Input error!\n");
        return 0;
    }

    bigint* x_temp = NULL;
    bigint* r = NULL;
    bigint* r_temp = NULL;
    bigint* Q = NULL;

    BI_assign(&x_temp, x);

    //BI_new(&Q, (x_temp->wordlen+n));
    BI_rightShift(&x_temp, WORDBITS*(n-1)); // x_temp <-- x >> w(n-1)
    BI_MUL(&Q, x_temp, T);                  // Q(Q_hat) <-- x_temp * T
    BI_rightShift(&Q, WORDBITS*(n+1));      // Q <-- Q >> w(n+1)
    BI_MUL(&r, N, Q);                       // r <-- N * Q
    BI_SUB(&r_temp, x, r);                  // r_temp <-- x - r
    
    while(1)
    {
        BI_show_hex(r_temp);
        printf("\n");
        BI_show_hex(N);
        printf("\n");
        if(BI_compare(r_temp, N)==-1)
            break;
        BI_SUB(&r, r_temp, N);      // r <— r_temp - N
        BI_assign(&r_temp, r);      // r_temp <— r
    }

    BI_assign(R, r_temp);

    BI_delete(&x_temp);
    BI_delete(&r);
    BI_delete(&r_temp);
    BI_delete(&Q);
    BI_delete(&x_temp);

    return 1;                                                                                                                                    
}