#ifndef PCLFILE_H_
#define PCLFILE_H_

#include <stdint.h>
#include <string.h>
#include <list>
#include <iostream>
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

#define MAGIC_SIZE 2
const uint8_t MAGIC[2] = { 0xd8, 0x12};

class RdInstr {
public:
  RdInstr(off64_t file_off) : command(), data(), file_off(file_off) {}
  string command;
  std::vector<uint8_t> data;
  off64_t file_off;

  bool matches(const string& sig, const bool report=false) {
    bool m = this->command == sig;
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

class Trace {
private:
  const uint8_t backlogSize;
  list<RdInstr*> backlog;
  static Trace* instance;
  Point penPos;

  Trace(): backlogSize(10), penPos(0,0) {}
public:
  static Trace* singleton();

  void logInstr(RdInstr* instr) {
    if(Config::singleton()->debugLevel >= LVL_DEBUG)
      cerr << penPos << "\t" << *instr << endl;

    if(backlog.size() >= backlogSize)
      backlog.erase(backlog.begin());

    backlog.push_back(instr);
  }

  void logPlotterStat(Point &penPos) {
    this->penPos = penPos;
  }

  list<RdInstr*>::iterator backlogIterator() {
    return backlog.begin();
  }

  list<RdInstr*>::iterator backlogEnd() {
    return backlog.end();
  }

  void info(string msg) {
    if(Config::singleton()->debugLevel >= LVL_INFO)
      cout << msg << endl;
  }

  void warn(string msg) {
    if(Config::singleton()->debugLevel >= LVL_WARN)
      cerr << "WARNING: " << msg << endl;
  }

  void debug(string msg) {
    if(Config::singleton()->debugLevel >= LVL_DEBUG)
      cerr << "DEBUG: " << msg << endl;
  }

  void printBacklog(ostream &os, string caller, string msg) {
    if(Config::singleton()->debugLevel < LVL_DEBUG)
      return;

    os << "=== " << caller << " trace: " << msg << ": " << endl;
    if(backlog.empty()){
      os << "(backlog N/A)" << endl;
    }else{
      for(list<RdInstr*>::iterator it = backlogIterator(); it != backlogEnd(); it++)
        os << "\t" << *(*it) << endl;
    }
    os << endl;
  }
};

#endif /* PCLFILE_H_ */
