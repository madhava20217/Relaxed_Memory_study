/**
 * the RAPL power unit that allows portability across architecture for power/energy calculation
 * * This implementation can also detect the topology of a node as block or cyclic, and automatically update the corresponding counter values to the correct socket 
 * Commented counters if any are not used in the current implementation
 * *
 * * Please check your architecture specification for supported counters and other information
 * Written in HiPeC Lab by  
		Sunil Kumar, sunilk@iiitd.ac.in
		Akshat Gupta, akshat17014@iiitd.ac.in
 **/



#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdio.h>  // for printf
#include <stdlib.h>  // for malloc - temp until share memory region allocated
#include <inttypes.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

/* Intel Xeon Power MSR register addresses  (change according to your machine) */
// register value for different scope
#define IA32_PERF_GLOBAL_CTRL_VALUE 0x70000000F // bit {0-3} tells us the number of PMC registers in use (i'th bit implies that PMC[i] is active)
#define IA32_FIXED_CTR_CTRL_VALUE 0x2 // Control register for Fixed Counter 
// Fixed CTRL register
#define IA32_FIXED_CTR_CTRL             0x38D // Controls for fixed ctr0, 1, and 2 
#define IA32_PERF_GLOBAL_CTRL           0x38F // Enables for fixed ctr0,1,and2 here
#define IA32_FIXED_CTR0                 0x309 // (R/W) Counts Instr_Retired.Any
/* RAPL defines */
#define MSR_RAPL_POWER_UNIT             0x606
#define MSR_PKG_ENERGY_STATUS           0x611
/************************************************************************/
// Machine configuration  (change according to your machine)
#define CORESperSOCKET 16
#define SOCKETSperNODE 1
#define NNODES 2
/************************************************************************/


int64_t numOfNodes = -1;
int64_t numOfSockets = -1;
int64_t numOfCores = -1;

uint64_t *energyWrap;
uint64_t *energySave;


static int isBlockTopology = 0; //Assuming non-block (cyclic etc.) topology for the system by default

uint64_t TOTAL_PWR_PKG_ENERGY[SOCKETSperNODE];
uint64_t LAST_PWR_PKG_ENERGY[SOCKETSperNODE];
uint64_t PWR_PKG_ENERGY_Core[SOCKETSperNODE];

uint64_t TOTAL_INST_RETIRED[SOCKETSperNODE * CORESperSOCKET];
uint64_t LAST_INST_RETIRED[SOCKETSperNODE * CORESperSOCKET];
uint64_t INST_RETIRED_CORE[SOCKETSperNODE * CORESperSOCKET];

uint64_t POWER_UNIT = 0;
double JOULE_UNIT = 0.0;  // convert energy counter in JOULE


uint64_t readMSR(uint32_t core , uint32_t name){
    int fd = -1;
    char filename[256];
    sprintf(filename, "/dev/cpu/%d/msr", core);
    fd = open(filename, O_RDONLY);
    if(fd < 0){
    	fprintf (stderr, "\n%s : open failed", filename);
    	return -1;
    }
    uint64_t data;
    if (pread(fd, &data, sizeof(data), name) != sizeof(data)) {
        perror("rdmsr:pread");
        exit(127);
    }
    close(fd);
    return data;
}

int writeMSR(int cpu, uint32_t reg, uint64_t data)
{
  int fd;
  char msr_file_name[64];

  sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
  fd = open(msr_file_name, O_WRONLY);
  if (fd < 0) {
    if (errno == ENXIO) {
      fprintf(stderr, "wrmsr: No CPU %d\n", cpu);
      exit(2);
    } else if (errno == EIO) {
      fprintf(stderr, "wrmsr: CPU %d doesn't support MSRs\n",
        cpu);
      exit(3);
    } else {
      perror("wrmsr@: open");
      exit(127);
    }
  }

    if (pwrite(fd, &data, sizeof data, reg) != sizeof data) {
        if (errno == EIO) {
            fprintf(stderr,
                "wrmsr: CPU %d cannot set MSR "
                "0x%08" PRIx32 " to 0x%016" PRIx64 "\n",
                cpu, reg, data);
            return(4);
        } else {
            perror("wrmsr: pwrite");
            return(127);
        }
    }

  close(fd);

  return(0);
}



