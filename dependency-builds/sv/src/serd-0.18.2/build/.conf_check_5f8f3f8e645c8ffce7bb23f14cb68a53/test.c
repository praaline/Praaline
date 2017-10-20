#include <stdio.h>

int main(int argc, char **argv) {
	void *p;
	(void)argc; (void)argv;
	p=(void*)(fileno);
	return 0;
}
