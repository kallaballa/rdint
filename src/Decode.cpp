#include <string>
#include <vector>
#include <cmath>
#include <cassert>
#include <iomanip>
#include "2D.hpp"
#include "Pcl.hpp"
#include "Plotter.hpp"
#include "Decode.hpp"

string byteToHexString(const uint8_t& b) {
	std::stringstream ss;
	ss << std::hex << std::setw(2) << std::setfill('0') << (int) b;
	return ss.str();
}

float fixed(float var, int roundDigits) {
	float f = pow(10, roundDigits);
	float value = (int) (var * f + .5);
	return (float) value / 100;
}

uint64_t parseUnsignedValue(const Data& data) {
	uint64_t value = 0;
	for (auto& b : data) {
		value *= 0x80;
		value += b;
	}
	return value;
}

int64_t parseSignedValue(const Data& data) {
	if (data.size() >= 1 && data[0] < 0x40) {
		return parseUnsignedValue(data);
	}
	Data dataInv;
	for (auto& b : data) {
		dataInv.push_back(0x7F ^ b);
	}
	int64_t valueInv = parseUnsignedValue(dataInv);
	return -(valueInv + 1);
}

struct CmdEmpty: CmdBase {
	CmdEmpty(Data& data) :
			CmdBase(data) {
	}
	virtual ~CmdEmpty() {
	}
	virtual void calcStats(Stats& stats) override {
		++stats.empty;
	}
	virtual COLORS getColor() override {
		return RED;
	}
	virtual string getName() override {
		return "empty command";
	}
	virtual std::vector<Param> getParams() {
		return {};
	}

	virtual void process(ProcState& procState) override {

	}
};

struct CmdGood: CmdBase {
	CmdGood(Data& data) :
			CmdBase(data) {
	}
	virtual ~CmdGood() {
	}

	virtual void calcStats(Stats& stats) override {
		++stats.good;
	}
	virtual COLORS getColor() override {
		return GREEN;
	}
	virtual void process(ProcState& procState) override {

	}
};

struct CmdIncomplete: CmdBase {
	CmdIncomplete(Data& data) :
			CmdBase(data) {
	}
	virtual ~CmdIncomplete() {
	}

	virtual void calcStats(Stats& stats) override {
		++stats.incomplete;
	}
	virtual COLORS getColor() override {
		return PINK;
	}
	virtual string getName() override {
		return "incomplete command";
	}
	virtual std::vector<Param> getParams() {
		return {};
	}

	virtual void process(ProcState& procState) override {

	}
};

struct CmdUnknown: CmdBase {
	CmdUnknown(Data& data) :
			CmdBase(data) {
	}
	virtual ~CmdUnknown() {
	}

	virtual void calcStats(Stats& stats) override {
		++stats.unknown;
	}
	virtual COLORS getColor() override {
		return YELLOW;
	}
	virtual string getName() override {
		return "unknown command";
	}

	virtual std::vector<Param> getParams() {
		return {};
	}

	virtual void process(ProcState& procState) override {

	}
};

struct CmdCoords: CmdGood {
	bool isMax;
	coord x;
	coord y;
	CmdCoords(Data& data) :
			CmdGood(data) {
		isMax = data[1] == 0x07;
		x = parseSignedValue(Data(data.begin() + 2, data.begin() + 7));
		y = parseSignedValue(Data(data.begin() + 7, data.begin() + 12));
	}
	virtual ~CmdCoords() {
	}

	virtual string getName() override {
		return string() + (isMax ? "max" : "min") + "imum absolute coordinates";
	}
	virtual std::vector<Param> getParams() override {
		return {
			ParamPhys("x", x, 1e-3, 3, "mm"),
			ParamPhys("y", y, 1e-3, 3, "mm")
		};
	}

	virtual void process(ProcState& procState) override {
		procState.setLimits(isMax, x, y);
	}
};

struct CmdCutMoveAbs: CmdGood {
	bool isCut;
	coord x;
	coord y;
	CmdCutMoveAbs(Data& data) :
			CmdGood(data) {
		isCut = data[0] == 0xA8;
		x = parseSignedValue(Data(data.begin() + 1, data.begin() + 6));
		y = parseSignedValue(Data(data.begin() + 6, data.begin() + 11));
	}

