#include "aP.hpp"
#include "dsp/digital.hpp"


//1V/Oct input to four detuned output with amount of randomness

struct Detuner : Module
{
	enum ParamIds
	{   
        DETUNE_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
        DETUNE_CV,
		INPUT_1,
		NUM_INPUTS
	};
	enum OutputIds
	{
        OUTPUT_1,
        OUTPUT_2,
        OUTPUT_3,
        OUTPUT_4,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

    int range = 0;
    float oldPitch;
    float r1, r2, r3, r4;
    bool stepMode = 0;
	Detuner() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);}


    float randomizza(){
        float r = random::uniform();
        if (range == 0){
            r = rescale(r, 0.0f, 1.0f, -5.f, 5.f);
        }
        else if (range == 1){
            r = rescale(r, 0.0f, 1.0f, 0.f, 10.f);
        }
        else if (range == 2){
            r = rescale(r, 0.0f, 1.0f, 0.f, -10.f);
        }
        return r;
    }




	void process(const ProcessArgs &args) override;

    json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "stepMode", json_integer((int) stepMode));
    json_object_set_new(rootJ, "range", json_integer((int) range));
    return rootJ;
	}
	void dataFromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "stepMode");
		if (sumJ)
			stepMode = json_integer_value(sumJ);

		json_t *extJ = json_object_get(rootJ, "range");
		if (extJ)
			range = json_integer_value(extJ);
	}
    void reset() {
		range = 0;
		stepMode = false;
	}
};

void Detuner::process(const ProcessArgs &args) {
	float amount = clamp(params[DETUNE_PARAM].getValue() + (inputs[DETUNE_CV].getVoltage(0)/10.f), 0.f, 1.f);
	float currentPitch = inputs[INPUT_1].getVoltage(0);
	for (int c = 0; c < 16; c++) {
		if (stepMode == true){
			if (currentPitch != oldPitch){
				r1 = randomizza();
				oldPitch = currentPitch;
			}
		}
		else {
			r1 = randomizza();
			oldPitch = currentPitch;
		}
		outputs[OUTPUT_1].setVoltage(currentPitch + (r1 * pow(amount,3), c));
	}
	outputs[OUTPUT_1].setChannels(16);
}

struct DetunerWidget : ModuleWidget
{
	DetunerWidget(Detuner *module) : ModuleWidget(module)
	{
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Detuner.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInput<aPJackRosso>(Vec(33, 52.6), module, Detuner::INPUT_1));

        addParam(createParam<aPKnob>(Vec(27.4, 103.9), module, Detuner::DETUNE_PARAM));
        addInput(createInput<aPJackArancione>(Vec(33, 146.5), module, Detuner::DETUNE_CV));

		addOutput(createOutput<aPJackRosa>(Vec(33, 194), module, Detuner::OUTPUT_1));
		addOutput(createOutput<aPJackFux>(Vec(33, 226.2), module, Detuner::OUTPUT_2));        
		addOutput(createOutput<aPJackViola>(Vec(33, 258.3), module, Detuner::OUTPUT_3));
		addOutput(createOutput<aPJackBlu>(Vec(33, 290.5), module, Detuner::OUTPUT_4));
        
	}
	~DetunerWidget(){ 
	}
	void appendContextMenu(Menu *menu) override;
};

	struct Range1MenuItem : MenuItem {
		Detuner *deTuner;
		void onAction(const event::Action &e) override {
			deTuner->range = 1;
		}

	};

	struct Range2MenuItem : MenuItem {
		Detuner *deTuner;
		void onAction(const event::Action &e) override {
			deTuner->range = 2;
		}

	};

	struct SteppedModeMenuItem : MenuItem {
		Detuner *deTuner;

		void onAction(const event::Action &e) override {
			deTuner->stepMode = !deTuner->stepMode;
			if(module != NULL){
				deTuner->stepMode = !deTuner->stepMode;
			}
		}
		void step() override {
		if(module != NULL){
			rightText = (deTuner->stepMode) ? "✔" : "";
		}
		MenuItem::step();
		}
	};

	struct Range0MenuItem : MenuItem {
		Detuner *deTuner;
		void onAction(const event::Action &e) override {
			deTuner->range = 0;
		}

	};

	void DetunerWidget::appendContextMenu(Menu *menu) {

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);
	
	Detuner *deTuner = dynamic_cast<Detuner*>(this->module)
	assert(deTuner);


		SteppedModeMenuItem *steppedMenuItem = new SteppedModeMenuItem();
		steppedMenuItem->text = "Stepped Mode";
		steppedMenuItem->deTuner = deTuner;
		menu->addChild(steppedMenuItem);

		MenuLabel *spacerLabel2 = new MenuLabel();
		menu->addChild(spacerLabel2);

		Range0MenuItem *range0MenuItem = new Range0MenuItem();
		range0MenuItem->text = "Range    ->  -5v / 5v";
		range0MenuItem->deTuner = deTuner;
		menu->addChild(range0MenuItem);
		
		Range1MenuItem *range1MenuItem = new Range1MenuItem();
		range1MenuItem->text = "Range    ->  0v / 10v";
		range1MenuItem->deTuner = deTuner;
		menu->addChild(range1MenuItem);

		Range2MenuItem *range2MenuItem = new Range2MenuItem();
		range2MenuItem->text = "Range    ->  -10v / 0v";
		range2MenuItem->deTuner = deTuner;
		menu->addChild(range2MenuItem);

	}






	

Model *modelDetuner = createModel<Detuner, DetunerWidget>("Detuner");
