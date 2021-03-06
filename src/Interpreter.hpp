#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "CLI.hpp"
#include "Config.hpp"
#include "Plotter.hpp"
#include "Decode.hpp"
#include "RdInstr.hpp"
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

	Interpreter() {
//    this->bitmapPlotter = new BitmapPlotter(plot->getWidth()/8, plot->getHeight(), Config::singleton()->clip);
	}
	;

	void applyCommand(RdInstr* rdInstr, ProcState* procState, bool print = true) {
		if(Debugger::getInstance())
			Debugger::getInstance()->announce(rdInstr);
		if (print && (Config::singleton()->debugLevel >= LVL_DEBUG
				|| Config::singleton()->interactive)) {
			cerr << std::dec << "[" << procState->x << ',' << procState->y << "] " << *rdInstr << " ->";
			for (auto& c : rdInstr->data) {
				cerr << " " << std::hex << std::setfill('0')
				<< std::setw(2) << (int) c;
			}
			cerr << endl;
		}

		CmdBase* cmd = parseCommand(rdInstr->data);
		if(print)
			std::cerr << "  " << make_color(cmd->toString(), cmd->getColor()) << std::endl << "> ";
		cmd->process(*procState);
	}

	void run(RdPlot *rdPlot, bool interactive) {
		RdInstr* rdInstr = nullptr;
		NullProcState nullPs;
		std::vector<RdInstr> header;
		while (rdPlot->good() && (rdInstr = rdPlot->expectInstr()) != nullptr) {
			header.push_back(*rdInstr);
			applyCommand(rdInstr, &nullPs, false);
		}

		if (rdInstr == nullptr) {
			rdPlot->invalidate("End of file reached without any absolute moves(?)");
		} else {
		  if (interactive) {
			Debugger::create(vectorPlotter);
			Debugger::getInstance()->setInteractive(true);
		  } else {
			Debugger::create();
		  }
			Statistic::init(nullPs.maxX, nullPs.maxY, 25.4);
			this->vectorPlotter = new VectorPlotter(nullPs.maxX, nullPs.maxY,
					Config::singleton()->clip);
			VectorProcState vecPs(*this->vectorPlotter);
			for(auto& instr : header) {
				applyCommand(&instr, &vecPs);
			}
			while (rdPlot->good() && (rdInstr = rdPlot->expectInstr())) {
				applyCommand(rdInstr, &vecPs);
			}

			if(interactive) {
				std::cerr << make_bold("End of file. Type any key to exit.")
						<< std::endl;
				char c;
				std::cin.read(&c, 1);
			}
		}
	}
};

#endif /* INTERPRETER_H_ */