	virtual string getName() override {
		return string() + (isCut ? "cut" : "move") + " to absolute position";
	}
	virtual std::vector<Param> getParams() override {
		return {
			ParamPhys("x", x, 1e-3, 3, "mm"),
			ParamPhys("y", y, 1e-3, 3, "mm")
		};
	}
	virtual void process(ProcState& procState) override {
		if (isCut) {
			procState.cutAbs(x, y);
		} else {
			procState.moveAbs(x, y);
		}
	}
};

struct CmdCutMoveRel: CmdGood {
	bool isCut;
	coord x;
	coord y;

	CmdCutMoveRel(Data &data) :
			CmdGood(data) {
		isCut = data[0] == 0xA9;
		x = parseSignedValue(Data(data.begin() + 1, data.begin() + 3));
		y = parseSignedValue(Data(data.begin() + 3, data.begin() + 5));
	}
	virtual string getName() {
		return string() + (isCut ? "cut" : "move") + " to relative position";
	}
	virtual std::vector<Param> getParams() {
		return {
			ParamPhys("x", x, 1e-3, 3, "mm"),
			ParamPhys("y", y, 1e-3, 3, "mm")
		};
	}
	virtual void process(ProcState& procState) {
		if (isCut) {
			procState.cutRel(x, y);
		} else {
			procState.moveRel(x, y);
		}
	}
};

struct CmdCutMoveRel1: CmdGood {
	bool isCut;
	bool isY;
	coord xy;
	CmdCutMoveRel1(Data& data) :
			CmdGood(data) {
		isCut = data[0] == 0xAA || data[0] == 0xAB;
		isY = data[0] == 0x8B || data[0] == 0xAB;
		xy = parseSignedValue(Data(data.begin() + 1, data.begin() + 3));
	}
	virtual string getName() override {
		return string() + (isCut ? "cut" : "move") + " to relative position in "
				+ (isY ? "Y" : "X") + " direction";
	}
	virtual std::vector<Param> getParams() override {
		return {
			ParamPhys(isY ? "y" : "x", xy, 1e-3, 3, "mm")
		};
	}
	virtual void process(ProcState& procState) override {
		coord x = isY ? 0 : xy;
		coord y = isY ? xy : 0;
		if (isCut) {
			procState.cutRel(x, y);
		} else {
			procState.moveRel(x, y);
		}
	}
};

struct CmdEnableDisable: CmdGood {
	dim devices;
	bool airBlower;
	CmdEnableDisable(Data& data) :
			CmdGood(data) {
		devices = parseUnsignedValue(Data(data.begin() + 2, data.begin() + 3));
		airBlower = (devices & 0x01) != 0;
	}
	virtual string getName() override {
		return "enable / disable helper devices";
	}

	virtual std::vector<Param> getParams() override {
		return {
			Param("air blower", airBlower ? "on" : "off"),
			Param("unknown devices", byteToHexString(devices & 0x7E))
		};
	}
	virtual void process(ProcState& procState) override {
	}
};

struct CmdSetColorLayer: CmdGood {
	dim layer;
	dim color;
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	CmdSetColorLayer(Data& data) :
			CmdGood(data) {
		layer = parseUnsignedValue(Data(data.begin() + 2, data.begin() + 3));
		color = parseUnsignedValue(Data(data.begin() + 3, data.begin() + 8));
		red = color & 0xFF;
		green = color >> 8 & 0xFF;
		blue = color >> 16 & 0xFF;
	}
	virtual string getName() override {
		return "set color for layer";
	}
	virtual std::vector<Param> getParams() override {
		return {
			Param("layer", std::to_string(layer)),
			Param("red", byteToHexString(red)),
			Param("green", byteToHexString(green)),
			Param("blue", byteToHexString(blue))
		};
	}
	virtual void process(ProcState& procState) override {
		procState.setLayerColor(layer, red, green, blue);
	}
};

struct CmdSetCurLayer: CmdGood {
	dim layer;
	CmdSetCurLayer(Data& data) :
			CmdGood(data) {
		layer = parseUnsignedValue(Data(data.begin() + 2, data.begin() + 3));
	}
	virtual string getName() override {
		return "set current layer";
	}
	virtual std::vector<Param> getParams() override {
		return {
			Param("layer", std::to_string(layer))
		};
	}
	virtual void process(ProcState& procState) override {
		procState.setCurLayer(layer);
	}
};

