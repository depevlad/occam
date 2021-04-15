#include "include/asm.h"

int main(int argc, char** argv) {
	const char *fileIn  = argv[1];
	const char *fileOut = argv[2];

	Assembler assembler(fileIn, fileOut);
	assembler.run();

	return 0;
}