#include <stdio.h>
#include <stdlib.h>

#include <generated/csr.h>

#include "../command.h"
#include "../helpers.h"
#include "../sdcard.h"

/**************************************
 *          SDcard frequency          *
 **************************************
 */
#ifdef CSR_SDCORE_BASE
static void sdclk(int nb_params, char **params)
{
	unsigned int frequ;
	char *c;

	printf("\n");
	if (nb_params < 1) {
		printf("sdclk <frequ>");
		return;
	}

	frequ = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect frequency");
		return;
	}

	sdclk_set_clk(frequ);
}

struct command_struct cmd_sdclk =
{
	.func = sdclk,
	.name = "sdclk",
	.help = "SDCard set clk frequency (Mhz)",
};

define_command(cmd_sdclk);

/**************************************
 *          SDcard frequency          *
 **************************************
 */
static void sdinit(int nb_params, char **params)
{
	sdcard_init();
}

struct command_struct cmd_sdinit =
{
	.func = sdinit,
	.name = "sdinit",
	.help = "SDCard initialization",
};

define_command(cmd_sdinit);

/**************************************
 *          SDcard frequency          *
 **************************************
 */
static void sdtest(int nb_params, char **params)
{
	unsigned int loops;
	char *c;

	printf("\n");
	if (nb_params < 1) {
		printf("sdtest <loops>");
		return;
	}

	loops = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect number of loops");
		return;
	}

	sdcard_test(loops);
}

struct command_struct cmd_sdtest =
{
	.func = sdtest,
	.name = "sdtest",
	.help = "SDCard set clk frequency (Mhz)",
};

define_command(cmd_sdtest);

#endif /* CSR_SDCORE_BASE */
