#ifndef PLOT_H_
#define PLOT_H_

#include <assert.h>
#include <map>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <boost/format.hpp>

#include "Config.hpp"

using std::ios;
using std::cin;
using std::cerr;
using std::endl;
using std::string;
using std::ifstream;
using std::numeric_limits;
using boost::format;

const string INSTR_MOVE_TO = "\x88\x00";
const string INSTR_CUT_TO = "\xA8\x00";

class RdPlot {
private:
  ifstream* inputfile;
  off64_t eof;
  bool valid;

  std::map<string, RdInstr*> settings;

  uint8_t descramble(uint8_t s) {
   uint8_t a = (s + 0xFF) & 0xFF;
   uint8_t b = a ^ 0x33;
   uint8_t p = b & 0x7E | b >> 7 & 0x01 | b << 7 & 0x80;
   return p;
  }

  RdInstr* readInstr() {
    off64_t off = this->inputfile->tellg();
    uint8_t c = descramble(this->inputfile->get());
    assert(c >= 0x80);
    RdInstr* instr = new RdInstr(off);
    instr->command = c;

    while(this->inputfile->good() && (c = descramble(this->inputfile->get())) < 0x80) {
    	instr->data.push_back(c);
    }

    this->inputfile->unget();
    return instr;
  }

public:
  RdInstr* currentInstr;

  RdPlot(ifstream *infile) :
    inputfile(infile), eof(numeric_limits<off64_t>::max()), valid(true), currentInstr(NULL) {
    this->readSettings();
  }

  virtual ~RdPlot() {
  }

  bool isValid() {
    return valid;
  }

  void readSettings() {
//    RdInstr* instr;
//    while ((instr = expectInstr()) && !instr->matches(INSTR_MOVE_TO)) {
//      settings[instr->command] = instr;
//    }
//    printSettings(cerr);
  }

  void printSettings(ostream& os) {
    if(Config::singleton()->debugLevel >= LVL_DEBUG && settings.size() > 0) {
      std::map<string, RdInstr*>::iterator it;

      os << "=== settings: " << endl;
      for (it = settings.begin(); it != settings.end(); it++) {
        os << "\t" << *((*it).second) << endl;
      }
    }
  }

  bool require(string signature) {
    if (isSet(signature))
      return true;
    else
      invalidate("missing setting: " + signature);

    return false;
  }

  bool isSet(string signature) {
    return this->settings.find(signature) != settings.end();
  }

  RdInstr* setting(string signature) {
    return this->settings[signature];
  }

  void invalidate(string msg) {
    this->valid = false;
    Trace::singleton()->printBacklog(cerr, "PCL", msg);
  }

  bool good() const {
    return this->valid && this->inputfile->good();
  }

  RdInstr* expectInstr(const char * expected = NULL) {
    if(!this->isValid())
      return NULL;

    RdInstr* instr = readInstr();

    if(instr->file_off == eof) {
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

enum RtlContext {
  PCL_CONTEXT, HPGL_CONTEXT, NONE
};

class Plot {
public:
  ifstream* inputfile;
  string title;
  RdPlot *currentPclPlot;
  bool valid;
  int64_t width;
  int64_t height;
  int64_t resolution;

  void readMagic() {
    char* buffer = new char[MAGIC_SIZE];
    this->inputfile->read(buffer, MAGIC_SIZE);

    if (memcmp(MAGIC, buffer, MAGIC_SIZE) != 0) {
      invalidate("magic doesn't match");
    }
  }

  void invalidate(string msg) {
    this->valid = false;
    Trace::singleton()->printBacklog(cerr, "RTL", msg);
  }

  bool checkPclContext() {
    if (currentPclPlot != NULL && currentPclPlot->isValid()) {
      return true;
    } else {
      return false;
    }
  }
public:

  Plot(ifstream *infile) :
    inputfile(infile), currentPclPlot(NULL), valid(true) {
    readMagic();
    currentPclPlot = new RdPlot(infile);
//assert(false);
//    if (currentPclPlot->require(PCL_WIDTH) && currentPclPlot->require(PCL_HEIGHT)
//        && currentPclPlot->require(PCL_PRINT_RESOLUTION)) {
//      this->width = currentPclPlot->setting(PCL_WIDTH);
//      this->height = currentPclPlot->setting(PCL_HEIGHT);
//      this->resolution = currentPclPlot->setting(PCL_PRINT_RESOLUTION);
//    } else
//      currentPclPlot->invalidate("can't find plot dimensions");
  }

  virtual ~Plot() {
  }

  bool isValid() {
    return valid;
  }

  uint32_t getWidth() {
    return 13000;
  }

  uint32_t getHeight() {
    return 9000;
  }

  uint32_t getResolution() {
    return 100;
  }

  RdPlot* requestPclPlot() {
    return currentPclPlot;
  }
};
#endif /* PLOT_H_ */
