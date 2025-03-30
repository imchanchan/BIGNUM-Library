
#include "TITAN_bigint.h"

// Create BigInt
void BI_new(bigint** x,  int wordlen)
{
    if((*x) != NULL) BI_delete(x);

    *x = (bigint*)malloc(sizeof(bigint));
    (*x)->sign = NONNEGATIVE;
    (*x)->wordlen = wordlen;
    (*x)->N = (word*)calloc(wordlen, sizeof(word));
    return;
}

// Delete BigInt
void BI_delete(bigint** x)
{
    if(*x == NULL) return;

#ifdef ZERORIZE
    array_init((*x)->N, (*x)->wordlen);
    //memset((*x)->N, 0, wordlen*(sizeof(word)));
#endif

    free((*x)->N);
    free(*x);
    *x = NULL;
}

// Refine BigInt
void BI_refine(bigint* x)
{
    if(x == NULL) return;

    int temp = x->wordlen;
    while(temp > 1)
    {
        if(x->N[temp-1] != 0) break;
        temp--;
    }
    if(x->wordlen != temp)
    {
        x->wordlen = temp;
        x->N = (word*)realloc(x->N, sizeof(word)*temp);
    }

    if((x->wordlen == 1) && (x->N[0] == 0x00))
        x->sign = NONNEGATIVE;
}

// Assign BigInt(Y<-X)
void BI_assign(bigint** y, bigint* x)
{
    if (*y != NULL)
        BI_delete(y);

    BI_new(y, x->wordlen);
    (*y)->sign = x->sign;
    memcpy((*y)->N, x->N, sizeof(word)*(x->wordlen));
    //array_copy((*y)->N, x->N, x->wordlen);
}

// Set One/Zero
void BI_set_one(bigint** x)
{
    BI_new(x, 1);
    (*x)->sign = NONNEGATIVE;
    (*x)->N[0] = 0x01;
}

void BI_set_zero(bigint** x)
{
    BI_new(x, 1);
    (*x)->sign = NONNEGATIVE;
    (*x)->N[0] = 0x0;
}

// Is One/Zero?
int Is_Zero(bigint* x)
{
    if (x->sign == 1 || x->N[0] != 0)   return 0;
    for (int j = x->wordlen; j > 0; j--) {
        if (x->N[j] != 0) return 0;
    }
    return 1;
}

int Is_One(bigint* x)
{
    if (x->sign == 1 || x->N[0] != 1)   return 0;
    for (int j = x->wordlen; j > 0; j--) {
        if (x->N[j] != 0) return 0;
    }
    return 1;
}

// Compare
// (A > B) -> 1
// (A = B) -> 0
// (A < B) -> -1
int compare_core(bigint* x, bigint* y)
{
    int x_len = x->wordlen;
    int y_len = y->wordlen;

    if (x_len > y_len) return 1;
    else if (x_len < y_len) return -1;
    else
    {
        for (int i = (x_len - 1); i > 0; i--)
        {
            if (x->N[i] > y->N[i]) return 1;
            else if (x->N[i] < y->N[i]) return -1;
        }
        return 0;
    }
}

int BI_compare(bigint* x, bigint* y)
{
    if ((x->sign == 0) && (y->sign == 1)) return 1;
    if ((x->sign == 1) && (y->sign == 0)) return -1;

    int res;
    res = compare_core(x, y);
    if (x->sign == 0) return res;
    else return (-1) * res;
}

// Left/Right Shift
int BI_leftShift(bigint** x, int r)
{
    int n, w;
    n = (*x)->wordlen;
    w = sizeof(word)*8; //32
    int i;
    int k, R;
    int j;
    bigint* temp = NULL;
    bigint* temp1 =NULL;

    // // case 1 : r>=wn -> A>>r=0
    // if(r >= w*n)
    // {
    //     for(int i=0; i<n; i++)
    //     {
    //         (*x)->N[i]=0;
    //     }
    //     return 1;
    // }

    // case 2 : r=wk : shift k word
    if (r%w==0)
    {
        k = r/w;
        
        // temp에 옮겨놓을 공간을 만든다.
        BI_new(&temp, n+k);
        temp->sign = (*x)->sign;

        // N[n+k-1]-N[k] : A배열 넣기
        memcpy(temp->N + k, (*x)->N, sizeof(word)*((*x)->wordlen)); 

        // x<-temp 대입해주기
        //temp가 x로 들어가면, x는 temp의 크기가 된다.
        BI_refine(*x);
        BI_assign(x, temp);
        BI_delete(&temp);
        
        return 1;
    }

    //case 3 : r = wk + R , where 0<R<32(8*4)
    if(r % w > 0)
    {   
        k = r/w; // word shift
        R = r%w; // N[j] : bit shift

        //word c = ;

        //temp에 옮겨놓을 공간을 만든다.
        BI_new(&temp1, n+k+1);
        temp1->sign = (*x)->sign;


        // N[0]~N[n-1]까지 R bit shift //
        //상위바이트부터 1개씩 내려가야 이전 바이트를 변함없이 바꿀수있다.
        temp1->N[n]=(*x)->N[n-1]>>(32-R);

        for(j=n-1; j>0; j--){
            temp1->N[j]=((*x)->N[j])<<R  | ((*x)->N[j-1])>>(32-R) ;
            //printf("%x08", temp1 -> N[j]);
        }

        //temp[0]을 R만큼 이동한다.
        temp1->N[0] = (*x)->N[0]<<R;

        
        // N[0]~N[n-1] k word shift //
        // temp1을 k word shift 한다음, temp에 넣어준다. -> temp를 x에 assign한다.
        BI_new(&temp, n+k+1);
        temp->sign = temp1->sign;
        
        // N[n+k-1]-N[k] : A배열 넣기
        //memcpy(temp->N + k, temp1->N, sizeof(word)*(temp1->wordlen)); 
        memcpy(temp->N + k, temp1->N, sizeof(word)*(temp1->wordlen)); 

        BI_assign(x, temp);
        BI_refine(*x);
        BI_delete(&temp1);
        BI_delete(&temp);
        return 1;

    }

    return 0;
}

