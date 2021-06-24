#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin *pluginInstance;
extern Model *model_3mountains;
// Declare each Model, defined in each module source file
// extern Model *modelMyModule;
struct MyBigOrangeKnob : SVGKnob {
	MyBigOrangeKnob() {
		box.size = Vec(17, 17);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RoundLargeOrangeKnob.svg")));
	}
};
struct MyMassiveOrangeKnob : SVGKnob {
	MyMassiveOrangeKnob() {
		box.size = Vec(21, 21);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RoundHugeOrangeKnob.svg")));
		
	}
};
struct MyMassiveWhiteKnob : SVGKnob {
	MyMassiveWhiteKnob() {
		box.size = Vec(20, 20);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RoundHugeWhiteKnob.svg")));
		
	}
};