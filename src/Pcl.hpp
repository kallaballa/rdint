#ifndef PCLFILE_H_
#define PCLFILE_H_

#include <stdint.h>
#include <string.h>
#include <list>
#include <iostream>
#include <iomanip>

#include <boost/format.hpp>
#include "2D.hpp"
#include "Config.hpp"
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ostream;
using std::list;
using boost::format;

// off_t is 64-bit on BSD-derived systems
#ifdef __APPLE__
typedef off_t off64_t;
#endif

class RdInstr {
	string byteToHexString(const uint8_t& b) {
		std::stringstream ss;
		ss << std::hex << std::setw(2) << std::setfill('0') << (int) b;
		return ss.str();
	}
public:
  RdInstr(off64_t file_off) : command(), data(), file_off(file_off) {}
  string command;
  std::vector<uint8_t> data;
  off64_t file_off;

  bool matches(const string& sig, const bool report=false) {
    bool m = byteToHexString(this->command.at(0)) == sig;
    if(!m && report && Config::singleton()->debugLevel >= LVL_WARN) {
      cerr << "expected: " << sig << " found: " << this->command << endl;
    }
    return m;
  }

  static const string pretty(char c) {
    if(!isgraph(c))
      return (format("(0x%02X)") % c).str();
    else
      return (format("%c") % c).str();
  }

  //FIXME friend used on a member function?
  friend ostream& operator <<(ostream &os, const RdInstr &instr) {
    format fmtInstr("(%08X)");
    fmtInstr % instr.file_off;

    return os << fmtInstr;
  }
};

#endif /* PCLFILE_H_ */
