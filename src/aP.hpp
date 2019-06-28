#include "rack.hpp"


using namespace rack;

// Forward-declare the Plugin, defined in Template.cpp
extern Plugin *pluginInstance;

// Forward-declare each Model, defined in each module source file
extern Model *modelMSMTRX;
extern Model *modelBuffer;
extern Model *modelTempo;
extern Model *modelDetuner;
extern Model *modelGrooveRecorder;
extern Model *modelVoltageRecorder;

template <typename BASE>
struct LedLight : BASE {
	LedLight() {
	  //this->box.size = Vec(20.0, 20.0);
	  this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

/////KNOBS

struct aPBigKnob : SVGKnob {
	aPBigKnob() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/aPbigknob.svg")));
	}
};

struct aPKnob : SVGKnob {
	aPKnob() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/aPlittleknob.svg")));
	}
};

/////BUTTONS

struct aPLedButton : app::SvgSwitch {
	aPLedButton() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/aPledbutton.svg")));
	}
};

struct aPLittleButton : SVGSwitch {
	aPLittleButton() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Comp/aPlittlebutton.svg")));
	}
};


/////JACKS

struct aPJackArancione : SVGPort {
	aPJackArancione() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackarancione.svg")));
	}
};

struct aPJackAzzurro : SVGPort {
	aPJackAzzurro() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackazzurro.svg")));
	}
};

struct aPJackBianco : SVGPort {
	aPJackBianco() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackbianco.svg")));
	}
};

struct aPJackBlu : SVGPort {
	aPJackBlu() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackblu.svg")));
	}
};

struct aPJackFux : SVGPort {
	aPJackFux() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackfux.svg")));
	}
};

struct aPJackGiallo : SVGPort {
	aPJackGiallo() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackgiallo.svg")));
	}
};

struct aPJackNero : SVGPort {
	aPJackNero() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjacknero.svg")));
	}
};

struct aPJackRosa : SVGPort {
	aPJackRosa() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackrosa.svg")));
	}
};

struct aPJackRosso : SVGPort {
	aPJackRosso() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackrosso.svg")));
	}
};

struct aPJackTurchese : SVGPort {
	aPJackTurchese() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackturchese.svg")));
	}
};

struct aPJackVerde : SVGPort {
	aPJackVerde() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackverde.svg")));
	}
};

struct aPJackViola : SVGPort {
	aPJackViola() {
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comp/jack/aPjackviola.svg")));
	}
};


