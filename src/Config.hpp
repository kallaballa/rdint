#ifndef CONFIG_H_
#define CONFIG_H_

#include "2D.hpp"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

enum DEBUG_LEVEL {
  LVL_QUIET, LVL_INFO, LVL_WARN, LVL_DEBUG
};
class Config {
private:
  Config(): interactive(false), autocrop(false), clip(NULL), screenSize(NULL), ifilename(NULL), rasterFilename(NULL), vectorFilename(NULL), combinedFilename(NULL), debugLevel(LVL_WARN) {};
  static Config* instance;
public:
  bool interactive;
  bool autocrop;
  BoundingBox *clip;
  BoundingBox *screenSize;
  char *ifilename;
  char *rasterFilename;
  char *vectorFilename;
  char *combinedFilename;
  DEBUG_LEVEL debugLevel;

  static Config* singleton();

  void parseCommandLine(int argc, char *argv[]);
  void printUsage();
};


#endif /* CONFIG_H_ */
