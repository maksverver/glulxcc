#include "stdio.h"
#include "glulx.h"
int main()
{
	printf("Magic: \"%c%c%c%c\" (%#x)\n",
		glulx_header->magic.c[0],
		glulx_header->magic.c[1],
		glulx_header->magic.c[2],
		glulx_header->magic.c[3],
		glulx_header->magic.u);
	printf("Glulx version: %d.%d.%d\n",
		glulx_header->version.major,
		glulx_header->version.minor,
		glulx_header->version.revision);
	printf("RAM start:     %08x\n", glulx_header->ramstart);
	printf("Ext. start:    %08x\n", glulx_header->extstart);
	printf("End of memory: %08x\n", glulx_header->endmem);
	printf("Stack size:    %08x\n", glulx_header->stack_size);
	printf("Entry point:   %08x\n", glulx_header->start_func);
	printf("Checksum:      %08x\n", glulx_header->checksum);
	return 0;
}