/* Function returns the physical package id (socket number) given a cpu */
int get_physical_package_id (int cpu)
{
  char path[256];
  FILE *fileP;
  int physicalPackageId;

  sprintf (path, "/sys/devices/system/cpu/cpu%d/topology/physical_package_id", cpu);

  fileP = fopen (path, "r");
  if (!fileP)
  {
    fprintf (stderr, "\n%s : open failed", path);
    return -1;
  }

  if (fscanf (fileP, "%d", &physicalPackageId) != 1)
  {
    fprintf (stderr, "\n%s: failed to parse from file", path);
    return -1;
  }

  fclose(fileP);
  return physicalPackageId;
}


void perfcounters_dump();
void perfcounters_read();

void perfcounters_init(){

    int sock;
    //Store local copies of the socket and core counts -- check for previous intialization
    if (numOfNodes == -1) numOfNodes = NNODES;
    if (numOfSockets == -1) numOfSockets = SOCKETSperNODE;
    if (numOfCores == -1) numOfCores = CORESperSOCKET; 

    energyWrap = (uint64_t *) malloc (sizeof (uint64_t) * numOfSockets);
    energySave = (uint64_t *) malloc (sizeof (uint64_t) * numOfSockets);
   
	for (int core = 0; core < numOfCores * numOfSockets; core++)
	{
		// set Global Counter to read instruction at user level only
	    writeMSR (core, IA32_PERF_GLOBAL_CTRL, IA32_PERF_GLOBAL_CTRL_VALUE);
       	writeMSR (core, IA32_FIXED_CTR_CTRL, IA32_FIXED_CTR_CTRL_VALUE);
	}

}
void perfcounters_start(){
    //compute power unit
    int correctedCoreNumber;
    int sock;
    POWER_UNIT = readMSR(0, MSR_RAPL_POWER_UNIT); // calculate once
    JOULE_UNIT = 1.0 / (1 << ((POWER_UNIT >> 8) & 0x1F));

    for (sock = 0; sock < numOfSockets; sock++)
    { 
		energyWrap[sock] = 0;
        energySave[sock] = 0;
        PWR_PKG_ENERGY_Core[sock] = 0;
        LAST_PWR_PKG_ENERGY[sock] = 0;
        TOTAL_PWR_PKG_ENERGY[sock] = 0;
        //Discover the topology of the system using the physical id and assign correct cores to sockets 
        if (sock == get_physical_package_id(sock))
        {
            correctedCoreNumber = sock;
        }
        else
        {
            correctedCoreNumber = sock * numOfCores;
            isBlockTopology = 1;
        }
        uint64_t energyStatus = readMSR(correctedCoreNumber, MSR_PKG_ENERGY_STATUS); // get energy MSR

        uint64_t energyCounter = energyStatus & 0xffffffff; // only 32 of 64 bits good 
        if (energyCounter < energySave[sock]) 
        { 
            // did I just wrap the counter?
            energyWrap[sock]++;
        }
        energySave[sock] = energyCounter;
        energyCounter = energyCounter + (energyWrap[sock]<<32);// number of wraps in upper 32 bits
        PWR_PKG_ENERGY_Core[sock] = energyCounter;
    }
	for (int core=0; core<numOfCores * numOfSockets; core++)
	{
		INST_RETIRED_CORE[core]=0;
		LAST_INST_RETIRED[core]=0;
		TOTAL_INST_RETIRED[core]=0;
		INST_RETIRED_CORE[core] = readMSR (core, IA32_FIXED_CTR0);
	}
}

void perfcounters_finalize(){
  perfcounters_dump();
  free(energyWrap);
  free(energySave);
}

