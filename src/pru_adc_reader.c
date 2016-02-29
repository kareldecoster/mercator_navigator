/* This pru_adc_reader.c program is a modified version of pruadc.c and mem2file
 * by Derek Molloy. The intention of this program is that it allows a BeagleBone
 * Black to communicate with an MCP3008/MCP3004 A/D converter.
 * 
 * This mem2file.c program is a modified version of devmem2 by Jan-Derk Bakker
 * as referenced below. This program was modified by Derek Molloy for the book
 * Exploring BeagleBone. It is used in Chapter 13 to dump the DDR External Memory
 * pool to a file. See: www.exploringbeaglebone.com/chapter13/
 *
 * devmem2.c: Simple program to read/write from/to any location in memory.
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 * The author can be reached at:
 *
 *  Jan-Derk Bakker
 *  Information and Communication Theory Group
 *  Faculty of Information Technology and Systems
 *  Delft University of Technology
 *  P.O. Box 5031
 *  2600 GA Delft
 *  The Netherlands
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include <pru_adc_reader.h>

#define ADC_PRU_NUM	   0   // using PRU0 for the ADC capture
#define CLK_PRU_NUM	   1   // using PRU1 for the sample clock
#define MMAP0_LOC   "/sys/class/uio/uio0/maps/map0/"
#define MMAP1_LOC   "/sys/class/uio/uio0/maps/map1/"
#define MAP_SIZE 0x0FFFFFFF
#define MAP_MASK (MAP_SIZE - 1)
#define ADC_MAXIMUM		1024	// MCP3008 is a 10 BIT ADC


enum CONTROL {
	PAUSED = 0,
	RUNNING = 1,
	UPDATE = 3
};

double VREF = 0;

// Short function to load a single unsigned int from a sysfs entry
unsigned int readFileValue(char filename[]){
   FILE* fp;
   unsigned int value = 0;
   fp = fopen(filename, "rt");
   fscanf(fp, "%x", &value);
   fclose(fp);
   return value;
}

/* This method should be called first. It loads the required Device Tree Overlay ( /sys/firmware/EBB-PRU-ADC-00A0.dtbo ). 
 * The following pins should be free to use, this means that HDMI should be disabled. 
 * p9_27 : SPI_CSO   -- Chip Select
 * p9_28 : SPI_D0    -- MISO
 * p9_29 : SPI_SCLK  -- Clock
 * p9_30 : SPI_D1    -- MOSI
 * It allso allocates frame_size memory where the PRU's will write to. The sampling_frequency should be set here. VREF is the reference voltage
 * used by the A/D converter.
 */
unsigned int pru_adc_reader_create(uint32_t frame_size, FREQUENCY_t sampling_frequency, double vref){
	if(getuid()!=0){
		printf("You must run this program as root. Exiting.\n");
		exit(EXIT_FAILURE);
	}
	//Load the Device Tree Overlay.
	system("echo EBB-PRU-ADC>/sys/devices/bone_capemgr.9/slots");
	
	VREF = vref;
	
	//Resize the default allocated shared memory to fit frame_size.
	system("rmmod uio_pruss");
	char modprobe_cmd[80];
	if( 0 == sprintf(modprobe_cmd, "modprobe uio_pruss extram_pool_sz=%#x",2*frame_size)){
		return EXIT_FAILURE;
	}
	system(modprobe_cmd);

	// Read in the location and address of the shared memory. This value changes
	// each time a new block of memory is allocated.
	unsigned int timerData[2];
	timerData[0] = sampling_frequency;
	timerData[1] = RUNNING;
	printf("The PRU clock state is set as period: %d (0x%x) and state: %d\n", timerData[0], timerData[0], timerData[1]);
	unsigned int PRU_data_addr = readFileValue(MMAP0_LOC "addr");
	printf("-> the PRUClock memory is mapped at the base address: %x\n", (PRU_data_addr + 0x2000));
	printf("-> the PRUClock on/off state is mapped at address: %x\n", (PRU_data_addr + 0x10000));

	// data for PRU0 based on the MCPXXXX datasheet
	unsigned int spiData[3];
	spiData[0] = 0x01800000;
	spiData[1] = readFileValue(MMAP1_LOC "addr");
	spiData[2] = readFileValue(MMAP1_LOC "size");
	printf("Sending the SPI Control Data: 0x%x\n", spiData[0]);
	printf("The DDR External Memory pool has location: 0x%x and size: 0x%x bytes\n", spiData[1], spiData[2]);
	int numberSamples = spiData[2]/2;
	printf("-> this space has capacity to store %d 16-bit samples (max)\n", numberSamples);

	// Allocate and initialize memory
	prussdrv_init ();
	prussdrv_open (PRU_EVTOUT_0);

	// Write the address and size into PRU0 Data RAM0. You can edit the value to
	// PRUSS0_PRU1_DATARAM if you wish to write to PRU1
	prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, spiData, 12);  // spi code
	prussdrv_pru_write_memory(PRUSS0_PRU1_DATARAM, 0, timerData, 8); // sample clock


	
	return EXIT_SUCCESS;
}

