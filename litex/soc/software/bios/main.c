// This file is Copyright (c) 2013-2014 Sebastien Bourdeauducq <sb@m-labs.hk>
// This file is Copyright (c) 2014-2019 Florent Kermarrec <florent@enjoy-digital.fr>
// This file is Copyright (c) 2015 Yann Sionneau <ys@m-labs.hk>
// This file is Copyright (c) 2015 whitequark <whitequark@whitequark.org>
// This file is Copyright (c) 2019 Ambroz Bizjak <ambrop7@gmail.com>
// This file is Copyright (c) 2019 Caleb Jamison <cbjamo@gmail.com>
// This file is Copyright (c) 2018 Dolu1990 <charles.papon.90@gmail.com>
// This file is Copyright (c) 2018 Felix Held <felix-github@felixheld.de>
// This file is Copyright (c) 2019 Gabriel L. Somlo <gsomlo@gmail.com>
// This file is Copyright (c) 2018 Jean-François Nguyen <jf@lambdaconcept.fr>
// This file is Copyright (c) 2018 Sergiusz Bazanski <q3k@q3k.org>
// This file is Copyright (c) 2016 Tim 'mithro' Ansell <mithro@mithis.com>
// This file is Copyright (c) 2020 Franck Jullien <franck.jullien@gmail.com>

// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <console.h>
#include <string.h>
#include <uart.h>
#include <system.h>
#include <id.h>
#include <irq.h>
#include <crc.h>

#include <generated/csr.h>
#include <generated/mem.h>
#include <generated/git.h>

#ifdef CSR_ETHMAC_BASE
#include <net/microudp.h>
#endif

#ifdef CSR_SPIFLASH_BASE
#include <spiflash.h>
#endif

#ifdef CSR_ETHPHY_MDIO_W_ADDR
#include <mdio.h>
#endif

#include "sdram.h"
#include "sdcard.h"
#include "boot.h"

#include "readline.h"
#include "helpers.h"
#include "command.h"

extern initcall_t __bios_initcalls_start[], __bios_initcalls_end[];
initcall_t *initcall;

static void boot_sequence(void)
{
	if(serialboot()) {
#ifdef FLASH_BOOT_ADDRESS
		flashboot();
#endif
#ifdef ROM_BOOT_ADDRESS
		romboot();
#endif
#ifdef CSR_SPISDCARD_BASE
		spisdcardboot();
#endif
#ifdef CSR_ETHMAC_BASE
#ifdef CSR_ETHPHY_MODE_DETECTION_MODE_ADDR
		eth_mode();
#endif
		netboot();
#endif
		printf("No boot medium found\n");
	}
}

int main(int i, char **c)
{
	char buffer[CMD_LINE_BUFFER_SIZE];
	char *params[MAX_PARAM];
	char *command;
	struct command_struct *cmd;
	int nb_params;
	int sdr_ok;

	irq_setmask(0);
	irq_setie(1);
	uart_init();

	printf("\n");
	printf("\e[1m        __   _ __      _  __\e[0m\n");
	printf("\e[1m       / /  (_) /____ | |/_/\e[0m\n");
	printf("\e[1m      / /__/ / __/ -_)>  <\e[0m\n");
	printf("\e[1m     /____/_/\\__/\\__/_/|_|\e[0m\n");
	printf("\e[1m   Build your hardware, easily!\e[0m\n");
	printf("\n");
	printf(" (c) Copyright 2012-2020 Enjoy-Digital\n");
	printf(" (c) Copyright 2007-2015 M-Labs\n");
	printf("\n");
	printf(" BIOS built on "__DATE__" "__TIME__"\n");
	crcbios();
	printf("\n");
	printf(" Migen git sha1: "MIGEN_GIT_SHA1"\n");
	printf(" LiteX git sha1: "LITEX_GIT_SHA1"\n");
	printf("\n");
	printf("--=============== \e[1mSoC\e[0m ==================--\n");
	printf("\e[1mCPU\e[0m:       ");
#ifdef __lm32__
	printf("LM32");
#elif __or1k__
	printf("MOR1KX");
#elif __picorv32__
	printf("PicoRV32");
#elif __vexriscv__
	printf("VexRiscv");
#elif __minerva__
	printf("Minerva");
#elif __rocket__
	printf("RocketRV64[imac]");
#elif __blackparrot__
        printf("BlackParrotRV64[ia]");
#else
	printf("Unknown");
#endif
	printf(" @ %dMHz\n", CONFIG_CLOCK_FREQUENCY/1000000);
	printf("\e[1mROM\e[0m:       %dKB\n", ROM_SIZE/1024);
	printf("\e[1mSRAM\e[0m:      %dKB\n", SRAM_SIZE/1024);
#ifdef CONFIG_L2_SIZE
	printf("\e[1mL2\e[0m:        %dKB\n", CONFIG_L2_SIZE/1024);
#endif
#ifdef MAIN_RAM_SIZE
	printf("\e[1mMAIN-RAM\e[0m:  %dKB\n", MAIN_RAM_SIZE/1024);
#endif
	printf("\n");

        sdr_ok = 1;

#if defined(CSR_ETHMAC_BASE) || defined(CSR_SDRAM_BASE)
    printf("--========== \e[1mInitialization\e[0m ============--\n");
#ifdef CSR_ETHMAC_BASE
	eth_init();
#endif
#ifdef CSR_SDRAM_BASE
	sdr_ok = sdrinit();
#else
#ifdef MAIN_RAM_TEST
	sdr_ok = memtest();
#endif
#endif
	if (sdr_ok !=1)
		printf("Memory initialization failed\n");
	printf("\n");
#endif

	for (initcall = __bios_initcalls_start; initcall < __bios_initcalls_end; initcall++)
		(*initcall)();

	if(sdr_ok) {
		printf("--============== \e[1mBoot\e[0m ==================--\n");
		boot_sequence();
		printf("\n");
	}

	printf("--============= \e[1mConsole\e[0m ================--\n");
	hist_init();
	printf("\n%s", PROMPT);
	while(1) {
		readline(buffer, CMD_LINE_BUFFER_SIZE);
		if (buffer[0] != 0) {
			nb_params = get_param(buffer, &command, params);
			cmd = command_dispatcher(command, nb_params, params);
			if (!cmd)
				printf("\nCommand not found");
		}
		printf("\n%s", PROMPT);
	}
	return 0;
}
