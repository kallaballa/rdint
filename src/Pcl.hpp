#ifndef PCLFILE_H_
#define PCLFILE_H_

#include <cstdint>
#include <iostream>
#include <boost/format.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ostream;
using boost::format;

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
  	format fmtInstr("(%08X)");
  	fmtInstr % instr.file_off;

  	return os << fmtInstr;
  }
};

#endif /* PCLFILE_H_ */
