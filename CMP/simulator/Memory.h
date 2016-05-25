#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void checkDmemory(int VA);
void checkImemory(int VA);
void initICMP();
void initDCMP();
int findITLB(int VPN);
int findIPT(int VPN);
void IPTmiss(int VPN);
void ITLBmiss(int VPN);
int findICA(int PPN);
void ICAmiss(int PPN);
int findDTLB(int VPN);
int findDPT(int VPN);
void DPTmiss(int VPN);
void DTLBmiss(int VPN);
int findDCA(int PPN);
void DCAmiss(int PPN);

extern int cycle;

int Ipage_size;
int Idisk_size;
int IPT_entries;
int ITLB_entries;
int ICA_size;
int ICA_associate;
int ICA_entries;
int Iblock_size;
int IMEM_size;
int IMEM_entries;
int IPageoffset;

int Dpage_size;
int Ddisk_size;
int DPT_entries;
int DTLB_entries;
int DCA_size;
int DCA_associate;
int DCA_entries;
int Dblock_size;
int DMEM_size;
int DMEM_entries;
int DPageoffset;

int IVPN;
int IPPN;

int DVPN;
int DPPN;

int iTLBmiss;
int iTLBhit;
int iPTmiss;
int iPThit;
int iCAmiss;
int iCAhit;


int dTLBmiss;
int dTLBhit;
int dPTmiss;
int dPThit;
int dCAmiss;
int dCAhit;
