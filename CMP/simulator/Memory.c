#include "Memory.h"

int Ipage_size=8;
int Idisk_size=1024;
int IPT_entries;
int ITLB_entries;
int ICA_size=16;
int ICA_associate=4;
int ICA_entries;
int IMEM_size=64;
int IMEM_entries;

typedef struct TLB_BLOCK
{
    int tag;
    int PPN;
    int last_cycle_used;
}TLB_block;

typedef struct PT_BLOCK
{
    int valid;
    int PPN;
}PT_block;

typedef struct CACHE_BLOCK
{
    int tag;
    int valid;
    int MRU;
}CA_block;

typedef struct MEMORY_BLOCK
{
    int last_cycle_used;
}MEM_block;

TLB_block *ITLB;
PT_block *IPT;
CA_block **ICA;
MEM_block *IMEM;

void initICMP()
{
    int i=0;
    IPT_entries = Idisk_size/Ipage_size;
    ITLB_entries = IPT_entries/4;
    ICA_entries = ICA_size/ICA_associate;
    IMEM_entries = IMEM_size/Ipage_size;
    ITLB = malloc(ITLB_entries * sizeof(TLB_block));
    IPT = malloc(IPT_entries * sizeof(PT_block));
    ICA = malloc(ICA_entries * sizeof(CA_block *));
    for(i=0;i<ICA_entries;i++)
    {
        ICA[i] = malloc(ICA_associate * sizeof(CA_block));
    }
    IMEM = malloc(IMEM_entries * sizeof(MEM_block));



}
void initDCMP(){};
void checkImemory(int VA){};
void checkDmemory(int VA){};
