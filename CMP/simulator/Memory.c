#include "Memory.h"

int Ipage_size=8;
int Idisk_size=1024;
int IPT_entries;
int ITLB_entries;
int ICA_size=16;
int ICA_associate=4;
int ICA_entries;
int Iblock_size=4;
int IMEM_size=64;
int IMEM_entries;

int IVPN;
int IPPN;

int iTLBmiss;
int iTLBhit;
int iPTmiss;
int iPThit;

typedef struct TLB_BLOCK
{
    int VPN;
    int PPN;
    int last_cycle_used;
    int valid;
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
    int valid;
}MEM_block;

TLB_block *ITLB;
PT_block *IPT;
CA_block **ICA;
MEM_block *IMEM;

void initICMP()
{
    int i=0,j=0;
    IPT_entries = Idisk_size/Ipage_size;
    ITLB_entries = IPT_entries/4;
    ICA_entries = ICA_size/ICA_associate/Iblock_size;
    IMEM_entries = IMEM_size/Ipage_size;
    ITLB = malloc(ITLB_entries * sizeof(TLB_block));
    IPT = malloc(IPT_entries * sizeof(PT_block));
    ICA = malloc(ICA_entries * sizeof(CA_block *));
    for(i=0;i<ICA_entries;i++)
    {
        ICA[i] = malloc(ICA_associate * sizeof(CA_block));
    }
    IMEM = malloc(IMEM_entries * sizeof(MEM_block));
    for(i=0;i<IPT_entries;i++)
    {
        IPT[i].PPN=0;
        IPT[i].valid=0;
    }
    for(i=0;i<ITLB_entries;i++)
    {
        ITLB[i].last_cycle_used=0;
        ITLB[i].PPN=0;
        ITLB[i].VPN=0;
        ITLB[i].valid=0;
    }
    for(i=0;i<IMEM_entries;i++)
    {
        IMEM[i].last_cycle_used=0;
        IMEM[i].valid=0;
    }
    for(i=0;i<ICA_entries;i++)
    {
        for(j=0;j<ICA_associate;j++)
        {
            ICA[i][j].MRU=0;
            ICA[i][j].tag=0;
            ICA[i][j].valid=0;
        }
    }

    iTLBhit=0;
    iTLBmiss=0;
    iPThit=0;
    iPTmiss=0;


}
void initDCMP(){};

int findITLB(int VPN)
{
    int i;
    for(i=0;i<ITLB_entries;i++)
    {
        if(ITLB[i].VPN==VPN && ITLB[i].valid==1)
        {
            return ITLB[i].PPN;
        }

    }
    return -1;
}

int findIPT(int VPN)
{
    if(IPT[VPN].valid==1)
        return IPT[VPN].PPN;
    else
        return -1;
}

void IPTmiss(int VPN)
{
    int i;
    /////////////////SWAP////////////////////
    int PPN=0;
    int min=0x7FFFFFFF;
    for(i=0;i<IMEM_entries;i++)
    {
        if(IMEM[i].valid==0)
        {
            PPN=i;
            break;
        }else
        {
          if(IMEM[i].last_cycle_used<min)
            {
                min=IMEM[i].last_cycle_used;
                PPN=i;
            }
        }
    }
    IMEM[PPN].last_cycle_used=cycle;
    IMEM[PPN].valid=1;
    /////////////////UPDATE PT//////////////////

    IPT[VPN].PPN=PPN;
    IPT[VPN].valid=1;

    ////////////////UPDATE TLB//////////////////
    min=0x7FFFFFFF;
    int temp;
    for(i=0;i<ITLB_entries;i++)
    {
        if(ITLB[i].valid==0)
        {
            temp=i;
            break;
        }else
        {
          if(ITLB[i].last_cycle_used<min)
            {
                min=ITLB[i].last_cycle_used;
                temp=i;
            }
        }
    }

    ITLB[temp].last_cycle_used=cycle;
    ITLB[temp].valid=1;
    ITLB[temp].PPN=PPN;
    ITLB[temp].VPN=VPN;

}

void ITLBmiss(int VPN)
{
    int min=0x7FFFFFFF;
    int i;
    int temp=0;
    int PPN;
    PPN = IPT[VPN].PPN;
    for(i=0;i<ITLB_entries;i++)
    {
        if(ITLB[i].valid==0)
        {
            temp=i;
            break;
        }else
        {
          if(ITLB[i].last_cycle_used<min)
            {
                min=ITLB[i].last_cycle_used;
                temp=i;
            }
        }
    }

    ITLB[temp].last_cycle_used=cycle;
    ITLB[temp].valid=1;
    ITLB[temp].PPN=PPN;
    ITLB[temp].VPN=VPN;
}

void checkImemory(int VA){
    IVPN = VA / Ipage_size;
    IPPN = findITLB(IVPN);

    if(IPPN==-1)           ///TLB miss
    {
        iTLBmiss++;
        IPPN=findIPT(IVPN);

        if(IPPN==-1)      ///PT miss
        {
            iPTmiss++;
            IPTmiss(IVPN);
        }
        else{           ///PT hit
            iPThit++;
            ITLBmiss(IVPN);
        }

    }else              ///TLB hit
    {
        iTLBhit++;
    }
    int find=0;
    //find = findICA(PPN);
    if(find==-1)        ///CA miss
    {
     //   ICAmiss();
    }else               ///CA hit
    {

    }

};
void checkDmemory(int VA){};
