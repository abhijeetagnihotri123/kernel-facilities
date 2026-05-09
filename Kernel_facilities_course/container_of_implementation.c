#include <stdio.h>

struct abc{
    int m;
    int n;
    int o;
  };

struct xyz{
    int a;
    struct abc mno;
    int b;
    float c;
    char d;
};

int main(){

    struct xyz pqr;

    printf("&pqr: %p\n" , &pqr);
    printf("&pqr.c : %p\n" , &pqr.d);

    void *p = &((struct xyz *)0) ->d;

    printf("Offset value : [%llu]\n" , (unsigned long long)p);


    unsigned long long int offset = (unsigned long long int)p;

    char *ptr_pqr = (char*)(&pqr.d) - offset;

    printf("Starting struct pqr address : [%p]\n" , ptr_pqr);



    return 0;
}