#include "Template.hpp"
#include "dsp/digital.hpp"
#include <iostream>
#include <cstdlib>
#include <random>

struct Tempo : Module
{
	enum ParamIds
	{	
		RATE_PARAM,
		RAND_BTN,
		NUM_PARAMS
	};
	enum InputIds
	{
		MAIN_INPUT,
		RATE_CV,
		NUM_INPUTS
	};
	enum OutputIds
	{
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		MULT_LED,
		SYNC_LED,
		NUM_LIGHTS
	};

	Tempo() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


SchmittTrigger clock_input;
SchmittTrigger rnd_trig;
SchmittTrigger cv_mult_trig;
PulseGenerator main_pulse;
float pulse_time = 0;
int currentStep = 0;
int prevStep = 0;
int delta = 0;
bool sync = false;
bool isMult;
int expStep;
int target;
int timer;
int rate = 1;
int index;
int ratearr [11] = {6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6};


void Tempo::step()
{
	if (inputs[RATE_CV].active){
		float rate_cv_val = rescale(inputs[RATE_CV].value, 0.0f, 10.f, 0.0f, 1.0f);
		float rate_val = rescale(params[RATE_PARAM].value, 0.0f, 1.0f, 0.0f, 1.0f);
		index = rescale(rate_cv_val*rate_val, 0.0f, 1.0f, 0, 11);
	}
	else {
		float rate_val = params[RATE_PARAM].value;
		index = rescale(rate_val, 0.0f, 1.0f, 0, 11);
	}
	
	if (index < 5) {
		isMult = false;
	}
	else {
		isMult = true;
	}
	

	if (rnd_trig.process(params[RAND_BTN].value)){
		index = rand() % 11;
		params[RATE_PARAM].value = rescale(rate, 0, 11, 0.0f, 1.0f);


	}
	rate = ratearr[index];
	lights[MULT_LED].value = isMult ? 1.0f : 0.0f;
	
	
	currentStep++;

	if (inputs[MAIN_INPUT].active){		
		//pulseDuration[i] = GetPulsingTime(stepGap, list_fRatio[rateRatioByEncoder]);				
		if (clock_input.process(rescale(inputs[MAIN_INPUT].value, 0.2f, 1.7f, 0.0f, 1.0f))){		
			//pulse_time = clock_input.GetPulsingTime(0, 1.0f);
			if (sync == true){					//VERIFICA SYNC
				int verifica = delta + prevStep;
				if (currentStep < verifica+ 5 && currentStep > verifica - 5){
					sync = true;
					if (!isMult && rate != 1){
						timer = currentStep;
					}
					prevStep = currentStep;	
				
				}
				else{
					sync = false;
					prevStep = 0;
				}
			}
			else if (sync == false){					
				if (prevStep == 0){
					prevStep = currentStep;
				//	timer = currentStep;
					sync = false;
				}
				else {
					delta = currentStep - prevStep;
					sync = true;
					timer = currentStep;
					prevStep = currentStep;
				}
			}
		}	
	}
	else {
		sync = false;
		prevStep = 0;
	}

	if (sync == true){
		if(isMult){
			target = delta * rate;		//CLOCK DIVIDER
		}
		else {
			target = delta / rate;		//CLOCK MULTIPLIER
		}
		expStep = target + timer;
		
		//expStep = timer + delta;
		if (currentStep == expStep){
			main_pulse.trigger(0.0001f);	
			timer = currentStep;
		}
	}
	
	lights[SYNC_LED].value = sync;
	bool out1 = main_pulse.process(1.0 / engineGetSampleRate());  
  	outputs[MAIN_OUTPUT].value = (out1 ? 10.0f : 0.0f);

}


struct TempoWidget : ModuleWidget
{
	TempoWidget(Tempo *module) : ModuleWidget(module)
	{

	
		setPanel(SVG::load(assetPlugin(plugin, "res/Tempo.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<TL1105>(Vec(126, 88), module, Tempo::RAND_BTN, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(126, 110), module, Tempo::MULT_LED));
		addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(126, 310), module, Tempo::SYNC_LED));


		addParam(ParamWidget::create<Rogan1PSGreen>(Vec(55.2, 175.1), module, Tempo::RATE_PARAM, 0.0f, 1.0f, 0.5f));
		addInput(Port::create<PJ301MPort>(Vec(108.3, 52.1), Port::INPUT, module, Tempo::RATE_CV));

		addInput(Port::create<PJ301MPort>(Vec(16, 112), Port::INPUT, module, Tempo::MAIN_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(78, 316), Port::OUTPUT, module, Tempo::MAIN_OUTPUT));
	

	}
	//void step() override;

};
/*
void TempoWidget::step() {
	TempoWidget::step();
	Tempo *myModule = dynamic_cast<Tempo*>(module);
	knobToMove->setValue(myModule->params[Tempo::RATE_PARAM].value);
	knobToMove->dirty = true;
	knobToMove->step();
}*/


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelTempo = Model::create<Tempo, TempoWidget>("aP", "Tempo", "Tempo - 4x Clock Multiplier - Divider", UTILITY_TAG);
