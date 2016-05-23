#include <stdio.h>
#include <stdlib.h>


int overflow_detect(int ans,int a,int b);
unsigned char cut_rs(int a);
unsigned char cut_rt(int a);
unsigned char cut_rd(int a);
unsigned char cut_shamt(int a);
unsigned char cut_func(int a);
short cut_immediate(int a);
unsigned short cut_immediate_unsigned(int a);
unsigned int cut_address(int a);
unsigned int combine(unsigned char a,unsigned char b,unsigned char c,unsigned char d);
unsigned short combine_two(unsigned char a, unsigned char b);
unsigned char* seperate(int in);
unsigned char* seperate_two(int in);
