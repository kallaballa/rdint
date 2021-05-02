#ifndef CLI_H_
#define CLI_H_

#include <string>
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <SDL/SDL.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "CImg.hpp"
#include "RdPlot.hpp"

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::list;
using std::stringstream;

using namespace cimg_library;

class Barrier {
public:
    explicit Barrier(std::size_t iCount) :
      mThreshold(iCount),
      mCount(iCount),
      mGeneration(0) {
    }

    void wait() {
        std::unique_lock<std::mutex> lLock{mMutex};
        auto lGen = mGeneration;
        if (!--mCount) {
            mGeneration++;
            mCount = mThreshold;
            mCond.notify_all();
        } else {
            mCond.wait(lLock, [this, lGen] { return lGen != mGeneration; });
        }
    }

private:
    std::mutex mMutex;
    std::condition_variable mCond;
    std::size_t mThreshold;
    std::size_t mCount;
    std::size_t mGeneration;
};

class Debugger {
public:
  std::mutex ia_mutex;
  void* vplotter;
  volatile bool interactive;
  CImgDisplay* canvas_disp;
  bool autoupdate;
  list<off64_t> breakpoints;
  string find;
  string lastCliCmd[2];
  std::thread* cli_thrd;
  static Debugger* instance;
  Barrier step_barrier;
  bool run = true;

  void exec(string cmd, string param) {
    if (cmd.compare("help") == 0) {
     cerr << "run               continue processing the plot" << endl\
    	  << "quit              exit the program" << endl\
          << "break <hexoff>    set a breakpoint at the given address" << endl\
          << "step  <num>       process the given number of operations" << endl\
          << "find <instr>      find the next occurence of the given instruction" << endl\
          << "update <on/off>   without parameter just update the screen with the current plot stat" << endl\
          << "                  passing 'on' enables live updating the screen" << endl\
          << "                  passing 'off' disables live updating the screen" << endl\
          << "dump <filename>   dump the current state of the plot rendering to file" << endl;
    } else if (cmd.compare("run") == 0) {
      this->consume();
    } else {
      if (cmd.compare("break") == 0) {
        off64_t off = strtoll(param.c_str(), NULL, 16);
        if (off > 0) {
          breakpoints.push_back(off);
          cerr << "=== seeking: " << off << endl;
        } else
        cerr << "=== invalid offset: " << off << endl;
      } else if (cmd.compare("step") == 0) {
        this->waitSteps(strtol(param.c_str(), NULL, 10));
      } else if (cmd.compare("find") == 0) {
        find = param;
        cerr << "=== searching: " << find << endl;
        this->setInteractive(false);
        this->consume();
        find = "";
      } else if (cmd.compare("update") == 0) {
        if (param.compare("on") == 0) {
          this->autoupdate = true;
          cerr << "=== auto update on" << endl;
        }
        else if (param.compare("off") == 0) {
          this->autoupdate = false;
          cerr << "=== auto update off" << endl;
        }
      } else if (cmd.compare("quit") == 0) {
    	SDL_Quit();
    	exit(0);
      } else {
    	  cerr << "Unknown rdint command. Type 'help' for instructions." << endl;
      }
      //   else if(vplotter != NULL)
      //     vplotter->getCanvas()->update();
      // } else if (cmd.compare("dump") == 0 && vplotter != NULL) {
      //   vplotter->getCanvas()->dump(param.c_str());
      //   cerr << "=== dumped: " << param << endl;
      // }
    }

    lastCliCmd[0] = cmd;
    lastCliCmd[1] = param;
  }

  void consume() {
    this->setInteractive(false);
    while (!isInteractive())
      this->step_barrier.wait();
  }

  void waitSteps(uint32_t steps = 1) {
    while (isInteractive() && steps--) {
      this->step_barrier.wait();
    }
  }

  void checkBreakpoints(RdInstr *instr) {
    if (breakpoints.size() > 0 && instr) {
      list<off64_t>::iterator it;
      off64_t bp = numeric_limits<off64_t>::max();
      for (it = breakpoints.begin(); it != breakpoints.end(); it++) {
        bp = *it;
        if (instr->file_off >= bp) {
          cerr << "=== breakpoint" << endl;
          setInteractive(true);
          this->step_barrier.wait();
          it = breakpoints.erase(it);
//          break;
        }
      }
    }
  }

  void checkSignatures(RdInstr *instr) {
    if (find.length() > 0 && instr && instr->matches(find)) {
      cerr << "=== found " << find << endl;
      setInteractive(true);
      this->step_barrier.wait();
    }
  }

  void checkStepBarrier() {
    if (isInteractive())
      this->step_barrier.wait();
    // if(this->autoupdate && vplotter != NULL)
    //   vplotter->getCanvas()->update();
  }


public:
  static void create(void *vplotter = NULL);
  static Debugger* getInstance();

  Debugger(void* vplotter) :
    vplotter(vplotter), interactive(false), canvas_disp(NULL), autoupdate(false), cli_thrd(NULL), step_barrier(2) {
  }

  virtual void loop() {
    string line;
		cerr << "type 'help' for a list of available commands." << endl << "> ";
    while (run && cin) {
      getline(cin, line);
      stringstream ss(line);
      string cmd;
      string param;
      if(line.length() > 0) {
        ss >> cmd;
        ss >> param;
        exec(cmd,param);
      } else
        exec(lastCliCmd[0], lastCliCmd[1]);
    }
    SDL_Quit();
    exit(0);
  }

  virtual bool isInteractive() {
    std::unique_lock<std::mutex> ia_lock(ia_mutex);
    return interactive;
  }

  virtual void setInteractive(bool i) {
	 std::unique_lock<std::mutex> ia_lock(ia_mutex);
    //FIXME inclomplete sync: old loop still might be reading from cin
    if (!this->interactive && i && this->cli_thrd == NULL)
     this->cli_thrd = new std::thread([&](){
    	this->instance->loop();
    });

    this->interactive = i;
  }

  virtual void quit() {
	  run = false;
  }

  virtual void announce(RdInstr* instr) {
    checkStepBarrier();
    checkBreakpoints(instr);
    checkSignatures(instr);
  }
};

#endif /* CLI_H_ */
