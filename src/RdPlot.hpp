#ifndef PLOT_H_
#define PLOT_H_

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <map>
#include <vector>
#include "Trace.hpp"

class RdPlot {
private:
	static constexpr size_t RD_MAGIC_SIZE = 2;
	static constexpr uint8_t RD_MAGIC[RD_MAGIC_SIZE] = { 0xd8, 0x12 };
	static constexpr size_t ABS_MOVE_MAGIC_SIZE = 2;
	static constexpr uint8_t ABS_MOVE_MAGIC[RD_MAGIC_SIZE] = { 0x88, 0x00 };

	static constexpr uint8_t SCRAMBLE_MAGIC = 0x33;

	std::ifstream* inputfile;
	off64_t eof;
	bool valid;

	std::map<string, RdInstr*> settings;

	void readMagic() {
		char* buffer = new char[RD_MAGIC_SIZE];
		this->inputfile->read(buffer, RD_MAGIC_SIZE);

		if (memcmp(RD_MAGIC, buffer, RD_MAGIC_SIZE) != 0) {
			invalidate("magic doesn't match");
		}
	}

	uint8_t descramble(uint8_t s) {
		uint8_t a = (s + 0xFF) & 0xFF;
		uint8_t b = a ^ SCRAMBLE_MAGIC;
		uint8_t p = (b & 0x7E) | (b >> 7 & 0x01) | (b << 7 & 0x80);
		return p;
	}

	RdInstr* readInstr() {
		off64_t off = this->inputfile->tellg();
		uint8_t c = descramble(this->inputfile->get());
		assert(c >= 0x80);
		RdInstr* instr = new RdInstr(off);
		instr->command = c;

		while (this->inputfile->good()
				&& (c = descramble(this->inputfile->get())) < 0x80) {
			instr->data.push_back(c);
		}

		this->inputfile->unget();
		return instr;
	}

public:
	RdInstr* currentInstr;

	RdPlot(std::ifstream *infile) :
			inputfile(infile), eof(numeric_limits<off64_t>::max()), valid(true), currentInstr(
					NULL) {
	}

	virtual ~RdPlot() {
	}

	bool isValid() {
		return valid;
	}

	void invalidate(string msg) {
		this->valid = false;
		Trace::singleton()->printBacklog(cerr, "RD", msg);
	}

	bool good() const {
		return this->valid && this->inputfile->good();
	}

	RdInstr* expectInstr(const char * expected = nullptr) {
		if (!this->isValid())
			return NULL;

		RdInstr* instr = readInstr();

		if (instr->file_off == eof) {
			invalidate("end of file");
			return NULL;
		}

		Trace::singleton()->logInstr(instr);

		if (expected && !instr->matches(expected, true))
			return NULL;
		else
			return instr;
	}
};
#endif /* PLOT_H_ */
