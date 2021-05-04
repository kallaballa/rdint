#ifndef SRC_DECODE_HPP_
#define SRC_DECODE_HPP_

#include <string>
#include <vector>
#include "Terminal.hpp"

using namespace std;
typedef std::vector<uint8_t> Data;

string byteToHexString(const uint8_t& b);
string makeFixedString(float var, int roundDigits);
uint64_t parseUnsignedValue(const Data& data);
int64_t parseSignedValue(const Data& data);

struct Param {
	string name_;
	string value_;
	Param(const string& name, const string& value) :
			name_(name), value_(value) {
	}

	virtual string toString() {
		return name_ + " " + value_;
	}
};

struct ParamPhys: public Param {
	float scale_;
	int roundDigits_;
	string unit_;

	ParamPhys(const string& name, const float& value, const float& scale,
			int round_digits, string unit) :
			Param(name, makeFixedString(value * scale, round_digits)), scale_(
					scale), roundDigits_(round_digits), unit_(unit) {
	}

	virtual string toString() override {
		return name_ + " " + value_ + " " + unit_;
	}
};

struct Stats {
	size_t empty = 0;
	size_t good = 0;
	size_t incomplete = 0;
	size_t unknown = 0;
};

struct Layer {
	size_t red = 0;
	size_t green = 0;
	size_t blue = 0;
	size_t pwr = 0;
	size_t speed = 0;
	bool valid;

	Layer(const bool& valid = true) :
			valid(valid) {
	}
};

struct ProcState {
	std::vector<Layer> layers;
	int16_t layerNo = -1;
	Layer layer;
	coord maxX = 0;
	coord maxY = 0;
	coord minX = 0;
	coord minY = 0;
	dim pwr = 0;
	dim speed = 0;
	coord x = 0;
	coord y = 0;

	ProcState() {
	}
	virtual ~ProcState() {
	}

	virtual void cut(const coord& x1, const coord& y1, const coord& x2,
			const coord& y2) = 0;
	virtual void setLimits(const bool& isMax, const coord& x,
			const coord& y) = 0;

	void cutAbs(const coord& x, const coord& y) {
		coord xs = 1300 - (x / 1000.0);
		coord ys = y / 1000.0;
		this->cut(this->x, this->y, xs, ys);
		this->x = xs;
		this->y = ys;
	}

	void cutRel(const coord& x, const coord& y) {
		coord xs = 1300 - (x / 1000.0);
		coord ys = y / 1000.0;

		this->cut(this->x, this->y, this->x + xs, this->y + ys);
		this->x += xs;
		this->y += ys;
	}

	Layer& getLayer(int16_t layerNo) {
		if (layerNo < 0) {
			assert(false);
		}
		while (static_cast<size_t>(layerNo) >= layers.size()) {
			layers.push_back(Layer());
		}
		return layers[layerNo];
	}

	void moveAbs(const coord& x, const coord& y) {
		this->x = 1300 - (x / 1000.0);
		this->y = y / 1000.0;
	}

	void moveRel(const coord& x, const coord& y) {
		this->x += 1300 - (x / 1000.0);
		this->y += y / 1000.0;
	}

	void setLayerColor(int16_t layerNo, uint8_t red, uint8_t green,
			uint8_t blue) {
		auto& layer = this->getLayer(layerNo);
		layer.red = red;
		layer.green = green;
		layer.blue = blue;
	}
	void setLayerPwr(int16_t layerNo, dim pwr) {
		this->getLayer(layerNo).pwr = pwr;
	}
	void setLayerSpeed(int16_t layerNo, dim speed) {
		this->getLayer(layerNo).speed = speed;
	}

	void setCurLayer(int16_t layerNo) {
		this->layerNo = layerNo;
		this->layer = this->getLayer(layerNo);
	}
	void setMaxLayer(int16_t layerNo) {
		this->getLayer(layerNo); // just make sure it exists
	}
	void setPwr(dim pwr) {
		this->pwr = pwr;
	}
	void setSpeed(dim speed) {
		this->speed = speed;
	}
};

class NullProcState: public ProcState {
public:
	NullProcState() {
	}

	virtual void cut(const coord& x1, const coord& y1, const coord& x2,
			const coord& y2) override {
	}

	virtual void setLimits(const bool& isMax, const coord& x, const coord& y)
			override {
		if (isMax) {
			maxX = 1300 - (x / 1000.0);
			maxY = y / 1000.0;
		} else {
			minX = 1300 - (x / 1000.0);
			minY = y / 1000.0;
		}
	}
};

class VectorPlotter;
class VectorProcState: public ProcState {
	VectorPlotter& vplot_;
public:
	VectorProcState(VectorPlotter& vplot) :
			vplot_(vplot) {
	}

	virtual void cut(const coord& x1, const coord& y1, const coord& x2,
			const coord& y2) override;

	virtual void setLimits(const bool& isMax, const coord& x, const coord& y)
			override {
		if (isMax) {
			maxX = 1300 - (x / 1000.0);
			maxY = y / 1000.0;
		} else {
			minX = 1300 - (x / 1000.0);
			minY = y / 1000.0;
		}
	}
};

struct CmdBase {
	Data data;
	CmdBase(Data& data) :
			data(data) {
	}
	virtual ~CmdBase() {
	}

	virtual void process(ProcState& procState) = 0;
	virtual void calcStats(Stats& stats) = 0;
	virtual TERM_COLORS getColor() = 0;
	virtual string getName() = 0;
	virtual std::vector<Param> getParams() = 0;

	virtual string toString() {
		auto cmdStr = getName();
		auto params = getParams();
		string sep = ": ";
		stringstream ss;
		ss << cmdStr;
		for (auto& p : params) {
			ss << sep << p.toString();
			sep = ", ";
		}
		return ss.str();
	}
};

CmdBase* parseCommand(Data& data);


#endif /* SRC_DECODE_HPP_ */
