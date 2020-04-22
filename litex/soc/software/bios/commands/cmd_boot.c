#include <stdio.h>
#include <stdlib.h>

#include <generated/csr.h>

#include "../command.h"
#include "../helpers.h"
#include "../boot.h"

/**************************************
 *            Flash boot              *
 **************************************
 */
#ifdef FLASH_BOOT_ADDRESS
static void flashboot_handler(int nb_params, char **params)
{
	flashboot();
}

struct command_struct cmd_flashboot =
{
	.func = flashboot_handler,
	.name = "flashboot",
	.help = "Boot from flash",
};

define_command(cmd_flashboot);
#endif

/**************************************
 *              ROM boot              *
 **************************************
 */
#ifdef ROM_BOOT_ADDRESS
static void romboot_handler(int nb_params, char **params)
{
	romboot();
}

struct command_struct cmd_romboot =
{
	.func = romboot_handler,
	.name = "romboot",
	.help = "Boot from embedded rom",
};

define_command(cmd_romboot);
#endif

/**************************************
 *            Serial boot             *
 **************************************
 */
static void serialboot_handler(int nb_params, char **params)
{
	serialboot();
}

struct command_struct cmd_serialboot =
{
	.func = serialboot_handler,
	.name = "serialboot",
	.help = "Boot via SFL",
};

define_command(cmd_serialboot);

/**************************************
 *          Ethernet boot             *
 **************************************
 */
#ifdef CSR_ETHMAC_BASE
static void netboot_handler(int nb_params, char **params)
{
	netboot();
}

struct command_struct cmd_netboot =
{
	.func = netboot_handler,
	.name = "netboot",
	.help = "Boot via TFTP",
};

define_command(cmd_netboot);
#endif

/**************************************
 *          Ethernet boot             *
 **************************************
 */
#ifdef CSR_SPISDCARD_BASE
static void spisdcardboot_handler(int nb_params, char **params)
{
	spisdcardboot();
}

struct command_struct cmd_spisdboot =
{
	.func = spisdcardboot_handler,
	.name = "spisdcardboot",
	.help = "Boot from SDCard via SPI hardware bitbang",
};

define_command(cmd_spisdboot);
#endif

