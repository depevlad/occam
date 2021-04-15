#include <cassert>

#include "include/Transputer.h"

int main(int argc, char** argv) {
	Chip chip;
	chip.load(std::string(argv[2]));

    return 0;
}
