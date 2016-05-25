#include "Memory.h"



typedef struct TLB_BLOCK
{
    int VPN;
    int PPN;
    int last_cycle_used;
    int valid;
} TLB_block;

typedef struct PT_BLOCK
{
    int valid;
    int PPN;
} PT_block;

typedef struct CACHE_BLOCK
{
    int tag;
    int valid;
    int MRU;
} CA_block;

typedef struct MEMORY_BLOCK
{
    int last_cycle_used;
    int valid;
} MEM_block;

TLB_block *ITLB,*DTLB;
PT_block *IPT,*DPT;
CA_block **ICA,**DCA;
MEM_block *IMEM,*DMEM;


/////////////////////////////////INSTRUCTION////////////////////////
void initICMP()
{
    int i=0,j=0;
    Ipage_size=8;
    Idisk_size=1024;
    ICA_size=16;
    ICA_associate=4;
    Iblock_size=4;
    IMEM_size=64;


    IPT_entries = Idisk_size/Ipage_size;
    ITLB_entries = IPT_entries/4;
    ICA_entries = ICA_size/ICA_associate/4;
    IMEM_entries = IMEM_size/Ipage_size;
    ITLB = malloc(ITLB_entries * sizeof(TLB_block));
    IPT = malloc(IPT_entries * sizeof(PT_block));
    IMEM = malloc(IMEM_entries * sizeof(MEM_block));
    ICA = malloc(ICA_entries * sizeof(CA_block *));
    for(i=0; i<ICA_entries; i++)
    {
        ICA[i] = malloc(ICA_associate * sizeof(CA_block));
    }
    for(i=0; i<IPT_entries; i++)
    {
        IPT[i].PPN=0;
        IPT[i].valid=0;
    }
    for(i=0; i<ITLB_entries; i++)
    {
        ITLB[i].last_cycle_used=0;
        ITLB[i].PPN=0;
        ITLB[i].VPN=0;
        ITLB[i].valid=0;
    }
    for(i=0; i<IMEM_entries; i++)
    {
        IMEM[i].last_cycle_used=0;
        IMEM[i].valid=0;
    }
    for(i=0; i<ICA_entries; i++)
    {
        for(j=0; j<ICA_associate; j++)
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


int findITLB(int VPN)
{
    int i;
    for(i=0; i<ITLB_entries; i++)
    {
        if(ITLB[i].VPN==VPN && ITLB[i].valid==1)
        {
            ITLB[i].last_cycle_used=cycle;
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
    int i,j;
    /////////////////SWAP////////////////////
    int PPN=0;
    int min=0x7FFFFFFF;
    int flag=0;
    for(i=0; i<IMEM_entries; i++)
    {
        if(IMEM[i].valid==0)
        {
            PPN=i;
            flag=1;
            break;
        }
        else
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

    if(flag==1)
    {
        IPT[VPN].PPN=PPN;
        IPT[VPN].valid=1;
    }
    else
    {
        for(i=0; i<IPT_entries; i++)
        {
            if(IPT[i].PPN==PPN)
            {
                IPT[i].valid=0;
            }
        }
        IPT[VPN].PPN=PPN;
        IPT[VPN].valid=1;

        for(i=0; i<ITLB_entries; i++)
        {
            if(ITLB[i].PPN==PPN)
            {
                ITLB[i].valid=0;
            }
        }
        for(j=0; j<Dpage_size; j+=4)
        {
            int PA = PPN * Ipage_size + j;
            int PAB = PA / Iblock_size;
            int index = PAB % ICA_entries;
            int tag = PA / Iblock_size / ICA_entries;

            for(i=0; i<ICA_entries; i++)
            {
                if(ICA[index][i].tag==tag)
                {
                    ICA[index][i].valid=0;
                    ICA[index][i].MRU=0;
                }
            }
        }
    }


    ////////////////UPDATE TLB//////////////////
    min=0x7FFFFFFF;
    int temp;
    for(i=0; i<ITLB_entries; i++)
    {
        if(ITLB[i].valid==0)
        {
            temp=i;
            break;
        }
        else
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
    for(i=0; i<ITLB_entries; i++)
    {
        if(ITLB[i].valid==0)
        {
            temp=i;
            break;
        }
        else
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
int findICA(int PPN)
{
    int i;
    int PA = PPN * Ipage_size + IPageoffset;
    int PAB = PA / Iblock_size;
    int index = PAB % ICA_entries;
    int tag = PA / Iblock_size / ICA_entries;

    for(i=0; i<ICA_associate; i++)
    {
        if(tag == ICA[index][i].tag && ICA[index][i].valid == 1)
        {
            ICA[index][i].MRU=1;
            return 1;
        }
    }
    return -1;
}

void ICAmiss(int PPN)
{
    int i;
    int PA = PPN * Ipage_size + IPageoffset;
    int PAB = PA / Iblock_size;
    int index =PAB % ICA_entries;
    int tag = PA / Iblock_size / ICA_entries;
    int flag=0;
    int put;

    if(ICA_associate==1)
    {
        ICA[index][0].MRU=0;
        ICA[index][0].tag=tag;
        ICA[index][0].valid=1;
    }
    else
    {
        for(i=0; i<ICA_associate; i++)
        {
            if(ICA[index][i].MRU==0)
            {

                if(flag==0)
                {
                    put = i;
                    flag=1;
                }
                else
                {
                    flag=2;
                    break;
                }
            }
        }
        if(flag==1)
        {
            for(i=0; i<ICA_associate; i++)
            {
                ICA[index][i].MRU=0;
            }
        }
        ICA[index][put].MRU=1;
        ICA[index][put].tag=tag;
        ICA[index][put].valid=1;
    }
    IMEM[PPN].last_cycle_used=cycle;

}

void checkImemory(int VA)
{
    IVPN = VA / Ipage_size;
    IPageoffset = VA % Ipage_size;
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
        else            ///PT hit
        {
            iPThit++;
            ITLBmiss(IVPN);
        }

    }
    else               ///TLB hit
    {
        iTLBhit++;
    }
    int find=0;
    IPPN = findITLB(IVPN);
    find = findICA(IPPN);
    if(find==-1)        ///CA miss
    {
        iCAmiss++;
        ICAmiss(IPPN);
    }
    else                ///CA hit
    {
        iCAhit++;
    }

}

////////////////////////////////DATA//////////////////////

void initDCMP()
{
    int i=0,j=0;
    Dpage_size=16;
    Ddisk_size=1024;
    DCA_size=16;
    DCA_associate=1;
    Dblock_size=4;
    DMEM_size=32;


    DPT_entries = Ddisk_size/Dpage_size;
    DTLB_entries = DPT_entries/4;
    DCA_entries = DCA_size/DCA_associate/4;
    DMEM_entries = DMEM_size/Dpage_size;
    DTLB = malloc(DTLB_entries * sizeof(TLB_block));
    DPT = malloc(DPT_entries * sizeof(PT_block));
    DMEM = malloc(DMEM_entries * sizeof(MEM_block));
    DCA = malloc(DCA_entries * sizeof(CA_block *));
    for(i=0; i<DCA_entries; i++)
    {
        DCA[i] = malloc(DCA_associate * sizeof(CA_block));
    }
    for(i=0; i<DPT_entries; i++)
    {
        DPT[i].PPN=0;
        DPT[i].valid=0;
    }
    for(i=0; i<DTLB_entries; i++)
    {
        DTLB[i].last_cycle_used=0;
        DTLB[i].PPN=0;
        DTLB[i].VPN=0;
        DTLB[i].valid=0;
    }
    for(i=0; i<DMEM_entries; i++)
    {
        DMEM[i].last_cycle_used=0;
        DMEM[i].valid=0;
    }
    for(i=0; i<DCA_entries; i++)
    {
        for(j=0; j<DCA_associate; j++)
        {
            DCA[i][j].MRU=0;
            DCA[i][j].tag=0;
            DCA[i][j].valid=0;
        }
    }

    dTLBhit=0;
    dTLBmiss=0;
    dPThit=0;
    dPTmiss=0;
}

int findDTLB(int VPN)
{
    int i;
    for(i=0; i<DTLB_entries; i++)
    {
        if(DTLB[i].VPN==VPN && DTLB[i].valid==1)
        {
            DTLB[i].last_cycle_used=cycle;
            return DTLB[i].PPN;
        }

    }
    return -1;
}

int findDPT(int VPN)
{
    if(DPT[VPN].valid==1)
        return DPT[VPN].PPN;
    else
        return -1;
}

void DPTmiss(int VPN)
{
    int i,j;
    /////////////////SWAP////////////////////
    int PPN=0;
    int min=0x7FFFFFFF;
    int flag=0;
    for(i=0; i<DMEM_entries; i++)
    {
        if(DMEM[i].valid==0)
        {
            PPN=i;
            flag=1;
            break;
        }
        else
        {
            if(DMEM[i].last_cycle_used<min)
            {
                min=DMEM[i].last_cycle_used;
                PPN=i;
            }
        }
    }
    DMEM[PPN].last_cycle_used=cycle;
    DMEM[PPN].valid=1;
    /////////////////UPDATE PT//////////////////
    if(flag==1)
    {
        DPT[VPN].PPN=PPN;
        DPT[VPN].valid=1;
    }
    else
    {
        for(i=0; i<DPT_entries; i++)
        {
            if(DPT[i].PPN==PPN)
            {
                DPT[i].valid=0;
            }
        }
        DPT[VPN].PPN=PPN;
        DPT[VPN].valid=1;
        for(i=0; i<DTLB_entries; i++)
        {
            if(DTLB[i].PPN==PPN)
            {
                DTLB[i].valid=0;
            }
        }

        for(j=0; j<Dpage_size; j+=4)
        {
            int PA = PPN * Dpage_size + j;
            int PAB = PA / Dblock_size;
            int index = PAB % DCA_entries;
            int tag = PA / Dblock_size / DCA_entries;

            for(i=0; i<DCA_entries; i++)
            {
                if(DCA[index][i].tag==tag)
                {
                    DCA[index][i].valid=0;
                    DCA[index][i].MRU=0;
                }
            }
        }


    }


    ////////////////UPDATE TLB//////////////////
    min=0x7FFFFFFF;
    int temp;
    for(i=0; i<DTLB_entries; i++)
    {
        if(DTLB[i].valid==0)
        {
            temp=i;
            break;
        }
        else
        {
            if(DTLB[i].last_cycle_used<min)
            {
                min=DTLB[i].last_cycle_used;
                temp=i;
            }
        }
    }

    DTLB[temp].last_cycle_used=cycle;
    DTLB[temp].valid=1;
    DTLB[temp].PPN=PPN;
    DTLB[temp].VPN=VPN;

}

void DTLBmiss(int VPN)
{
    int min=0x7FFFFFFF;
    int i;
    int temp=0;
    int PPN;
    PPN = DPT[VPN].PPN;
    for(i=0; i<DTLB_entries; i++)
    {
        if(DTLB[i].valid==0)
        {
            temp=i;
            break;
        }
        else
        {
            if(DTLB[i].last_cycle_used<min)
            {
                min=DTLB[i].last_cycle_used;
                temp=i;
            }
        }
    }

    DTLB[temp].last_cycle_used=cycle;
    DTLB[temp].valid=1;
    DTLB[temp].PPN=PPN;
    DTLB[temp].VPN=VPN;
}
int findDCA(int PPN)
{
    int i;
    int PA = PPN * Dpage_size + DPageoffset;
    int PAB = PA / Dblock_size;
    int index = PAB % DCA_entries;
    int tag = PA / Dblock_size / DCA_entries;

    for(i=0; i<DCA_associate; i++)
    {
        if(tag == DCA[index][i].tag && DCA[index][i].valid == 1)
        {
            DCA[index][i].MRU=1;
            return 1;
        }
    }
    return -1;
}

void DCAmiss(int PPN)
{
    int i;
    int PA = PPN * Dpage_size + DPageoffset;
    int PAB = PA / Dblock_size;
    int index =PAB % DCA_entries;
    int tag = PA / Dblock_size / DCA_entries;
    int flag=0;
    int put;
    if(DCA_associate==1)
    {
        DCA[index][0].MRU=0;
        DCA[index][0].tag=tag;
        DCA[index][0].valid=1;
    }
    else
    {
        for(i=0; i<DCA_associate; i++)
        {
            if(DCA[index][i].MRU==0)
            {

                if(flag==0)
                {
                    put = i;
                    flag=1;
                }
                else
                {
                    flag=2;
                    break;
                }
            }
        }
        if(flag==1)
        {
            for(i=0; i<DCA_associate; i++)
            {
                DCA[index][i].MRU=0;
            }
        }
        DCA[index][put].MRU=1;
        DCA[index][put].tag=tag;
        DCA[index][put].valid=1;
    }

    DMEM[PPN].last_cycle_used=cycle;

}

void checkDmemory(int VA)
{
    DVPN = VA / Dpage_size;
    DPageoffset = VA % Dpage_size;
    DPPN = findDTLB(DVPN);

    if(DPPN==-1)           ///TLB miss
    {
        dTLBmiss++;
        DPPN=findDPT(DVPN);

        if(DPPN==-1)      ///PT miss
        {
            dPTmiss++;
            DPTmiss(DVPN);
        }
        else            ///PT hit
        {
            dPThit++;
            DTLBmiss(DVPN);
        }

    }
    else               ///TLB hit
    {
        dTLBhit++;
    }
    int find=0;
    DPPN = findDTLB(DVPN);
    find = findDCA(DPPN);

    if(find==-1)        ///CA miss
    {
        dCAmiss++;
        DCAmiss(DPPN);
    }
    else                ///CA hit
    {
        dCAhit++;
    }

}

