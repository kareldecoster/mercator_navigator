/* This mercator_data_acquisition_unit.c program is a modified version of pruadc.c and mem2file
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
 
#ifndef MERCATOR_DATA_ACQUISITION_UNIT_H
#define MERCATOR_DATA_ACQUISITION_UNIT_H
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

typedef enum {    // measured and calibrated, but can be calculated
	FREQ_12_5MHz =  1,
	FREQ_6_25MHz =  5,
	FREQ_5MHz    =  7,
	FREQ_3_85MHz = 10,
	FREQ_1MHz   =  45,
	FREQ_500kHz =  95,
	FREQ_250kHz = 245,
	FREQ_100kHz = 495,
	FREQ_76_8kHz = 658,
	FREQ_25kHz = 1995,
	FREQ_10kHz = 4995,
	FREQ_5kHz =  9995,
	FREQ_2kHz = 24995,
	FREQ_1kHz = 49995
}FREQUENCY_t;

/* This method should be called first. It loads the required Device Tree Overlay (/sys/firmware/EBB-PRU-ADC-00A0.dtbo). 
 * The following pins should be free to use, this means that HDMI should be disabled. 
 * p9_27 : SPI_CSO   -- Chip Select
 * p9_28 : SPI_D0    -- MISO
 * p9_29 : SPI_SCLK  -- Clock
 * p9_30 : SPI_D1    -- MOSI
 * It allso allocates frame_size memory where the PRU's will write to. The sampling_frequency should be set here. VREF is the reference voltage
 * used by the A/D converter.
 */
unsigned int mdau_create(uint32_t frame_size, FREQUENCY_t samping_frequency, double VREF);


/* This method frees the shared memory and disables the PRU's again.
 * Take not that the Device Tree Overlay will NOT be unloaded, as doing this may result in an unstable system.
 */
unsigned int mdau_destroy();


/* This method programs the PRU's and makes them execute their program. ./PRUADC.bin and ./PRUClock.bin should be present.
 * The PRU's fill the shared memory with 10-bit samples stored as uint_16. This method allocates memory to copy these values as an array
 * of doubles and returns a pointer to this array. 
 */
double* mdau_read_frame();

#endif
