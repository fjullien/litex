// This file is Copyright (c) 2013-2014 Sebastien Bourdeauducq <sb@m-labs.hk>
// This file is Copyright (c) 2013-2020 Florent Kermarrec <florent@enjoy-digital.fr>
// This file is Copyright (c) 2018 Chris Ballance <chris.ballance@physics.ox.ac.uk>
// This file is Copyright (c) 2018 Dolu1990 <charles.papon.90@gmail.com>
// This file is Copyright (c) 2019 Gabriel L. Somlo <gsomlo@gmail.com>
// This file is Copyright (c) 2018 Jean-François Nguyen <jf@lambdaconcept.fr>
// This file is Copyright (c) 2018 Sergiusz Bazanski <q3k@q3k.org>
// This file is Copyright (c) 2018 Tim 'mithro' Ansell <me@mith.ro>
// License: BSD

#include <generated/csr.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef CSR_SDRAM_BASE
#include <generated/sdram_phy.h>
#endif
#include <generated/mem.h>
#include <hw/flags.h>
#include <system.h>

#include "sdram.h"

// FIXME(hack): If we don't have main ram, just target the sram instead.
#ifndef MAIN_RAM_BASE
#define MAIN_RAM_BASE SRAM_BASE
#endif

__attribute__((unused)) static void cdelay(int i)
{
	while(i > 0) {
#if defined (__lm32__)
		__asm__ volatile("nop");
#elif defined (__or1k__)
		__asm__ volatile("l.nop");
#elif defined (__picorv32__)
		__asm__ volatile("nop");
#elif defined (__vexriscv__)
		__asm__ volatile("nop");
#elif defined (__minerva__)
		__asm__ volatile("nop");
#elif defined (__rocket__)
		__asm__ volatile("nop");
#elif defined (__powerpc__)
		__asm__ volatile("nop");
#elif defined (__microwatt__)
		__asm__ volatile("nop");
#elif defined (__blackparrot__)
		__asm__ volatile("nop");
#elif defined (__serv__)
		__asm__ volatile("nop");
#else
#error Unsupported architecture
#endif
		i--;
	}
}

#ifdef CSR_SDRAM_BASE

#define DFII_ADDR_SHIFT CONFIG_CSR_ALIGNMENT/8

#define CSR_DATA_BYTES CONFIG_CSR_DATA_WIDTH/8

#define DFII_PIX_DATA_BYTES DFII_PIX_DATA_SIZE*CSR_DATA_BYTES

void sdrsw(void)
{
	sdram_dfii_control_write(DFII_CONTROL_CKE|DFII_CONTROL_ODT|DFII_CONTROL_RESET_N);
	printf("SDRAM now under software control\n");
}

void sdrhw(void)
{
	sdram_dfii_control_write(DFII_CONTROL_SEL);
	printf("SDRAM now under hardware control\n");
}

void sdrrow(char *_row)
{
	char *c;
	unsigned int row;

	if(*_row == 0) {
		sdram_dfii_pi0_address_write(0x0000);
		sdram_dfii_pi0_baddress_write(0);
		command_p0(DFII_COMMAND_RAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
		cdelay(15);
		printf("Precharged\n");
	} else {
		row = strtoul(_row, &c, 0);
		if(*c != 0) {
			printf("incorrect row\n");
			return;
		}
		sdram_dfii_pi0_address_write(row);
		sdram_dfii_pi0_baddress_write(0);
		command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CS);
		cdelay(15);
		printf("Activated row %d\n", row);
	}
}

void sdrrdbuf(int dq)
{
	int i, p;
	int first_byte, step;
	unsigned char buf[DFII_PIX_DATA_BYTES];

	if(dq < 0) {
		first_byte = 0;
		step = 1;
	} else {
		first_byte = DFII_PIX_DATA_BYTES/2 - 1 - dq;
		step = DFII_PIX_DATA_BYTES/2;
	}

	for(p=0;p<SDRAM_PHY_PHASES;p++) {
		csr_rd_buf_uint8(sdram_dfii_pix_rddata_addr[p],
				 buf, DFII_PIX_DATA_BYTES);
		for(i=first_byte;i<DFII_PIX_DATA_BYTES;i+=step)
			printf("%02x", buf[i]);
	}
	printf("\n");
}

