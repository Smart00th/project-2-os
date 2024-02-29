#include <stdio.h>
#include <unistd.h>

int main() {
	fork();
	mkdir("tmp1");
	getpid();
	mkdir("tmp");
	chdir("tmp");

	return 0;
}