#include "kernel.h"

#include <iostream.h>

extern int userMain(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	cout << "\nKernel running!\n\n";

	init();

	int result = userMain(argc, argv);
	cout << "\nUser program ended! Code : " << result;

	restore();

	return result;
}