void sdrrd(char *startaddr, char *dq)
{
	char *c;
	unsigned int addr;
	int _dq;

	if(*startaddr == 0) {
		printf("sdrrd <address>\n");
		return;
	}
	addr = strtoul(startaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	if(*dq == 0)
		_dq = -1;
	else {
		_dq = strtoul(dq, &c, 0);
		if(*c != 0) {
			printf("incorrect DQ\n");
			return;
		}
	}

	sdram_dfii_pird_address_write(addr);
	sdram_dfii_pird_baddress_write(0);
	command_prd(DFII_COMMAND_CAS|DFII_COMMAND_CS|DFII_COMMAND_RDDATA);
	cdelay(15);
	sdrrdbuf(_dq);
}

void sdrrderr(char *count)
{
	int addr;
	char *c;
	int _count;
	int i, j, p;
	unsigned char prev_data[SDRAM_PHY_PHASES][DFII_PIX_DATA_BYTES];
	unsigned char errs[SDRAM_PHY_PHASES][DFII_PIX_DATA_BYTES];
	unsigned char new_data[DFII_PIX_DATA_BYTES];

	if(*count == 0) {
		printf("sdrrderr <count>\n");
		return;
	}
	_count = strtoul(count, &c, 0);
	if(*c != 0) {
		printf("incorrect count\n");
		return;
	}

	for(p=0;p<SDRAM_PHY_PHASES;p++)
		for(i=0;i<DFII_PIX_DATA_BYTES;i++)
			errs[p][i] = 0;

	for(addr=0;addr<16;addr++) {
		sdram_dfii_pird_address_write(addr*8);
		sdram_dfii_pird_baddress_write(0);
		command_prd(DFII_COMMAND_CAS|DFII_COMMAND_CS|DFII_COMMAND_RDDATA);
		cdelay(15);
		for(p=0;p<SDRAM_PHY_PHASES;p++)
			csr_rd_buf_uint8(sdram_dfii_pix_rddata_addr[p],
					 prev_data[p], DFII_PIX_DATA_BYTES);

		for(j=0;j<_count;j++) {
			command_prd(DFII_COMMAND_CAS|DFII_COMMAND_CS|DFII_COMMAND_RDDATA);
			cdelay(15);
			for(p=0;p<SDRAM_PHY_PHASES;p++) {
				csr_rd_buf_uint8(sdram_dfii_pix_rddata_addr[p],
						 new_data, DFII_PIX_DATA_BYTES);
				for(i=0;i<DFII_PIX_DATA_BYTES;i++) {
					errs[p][i] |= prev_data[p][i] ^ new_data[i];
					prev_data[p][i] = new_data[i];
				}
			}
		}
	}

	for(p=0;p<SDRAM_PHY_PHASES;p++)
		for(i=0;i<DFII_PIX_DATA_BYTES;i++)
			printf("%02x", errs[p][i]);
	printf("\n");
	for(p=0;p<SDRAM_PHY_PHASES;p++)
		for(i=0;i<DFII_PIX_DATA_BYTES;i++)
			printf("%2x", DFII_PIX_DATA_BYTES/2 - 1 - (i % (DFII_PIX_DATA_BYTES/2)));
	printf("\n");
}

void sdrwr(char *startaddr)
{
	int i, p;
	char *c;
	unsigned int addr;
	unsigned char buf[DFII_PIX_DATA_BYTES];

	if(*startaddr == 0) {
		printf("sdrwr <address>\n");
		return;
	}
	addr = strtoul(startaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}

	for(p=0;p<SDRAM_PHY_PHASES;p++) {
		for(i=0;i<DFII_PIX_DATA_BYTES;i++)
			buf[i] = 0x10*p + i;
		csr_wr_buf_uint8(sdram_dfii_pix_wrdata_addr[p],
				 buf, DFII_PIX_DATA_BYTES);
	}

	sdram_dfii_piwr_address_write(addr);
	sdram_dfii_piwr_baddress_write(0);
	command_pwr(DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS|DFII_COMMAND_WRDATA);
}

#ifdef CSR_DDRPHY_BASE

#if defined(DDRPHY_CMD_DELAY) || defined(USDDRPHY_DEBUG)
void ddrphy_cdly(unsigned int delay) {
	printf("Setting clk/cmd delay to %d taps\n", delay);
#if CSR_DDRPHY_EN_VTC_ADDR
	ddrphy_en_vtc_write(0);
#endif
	ddrphy_cdly_rst_write(1);
	while (delay > 0) {
		ddrphy_cdly_inc_write(1);
		cdelay(1000);
		delay--;
	}
#if CSR_DDRPHY_EN_VTC_ADDR
	ddrphy_en_vtc_write(1);
#endif
}
#endif

#ifdef SDRAM_PHY_WRITE_LEVELING_CAPABLE
void sdrwlon(void)
{
	sdram_dfii_pi0_address_write(DDRX_MR1 | (1 << 7));
	sdram_dfii_pi0_baddress_write(1);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
	ddrphy_wlevel_en_write(1);
}

void sdrwloff(void)
{
	sdram_dfii_pi0_address_write(DDRX_MR1);
	sdram_dfii_pi0_baddress_write(1);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
	ddrphy_wlevel_en_write(0);
}

static void write_delay_rst(int module) {
#ifdef SDRAM_PHY_WRITE_LEVELING_REINIT
	int i;
#endif

	/* sel module */
	ddrphy_dly_sel_write(1 << module);

	/* rst delay */
	ddrphy_wdly_dq_rst_write(1);
	ddrphy_wdly_dqs_rst_write(1);
#ifdef SDRAM_PHY_WRITE_LEVELING_REINIT
	for(i=0; i<ddrphy_half_sys8x_taps_read(); i++)
		ddrphy_wdly_dqs_inc_write(1);
#endif

	/* unsel module */
	ddrphy_dly_sel_write(0);
}

static void write_delay_inc(int module) {
	/* sel module */
	ddrphy_dly_sel_write(1 << module);

	/* inc delay */
	ddrphy_wdly_dq_inc_write(1);
	ddrphy_wdly_dqs_inc_write(1);

	/* unsel module */
	ddrphy_dly_sel_write(0);
}

static int write_level_scan(int *delays, int loops, int show)
{
	int i, j, k;

	int err_ddrphy_wdly;

	unsigned char taps_scan[SDRAM_PHY_DELAYS];

	int one_window_active;
	int one_window_start, one_window_best_start;
	int one_window_count, one_window_best_count;

	unsigned char buf[DFII_PIX_DATA_BYTES];

	int ok;

	err_ddrphy_wdly = SDRAM_PHY_DELAYS - ddrphy_half_sys8x_taps_read();

	sdrwlon();
	cdelay(100);
	for(i=0;i<SDRAM_PHY_MODULES;i++) {
		if (show)
			printf("m%d: |", i);

		/* rst delay */
		write_delay_rst(i);

		/* scan write delay taps */
		for(j=0;j<err_ddrphy_wdly;j++) {
			int zero_count = 0;
			int one_count = 0;
			int show_iter = show;
#if SDRAM_PHY_DELAYS > 32
			show_iter = (j%16 == 0) && show;
#endif
			for (k=0; k<loops; k++) {
				ddrphy_wlevel_strobe_write(1);
				cdelay(10);
				csr_rd_buf_uint8(sdram_dfii_pix_rddata_addr[0],
						 buf, DFII_PIX_DATA_BYTES);
				if (buf[SDRAM_PHY_MODULES-1-i] != 0)
					one_count++;
				else
					zero_count++;
			}
			if (one_count > zero_count)
				taps_scan[j] = 1;
			else
				taps_scan[j] = 0;
			if (show_iter)
				printf("%d", taps_scan[j]);
			write_delay_inc(i);
			cdelay(10);
		}
		if (show)
			printf("|");

		/* find longer 1 window and set delay at the 0/1 transition */
		one_window_active = 0;
		one_window_start = 0;
		one_window_count = 0;
		one_window_best_start = 0;
		one_window_best_count = -1;
		delays[i] = -1;
		for(j=0;j<err_ddrphy_wdly;j++) {
			if (one_window_active) {
				if ((taps_scan[j] == 0) | (j == err_ddrphy_wdly - 1)) {
					one_window_active = 0;
					one_window_count = j - one_window_start;
					if (one_window_count > one_window_best_count) {
						one_window_best_start = one_window_start;
						one_window_best_count = one_window_count;
					}
				}
			} else {
				if (taps_scan[j]) {
					one_window_active = 1;
					one_window_start = j;
				}
			}
		}
		/* succeed only if the start of a 1s window has been found */
		if (one_window_best_count > 0 && one_window_best_start > 0) {
			delays[i] = one_window_best_start;

			/* configure write delay */
			write_delay_rst(i);
			for(j=0; j<delays[i]; j++)
				write_delay_inc(i);
		}
		if (show)
			printf(" delay: %02d\n", delays[i]);
	}

	sdrwloff();

	ok = 1;
	for(i=SDRAM_PHY_MODULES-1;i>=0;i--) {
		if(delays[i] < 0)
			ok = 0;
	}

	return ok;
}

static void write_level_cdly_range(unsigned int *best_error, int *best_cdly,
		int cdly_start, int cdly_stop, int cdly_step)
{
	int cdly;
	int cdly_actual = 0;
	int delays[SDRAM_PHY_MODULES];

	/* scan through the range */
	ddrphy_cdly_rst_write(1);
	for (cdly = cdly_start; cdly < cdly_stop; cdly += cdly_step) {
		/* increment cdly to current value */
		while (cdly_actual < cdly) {
			ddrphy_cdly_inc_write(1);
			cdelay(10);
			cdly_actual++;
		}

		/* write level using this delay */
		if (write_level_scan(delays, 8, 0)) {
			/* use the mean of delays for error calulation */
			int delay_mean = 0;
			for (int i=0; i < SDRAM_PHY_MODULES; ++i) {
				delay_mean += delays[i];
			}
			delay_mean /= SDRAM_PHY_MODULES;

			/* we want it to be at the start */
			int ideal_delay = 4*SDRAM_PHY_DELAYS/32;
			int error = ideal_delay - delay_mean;
			if (error < 0)
				error *= -1;

			if (error < *best_error) {
				*best_cdly = cdly;
				*best_error = error;
			}
			printf("1");
		} else {
			printf("0");
		}
	}
}

int write_level(void)
{
	int delays[SDRAM_PHY_MODULES];
	unsigned int best_error = ~0u;
	int best_cdly = -1;
	int cdly_range_start;
	int cdly_range_end;
	int cdly_range_step;

	printf("Command/Clk scan:\n");

	/* Center write leveling by varying cdly. Searching through all possible
	 * values is slow, but we can use a simple optimization method of iterativly
	 * scanning smaller ranges with decreasing step */
	cdly_range_start = 0;
	cdly_range_end = SDRAM_PHY_DELAYS;
	if (SDRAM_PHY_DELAYS > 32)
		cdly_range_step = SDRAM_PHY_DELAYS/8;
	else
		cdly_range_step = 1;
	while (cdly_range_step > 0) {
		printf("|");
		write_level_cdly_range(&best_error, &best_cdly,
				cdly_range_start, cdly_range_end, cdly_range_step);

		/* small optimization - stop if we have zero error */
		if (best_error == 0)
			break;

		/* use best result as the middle of next range */
		cdly_range_start = best_cdly - cdly_range_step;
		cdly_range_end = best_cdly + cdly_range_step + 1;
		if (cdly_range_start < 0)
			cdly_range_start = 0;
		if (cdly_range_end > 512)
			cdly_range_end = 512;

		cdly_range_step /= 4;
	}
	printf("| best: %d\n", best_cdly);

	/* if we found any working delay then set it */
	if (best_cdly >= 0) {
		ddrphy_cdly_rst_write(1);
		for (int i = 0; i < best_cdly; ++i) {
			ddrphy_cdly_inc_write(1);
			cdelay(10);
		}
	}

	printf("Data scan:\n");

	/* re-run write leveling the final time */
	if (!write_level_scan(delays, 128, 1))
		return 0;

	return best_cdly >= 0;
}


#endif /*  SDRAM_PHY_WRITE_LEVELING_CAPABLE */

static void read_delay_rst(int module) {
	/* sel module */
	ddrphy_dly_sel_write(1 << module);

	/* rst delay */
	ddrphy_rdly_dq_rst_write(1);

	/* unsel module */
	ddrphy_dly_sel_write(0);

#ifdef SDRAM_PHY_ECP5DDRPHY
	/* Sync all DQSBUFM's, By toggling all dly_sel (DQSBUFM.PAUSE) lines. */
	ddrphy_dly_sel_write(0xFF);
	ddrphy_dly_sel_write(0);
#endif
}

static void read_delay_inc(int module) {
	/* sel module */
	ddrphy_dly_sel_write(1 << module);

	/* inc delay */
	ddrphy_rdly_dq_inc_write(1);

	/* unsel module */
	ddrphy_dly_sel_write(0);

#ifdef SDRAM_PHY_ECP5DDRPHY
	/* Sync all DQSBUFM's, By toggling all dly_sel (DQSBUFM.PAUSE) lines. */
	ddrphy_dly_sel_write(0xFF);
	ddrphy_dly_sel_write(0);
#endif
}

static void read_bitslip_rst(char m)
{
	/* sel module */
	ddrphy_dly_sel_write(1 << m);

	/* inc delay */
	ddrphy_rdly_dq_bitslip_rst_write(1);

	/* unsel module */
	ddrphy_dly_sel_write(0);
}


static void read_bitslip_inc(char m)
{
	/* sel module */
	ddrphy_dly_sel_write(1 << m);

	/* inc delay */
	ddrphy_rdly_dq_bitslip_write(1);

	/* unsel module */
	ddrphy_dly_sel_write(0);
}

static int read_level_scan(int module, int bitslip)
{
	unsigned int prv;
	unsigned char prs[SDRAM_PHY_PHASES][DFII_PIX_DATA_BYTES];
	unsigned char tst[DFII_PIX_DATA_BYTES];
	int p, i;
	int score;

	/* Generate pseudo-random sequence */
	prv = 42;
	for(p=0;p<SDRAM_PHY_PHASES;p++)
		for(i=0;i<DFII_PIX_DATA_BYTES;i++) {
			prv = 1664525*prv + 1013904223;
			prs[p][i] = prv;
		}

	/* Activate */
	sdram_dfii_pi0_address_write(0);
	sdram_dfii_pi0_baddress_write(0);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CS);
	cdelay(15);

	/* Write test pattern */
	for(p=0;p<SDRAM_PHY_PHASES;p++)
		csr_wr_buf_uint8(sdram_dfii_pix_wrdata_addr[p],
				 prs[p], DFII_PIX_DATA_BYTES);
	sdram_dfii_piwr_address_write(0);
	sdram_dfii_piwr_baddress_write(0);
	command_pwr(DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS|DFII_COMMAND_WRDATA);

	/* Calibrate each DQ in turn */
	sdram_dfii_pird_address_write(0);
	sdram_dfii_pird_baddress_write(0);
	score = 0;

	printf("m%d, b%d: |", module, bitslip);
	read_delay_rst(module);
	for(i=0;i<SDRAM_PHY_DELAYS;i++) {
		int working = 1;
		int show = 1;
#if SDRAM_PHY_DELAYS > 32
		show = (i%16 == 0);
#endif
#ifdef SDRAM_PHY_ECP5DDRPHY
		ddrphy_burstdet_clr_write(1);
#endif
		command_prd(DFII_COMMAND_CAS|DFII_COMMAND_CS|DFII_COMMAND_RDDATA);
		cdelay(15);
		for(p=0;p<SDRAM_PHY_PHASES;p++) {
			/* read back test pattern */
			csr_rd_buf_uint8(sdram_dfii_pix_rddata_addr[p],
					 tst, DFII_PIX_DATA_BYTES);
			/* verify bytes matching current 'module' */
			if (prs[p][  SDRAM_PHY_MODULES-1-module] != tst[  SDRAM_PHY_MODULES-1-module] ||
			    prs[p][2*SDRAM_PHY_MODULES-1-module] != tst[2*SDRAM_PHY_MODULES-1-module])
				working = 0;
		}
#ifdef SDRAM_PHY_ECP5DDRPHY
		if (((ddrphy_burstdet_seen_read() >> module) & 0x1) != 1)
			working = 0;
#endif
		if (show)
			printf("%d", working);
		score += working;
		read_delay_inc(module);
	}
	printf("| ");

	/* Precharge */
	sdram_dfii_pi0_address_write(0);
	sdram_dfii_pi0_baddress_write(0);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
	cdelay(15);

	return score;
}

