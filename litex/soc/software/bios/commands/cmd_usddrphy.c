#include <stdio.h>
#include <stdlib.h>

#include <generated/csr.h>

#include "../command.h"
#include "../helpers.h"
#include "../sdram.h"

/**************************************
 *            CLK/CMD delay           *
 **************************************
 */
#ifdef USDDRPHY_DEBUG
static void sdram_cdly(int nb_params, char **params)
{
	unsigned int delay;
	char *c;

	printf("\n");
	if (nb_params < 1) {
		printf("sdram_cdly <delay>");
		return;
	}

	delay = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect delay");
		return;
	}

	ddrphy_cdly(delay);
}

struct command_struct cmd_sdram_cdly =
{
	.func = sdram_cdly,
	.name = "sdram_cdly",
	.help = "Set SDRAM clk/cmd delay",
};

define_command(cmd_sdram_cdly);

/**************************************
 *         SDRAM calibration          *
 **************************************
 */
static void sdram_cal(int nb_params, char **params)
{
	sdrcal();
}

struct command_struct cmd_sdram_cal =
{
	.func = sdram_cal,
	.name = "sdram_cal",
	.help = "Run SDRAM calibration",
};

define_command(cmd_sdram_cal);

/**************************************
 *           SDRAM read MPR           *
 **************************************
 */
static void sdram_mpr(int nb_params, char **params)
{
	sdrmpr();
}

struct command_struct cmd_sdram_mpr =
{
	.func = sdram_mpr,
	.name = "sdram_mpr",
	.help = "Read SDRAM MPR",
};

define_command(cmd_sdram_mpr);

/**************************************
 *            CLK/CMD delay           *
 **************************************
 */
static void sdram_mrwr(int nb_params, char **params)
{
	unsigned int reg;
	unsigned int value;
	char *c;

	printf("\n");
	if (nb_params < 2) {
		printf("sdram_mrwr <reg> <value>");
		return;
	}

	reg = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect register value");
		return;
	}

	value = strtoul(params[1], &c, 0);
	if (*c != 0) {
		printf("Incorrect value");
		return;
	}

	sdrsw();
	printf("Writing 0x%04x to SDRAM mode register %d\n", value, reg);
	sdrmrwr(reg, value);
	sdrhw();
}

struct command_struct cmd_sdram_mrwr =
{
	.func = sdram_mrwr,
	.name = "sdram_mrwr",
	.help = "Write SDRAM mode registers",
};

define_command(cmd_sdram_mrwr);

#endif /* USDDRPHY_DEBUG */