int BI_rightShift(bigint** x, int r)
{
    int n, w;
    n = (*x)->wordlen;
    w = sizeof(word)*8;

    int i;
    int k, R;
    int j;
    
    bigint* temp = NULL;

    // case 1 : r>=wn --> A<<r =0
    if(r >= w*n)
    {
        for(int i=0; i<n ; i++)
        {
            (*x)->N[i]=0;
        }
        return 1;
    }

    // case 2 : r=wk : x<- x >> r (=x>>wk)
    else if(r % w ==0)
    {
        k = r/w ;
        
        // temp를 만들어줄 필요가 없다. = 뒤에서 복사해오는 것이기 때문.
        for (i=0; i<n-k; i++)
        {
            (*x)->N[i] = (*x)->N[i+k]; // N[0]-N[n-k-1] : A배열 넣기
        }
        for (i=n-k; i<n; i++)
        {
            (*x)->N[i]=0; //N[n-k]-N[n-1] : 0 넣기
        }
        BI_refine(*x);
        return 1;
    }

    // case 3 : r=wk + r' , where 0<r'<w 
    if(r % w > 0)
    {   
        int R,k;
        R = r % w;
        k = r / w;

        // N[0]~N[n-1]까지 R bit shift //
        for(int j = 0; j<n; j++){
            (*x)->N[j] = ((*x)->N[j+1]<<(w-R)) | ((*x)->N[j]>>R ); 
        }

        // N[0]~N[n-1] k word shift //
        for (i=0; i<n-k; i++)
        {
            (*x)->N[i] = (*x)->N[i+k]; // N[0]-N[n-k-1] : A배열 넣기
        }
        for (i=n-k; i<n; i++)
        {
            (*x)->N[i]=0; //N[n-k]-N[n-1] : 0 넣기
        }
        BI_refine(*x);
        return 1;
    }

    BI_delete(&temp);
    return 0;
}


int BI_reduction(bigint* x, int r)
{
    int x_len = x->wordlen;

    if(r==0)
        memset(x->N, 0, x->wordlen*(sizeof(word)));

    else
    {
        int w = sizeof(word);

        // case : r >= wn
        if(r >= w*x_len){}

        // case : r = wk
        int k = r/w;
        if(!(r % w))   
        {
            for(int i=0; i<k; i++)
            {
                x->N[i] = x->N[i+(x_len-k)];
            }
            //a->wordlen = k;
        }

        // case : r = wk + r' (0 < r' < w) -> exception..?
        else                
        {
            for(int i=1; i<(k+1); i++)
            {
                x->N[i] = x->N[i+(x_len-k-1)];
            }
            word mask = (1U << (r%w)) - 1; // bit mask : 2^n-1
            // 1U : unsigned int
            x->N[0] = x->N[k-1] & mask;
        }
        BI_refine(x);
    }
}

int decStr_to_hexArr(const char* decStr, word* hexArr)
{
}

int binStr_to_hexArr(const char* binStr, word* hexArr, int wordlen)
{
    int idx = wordlen-1;
    int pos = 0;
    word c;

    for(int i=(strlen(binStr)-1); i>=0; i--)
    {
        if((binStr[i] =='0')||(binStr[i] =='1'))
        {
            c = (word)(binStr[i] - '0');
            hexArr[idx] |= (c << pos);    
        }
        else
        {
            printf("Invaild Input\n");
            return 0;
        }

        pos++;

        if ((pos==32) || (i==0)) {
            idx--;
            pos = 0;
        }
    }
    return wordlen;
}