static void read_level(int module)
{
	unsigned int prv;
	unsigned char prs[SDRAM_PHY_PHASES][DFII_PIX_DATA_BYTES];
	unsigned char tst[DFII_PIX_DATA_BYTES];
	int p, i;
	int working;
	int delay, delay_min, delay_max;

	printf("delays: ");

	/* Generate pseudo-random sequence */
	prv = 42;
	for(p=0;p<SDRAM_PHY_PHASES;p++)
		for(i=0;i<DFII_PIX_DATA_BYTES;i++) {
			prv = 1664525*prv + 1013904223;
			prs[p][i] = prv;
		}

	/* Activate */
	sdram_dfii_pi0_address_write(0);
	sdram_dfii_pi0_baddress_write(0);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CS);
	cdelay(15);

	/* Write test pattern */
	for(p=0;p<SDRAM_PHY_PHASES;p++)
		csr_wr_buf_uint8(sdram_dfii_pix_wrdata_addr[p],
				 prs[p], DFII_PIX_DATA_BYTES);
	sdram_dfii_piwr_address_write(0);
	sdram_dfii_piwr_baddress_write(0);
	command_pwr(DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS|DFII_COMMAND_WRDATA);

	/* Calibrate each DQ in turn */
	sdram_dfii_pird_address_write(0);
	sdram_dfii_pird_baddress_write(0);

	/* Find smallest working delay */
	delay = 0;
	read_delay_rst(module);
	while(1) {
#ifdef SDRAM_PHY_ECP5DDRPHY
		ddrphy_burstdet_clr_write(1);
#endif
		command_prd(DFII_COMMAND_CAS|DFII_COMMAND_CS|DFII_COMMAND_RDDATA);
		cdelay(15);
		working = 1;
		for(p=0;p<SDRAM_PHY_PHASES;p++) {
			/* read back test pattern */
			csr_rd_buf_uint8(sdram_dfii_pix_rddata_addr[p],
					 tst, DFII_PIX_DATA_BYTES);
			/* verify bytes matching current 'module' */
			if (prs[p][  SDRAM_PHY_MODULES-1-module] != tst[  SDRAM_PHY_MODULES-1-module] ||
			    prs[p][2*SDRAM_PHY_MODULES-1-module] != tst[2*SDRAM_PHY_MODULES-1-module])
				working = 0;
		}
#ifdef SDRAM_PHY_ECP5DDRPHY
		if (((ddrphy_burstdet_seen_read() >> module) & 0x1) != 1)
			working = 0;
#endif
		if(working)
			break;
		delay++;
		if(delay >= SDRAM_PHY_DELAYS)
			break;
		read_delay_inc(module);
	}
	delay_min = delay;

	/* Get a bit further into the working zone */
