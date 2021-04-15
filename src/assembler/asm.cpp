#include "include/asm.h"

#include <iostream>

void Assembler::scanLabels() {
	std::string line;

	do {
		std::getline(fileIn, line);

		if (line.length() > MAX_LINE_LENGTH) {
			throw BException("Cannot assemble line %s - exceeds maximum length "
							 "of %u", line.c_str(), MAX_LINE_LENGTH);
		}
		
		if (line.back() == ':') {
			allLabels.emplace(line, line);
		}
	} while (line != "");

	// We'll need to scan the file again.
	fileIn.clear();
	fileIn.seekg(0);
}

void Assembler::scanInstructions() {
	std::string line;

	do {
		std::getline(fileIn, line);

		if (line.back() == ':') {
			allLines.emplace_back(
				std::in_place_type<Label*>, getLabel(line));
		} else {
			auto itSpace = std::find(line.begin(), line.end(), ' ');

			if (itSpace == line.end()) {
				throw BException("Cannot assemble line %s - expected "
								 "instruction to have an operand part and "
								 "a value part, separated by a space.", 
								 line.c_str());
			}

			std::string instrDesc(line.begin(), itSpace);
			std::string instrVal(itSpace + 1, line.end());

			allInstr.emplace_back(instrDesc, instrVal);
			allLines.emplace_back(
				std::in_place_type<Instruction*>, &allInstr.back());
		}
	} while (line != "");
}

void Assembler::calculateOffsets() {
	u32 byteOffset = 0;

	for (const auto& line: allLines) {
		if (std::holds_alternative<Label*>(line)) {
			Label *label  = std::get<Label*>(line);
			label->offset = byteOffset;
		} else {
			Instruction* instr = std::get<Instruction*>(line);
			instr->estSize = instr->estimateSize();
			byteOffset += instr->estSize;
		}
	}
}

void Assembler::assemble() {
	u32 byteOffset = 0;

	for (const auto& instr: allInstr) {
		std::vector<u8> assembledInstr = instr.assemble(byteOffset);

		for (const u8 instrByte: assembledInstr) {
			fileOut << instrByte;
		}

		byteOffset += assembledInstr.size();
	}
}

void Assembler::run() {
    Instruction::assemblerPtr = std::unique_ptr<Assembler>(this);
    scanLabels();
    scanInstructions();
    calculateOffsets();
    assemble();
}
