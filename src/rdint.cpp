#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "CImg.hpp"
#include "Interpreter.hpp"
#include "2D.hpp"
#include "Plotter.hpp"
#include <stdlib.h>

#include "Config.hpp"
#ifdef PCLINT_USE_SDL
#include <SDL.h>
#endif

using std::ofstream;
using std::ifstream;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

using namespace cimg_library;

Config* Config::instance = NULL;

Config* Config::singleton() {
  if (instance == NULL)
    instance = new Config();

  return instance;
}

Trace* Trace::instance = NULL;
Trace* Trace::singleton() {
  if (instance == NULL)
    instance = new Trace();

  return instance;
}

Debugger* Debugger::instance = NULL;
class VectorPlotter;
void Debugger::create(void *vplotter) {
  if(instance == NULL)
    instance = new Debugger((void *)vplotter);
}

Debugger* Debugger::getInstance() {
  return instance;
}

Statistic* Statistic::instance = NULL;
Statistic* Statistic::init(uint32_t width, uint32_t height, uint16_t resolution) {
  assert(instance == NULL);
  return instance = new Statistic(width, height, 100);
}

Statistic* Statistic::singleton() {
  assert(instance != NULL);
  return instance;
}

int main(int argc, char *argv[]) {
  Trace* trace = Trace::singleton();
  Config* config = Config::singleton();
  config->parseCommandLine(argc, argv);
  ifstream *infile = new ifstream(config->ifilename, ios::in | ios::binary);
  RdPlot* plot = new RdPlot(infile);
  Statistic::init(1300, 900, 100);

  Interpreter intr;
  if (config->interactive) {
    Debugger::create(intr.vectorPlotter);
    Debugger::getInstance()->setInteractive(true);
  } else
    Debugger::create();

  intr.run(plot);

  BoundingBox& vBox = intr.vectorPlotter->getBoundingBox();
  if (vBox.isValid()) {
    if (config->vectorFilename != NULL)
      intr.vectorPlotter->dumpCanvas(string(config->vectorFilename));
  } else {
    trace->warn("Vector image is empty.");
  }

//  BoundingBox& bmpBox = intr.bitmapPlotter->getBoundingBox();
//  if (bmpBox.isValid()) {
//    if (config->rasterFilename != NULL)
//      intr.bitmapPlotter->dumpCanvas(string(config->rasterFilename));
//  } else {
//    trace->warn("Bitmap image is empty.");
//  }

  if (config->debugLevel >= LVL_INFO) {
    Statistic::singleton()->printSlot(cout, SLOT_VECTOR);
//    Statistic::singleton()->printSlot(cout, SLOT_RASTER);
  }

  return 0;
}