#if SDRAM_PHY_DELAYS > 32
	for(i=0;i<16;i++) {
		delay += 1;
		read_delay_inc(module);
	}
#else
	delay++;
	read_delay_inc(module);
#endif

	/* Find largest working delay */
	while(1) {
#ifdef SDRAM_PHY_ECP5DDRPHY
		ddrphy_burstdet_clr_write(1);
#endif
		command_prd(DFII_COMMAND_CAS|DFII_COMMAND_CS|DFII_COMMAND_RDDATA);
		cdelay(15);
		working = 1;
		for(p=0;p<SDRAM_PHY_PHASES;p++) {
			/* read back test pattern */
			csr_rd_buf_uint8(sdram_dfii_pix_rddata_addr[p],
					 tst, DFII_PIX_DATA_BYTES);
			/* verify bytes matching current 'module' */
			if (prs[p][  SDRAM_PHY_MODULES-1-module] != tst[  SDRAM_PHY_MODULES-1-module] ||
			    prs[p][2*SDRAM_PHY_MODULES-1-module] != tst[2*SDRAM_PHY_MODULES-1-module])
				working = 0;
		}
#ifdef SDRAM_PHY_ECP5DDRPHY
		if (((ddrphy_burstdet_seen_read() >> module) & 0x1) != 1)
			working = 0;
#endif
		if(!working)
			break;
		delay++;
		if(delay >= SDRAM_PHY_DELAYS)
			break;
		read_delay_inc(module);
	}
	delay_max = delay;

	if (delay_min >= SDRAM_PHY_DELAYS)
		printf("-");
	else
		printf("%02d+-%02d", (delay_min+delay_max)/2, (delay_max-delay_min)/2);

	/* Set delay to the middle */
	read_delay_rst(module);
	for(i=0;i<(delay_min+delay_max)/2;i++)
		read_delay_inc(module);

	/* Precharge */
	sdram_dfii_pi0_address_write(0);
	sdram_dfii_pi0_baddress_write(0);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
	cdelay(15);
}
#endif /* CSR_DDRPHY_BASE */

