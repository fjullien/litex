#ifndef __HELPERS_H__
#define __HELPERS_H__

typedef int (*initcall_t)(void);

#define __define_initcall(level,fn,id) \
	static initcall_t __initcall_##fn##id __attribute__((__used__)) \
	__attribute__((__section__(".initcall." level))) = fn

#define early_initcall(fn)		__define_initcall("0",fn,0)
#define late_initcall(fn)		__define_initcall("1",fn,1)

void dump_bytes(unsigned int *ptr, int count, unsigned long addr);
void crcbios(void);
int get_param(char *buf, char **cmd, char **params);

#endif
