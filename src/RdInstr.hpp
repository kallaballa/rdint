#ifndef PCLFILE_H_
#define PCLFILE_H_

#include <cstdint>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ostream;

// off_t is 64-bit on BSD-derived systems
#ifdef __APPLE__
typedef off_t off64_t;
#endif

class RdInstr {
public:
  RdInstr(off64_t file_off) : command(), data(), file_off(file_off) {}
  string command;
  std::vector<uint8_t> data;
  off64_t file_off;

  bool matches(const string& sig, const bool report=false);
  static const string pretty(char c);
  friend ostream& operator <<(ostream &os, const RdInstr &instr) {
	std::stringstream ss;
	ss << std::hex << "(" << std::setw(8) << std::setfill('0') << instr.file_off << ")";
	return os << ss.str();
  }
};

#endif /* PCLFILE_H_ */
