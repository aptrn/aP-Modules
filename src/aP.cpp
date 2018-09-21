#include "aP.hpp"


Plugin *plugin;


void init(Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	// Add all Models defined throughout the plugin
	p->addModel(modelMSMTRX);
    p->addModel(modelBuffer);
	p->addModel(modelTempo);
	p->addModel(modelhpMeter);
	p->addModel(modelDetuner);
	p->addModel(modelGrooveRecorder);
	p->addModel(modelVoltageRecorder);
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