#endif /* CSR_SDRAM_BASE */

static unsigned int seed_to_data_32(unsigned int seed, int random)
{
	if (random)
		return 1664525*seed + 1013904223;
	else
		return seed + 1;
}

static unsigned short seed_to_data_16(unsigned short seed, int random)
{
	if (random)
		return 25173*seed + 13849;
	else
		return seed + 1;
}

#define ONEZERO 0xAAAAAAAA
#define ZEROONE 0x55555555

#ifndef MEMTEST_BUS_SIZE
#define MEMTEST_BUS_SIZE (512)
#endif

//#define MEMTEST_BUS_DEBUG

static int memtest_bus(void)
{
	volatile unsigned int *array = (unsigned int *)MAIN_RAM_BASE;
	int i, errors;
	unsigned int rdata;

	errors = 0;

	for(i=0;i<MEMTEST_BUS_SIZE/4;i++) {
		array[i] = ONEZERO;
	}
	flush_cpu_dcache();
#ifdef CONFIG_L2_SIZE
	flush_l2_cache();
#endif
	for(i=0;i<MEMTEST_BUS_SIZE/4;i++) {
		rdata = array[i];
		if(rdata != ONEZERO) {
			errors++;
#ifdef MEMTEST_BUS_DEBUG
			printf("[bus: 0x%0x]: 0x%08x vs 0x%08x\n", i, rdata, ONEZERO);
#endif
		}
	}

	for(i=0;i<MEMTEST_BUS_SIZE/4;i++) {
		array[i] = ZEROONE;
	}
	flush_cpu_dcache();
#ifdef CONFIG_L2_SIZE
	flush_l2_cache();
#endif
	for(i=0;i<MEMTEST_BUS_SIZE/4;i++) {
		rdata = array[i];
		if(rdata != ZEROONE) {
			errors++;
#ifdef MEMTEST_BUS_DEBUG
			printf("[bus 0x%0x]: 0x%08x vs 0x%08x\n", i, rdata, ZEROONE);
#endif
		}
	}

	return errors;
}

