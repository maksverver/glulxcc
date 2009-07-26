#include "glulx.h"
#include "stdio.h"

int main()
{
	int val, i;
	val = glulx_gestalt(glulx_GlulxVersion, 0);
	printf("Glulx version: %d.%d.%d\n", val>>16, (val&0xff00)>>8, val&0xff);
	val = glulx_gestalt(glulx_TerpVersion, 0);
	printf("Interpreter version: %d.%d.%d (%#x)\n", val>>16, (val&0xff00)>>8, val&0xff, val);
	printf("Supports ResizeMem: %d\n", glulx_gestalt(glulx_ResizeMem, 0));
	printf("Supports Undo: %d\n", glulx_gestalt(glulx_Undo, 0));
	printf("Supported I/O subsystems:");
	for (i = 0; i < 100; ++i) if (glulx_gestalt(glulx_IOSystem, i)) printf(" %d", i);
	printf("\n");
	printf("Supports Unicode: %d\n", glulx_gestalt(glulx_Unicode, 0));
	printf("Supports MemCopy: %d\n", glulx_gestalt(glulx_MemCopy, 0));
	printf("Supports MAlloc: %d\n", glulx_gestalt(glulx_MAlloc, 0));
	printf("MAlloc heap start: %d\n", glulx_gestalt(glulx_MAllocHeap, 0));
	printf("Supports Acceleration: %d\n", glulx_gestalt(glulx_Acceleration, 0));
	printf("Supported accelerated functions:");
	for (i = 0; i < 100; ++i) if (glulx_gestalt(glulx_AccelFunc, i)) printf(" %d", i);
	printf("\n");
	return 0;
}
