#ifndef SRC_TRACE_HPP_
#define SRC_TRACE_HPP_

#include <cstdint>
#include <string>
#include <list>
#include "2D.hpp"
#include "RdInstr.hpp"

class Trace {
private:
  const uint8_t backlogSize;
  std::list<RdInstr*> backlog;
  static Trace* instance;
  Point penPos;

  Trace(): backlogSize(10), penPos(0,0) {}
public:
  static Trace* singleton();

  void logInstr(RdInstr* instr);
  void logPlotterStat(Point &penPos);
  std::list<RdInstr*>::iterator backlogIterator();
  std::list<RdInstr*>::iterator backlogEnd();
  void info(string msg);
  void warn(string msg);
  void debug(string msg);
  void printBacklog(ostream &os, string caller, string msg);
};


#endif /* SRC_TRACE_HPP_ */