#ifndef MEMTEST_DATA_SIZE
#define MEMTEST_DATA_SIZE (2*1024*1024)
#endif
#define MEMTEST_DATA_RANDOM 1

//#define MEMTEST_DATA_DEBUG

static int memtest_data(void)
{
	volatile unsigned int *array = (unsigned int *)MAIN_RAM_BASE;
	int i, errors;
	unsigned int seed_32;
	unsigned int rdata;

	errors = 0;
	seed_32 = 0;

	for(i=0;i<MEMTEST_DATA_SIZE/4;i++) {
		seed_32 = seed_to_data_32(seed_32, MEMTEST_DATA_RANDOM);
		array[i] = seed_32;
	}

	seed_32 = 0;
	flush_cpu_dcache();
#ifdef CONFIG_L2_SIZE
	flush_l2_cache();
#endif
	for(i=0;i<MEMTEST_DATA_SIZE/4;i++) {
		seed_32 = seed_to_data_32(seed_32, MEMTEST_DATA_RANDOM);
		rdata = array[i];
		if(rdata != seed_32) {
			errors++;
#ifdef MEMTEST_DATA_DEBUG
			printf("[data 0x%0x]: 0x%08x vs 0x%08x\n", i, rdata, seed_32);
#endif
		}
	}

	return errors;
}
#ifndef MEMTEST_ADDR_SIZE
#define MEMTEST_ADDR_SIZE (32*1024)
#endif
#define MEMTEST_ADDR_RANDOM 0

