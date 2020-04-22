#include <stdio.h>
#include <stdlib.h>

#include "../command.h"
#include "../helpers.h"

/**************************************
 *            Memory read             *
 **************************************
 */
static void mr(int nb_params, char **params)
{
	char *c;
	unsigned int *addr;
	unsigned int length;

	printf("\n");
	if (nb_params < 1) {
		printf("mr <address> [length]");
		return;
	}
	addr = (unsigned int *)strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect address");
		return;
	}
	if (nb_params == 1) {
		length = 4;
	} else {
		length = strtoul(params[1], &c, 0);
		if(*c != 0) {
			printf("\nIncorrect length");
			return;
		}
	}

	dump_bytes(addr, length, (unsigned long)addr);
}

struct command_struct cmd_mr =
{
	.func = mr,
	.name = "mr",
	.help = "Read address space",
};

define_command(cmd_mr);

/**************************************
 *            Memory write            *
 **************************************
 */
static void mw(int nb_params, char **params)
{
	char *c;
	unsigned int *addr2;
	unsigned int value2;
	unsigned int count2;
	unsigned int i;

	printf("\n");
	if (nb_params < 2) {
		printf("mw <address> <value> [count]");
		return;
	}

	addr2 = (unsigned int *)strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect address");
		return;
	}

	value2 = strtoul(params[1], &c, 0);
	if(*c != 0) {
		printf("Incorrect value");
		return;
	}

	if (nb_params == 2) {
		count2 = 1;
	} else {
		count2 = strtoul(params[2], &c, 0);
		if(*c != 0) {
			printf("Incorrect count");
			return;
		}
	}

	for (i = 0; i < count2; i++)
		*addr2++ = value2;
}

struct command_struct cmd_mw =
{
	.func = mw,
	.name = "mw",
	.help = "Write address space",
};

define_command(cmd_mw);

/**************************************
 *            Memory copy             *
 **************************************
 */
static void mc(int nb_params, char **params)
{
	char *c;
	unsigned int *dstaddr2;
	unsigned int *srcaddr2;
	unsigned int count2;
	unsigned int i;

	printf("\n");
	if (nb_params < 2) {
		printf("mc <dst> <src> [count]");
		return;
	}

	dstaddr2 = (unsigned int *)strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect destination address");
		return;
	}

	srcaddr2 = (unsigned int *)strtoul(params[1], &c, 0);
	if (*c != 0) {
		printf("Incorrect source address");
		return;
	}

	if (nb_params == 2) {
		count2 = 1;
	} else {
		count2 = strtoul(params[2], &c, 0);
		if (*c != 0) {
			printf("Incorrect count");
			return;
		}
	}

	for (i = 0; i < count2; i++)
		*dstaddr2++ = *srcaddr2++;
}

struct command_struct cmd_mc =
{
	.func = mc,
	.name = "mc",
	.help = "Copy address space",
};

define_command(cmd_mc);
