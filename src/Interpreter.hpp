#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include "Pcl.hpp"
#include "CLI.hpp"
#include "Config.hpp"
#include "Plotter.hpp"
#include "Decode.hpp"
#include "RdPlot.hpp"

using std::string;
using std::stringstream;

class Interpreter {
private:
  RdInstr* nextRdInstr(RdPlot* rdPlot, const char* expected = NULL) {
    RdInstr* instr = rdPlot->expectInstr(expected);
    Debugger::getInstance()->announce(instr);
    return instr;
  }

public:
  VectorPlotter* vectorPlotter = nullptr;
//  BitmapPlotter* bitmapPlotter = nullptr;

  Interpreter()  {
//    this->bitmapPlotter = new BitmapPlotter(plot->getWidth()/8, plot->getHeight(), Config::singleton()->clip);
  };

  void run(RdPlot *rdPlot) {
    RdInstr* rdInstr;
    this->vectorPlotter = new VectorPlotter(1300,900, Config::singleton()->clip);
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
    std::cerr << make_bold("End of file. Type any key to exit.") << std::endl;
    char c;
    std::cin.read(&c, 1);
  }
};

#endif /* INTERPRETER_H_ */