//#define MEMTEST_ADDR_DEBUG

static int memtest_addr(void)
{
	volatile unsigned int *array = (unsigned int *)MAIN_RAM_BASE;
	int i, errors;
	unsigned short seed_16;
	unsigned short rdata;

	errors = 0;
	seed_16 = 0;

	for(i=0;i<MEMTEST_ADDR_SIZE/4;i++) {
		seed_16 = seed_to_data_16(seed_16, MEMTEST_ADDR_RANDOM);
		array[(unsigned int) seed_16] = i;
	}

	seed_16 = 0;
	flush_cpu_dcache();
#ifdef CONFIG_L2_SIZE
	flush_l2_cache();
#endif
	for(i=0;i<MEMTEST_ADDR_SIZE/4;i++) {
		seed_16 = seed_to_data_16(seed_16, MEMTEST_ADDR_RANDOM);
		rdata = array[(unsigned int) seed_16];
		if(rdata != i) {
			errors++;
#ifdef MEMTEST_ADDR_DEBUG
			printf("[addr 0x%0x]: 0x%08x vs 0x%08x\n", i, rdata, i);
#endif
		}
	}

	return errors;
}

static void memspeed(void)
{
	volatile unsigned int *array = (unsigned int *)MAIN_RAM_BASE;
	int i;
	unsigned int start, end;
	unsigned long write_speed;
	unsigned long read_speed;
	__attribute__((unused)) unsigned int data;

	/* init timer */
	timer0_en_write(0);
	timer0_reload_write(0);
	timer0_load_write(0xffffffff);
	timer0_en_write(1);

	/* write speed */
	timer0_update_value_write(1);
	start = timer0_value_read();
	for(i=0;i<MEMTEST_DATA_SIZE/4;i++) {
		array[i] = i;
	}
	timer0_update_value_write(1);
	end = timer0_value_read();
	write_speed = (8*MEMTEST_DATA_SIZE*(CONFIG_CLOCK_FREQUENCY/1000000))/(start - end);

	/* flush CPU and L2 caches */
	flush_cpu_dcache();
#ifdef CONFIG_L2_SIZE
	flush_l2_cache();
#endif

	/* read speed */
	timer0_en_write(1);
	timer0_update_value_write(1);
	start = timer0_value_read();
	for(i=0;i<MEMTEST_DATA_SIZE/4;i++) {
		data = array[i];
	}
	timer0_update_value_write(1);
	end = timer0_value_read();
	read_speed = (8*MEMTEST_DATA_SIZE*(CONFIG_CLOCK_FREQUENCY/1000000))/(start - end);

	printf("Memspeed Writes: %dMbps Reads: %dMbps\n", write_speed, read_speed);
}

int memtest(void)
{
	int bus_errors, data_errors, addr_errors;

	bus_errors = memtest_bus();
	if(bus_errors != 0)
		printf("Memtest bus failed: %d/%d errors\n", bus_errors, 2*128);

	data_errors = memtest_data();
	if(data_errors != 0)
		printf("Memtest data failed: %d/%d errors\n", data_errors, MEMTEST_DATA_SIZE/4);

	addr_errors = memtest_addr();
	if(addr_errors != 0)
		printf("Memtest addr failed: %d/%d errors\n", addr_errors, MEMTEST_ADDR_SIZE/4);

	if(bus_errors + data_errors + addr_errors != 0)
		return 0;
	else {
		printf("Memtest OK\n");
		memspeed();
		return 1;
	}
}

#ifdef CSR_SDRAM_BASE

#if defined(SDRAM_PHY_WRITE_LEVELING_CAPABLE) || defined(SDRAM_PHY_READ_LEVELING_CAPABLE)

static void read_leveling(void)
{
	int module;
	int bitslip;
	int score;
	int best_score;
	int best_bitslip;

	for(module=0; module<SDRAM_PHY_MODULES; module++) {
		/* scan possible read windows */
		best_score = 0;
		best_bitslip = 0;
		for(bitslip=0; bitslip<SDRAM_PHY_BITSLIPS; bitslip++) {
			/* compute score */
			score = read_level_scan(module, bitslip);
			read_level(module);
			printf("\n");
			if (score > best_score) {
				best_bitslip = bitslip;
				best_score = score;
			}
			/* exit */
			if (bitslip == SDRAM_PHY_BITSLIPS-1)
				break;
			/* increment bitslip */
			read_bitslip_inc(module);
		}

		/* select best read window */
		printf("best: m%d, b%d ", module, best_bitslip);
		read_bitslip_rst(module);
		for (bitslip=0; bitslip<best_bitslip; bitslip++)
			read_bitslip_inc(module);

		/* re-do leveling on best read window*/
		read_level(module);
		printf("\n");
	}
}

