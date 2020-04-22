#include <stdio.h>
#include <stdlib.h>

#include <id.h>
#include <generated/csr.h>
#include <crc.h>
#include <system.h>

#include "../command.h"
#include "../helpers.h"

/**************************************
 *               Help                 *
 **************************************
 */
static void help(int nb_params, char **params)
{
	struct command_struct * const *cmd;

	puts("\n\nLiteX BIOS, available commands:\n");

	for (cmd = __bios_cmd_start; cmd != __bios_cmd_end; cmd++)
		printf("%-16s - %s\n", (*cmd)->name, (*cmd)->help ? (*cmd)->help : "-");
}

struct command_struct cmd_help =
{
	.func = help,
	.name = "help",
	.help = "Print this help",
};

define_command(cmd_help);

/**************************************
 *               Ident                *
 **************************************
 */
static void ident(int nb_params, char **params)
{
	char buffer[IDENT_SIZE];

	get_ident(buffer);
	printf("\nIdent: %s", *buffer ? buffer : "-");
}

struct command_struct cmd_ident =
{
	.func = ident,
	.name = "ident",
	.help = "Display identifier",
};

define_command(cmd_ident);

/**************************************
 *               Reboot               *
 **************************************
 */
#ifdef CSR_CTRL_BASE
static void reboot(int nb_params, char **params)
{
	ctrl_reset_write(1);
}

struct command_struct cmd_reboot =
{
	.func = reboot,
	.name = "reboot",
	.help = "Reset processor",
};

define_command(cmd_reboot);
#endif

/**************************************
 *               CRC                 *
 **************************************
 */
static void crc(int nb_params, char **params)
{
	char *c;
	unsigned int addr;
	unsigned int length;

	printf("\n");
	if (nb_params < 2) {
		printf("crc <address> <length>");
		return;
	}

	addr = strtoul(params[0], &c, 0);
	if (*c != 0) {
		printf("Incorrect address");
		return;
	}

	length = strtoul(params[1], &c, 0);
	if (*c != 0) {
		printf("Incorrect length");
		return;
	}

	printf("CRC32: %08x", crc32((unsigned char *)addr, length));
}

struct command_struct cmd_crc =
{
	.func = crc,
	.name = "crc",
	.help = "Compute CRC32 of a part of the address space",
};

define_command(cmd_crc);

/**************************************
 *            Flush cache             *
 **************************************
 */
static void flush_cpu_dcache_handler(int nb_params, char **params)
{
	flush_cpu_dcache();
}

struct command_struct cmd_flush_dcache =
{
	.func = flush_cpu_dcache_handler,
	.name = "flush_cpu_dcache",
	.help = "Flush CPU data cache",
};

define_command(cmd_flush_dcache);

#ifdef CONFIG_L2_SIZE
static void flush_l2_cache_handler(int nb_params, char **params)
{
	flush_l2_cache();
}

struct command_struct cmd_flush_l2 =
{
	.func = flush_l2_cache_handler,
	.name = "flush_l2_cache",
	.help = "Flush L2 cache",
};

define_command(cmd_flush_l2);
#endif
