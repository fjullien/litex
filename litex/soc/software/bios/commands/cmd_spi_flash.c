#include <stdio.h>
#include <stdlib.h>

#include <generated/csr.h>

#include "../command.h"
#include "../helpers.h"

#if (defined CSR_SPIFLASH_BASE && defined SPIFLASH_PAGE_SIZE)

/**************************************
 *            Flash write             *
 **************************************
 */
static void fw(int nb_params, char **params)
{
	char *c;
	unsigned int addr2;
	unsigned int value2;
	unsigned int count2;
	unsigned int i;

	printf("\n");
	if (nb_params < 2) {
		printf("fw <offset> <value> [count]");
		return;
	}

	addr2 = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect offset");
		return;
	}

	value2 = strtoul(params[1], &c, 0);
	if (*c != 0) {
		printf("Incorrect value");
		return;
	}

	if (nb_params == 2) {
		count2 = 1;
	} else {
		count2 = strtoul(count, &c, 0);
		if (*c != 0) {
			printf("Incorrect count");
			return;
		}
	}

	for (i = 0; i < count2; i++)
		write_to_flash(addr2 + i * 4, (unsigned char *)&value2, 4);
}

struct command_struct cmd_fw =
{
	.func = fw,
	.name = "fw",
	.help = "Write to flash",
};

define_command(cmd_fw);

/**************************************
 *            Flash erase             *
 **************************************
 */
static void fe(void)
{
	erase_flash();
	printf("flash erased\n");
}

struct command_struct cmd_fe =
{
	.func = fe,
	.name = "fe",
	.help = "Erase whole flash",
};

define_command(cmd_fe);

#endif
