#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "function.h"
#include "Memory.h"

FILE *iimage;
FILE *dimage;
FILE *snapshot;
FILE *report;

int cycle;
int reg[32];
int PC;
int PC_start;
unsigned char ii[1024];
unsigned char di[1024];
int iim[256];
unsigned char dim[1024];
int temp;

extern int iTLBmiss;
extern int iTLBhit;
extern int iPTmiss;
extern int iPThit;
extern int iCAmiss;
extern int iCAhit;

extern int dTLBmiss;
extern int dTLBhit;
extern int dPTmiss;
extern int dPThit;
extern int dCAmiss;
extern int dCAhit;

int command[15];

int main(int argc, char* argv[])
{
    iimage = fopen("./iimage.bin","rb");
    dimage = fopen("./dimage.bin","rb");
    snapshot = fopen("./snapshot.rpt","w");
    report = fopen("./report.rpt","w");
    if(argc==11)
    {
        command[1]=atoi(argv[1]);
        command[2]=atoi(argv[2]);
        command[3]=atoi(argv[3]);
        command[4]=atoi(argv[4]);
        command[5]=atoi(argv[5]);
        command[6]=atoi(argv[6]);
        command[7]=atoi(argv[7]);
        command[8]=atoi(argv[8]);
        command[9]=atoi(argv[9]);
        command[10]=atoi(argv[10]);
    }else
    {
        command[1]=64;
        command[2]=32;
        command[3]=8;
        command[4]=16;
        command[5]=16;
        command[6]=4;
        command[7]=4;
        command[8]=16;
        command[9]=4;
        command[10]=1;
    }

    initICMP(command[1],command[3],command[5],command[6],command[7]);
    initDCMP(command[2],command[4],command[8],command[9],command[10]);

    int sdata=0,sins=0;
    int i,j;


    memset(reg,0,sizeof(reg));
    memset(ii,0,sizeof(ii));
    memset(di,0,sizeof(di));
    memset(iim,0,sizeof(iim));
    memset(dim,0,sizeof(dim));
    PC=0;

    fseek(iimage , 0 , SEEK_END);


    rewind (iimage);

    fread(ii,sizeof(unsigned char),8,iimage);

    fseek(dimage , 0 , SEEK_END);


    rewind (dimage);

    fread(di,sizeof(unsigned char),8,dimage);

    /*
    for(i=0;i<siimage;i++)
    printf("%x\n",ii[i]);
    */
    reg[29]=combine(di[0],di[1],di[2],di[3]);
    sdata=combine(di[4],di[5],di[6],di[7]);



    fread(di,sizeof(unsigned char),sdata*4,dimage);

    for(i=0; i<sdata*4; i++)
    {
        dim[i]=di[i];
    }

    PC=combine(ii[0],ii[1],ii[2],ii[3]);
    PC_start=PC;
    sins=combine(ii[4],ii[5],ii[6],ii[7]);

    fread(ii,sizeof(unsigned char),sins*4,iimage);
    for(i=0; i<sins; i++)
    {
        iim[i]=combine(ii[i*4],ii[1+i*4],ii[2+i*4],ii[3+i*4]);
        //  printf("i=%d %x\n",i,iim[i]);
    }
    /*
    for(i=0;i<sins;i++)
    printf("%x\n",iim[i]);
    */

    /**
    ii : string of all the data int iimage.bin
    di : string of all the data int dimage.bin
    PC : current PC
    reg : int array of all the register
    iim : int array of all the instruction
    dim : unsigned char array of all the data in memory
    sins: int of the number of instruction
    sdata : int of the number of memory (word)
    **/

    unsigned char op=0;
    unsigned char rs=0;
    unsigned char rt=0;
    unsigned char rd=0;
    unsigned char shamt=0;
    unsigned char funct=0;
    short immediate=0;
    unsigned short unsigned_immediate=0;
    unsigned int address=0;

    int read=0;
    int errors[4];
    unsigned char *getting;
    getting = (unsigned char*)malloc(sizeof(unsigned char)*4);
    int flag=0;

    i=0;
    cycle=0;
    while(1)
    {
        memset(errors,0,sizeof(errors));
        //printf("i:%d   ",i);

        //printf("%x ",iim[i]);
        //printf("%x ",op);
        fprintf(snapshot,"cycle %d\n",cycle);
        for(j=0; j<32; j++)
        {
            fprintf(snapshot,"$%02d: 0x%08X\n",j,reg[j]);
        }
        fprintf(snapshot,"PC: 0x%08X\n",PC);

        checkImemory(PC);

        if(PC>=PC_start)
        {
            op=(unsigned)iim[i]>>26;



            switch(op)
            {
            case 0x00:
            {
                funct=cut_func(iim[i]);

                switch(funct)
                {
                case 0x20:
                {

                    rs=cut_rs(iim[i]);
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);

                    temp=reg[rs]+reg[rt];
                    reg[rd]=reg[rs]+reg[rt];
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x21:
                {
                    rs=cut_rs(iim[i]);
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);
                    reg[rd]=reg[rs]+reg[rt];
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x22:
                {
                    rs=cut_rs(iim[i]);
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);
                    temp=reg[rs]+(-1)*reg[rt];
                    reg[rd]=reg[rs]+(-1)*reg[rt];
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x24:
                {
                    rs=cut_rs(iim[i]);
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);

                    reg[rd]=reg[rs]&reg[rt];
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x25:
                {
                    rs=cut_rs(iim[i]);
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);

                    reg[rd]=reg[rs]|reg[rt];
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x26:
                {
                    rs=cut_rs(iim[i]);
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);

                    reg[rd]=reg[rs]^reg[rt];
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x27:
                {
                    rs=cut_rs(iim[i]);
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);

                    reg[rd]=~(reg[rs]|reg[rt]);
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x28:
                {
                    rs=cut_rs(iim[i]);
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);

                    reg[rd]=~(reg[rs]&reg[rt]);
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x2A:
                {
                    rs=cut_rs(iim[i]);
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);

                    if(reg[rs]<reg[rt])reg[rd]=1;
                    else reg[rd]=0;
                    if(rd==0) reg[rd]=0;

                    PC+=4;
                    break;
                }
                case 0x00:
                {
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);
                    shamt=cut_shamt(iim[i]);


                    reg[rd]=reg[rt]<<shamt;
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x02:
                {
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);
                    shamt=cut_shamt(iim[i]);

                    reg[rd]=(unsigned)reg[rt]>>shamt;
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x03:
                {
                    rt=cut_rt(iim[i]);
                    rd=cut_rd(iim[i]);
                    shamt=cut_shamt(iim[i]);

                    reg[rd]=reg[rt]>>shamt;
                    if(rd==0) reg[rd]=0;
                    PC+=4;
                    break;
                }
                case 0x08:
                {

                    rs=cut_rs(iim[i]);
                    PC=reg[rs];
                    break;
                }

                }
                break;
            }
            case 0x08:
            {

                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);
                temp=reg[rs]+immediate;

                reg[rt]=reg[rs]+immediate;
                if(rt==0)
                    reg[rt]=0;
                PC+=4;
                break;
            }
            case 0x09:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                reg[rt]=reg[rs]+immediate;
                if(rt==0) reg[rt]=0;
                PC+=4;
                break;
            }
            case 0x23:
            {

                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                read= reg[rs]+immediate;
                checkDmemory(read);
                reg[rt]=(int)combine(dim[read],dim[read+1],dim[read+2],dim[read+3]);

                if(rt==0) reg[rt]=0;


                PC+=4;
                break;
            }
            case 0x21:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                read= reg[rs]+immediate;
                checkDmemory(read);
                reg[rt]=(short)combine_two(dim[read],dim[read+1]);
                if(rt==0) reg[rt]=0;

                PC+=4;
                break;
            }
            case 0x25:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                read= reg[rs]+immediate;
                checkDmemory(read);
                reg[rt]=combine_two(dim[read],dim[read+1]);
                if(rt==0) reg[rt]=0;

                PC+=4;
                break;
            }
            case 0x20:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                read= reg[rs]+immediate;
                checkDmemory(read);
                reg[rt]=(char)dim[read];
                if(rt==0) reg[rt]=0;

                PC+=4;
                break;
            }
            case 0x24:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                read= reg[rs]+immediate;
                checkDmemory(read);
                reg[rt]=(unsigned)dim[read];
                if(rt==0) reg[rt]=0;

                PC+=4;
                break;
            }
            case 0x2B:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                getting=seperate(reg[rt]);
                read= reg[rs]+immediate;
                checkDmemory(read);

                dim[read]=getting[0];
                dim[read+1]=getting[1];
                dim[read+2]=getting[2];
                dim[read+3]=getting[3];

                PC+=4;
                break;
            }
            case 0x29:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                getting=seperate_two(reg[rt]);
                read= reg[rs]+immediate;
                checkDmemory(read);
                dim[read]=getting[0];
                dim[read+1]=getting[1];
                PC+=4;
                break;
            }
            case 0x28:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                getting[0]=(unsigned char)(reg[rt]&0x000000FF);
                read= reg[rs]+immediate;
                checkDmemory(read);
                dim[read]=getting[0];
                PC+=4;
                break;
            }
            case 0x0F:
            {
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                reg[rt]=immediate<<16;
                if(rt==0) reg[rt]=0;
                PC+=4;
                break;
            }
            case 0x0C:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                unsigned_immediate=cut_immediate_unsigned(iim[i]);

                reg[rt]=reg[rs]&unsigned_immediate;
                if(rt==0) reg[rt]=0;
                PC+=4;
                break;
            }
            case 0x0D:
            {

                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                unsigned_immediate=cut_immediate_unsigned(iim[i]);


                reg[rt]=(reg[rs]|unsigned_immediate);
                if(rt==0) reg[rt]=0;
                PC+=4;
                break;
            }
            case 0x0E:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                unsigned_immediate=cut_immediate_unsigned(iim[i]);
                reg[rt]=~(reg[rs]|unsigned_immediate);
                if(rt==0) reg[rt]=0;
                PC+=4;
                break;
            }
            case 0x0A:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);

                if(reg[rs]<immediate) reg[rt]=1;
                else reg[rt]=0;

                if(rt==0) reg[rt]=0;
                PC+=4;
                break;
            }
            case 0x04:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);
                if(reg[rs]==reg[rt])
                {
                    read=immediate*4+4;
                    PC+=read;
                }
                else PC+=4;
                break;
            }
            case 0x05:
            {
                rs=cut_rs(iim[i]);
                rt=cut_rt(iim[i]);
                immediate=cut_immediate(iim[i]);
                if(reg[rs]!=reg[rt])
                {
                    read=immediate*4+4;
                    PC+=read;
                }
                else PC+=4;
                break;
            }
            case 0x07:
            {
                rs=cut_rs(iim[i]);
                immediate=cut_immediate(iim[i]);
                if(reg[rs]>0)
                {
                    read=immediate*4+4;
                    PC+=read;
                }
                else PC+=4;
                break;
            }
            case 0x02:
            {
                address=cut_address(iim[i]);
                address=address<<2;
                PC+=4;
                PC=(unsigned)PC>>28;
                PC=PC<<28;
                PC=(unsigned)PC|address;
                break;
            }
            case 0x03:
            {
                address=cut_address(iim[i]);
                address=address<<2;
                PC+=4;
                reg[31]=PC;

                PC=(unsigned)PC>>28;
                PC=PC<<28;
                PC=(unsigned)PC|address;
                break;
            }
            case 0x3F:
            {
                //  printf("halt\n");
                flag=1;
                break;
            }


            }

        }
        else PC+=4;

        i=(PC-PC_start)/4;



        cycle++;

        fprintf(snapshot,"\n\n");

        if(flag==1) break;
    }

    fprintf(report,"ICache :\n");
    fprintf(report,"# hits: %d\n# misses: %d\n\n",iCAhit,iCAmiss);
    fprintf(report,"DCache :\n");
    fprintf(report,"# hits: %d\n# misses: %d\n\n",dCAhit,dCAmiss);
    fprintf(report,"ITLB :\n");
    fprintf(report,"# hits: %d\n# misses: %d\n\n",iTLBhit,iTLBmiss);
    fprintf(report,"DTLB :\n");
    fprintf(report,"# hits: %d\n# misses: %d\n\n",dTLBhit,dTLBmiss);
    fprintf(report,"IPageTable :\n");
    fprintf(report,"# hits: %d\n# misses: %d\n\n",iPThit,iPTmiss);
    fprintf(report,"DPageTable :\n");
    fprintf(report,"# hits: %d\n# misses: %d\n\n",dPThit,dPTmiss);

    return 0;
}
