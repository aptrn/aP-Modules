#include "aP.hpp"
#include "dsp/digital.hpp"

//Record signal into Buffer with freeze option

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
	


	dsp::SchmittTrigger freeze_trig;
	dsp::SchmittTrigger clear_trig;
	dsp::SchmittTrigger cv_freeze_trig;
	dsp::PulseGenerator eoc;

	int buffersize = 176400;
	bool freeze = 0;
	float bufferinoL[176400] = {0.0};
	float bufferinoR[176400] = {0.0};
	float PH = 0.0;
	float RH = 0.0;
	bool fadesOn = false;
	float fadeAmount = 1.0f;
	int fade = 0;
	
	Buffer() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);}
	void process(const ProcessArgs &args) override;

	json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "Fades", json_integer((bool) fadesOn));
    json_object_set_new(rootJ, "Freeze", json_integer((bool) freeze));
    return rootJ;
	}
	void dataFromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "Fades");
		if (sumJ)
			fadesOn = json_integer_value(sumJ);

		json_t *extJ = json_object_get(rootJ, "Freeze");
		if (extJ)
			freeze = json_integer_value(extJ);
	}
    void reset() {
		freeze = false;
		fadesOn = false;
	}
};

float map(float input, float old_min, float old_max, float new_min, float new_max){
float scalato = (((input - old_min)* (new_max - new_min)) / (old_max - old_min)) + new_min;
return scalato;
}

