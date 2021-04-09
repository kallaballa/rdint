#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include "Pcl.hpp"
#include "Plot.hpp"
#include "CLI.hpp"
#include "Config.hpp"
#include "Plotter.hpp"
#include "Decode.hpp"

using std::string;
using std::stringstream;


class Interpreter {

private:
	string make_color(const string& s, const COLORS& c) {
	  return "\033[0;" + std::to_string(c) + "m" + s + "\033[0;39m";
	}

	string make_bold(const string& s) {
	  return "\033[1m" + s + "\033[0m";
	}

  dim width;
  dim height;

  RdInstr* nextRdInstr(RdPlot* rdPlot, const char* expected = NULL) {
    RdInstr* instr = rdPlot->expectInstr(expected);
    Debugger::getInstance()->announce(instr);
    return instr;
  }

public:
  VectorPlotter* vectorPlotter;
  BitmapPlotter* bitmapPlotter;
  Plot* rtlplot;

  Interpreter(Plot* plot): width(0), height(0), bitmapPlotter(NULL), rtlplot(plot){
    this->bitmapPlotter = new BitmapPlotter(plot->getWidth()/8, plot->getHeight(), Config::singleton()->clip);
    this->vectorPlotter = new VectorPlotter(plot->getWidth(),plot->getHeight(), Config::singleton()->clip);
  };

  void renderRdPlot(RdPlot *rdPlot) {
    RdInstr* rdInstr;
    VectorPlotterProc proc(*this->vectorPlotter);
    while (rdPlot->good() && (rdInstr = rdPlot->expectInstr())) {
      Debugger::getInstance()->announce(rdInstr);
      if (Config::singleton()->debugLevel >= LVL_DEBUG || Config::singleton()->interactive) {
        cerr << *rdInstr << " -> " << std::hex << std::setfill('0') << std::setw(2) << (0xFF & (int)rdInstr->command.at(0)) << " ";
        for(auto& c : rdInstr->data) {
        	cerr << (int)c;
        }
        cerr << endl;
      }
      std::vector<uint8_t> data;
      data.push_back(rdInstr->command.at(0));
      for(auto& b : rdInstr->data){
    	  data.push_back(b);
      }

      CmdBase* cmd = parseCommand(data);
      std::cerr << make_color(cmd->toString(), cmd->getColor()) << std::endl << '>';
      cmd->process(proc);

    }
  }
};

#endif /* INTERPRETER_H_ */
