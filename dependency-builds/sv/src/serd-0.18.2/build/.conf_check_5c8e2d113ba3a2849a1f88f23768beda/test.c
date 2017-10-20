#include <fcntl.h>

int main(int argc, char **argv) {
	void *p;
	(void)argc; (void)argv;
	p=(void*)(posix_fadvise);
	return 0;
}
