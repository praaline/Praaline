#include <stdlib.h>

int main(int argc, char **argv) {
	void *p;
	(void)argc; (void)argv;
	p=(void*)(posix_memalign);
	return 0;
}
