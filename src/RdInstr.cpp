#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <boost/format.hpp>
#include "Config.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ostream;
using boost::format;

#include "RdInstr.hpp"

// off_t is 64-bit on BSD-derived systems
#ifdef __APPLE__
typedef off_t off64_t;
#endif

string byteToHexString2(const uint8_t& b) {
	std::stringstream ss;
	ss << std::hex << std::setw(2) << std::setfill('0') << (int) b;
	return ss.str();
}

bool RdInstr::matches(const string& sig, const bool report) {
	bool m = byteToHexString2(this->command.at(0)) == sig;
	if (!m && report && Config::singleton()->debugLevel >= LVL_WARN) {
		cerr << "expected: " << sig << " found: " << this->command << endl;
	}
	return m;
}

const string RdInstr::pretty(char c) {
	if (!isgraph(c))
		return (format("(0x%02X)") % c).str();
	else
		return (format("%c") % c).str();
}


