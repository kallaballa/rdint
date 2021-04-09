#ifndef SRC_DECODE_HPP_
#define SRC_DECODE_HPP_

using std::string;
typedef std::vector<uint8_t> Data;

static string byteToHexString(const uint8_t& b);
static float fixed(float var, int roundDigits);
static uint64_t parseUnsignedValue(const Data& data);
static int64_t parseSignedValue(const Data& data);

struct Param {
	string name_;
	string value_;
	Param(const string& name, const string& value) :
			name_(name), value_(value) {
	}
	string toString() {
		return name_ + " " + value_;
	}
};

struct ParamPhys: public Param {
	float scale_;
	int roundDigits_;
	string unit_;

	ParamPhys(const string& name, const float& value, const float& scale,
			int round_digits, string unit) :
			Param(name, std::to_string(fixed(value * scale, round_digits))), scale_(
					scale), roundDigits_(round_digits), unit_(unit) {
	}
	string toString() {
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
		this->cut(this->x, this->y, x, y);
		this->x = x;
		this->y = y;
	}

	void cutRel(const coord& x, const coord& y) {
		this->cut(this->x, this->y, this->x + x, this->y + y);
		this->x += x;
		this->y += y;
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
		this->x = x;
		this->y = y;
	}

	void moveRel(const coord& x, const coord& y) {
		this->x += x;
		this->y += y;
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

class VectorPlotter;
class VectorPlotterProc: public ProcState {
	VectorPlotter& vplot_;
	float scale;
public:
	VectorPlotterProc(VectorPlotter& vplot) :
			vplot_(vplot) {
	}

	virtual void cut(const coord& x1, const coord& y1, const coord& x2,
			const coord& y2) override;

	virtual void setLimits(const bool& isMax, const coord& x, const coord& y)
			override {
		if (isMax) {
			maxX = x;
			maxY = y;
		} else {
			minX = x;
			minY = y;
		}

		 float dx = maxX - minX;
		 float dy = maxY - minY;
		 if (dx < 1) { dx = 1; }
		 if (dy < 1) { dy = 1; }
		 float width = Config::singleton()->screenSize->ul.x;
		 float height = Config::singleton()->screenSize->ul.y;

		 float scaleX = width / dx;
		 float scaleY = height / dy;
		 scale = scaleX < scaleY ? scaleX : scaleY;
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
	virtual string getHtmlColor() = 0;
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