int _write_level_cdly_scan = 1;

int sdrlevel(void)
{
	int module;
	sdrsw();

	for(module=0; module<SDRAM_PHY_MODULES; module++) {
#ifdef SDRAM_PHY_WRITE_LEVELING_CAPABLE
		write_delay_rst(module);
#endif
		read_delay_rst(module);
		read_bitslip_rst(module);
	}

#ifdef SDRAM_PHY_WRITE_LEVELING_CAPABLE
	printf("Write leveling:\n");
	if (_write_level_cdly_scan) {
		write_level();
	} else {
		/* use only the current cdly */
		int delays[SDRAM_PHY_MODULES];
		write_level_scan(delays, 128, 1);
	}
#endif

#ifdef SDRAM_PHY_READ_LEVELING_CAPABLE
	printf("Read leveling:\n");
	read_leveling();
#endif

	return 1;
}
#endif

int sdrinit(void)
{
	printf("Initializing SDRAM...\n");

#ifdef CSR_DDRCTRL_BASE
	ddrctrl_init_done_write(0);
	ddrctrl_init_error_write(0);
#endif

	init_sequence();
#ifdef CSR_DDRPHY_BASE
#ifdef DDRPHY_CMD_DELAY
	ddrphy_cdly(DDRPHY_CMD_DELAY);
#endif
#if CSR_DDRPHY_EN_VTC_ADDR
	ddrphy_en_vtc_write(0);
#endif
#if defined(SDRAM_PHY_WRITE_LEVELING_CAPABLE) || defined(SDRAM_PHY_READ_LEVELING_CAPABLE)
	sdrlevel();
#endif
#if CSR_DDRPHY_EN_VTC_ADDR
	ddrphy_en_vtc_write(1);
#endif
#endif
	sdrhw();
	if(!memtest()) {
#ifdef CSR_DDRCTRL_BASE
		ddrctrl_init_done_write(1);
		ddrctrl_init_error_write(1);
#endif
		return 0;
	}
#ifdef CSR_DDRCTRL_BASE
	ddrctrl_init_done_write(1);
#endif

	return 1;
}

#ifdef USDDRPHY_DEBUG

#define MPR0_SEL (0 << 0)
#define MPR1_SEL (1 << 0)
#define MPR2_SEL (2 << 0)
#define MPR3_SEL (3 << 0)

#define MPR_ENABLE (1 << 2)

#define MPR_READ_SERIAL    (0 << 11)
#define MPR_READ_PARALLEL  (1 << 11)
#define MPR_READ_STAGGERED (2 << 11)

void sdrcal(void)
{
#ifdef CSR_DDRPHY_BASE
#if CSR_DDRPHY_EN_VTC_ADDR
	ddrphy_en_vtc_write(0);
#endif
	sdrlevel();
#if CSR_DDRPHY_EN_VTC_ADDR
	ddrphy_en_vtc_write(1);
#endif
#endif
	sdrhw();
}

void sdrmrwr(char reg, int value) {
	sdram_dfii_pi0_address_write(value);
	sdram_dfii_pi0_baddress_write(reg);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
}

static void sdrmpron(char mpr)
{
	sdrmrwr(3, MPR_READ_SERIAL | MPR_ENABLE | mpr);
}

static void sdrmproff(void)
{
	sdrmrwr(3, 0);
}

void sdrmpr(void)
{
	int module, phase;
	unsigned char buf[DFII_PIX_DATA_BYTES];
	printf("Read SDRAM MPR...\n");

	/* rst phy */
	for(module=0; module<SDRAM_PHY_MODULES; module++) {
#ifdef SDRAM_PHY_WRITE_LEVELING_CAPABLE
		write_delay_rst(module);
#endif
		read_delay_rst(module);
		read_bitslip_rst(module);
	}

	/* software control */
	sdrsw();

	printf("Reads with MPR0 (0b01010101) enabled...\n");
	sdrmpron(MPR0_SEL);
	command_prd(DFII_COMMAND_CAS|DFII_COMMAND_CS|DFII_COMMAND_RDDATA);
	cdelay(15);
	for (module=0; module < SDRAM_PHY_MODULES; module++) {
		printf("m%d: ", module);
		for(phase=0; phase<SDRAM_PHY_PHASES; phase++) {
			csr_rd_buf_uint8(sdram_dfii_pix_rddata_addr[phase],
					 buf, DFII_PIX_DATA_BYTES);
			printf("%d", buf[  SDRAM_PHY_MODULES-module-1] & 0x1);
			printf("%d", buf[2*SDRAM_PHY_MODULES-module-1] & 0x1);
		}
		printf("\n");
	}
	sdrmproff();

	/* hardware control */
	sdrhw();
}

void sdr_cdly_scan(int enabled)
{
	printf("Turning cdly scan %s\n", enabled ? "ON" : "OFF");
	_write_level_cdly_scan = enabled;
}

#endif

#endif