int hexStr_to_hexArr(char* hexStr, word* hexArr, int wordlen)
{
    int idx = 0;
    int pos = 0;
    word c;

    for(int i=strlen(hexStr)-1; i>=0; i--)
    {
        if (((hexStr[i]>='0')&&(hexStr[i]<='9')) || ((hexStr[i]>='a')&&(hexStr[i]<='f')) || ((hexStr[i]>='A')&&(hexStr[i]<='F')))
        {
            if((hexStr[i]>='0')&&(hexStr[i]<='9'))
                c = (word)(hexStr[i] - '0');
            else if((hexStr[i]>='A')&&(hexStr[i]<='F'))
                c = (word)(hexStr[i] - 'A') + 10;
            else if((hexStr[i]>='a')&&(hexStr[i]<='f'))
                c = (word)(hexStr[i] - 'a') + 10;
        }
        
        else
        {
            printf("Invaild Input\n");
            return 0;
        }

        hexArr[idx] |= (c << (pos*4));  // a << (pos*4) = a x (16^pos)
        pos++;

        if((pos==8)||(i==0))
        {
            idx++;
            pos = 0;
        }
    }
    return wordlen;
}

// 2.4.1 Set_bigint_by_array
int BI_set_by_array(bigint** x, int sign, const word *a, int wordlen)
{   
    if(a==NULL)
    {
        printf("Invaild Input\n");
        return 0;
    }
    
    // allocate for bigint structure
    BI_new(x, wordlen);
    (*x)->sign = sign;
    
    memcpy((*x)->N, a, sizeof(word)*wordlen);


    return wordlen; 
}

// 2.4.1 Set_bigint_by_string
// base : binary(2), decimal(10), hex(16)
int BI_set_by_string(bigint** x, int sign, char* a, int base)
{
    int strLen, wordLen;
    int temp;
    strLen = strlen(a);

    switch (base)
    {
    case BINARY:
        {
            wordLen = ((strLen-1)/WORDBITS) + 1;
            BI_new(x, wordLen-1);
            (*x)->sign = sign;

            wordLen = binStr_to_hexArr(a, (*x)->N, wordLen);
            break;
        }
    case DECIMAL:
        {
            wordLen = decStr_to_hexArr(a, (*x)->N);
            break;
        }
    case HEXA:
        {
            if(((strLen)%(WORDBITS/4))!=0)
                wordLen = ((strLen)/(WORDBITS/4)) + 1;
            else if(((strLen)%(WORDBITS/4))==0)
                wordLen = ((strLen)/(WORDBITS/4));
            BI_new(x, wordLen);
            (*x)->sign = sign;
            
            wordLen = hexStr_to_hexArr(a, (*x)->N, wordLen);
            
            break;
        }
    default:
        {
            printf("Unsupported base\n");
            return 0; // 예외 처리: 지원하지 않는 base 값일 경우 0 반환
        }
    }   
    return wordLen;
}

// Show BigInt
void BI_show_hex(bigint* x)
{
    if(x->sign == NEGATIVE) printf("-0x");
    else printf("0x");
    for (int i = x->wordlen-1; i >=0 ; i--) {
        printf("%08x", x->N[i]);
    }
}

void BI_show_bin(bigint* x)
{
    if(x->sign == NEGATIVE) printf("-");

    word* p = x->N;

    for (int i = 0; i < x->wordlen; i++) {
        for (int j=WORDBITS-1; j>=0; j--)
        {
            printf("%d", (p[i]>>j)&0x01);
        }
        printf("  ");
    }
}

void array_new_rand(word **const a, const int wordlen) 
{    // 추가
    if ( *a != NULL ) {
#ifdef ZERORIZE
        array_init(*a, wordlen);
#endif
        free(*a);
        *a = NULL;
    }

    (*a) = (word *)calloc(wordlen, sizeof(word)); // [word_n-1] [word_n-2] .. [word_0]: wordlen개

    byte *p = (byte *)(*a);
    int cnt = wordlen * sizeof(word);

    while ( cnt > 0 ) {
        *p = rand() & 0xff; // rand = DRBG
        p++;
        cnt--;
    }
}

void array_rand(word* dst, int wordlen)
{
    byte* p = (byte*)dst;
    int cnt = wordlen * sizeof(word);

    

    while (cnt > 0) {
        *p = rand() & 0xff;
        p++;
        cnt--;
    }
}

// Generate Random BigInt
void BI_gen_rand(bigint** x, int wordlen)
{
    // (*x) = (bigint*)malloc(sizeof(bigint));
    // (*x)->sign = sign;
    // (*x)->wordlen = wordlen;
    // (*x)->N = (word*)malloc(wordlen * sizeof(int));
    BI_new(x, wordlen);
    (*x)->sign = rand()%2;
   
    array_rand((*x)->N, wordlen);
    BI_refine(*x);
}