struct CmdSetMaxLayer: CmdGood {
	dim layer;
	CmdSetMaxLayer(Data& data) :
			CmdGood(data) {
		layer = parseUnsignedValue(Data(data.begin() + 2, data.begin() + 3));
	}
	virtual string getName() override {
		return "set maximum layer number";
	}
	virtual std::vector<Param> getParams() override {
		return {
			Param("layer", std::to_string(layer))
		};
	}
	virtual void process(ProcState& procState) override {
		procState.setMaxLayer(layer);
	}
};

struct CmdSetPwr: CmdGood {
	int16_t laserNo;
	bool isMax;
	dim pwr;
	CmdSetPwr(Data& data) :
			CmdGood(data) {
		laserNo = ((data[1] & 0x20) >> 5) + 1;
		isMax = ((data[1] & 0x02) != 0);
		pwr = parseUnsignedValue(Data(data.begin() + 2, data.begin() + 4));
	}

	virtual string getName() override {
		return string() + "set " + (isMax ? "max" : "min")
				+ "imum power for laser " + std::to_string(laserNo);
	}

	virtual std::vector<Param> getParams() override {
		return {
			ParamPhys("power", pwr, 100.0 / 0x3FFF, 2, "%")
		};
	}
	virtual void process(ProcState& procState) override {
		procState.setPwr(pwr);
	}
};

struct CmdSetPwrLayer : CmdGood {
	int16_t laserNo;
	int16_t layer;
	bool isMax;
	dim pwr;

	CmdSetPwrLayer(Data& data) : CmdGood(data) {
    laserNo = ((data[1] & 0x70) >> 4) - 2;
    isMax = ((data[1] & 0x02) != 0);
    layer = parseUnsignedValue(Data(data.begin() + 2, data.begin() + 3));
    pwr = parseUnsignedValue(Data(data.begin() + 3, data.begin() + 5));
  }
  virtual string getName() override {
    return string() + "set " + (isMax ? "max" : "min") + "imum power for laser "
           + std::to_string(laserNo) + " for layer";
  }

  virtual std::vector<Param> getParams() override {
    return {
      Param("layer", std::to_string(layer)),
      ParamPhys("power", pwr, 100.0 / 0x3FFF, 2, "%")
    };
  }
  virtual void process(ProcState& procState) override {
    procState.setLayerPwr(layer, pwr);
  }
};

struct CmdSetSpeed : CmdGood {
	dim speed;
CmdSetSpeed(Data& data) : CmdGood(data) {
    speed = parseUnsignedValue(Data(data.begin() + 2, data.begin() + 7));
  }
  virtual string getName() override {
    return "set speed";
  }
  virtual std::vector<Param> getParams() override {
    return {
      ParamPhys("speed", speed, 1e-3, 3, "mm/s")
    };
  }
  virtual void process(ProcState& procState) override {
	  procState.setSpeed(speed);
  }
};

struct CmdSetSpeedLayer : CmdGood {
	int16_t layer;
	dim speed;
	CmdSetSpeedLayer(Data& data) : CmdGood(data) {
    layer = parseUnsignedValue(Data(data.begin() + 2, data.begin() + 3));
    speed = parseUnsignedValue(Data(data.begin() + 3, data.begin() + 8));
  }

  virtual string getName() override {
    return "set speed for layer";
  }
  virtual std::vector<Param> getParams() override {
    return {
      Param("layer", std::to_string(layer)),
      ParamPhys("speed", speed, 1e-3, 3, "mm/s")
    };
  }

  virtual void process(ProcState& procState) override {
    procState.setLayerSpeed(layer, speed);
  }
};

CmdBase* parseCmdCoords(Data& data) {
	if (data.size() != 12 || data[0] != 0xE7
			|| (data[1] != 0x03 && data[1] != 0x07)) {
		return new CmdUnknown(data);
	}
	return new CmdCoords(data);
}

CmdBase* parseCmdCutMoveAbs(Data& data) {
	if (data.size() != 11 || (data[0] != 0x88 && data[0] != 0xA8)) {
		return new CmdUnknown(data);
	}
	return new CmdCutMoveAbs(data);
}

CmdBase* parseCmdCutMoveRel(Data& data) {
	if (data.size() != 5 || (data[0] != 0x89 && data[0] != 0xA9)) {
		return new CmdUnknown(data);
	}
	return new CmdCutMoveRel(data);
}

CmdBase* parseCmdCutMoveRel1(Data& data) {
	if (data.size() != 3
			|| (data[0] != 0x8A && data[0] != 0x8B && data[0] != 0xAA
					&& data[0] != 0xAB)) {
		return new CmdUnknown(data);
	}
	return new CmdCutMoveRel1(data);
}

