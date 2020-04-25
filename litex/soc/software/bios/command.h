#ifndef __COMMAND_H__
#define __COMMAND_H__

#define MAX_PARAM	8
#define CMD_MAX_SIZE	32	/* Used in string list, complete.c */
#define MAX_CMDS	40	/* Used in string list, complete.c */

struct command_struct {
	void (*func)(int nb_params, char **params);
	const char *name;
	const char *help;
};

extern struct command_struct *const __bios_cmd_start[];
extern struct command_struct *const __bios_cmd_end[];

#define define_command(name) \
	const struct command_struct *__bios_cmd_##name __attribute__((__used__)) \
	__attribute__((__section__(".bios_cmd"))) = &name


struct command_struct *command_dispatcher(char *command, int nb_params, char **params);

#endif /* COMMAND_H_ */