void perfcounters_read(){
	int sock;
    int correctedCoreNumber;
	for (sock = 0; sock < numOfSockets; sock++)
	{
        //Discover the topology of the system using the physical id and assign correct cores to sockets 
        if (sock == get_physical_package_id(sock))
        {
            correctedCoreNumber = sock;
        }
        else
        {
            correctedCoreNumber = sock * numOfCores;
            isBlockTopology = 1;
        }

		uint64_t energyStatus = readMSR(correctedCoreNumber, MSR_PKG_ENERGY_STATUS); // get energy MSR

		uint64_t energyCounter = energyStatus & 0xffffffff; // only 32 of 64 bits good 
		if (energyCounter < energySave[sock]) 
		{ 
		  // did I just wrap the counter?
		  energyWrap[sock]++;
		}
		energySave[sock] = energyCounter;
		energyCounter = energyCounter + (energyWrap[sock]<<32);// number of wraps in upper 32 bits
	   
	    LAST_PWR_PKG_ENERGY[sock] = energyCounter - PWR_PKG_ENERGY_Core[sock];
	    TOTAL_PWR_PKG_ENERGY[sock] += LAST_PWR_PKG_ENERGY[sock];
		PWR_PKG_ENERGY_Core[sock] = energyCounter;
	}
	for (int core=0; core<numOfCores * numOfSockets; core++)
	{
		uint64_t instruction = readMSR (core, IA32_FIXED_CTR0);
		LAST_INST_RETIRED[core] = instruction - INST_RETIRED_CORE[core];
		TOTAL_INST_RETIRED[core] += LAST_INST_RETIRED[core];
		INST_RETIRED_CORE[core] = instruction;
	}
    
}

void perfcounters_stop(){
    perfcounters_read();
}

void perfcounters_dump(){
  int i;
    fprintf(stdout,"\n============================ Tabulate Statistics ============================\n");
    // print all events
    fprintf(stdout,"%s\t","PWR_PKG_ENERGY");
	fprintf(stdout,"%s\t","INST_RETIRED");
    fprintf(stdout,"\n");
    
    //printf("power %f \n", LAST_PWR_PKG_ENERGY[0]*JOULE_UNIT);
    double res=0;
    for(i=0; i<numOfSockets; i++) {
      res += ((double)TOTAL_PWR_PKG_ENERGY[i])*JOULE_UNIT;
    }
    fprintf(stdout,"%f\t",res);
	res = 0;
	for(i=0;i<numOfSockets*numOfCores;i++) {
		res += ((double)TOTAL_INST_RETIRED[i]);
	}
    fprintf(stdout,"%f\t",res);
    fprintf(stdout,"\n=============================================================================\n");
    fflush(stdout);

}



int main(){

    perfcounters_init(); // call once
    perfcounters_start();

    sleep(2);
    perfcounters_read();

    printf("power %f \n", LAST_PWR_PKG_ENERGY[0]*JOULE_UNIT);
	printf("inst %ld \n", LAST_INST_RETIRED[0]);

    sleep(2);
    perfcounters_read();
    printf("power %f \n", LAST_PWR_PKG_ENERGY[0]*JOULE_UNIT);
	printf("inst %ld \n", LAST_INST_RETIRED[0]);

    sleep(2);
    perfcounters_read();
    printf("power %f \n", LAST_PWR_PKG_ENERGY[0]*JOULE_UNIT);
	printf("inst %ld \n", LAST_INST_RETIRED[0]);
    
	sleep(2);
    perfcounters_read();
    printf("power %f \n", LAST_PWR_PKG_ENERGY[0]*JOULE_UNIT);
	printf("inst %ld \n", LAST_INST_RETIRED[0]);

    sleep(2);
    perfcounters_read();
    printf("power %f \n", LAST_PWR_PKG_ENERGY[0]*JOULE_UNIT);
	printf("inst %ld \n", LAST_INST_RETIRED[0]);

    perfcounters_stop(); 
    perfcounters_finalize(); // call once

    return 0;
}