CmdBase* parseCmdEnableDisable(Data& data) {
	if (data.size() != 3 || data[0] != 0xCA || data[1] != 0x01) {
		return new CmdUnknown(data);
	}
	return new CmdEnableDisable(data);
}

CmdBase* parseCmdSetColorLayer(Data& data) {
  if (data.size() != 8 || data[0] != 0xCA || data[1] != 0x06) {
    return new CmdUnknown(data);
  }
  return new CmdSetColorLayer(data);
}

CmdBase* parseCmdSetCurLayer(Data& data) {
  if (data.size() != 3 || data[0] != 0xCA || data[1] != 0x02) {
    return new CmdUnknown(data);
  }
  return new CmdSetCurLayer(data);
}

CmdBase* parseCmdSetMaxLayer(Data& data) {
  if (data.size() != 3 || data[0] != 0xCA || data[1] != 0x22) {
    return new CmdUnknown(data);
  }
  return new CmdSetMaxLayer(data);
}

CmdBase* parseCmdSetPwr(Data& data) {
  if (data.size() != 4 || data[0] != 0xC6 ||
      (data[1] != 0x01 && data[1] != 0x02 &&
       data[1] != 0x21 && data[1] != 0x22)) {
    return new CmdUnknown(data);
  }
  return new CmdSetPwr(data);
}

CmdBase* parseCmdSetPwrLayer(Data& data) {
  if (data.size() != 5 || data[0] != 0xC6 ||
      (data[1] != 0x31 && data[1] != 0x32 &&
       data[1] != 0x41 && data[1] != 0x42)) {
    return new CmdUnknown(data);
  }
  return new CmdSetPwrLayer(data);
}

CmdBase* parseCmdSetSpeed(Data& data) {
  if (data.size() != 7 || data[0] != 0xC9 || data[1] != 0x02) {
    return new CmdUnknown(data);
  }
  return new CmdSetSpeed(data);
}

CmdBase* parseCmdSetSpeedLayer(Data& data) {
  if (data.size() != 8 || data[0] != 0xC9 || data[1] != 0x04) {
    return new CmdUnknown(data);
  }
  return new CmdSetSpeedLayer(data);
}

CmdBase* parseCommand(Data& data) {
  if (data.size() < 1) {
    return new CmdEmpty(data);
  }
  if (data[0] < 0x80) {
    return new CmdIncomplete(data);
  }
  switch (data[0]) {
    case 0x88: return parseCmdCutMoveAbs(data);
    case 0x89: return parseCmdCutMoveRel(data);
    case 0x8A: return parseCmdCutMoveRel1(data);
    case 0x8B: return parseCmdCutMoveRel1(data);
    case 0xA8: return parseCmdCutMoveAbs(data);
    case 0xA9: return parseCmdCutMoveRel(data);
    case 0xAA: return parseCmdCutMoveRel1(data);
    case 0xAB: return parseCmdCutMoveRel1(data);
    case 0xC6:
      if (data.size() >= 2) {
        switch (data[1]) {
          case 0x01:
          case 0x02:
          case 0x21:
          case 0x22: return parseCmdSetPwr(data);
          case 0x31:
          case 0x32:
          case 0x41:
          case 0x42: return parseCmdSetPwrLayer(data);
        }
      }
      break;
    case 0xC9:
      if (data.size() >= 2) {
        switch (data[1]) {
          case 0x02: return parseCmdSetSpeed(data);
          case 0x04: return parseCmdSetSpeedLayer(data);
        }
      }
      break;
    case 0xCA:
      if (data.size() >= 2) {
        switch (data[1]) {
          case 0x01: return parseCmdEnableDisable(data);
          case 0x02: return parseCmdSetCurLayer(data);
          case 0x06: return parseCmdSetColorLayer(data);
          case 0x22: return parseCmdSetMaxLayer(data);
        }
      }
      break;
    case 0xE7:
      if (data.size() >= 2) {
        switch (data[1]) {
          case 0x03:
          case 0x07: return parseCmdCoords(data);
        }
      }
      break;
  }
  return new CmdUnknown(data);
}

void VectorPlotterProc::cut(const coord& x1, const coord& y1, const coord& x2,
		const coord& y2) {
	vplot_.move(x1 * scale, y1 * scale);
	vplot_.penDown();
	vplot_.move(x2 * scale, y2 * scale);
	vplot_.penUp();
}


