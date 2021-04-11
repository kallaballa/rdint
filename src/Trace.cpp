#include "Trace.hpp"
#include "Config.hpp"

Trace* Trace::instance = NULL;
Trace* Trace::singleton() {
  if (instance == NULL)
    instance = new Trace();

  return instance;
}

void Trace::logInstr(RdInstr* instr) {
	if (Config::singleton()->debugLevel >= LVL_DEBUG)
		cerr << penPos << "\t" << *instr << endl;

	if (backlog.size() >= backlogSize)
		backlog.erase(backlog.begin());

	backlog.push_back(instr);
}

void Trace::logPlotterStat(Point &penPos) {
	this->penPos = penPos;
}

std::list<RdInstr*>::iterator Trace::backlogIterator() {
	return backlog.begin();
}

std::list<RdInstr*>::iterator Trace::backlogEnd() {
	return backlog.end();
}

void Trace::info(string msg) {
	if (Config::singleton()->debugLevel >= LVL_INFO)
		cout << msg << endl;
}

void Trace::warn(string msg) {
	if (Config::singleton()->debugLevel >= LVL_WARN)
		cerr << "WARNING: " << msg << endl;
}

void Trace::debug(string msg) {
	if (Config::singleton()->debugLevel >= LVL_DEBUG)
		cerr << "DEBUG: " << msg << endl;
}

void Trace::printBacklog(ostream &os, string caller, string msg) {
	if (Config::singleton()->debugLevel < LVL_DEBUG)
		return;

	os << "=== " << caller << " trace: " << msg << ": " << endl;
	if (backlog.empty()) {
		os << "(backlog N/A)" << endl;
	} else {
		for (std::list<RdInstr*>::iterator it = backlogIterator();
				it != backlogEnd(); it++)
			os << "\t" << *(*it) << endl;
	}
	os << endl;
}

