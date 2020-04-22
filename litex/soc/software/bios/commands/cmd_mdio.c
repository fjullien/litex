#include <stdio.h>
#include <stdlib.h>

#include <generated/csr.h>

#include "../command.h"
#include "../helpers.h"

#ifdef CSR_ETHPHY_MDIO_W_ADDR

/**************************************
 *            MDIO write             *
 **************************************
 */
static void mdiow(int nb_params, char **params)
{
	char *c;
	unsigned int phyadr2;
	unsigned int reg2;
	unsigned int val2;

	if (nb_params < 3) {
		printf("\nmdiow <phyadr> <reg> <value>");
		return;
	}

	phyadr2 = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("\nIncorrect phyadr");
		return;
	}

	reg2 = strtoul(params[1], &c, 0);
	if (*c != 0) {
		printf("\nIncorrect reg");
		return;
	}

	val2 = strtoul(params[2], &c, 0);
	if (*c != 0) {
		printf("\nIncorrect val");
		return;
	}

	mdio_write(phyadr2, reg2, val2);
}

struct command_struct cmd_mdiow =
{
	.func = mdiow,
	.name = "mdiow",
	.help = "Write MDIO register",
};

define_command(cmd_mdiow);

/**************************************
 *            MDIO read               *
 **************************************
 */
static void mdior(int nb_params, char **params)
{
	char *c;
	unsigned int phyadr2;
	unsigned int reg2;
	unsigned int val;

	if (nb_params < 2) {
		printf("\nmdior <phyadr> <reg>");
		return;
	}

	phyadr2 = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("\nIncorrect phyadr");
		return;
	}

	reg2 = strtoul(params[1], &c, 0);
	if (*c != 0) {
		printf("\nIncorrect reg");
		return;
	}

	val = mdio_read(phyadr2, reg2);
	printf("\nreg %d: 0x%04x", reg2, val);
}

struct command_struct cmd_mdior =
{
	.func = mdior,
	.name = "mdior",
	.help = "Read MDIO register",
};

define_command(cmd_mdior);

/**************************************
 *            MDIO dump               *
 **************************************
 */
static void mdiod(int nb_params, char **params)
{
	char *c;
	unsigned int phyadr2;
	unsigned int count2;
	unsigned int val;
	int i;

	if (nb_params < 2) {
		printf("\nmdiod <phyadr> <count>");
		return;
	}

	phyadr2 = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("\nIncorrect phyadr");
		return;
	}

	count2 = strtoul(params[1], &c, 0);
	if (*c != 0) {
		printf("\nIncorrect count");
		return;
	}

	printf("MDIO dump @0x%x:\n", phyadr2);
	for (i = 0; i < count2; i++) {
		val = mdio_read(phyadr2, i);
		printf("reg %d: 0x%04x", i, val);
	}
}

struct command_struct cmd_mdiod =
{
	.func = mdiod,
	.name = "mdiod",
	.help = "Dump MDIO registers",
};

define_command(cmd_mdior);

#endif
