#include "aP.hpp"
#include "dsp/digital.hpp"



struct Buffer : Module
{
	enum ParamIds
	{
		DRYWET_PARAM,
		FREEZE_SWITCH,
		CLEAR_BTN,
        START_PARAM,
		WIDTH_PARAM,
		RATE_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		LEFT_INPUT,
		RIGHT_INPUT,
		FREEZE_CV,
		DRYWET_CV,
		START_CV,
		WIDTH_CV,
		RATE_CV,
		RESET_CV,
		NUM_INPUTS
	};
	enum OutputIds
	{
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		EOC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		FREEZE_LED,
		NUM_LIGHTS
	};
	


	SchmittTrigger freeze_trig;
	SchmittTrigger clear_trig;
	SchmittTrigger cv_freeze_trig;
	PulseGenerator eoc;

	int buffersize = 176400;
	bool freeze = 0;
	float bufferinoL[176400] = {0.0};
	float bufferinoR[176400] = {0.0};
	float PH = 0.0;
	float RH = 0.0;

	
	Buffer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};




float map(float input, float old_min, float old_max, float new_min, float new_max){
float scalato = (((input - old_min)* (new_max - new_min)) / (old_max - old_min)) + new_min;
return scalato;
}

void Buffer::step()
{
	float drywet = params[DRYWET_PARAM].value + (inputs[DRYWET_CV].value/ 10);
	float dry_l  = inputs[LEFT_INPUT].value;
	float dry_r = inputs[RIGHT_INPUT].value;

	int start_cv = map(inputs[START_CV].value, 0.0f, 10.0f, 0, buffersize-2);
	int width_cv = map(inputs[WIDTH_CV].value, 0.0f, 10.0f, 0, buffersize-2);

	int start_knob = map(params[START_PARAM].value, 0.0f, 1.0f, 0, buffersize-2);
	int width_knob = map(params[WIDTH_PARAM].value, 0.0f, 1.0f, 0, buffersize-2);
	
	float rate = params[RATE_PARAM].value + inputs[RATE_CV].value;

	int start = start_knob + start_cv;
	int width = width_knob + width_cv;

	int end = (start + width) % (buffersize-2);

	if (clear_trig.process(params[CLEAR_BTN].value)){
		for (int i = 0; i < buffersize; i++){
		bufferinoL[i] = 0.0;
		bufferinoR[i] = 0.0;
		}
	}

	if (freeze_trig.process(params[FREEZE_SWITCH].value) || cv_freeze_trig.process(inputs[FREEZE_CV].value)){
		  if (freeze == 0){
			  freeze = 1;
		  }
		  else if (freeze == 1){
			  freeze = 0;
		  }	
	}
	lights[FREEZE_LED].value = freeze ? 1.0f : 0.0f;
	
	RH++;
	PH = (PH + rate);

	if (rate > 0){
		if (start < end){
			if (PH > end){
				PH = start;
				eoc.trigger(0.0001f);
			}
		}
		else if (start > end){
			if (PH > end && PH < start){
				PH = start;
				eoc.trigger(0.0001f);
			}
			else if (PH > start && PH > buffersize-2){
				PH = 0;
			}
		}
	}
	if (rate < 0){
		if (start < end){
			if (PH < start){
				PH = end;
				eoc.trigger(0.0001f);
			}
		}
		else if (start > end){
			if (PH < start && PH > start){
				PH = end;
				eoc.trigger(0.0001f);
			}
			else if (PH < end && PH < 0){
				PH = buffersize-2;
			}
		}
	}
	
	int PI = (static_cast<int>(PH)) % (buffersize-2);
	int RI = (static_cast<int>(RH)) % (buffersize-2);

	//PI = PI % (buffersize -1);
	//RI = RI % (buffersize -1);



//RECORDING
	if (freeze == 0){
		bufferinoL[RI] = dry_l;
		bufferinoR[RI] = dry_r;
	}

//PLAYING

	float A = bufferinoR[PI];
	float B = bufferinoR[PI+1];
	float wet_l = A * (1 -(PH - PI)) + B * (PH - PI);
	A = bufferinoR[PI];
	B = bufferinoR[PI+1];
	float wet_r = A * (1 -(PH - PI)) + B * (PH - PI);

//OUTPUT
	float eoc_pulse = eoc.process(1.0 / engineGetSampleRate());  
  	outputs[EOC_OUTPUT].value = (eoc_pulse ? 10.0f : 0.0f);
	outputs[LEFT_OUTPUT].value  = (dry_l * map(drywet, 0.0, 1.0, 1.0, 0.0)) + (wet_l * drywet);
	outputs[RIGHT_OUTPUT].value = (dry_r * map(drywet, 0.0, 1.0, 1.0, 0.0)) + (wet_r * drywet);
}

struct BufferWidget : ModuleWidget
{
	BufferWidget(Buffer *module) : ModuleWidget(module)
	{
		setPanel(SVG::load(assetPlugin(plugin, "res/Buffer.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

 

		addParam(ParamWidget::create<LEDBezel>(Vec(64.4, 53), module, Buffer::FREEZE_SWITCH, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(67.1, 55.2), module, Buffer::FREEZE_LED));
		addParam(ParamWidget::create<TL1105>(Vec(126, 88), module, Buffer::CLEAR_BTN, 0.0, 1.0, 0.0));

		addParam(ParamWidget::create<Rogan6PSWhite>(Vec(30.4, 82.9), module, Buffer::RATE_PARAM, -4.0, 4.0, 1.0));
		addParam(ParamWidget::create<Rogan1PSGreen>(Vec(10.6, 175.1), module, Buffer::START_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<Rogan1PSRed>(Vec(101.7, 175.1), module, Buffer::WIDTH_PARAM, 0.001f, 0.999999f, 0.999999f));
		addParam(ParamWidget::create<Rogan1PSBlue>(Vec(55.2, 175.1), module, Buffer::DRYWET_PARAM, 0.0f, 1.0f, 1.0f));



		addInput(Port::create<PJ301MPort>(Vec(108.3, 52.1), Port::INPUT, module, Buffer::FREEZE_CV));
		addInput(Port::create<PJ301MPort>(Vec(17.9, 52.1), Port::INPUT, module, Buffer::RATE_CV));



		addInput(Port::create<PJ301MPort>(Vec(19, 229), Port::INPUT, module, Buffer::START_CV));
		addInput(Port::create<PJ301MPort>(Vec(63.5, 229), Port::INPUT, module, Buffer::DRYWET_CV));
		addInput(Port::create<PJ301MPort>(Vec(108.1, 229), Port::INPUT, module, Buffer::WIDTH_CV));
		
		addInput(Port::create<PJ301MPort>(Vec(63.5, 309.6), Port::INPUT, module, Buffer::RESET_CV));
		addOutput(Port::create<PJ301MPort>(Vec(63.5, 279), Port::OUTPUT, module, Buffer::EOC_OUTPUT));


		addInput(Port::create<PJ301MPort>(Vec(19, 279), Port::INPUT, module, Buffer::LEFT_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(19, 309.6), Port::INPUT, module, Buffer::RIGHT_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(107.8, 279), Port::OUTPUT, module, Buffer::LEFT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(107.8, 309.6), Port::OUTPUT, module, Buffer::RIGHT_OUTPUT));
	

	}
};

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelBuffer = Model::create<Buffer, BufferWidget>("aP", "BUFFER", "Buffer Sampler", SAMPLER_TAG);






