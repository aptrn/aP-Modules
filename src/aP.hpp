#include "rack.hpp"


using namespace rack;

// Forward-declare the Plugin, defined in Template.cpp
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file
extern Model *modelMSMTRX;
extern Model *modelBuffer;
extern Model *modelTempo;
extern Model *modelhpMeter;
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
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/aPbigknob.svg")));
	}
};

struct aPKnob : SVGKnob {
	aPKnob() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/aPlittleknob.svg")));
	}
};

/////BUTTONS

struct aPLedButton : SVGSwitch, MomentarySwitch {
	aPLedButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Comp/aPledbutton.svg")));
	}
};

struct aPLittleButton : SVGSwitch, MomentarySwitch {
	aPLittleButton() {
		addFrame(SVG::load(assetPlugin(plugin,"res/Comp/aPlittlebutton.svg")));
	}
};


/////JACKS

struct aPJackArancione : SVGPort {
	aPJackArancione() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackarancione.svg")));
	}
};

struct aPJackAzzurro : SVGPort {
	aPJackAzzurro() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackazzurro.svg")));
	}
};

struct aPJackBianco : SVGPort {
	aPJackBianco() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackbianco.svg")));
	}
};

struct aPJackBlu : SVGPort {
	aPJackBlu() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackblu.svg")));
	}
};

struct aPJackFux : SVGPort {
	aPJackFux() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackfux.svg")));
	}
};

struct aPJackGiallo : SVGPort {
	aPJackGiallo() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackgiallo.svg")));
	}
};

struct aPJackNero : SVGPort {
	aPJackNero() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjacknero.svg")));
	}
};

struct aPJackRosa : SVGPort {
	aPJackRosa() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackrosa.svg")));
	}
};

struct aPJackRosso : SVGPort {
	aPJackRosso() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackrosso.svg")));
	}
};

struct aPJackTurchese : SVGPort {
	aPJackTurchese() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackturchese.svg")));
	}
};

struct aPJackVerde : SVGPort {
	aPJackVerde() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackverde.svg")));
	}
};

struct aPJackViola : SVGPort {
	aPJackViola() {
		setSVG(SVG::load(assetPlugin(plugin, "res/Comp/jack/aPjackviola.svg")));
	}
};