/* This method frees the shared memory and disables the PRU's again. 
 * Take not that the Device Tree Overlay will NOT be unloaded, as doing this may result in an unstable system.
 */
unsigned int pru_adc_reader_destroy(){
	// Disable PRU and close memory mappings 
	prussdrv_pru_disable(ADC_PRU_NUM);
	prussdrv_pru_disable(CLK_PRU_NUM);

	prussdrv_exit ();

	if(system("rmmod uio_pruss")==1){
		return EXIT_FAILURE;
	}	
	return EXIT_SUCCESS;
}

/* This method programs the PRU's and makes them execute their program. ./PRUADC.bin and ./PRUClock.bin should be present.
 * The PRU's fill the shared memory with 10-bit samples stored as uint_16. This method allocates memory to copy these values as an array
 * of doubles and returns a pointer to this array. Returns NULL if VREF are not set. To set these, call pru_adc_reader_create.
 */
double* pru_adc_reader_read_frame(){
	if(VREF == 0 ){
		return NULL;
	}
	
	// Initialize structure used by prussdrv_pruintc_intc
	// PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	
	// Map the PRU's interrupts
	prussdrv_pruintc_init(&pruss_intc_initdata);
	
	// Load and execute the PRU program on the PRU
	prussdrv_exec_program (ADC_PRU_NUM, "./PRUADC.bin");
	prussdrv_exec_program (CLK_PRU_NUM, "./PRUClock.bin");
	printf("EBBClock PRU1 program now running\n");

	// Wait for event completion from PRU, returns the PRU_EVTOUT_0 number
	int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);
	printf("EBBADC PRU0 program completed, event number %d.\n", n);
	
	int fd;
    void *map_base, *virt_addr;
    unsigned long read_result;
    unsigned int addr = readFileValue(MMAP1_LOC "addr");
    unsigned int dataSize = readFileValue(MMAP1_LOC "size");
    unsigned int frame_size = dataSize / 2;
    off_t target = addr;

    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1){
	printf("Failed to open memory!");
	return NULL;
    }
    fflush(stdout);

    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
       printf("Failed to map base address");
       return NULL;
    }
    fflush(stdout);
    
	double* samples = malloc(frame_size*sizeof(double));
	if(samples == NULL){
		return NULL;
	}
	
    int i=0;
    for(i=0; i<frame_size; i++){
	virt_addr = map_base + (target & MAP_MASK);
        read_result = *((uint16_t *) virt_addr);
        //printf("Value at address 0x%X (%p): 0x%X\n", target, virt_addr, read_result);
        //printf("%d %lu\n",i, read_result);
        samples[i] = VREF * read_result / ADC_MAXIMUM ;
        target+=2;                   // 2 bytes per sample
    }
    fflush(stdout);

    if(munmap(map_base, MAP_SIZE) == -1) {
       printf("Failed to unmap memory");
       return NULL;
    }
    close(fd);
    return samples;
}