void Buffer::process(const ProcessArgs &args)
{
	float drywet = params[DRYWET_PARAM].getValue() + (inputs[DRYWET_CV].getVoltage()/ 10);
	float dry_l  = inputs[LEFT_INPUT].getVoltage();
	float dry_r = inputs[RIGHT_INPUT].getVoltage();

	int start_cv = map(abs(inputs[START_CV].getVoltage()), 0.0f, 10.0f, 0, buffersize-2);
	int width_cv = map(abs(inputs[WIDTH_CV].getVoltage()), 0.0f, 10.0f, 0, buffersize-2);

	int start_knob = map(params[START_PARAM].getValue(), 0.0f, 1.0f, 0, buffersize-2);
	int width_knob = map(params[WIDTH_PARAM].getValue(), 0.0f, 1.0f, 0, buffersize-2);
	
	float rate = params[RATE_PARAM].getValue() + inputs[RATE_CV].getVoltage();
	int start = start_knob + start_cv;
	int width = width_knob + width_cv;
	int end = (start + width) % (buffersize-2);

	if (clear_trig.process(params[CLEAR_BTN].getValue())){
		for (int i = 0; i < buffersize; i++){
		bufferinoL[i] = 0.0;
		bufferinoR[i] = 0.0;
		}
	}

	if (freeze_trig.process(params[FREEZE_SWITCH].getValue()) || cv_freeze_trig.process(inputs[FREEZE_CV].getVoltage())){
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
				fade = 1;
			}
		}
		else if (start > end){
			if (PH > end && PH < start){
				PH = start;
				eoc.trigger(0.0001f);
				fade = 1;
			}
			else if (PH > start && PH > buffersize-2){
				PH = 0;
				fade = 1;
			}
		}
	}
	if (rate < 0){
		if (start < end){
			if (PH < start){
				PH = end;
				eoc.trigger(0.0001f);
				fade = 1;
			}
		}
		else if (start > end){
			if (PH < start && PH > start){
				PH = end;
				eoc.trigger(0.0001f);
				fade = 1;
			}
			else if (PH < end && PH < 0){
				PH = buffersize-2;
				fade = 1;
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

	float A = bufferinoL[PI];
	float B = bufferinoL[PI+1];
	float wet_l = A * (1 -(PH - PI)) + B * (PH - PI);
	A = bufferinoR[PI];
	B = bufferinoR[PI+1];
	float wet_r = A * (1 -(PH - PI)) + B * (PH - PI);


//FADES

	if (fadesOn == true){
		if (fade > 0){
			fade++;
			if (fade < 44*3){
				fadeAmount = rescale(fade, 0, 44*3, 0.000001f, 1.0f);
			}
			else {
				fade = 0;
				fadeAmount = 1.0f;
			}
		}
		wet_l = wet_l* fadeAmount;
		wet_r = wet_r* fadeAmount;
	}


//OUTPUT
	float eoc_pulse = eoc.process(1.0 / args.sampleRate);  
  	outputs[EOC_OUTPUT].setVoltage((eoc_pulse ? 10.0f : 0.0f));
	outputs[LEFT_OUTPUT].value  = (dry_l * map(drywet, 0.0, 1.0, 1.0, 0.0)) + (wet_l  *drywet);
	outputs[RIGHT_OUTPUT].setVoltage((dry_r * map(drywet, 0.0, 1.0, 1.0, 0.0)) + (wet_r  *drywet));
}

struct BufferWidget : ModuleWidget
{
	BufferWidget(Buffer *module) : ModuleWidget(module)
	{
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buffer.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

 

		addParam(createParam<aPLedButton>(Vec(64.8, 53.5), module, Buffer::FREEZE_SWITCH));
		addChild(createLight<LedLight<RedLight>>(Vec(67.5, 55.7), module, Buffer::FREEZE_LED));
		addParam(createParam<aPLittleButton>(Vec(126.8, 88.8), module, Buffer::CLEAR_BTN));

		addParam(createParam<aPBigKnob>(Vec(32.9, 82), module, Buffer::RATE_PARAM));
		addParam(createParam<aPKnob>(Vec(14, 173), module, Buffer::START_PARAM));
		addParam(createParam<aPKnob>(Vec(102, 172), module, Buffer::WIDTH_PARAM));
		addParam(createParam<aPKnob>(Vec(59.7, 173), module, Buffer::DRYWET_PARAM));



		addInput(createInput<aPJackRosso>(Vec(109.7, 52.3), module, Buffer::FREEZE_CV));
		addInput(createInput<aPJackNero>(Vec(17.9, 52.1), module, Buffer::RATE_CV));



		addInput(createInput<aPJackVerde>(Vec(19.2, 228.5), module, Buffer::START_CV));
		addInput(createInput<aPJackTurchese>(Vec(63.7, 228.5), module, Buffer::DRYWET_CV));
		addInput(createInput<aPJackRosa>(Vec(107.7, 228.8), module, Buffer::WIDTH_CV));
	
		addOutput(createOutput<aPJackAzzurro>(Vec(63.2, 279.9), module, Buffer::EOC_OUTPUT));		
		addInput(createInput<aPJackBlu>(Vec(63.2, 308.5), module, Buffer::RESET_CV));


		addInput(createInput<aPJackGiallo>(Vec(19.6, 279.9), module, Buffer::LEFT_INPUT));
		addInput(createInput<aPJackArancione>(Vec(19.6, 308.6), module, Buffer::RIGHT_INPUT));
		addOutput(createOutput<aPJackFux>(Vec(107.7, 279.9), module, Buffer::LEFT_OUTPUT));
		addOutput(createOutput<aPJackViola>(Vec(107.7, 308.6), module, Buffer::RIGHT_OUTPUT));
	
	}

	void appendContextMenu(Menu *menu) override {
		Buffer *buFfer = dynamic_cast<Buffer*>(this->module);

		//Menu *menu = ModulecreateWidgetContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);
		assert(buFfer);
			struct FadesMenuItem : MenuItem {
				Buffer *buFfer;
				void onAction(const event::Action &e) override {
					if (buFfer->fadesOn == true){
						buFfer->fadesOn = false;
					}
					else if (buFfer->fadesOn == false){
						buFfer->fadesOn = true;
					}
				}
			};
		FadesMenuItem *fadesMenuItem = new FadesMenuItem();
		fadesMenuItem->text = "Fades";
		fadesMenuItem->buFfer = buFfer;
		menu->addChild(fadesMenuItem);
	}

};






Model *modelBuffer = createModel<Buffer, BufferWidget>("Buffer-Sampler");






