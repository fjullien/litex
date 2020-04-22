#include <stdio.h>
#include <stdlib.h>

#include <generated/csr.h>

#include "../command.h"
#include "../helpers.h"
#include "../sdram.h"

#ifdef CSR_SDRAM_BASE
/**************************************
 *            Memory read             *
 **************************************
 */
static void sdrrow_handler(int nb_params, char **params)
{
	char *c;
	unsigned int row;

	printf("\n");
	if (nb_params < 1) {
		sdrrow(0);
		printf("Precharged");
	}
	
	row = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect row");
		return;
	}

	sdrrow(row);
	printf("Activated row %d", row);
}

struct command_struct cmd_sdrrow =
{
	.func = sdrrow_handler,
	.name = "sdrrow",
	.help = "Precharge/Activate row",
};

define_command(cmd_sdrrow);

/**************************************
 *          SDRAM control             *
 **************************************
 */
static void sdrsw_handler(int nb_params, char **params)
{
	printf("\n");
	sdrsw();
}

struct command_struct cmd_sdrsw =
{
	.func = sdrsw_handler,
	.name = "sdrsw",
	.help = "Gives SDRAM control to SW",
};

define_command(cmd_sdrsw);

static void sdrhw_handler(int nb_params, char **params)
{
	printf("\n");
	sdrhw();
}

struct command_struct cmd_sdrhw =
{
	.func = sdrhw_handler,
	.name = "sdrhw",
	.help = "Gives SDRAM control to HW",
};

define_command(cmd_sdrhw);

/**************************************
 *        SDRAM read buffer           *
 **************************************
 */
static void sdrrdbuf_handler(int nb_params, char **params)
{
	printf("\n");
	sdrrdbuf(-1);
}

struct command_struct cmd_sdrrdbuf =
{
	.func = sdrrdbuf_handler,
	.name = "sdrrdbuf",
	.help = "Dump SDRAM read buffer",
};

define_command(cmd_sdrrdbuf);

/**************************************
 *            SDRAM read              *
 **************************************
 */
static void sdrrd_handler(int nb_params, char **params)
{
	unsigned int addr;
	int dq;
	char *c;

	printf("\n");
	if (nb_params < 1) {
		printf("sdrrd <address>");
		return;
	}

	addr = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect address");
		return;
	}

	if (nb_params < 2)
		dq = -1;
	else {
		dq = strtoul(params[1], &c, 0);
		if (*c != 0) {
			printf("Incorrect DQ");
			return;
		}
	}

	sdrrd(addr, dq);
}

struct command_struct cmd_sdrrd =
{
	.func = sdrrd_handler,
	.name = "sdrrd",
	.help = "Read SDRAM data",
};

define_command(cmd_sdrrd);

/**************************************
 *            SDRAM error             *
 **************************************
 */
static void sdrrderr_handler(int nb_params, char **params)
{
	int count;
	char *c;

	printf("\n");
	if (nb_params < 1) {
		printf("sdrrderr <count>");
		return;
	}

	count = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect count");
		return;
	}

	sdrrderr(count);
}

struct command_struct cmd_sdrrderr =
{
	.func = sdrrderr_handler,
	.name = "sdrrderr",
	.help = "Print SDRAM read errors",
};

define_command(cmd_sdrrderr);

/**************************************
 *       SDRAM write test pattern     *
 **************************************
 */
static void sdrwr_handler(int nb_params, char **params)
{
	unsigned int addr;
	char *c;

	printf("\n");
	if (nb_params < 1) {
		printf("sdrwr <address>");
		return;
	}

	addr = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect address");
		return;
	}

	sdrwr(addr);
}

struct command_struct cmd_sdrwr =
{
	.func = sdrwr_handler,
	.name = "sdrwr",
	.help = "Write SDRAM test data",
};

define_command(cmd_sdrwr);

/**************************************
 *             SDRAM init             *
 **************************************
 */
 #ifdef CSR_DDRPHY_BASE
static void sdrinit_handler(int nb_params, char **params)
{
	printf("\n");
	sdrinit();
}

struct command_struct cmd_sdrinit =
{
	.func = sdrinit_handler,
	.name = "sdrinit",
	.help = "Start SDRAM initialisation",
};

define_command(cmd_sdrinit);
#endif

/**************************************
 *         Write leveling ON          *
 **************************************
 */
#if defined(CSR_DDRPHY_BASE) && defined(SDRAM_PHY_WRITE_LEVELING_CAPABLE)
static void sdrwlon_handler(int nb_params, char **params)
{
	printf("\n");
	sdrwlon();
}

struct command_struct cmd_sdrwlon =
{
	.func = sdrwlon_handler,
	.name = "sdrwlon",
	.help = "Enable write leveling",
};

define_command(cmd_sdrwlon);

/**************************************
 *         Write leveling OFF         *
 **************************************
 */
static void sdrwloff_handler(int nb_params, char **params)
{
	printf("\n");
	sdrwloff();
}

struct command_struct cmd_sdrwloff =
{
	.func = sdrwloff_handler,
	.name = "sdrwloff",
	.help = "Disable write leveling",
};

define_command(cmd_sdrwloff);
#endif

/**************************************
 *           Read leveling            *
 **************************************
 */
#ifdef CSR_DDRPHY_BASE
static void sdrlevel_handler(int nb_params, char **params)
{
	printf("\n");
	sdrlevel();
}

struct command_struct cmd_sdrlevel =
{
	.func = sdrlevel_handler,
	.name = "sdrlevel",
	.help = "Perform read/write leveling",
};

define_command(cmd_sdrlevel);
#endif

/**************************************
 *           Memory test              *
 **************************************
 */
static void memtest_handler(int nb_params, char **params)
{
	printf("\n");
	memtest();
}

struct command_struct cmd_memtest =
{
	.func = memtest_handler,
	.name = "memtest",
	.help = "Run a memory test",
};

define_command(cmd_memtest);

#endif /* CSR_SDRAM_BASE */
