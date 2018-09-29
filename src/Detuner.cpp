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
	Detuner() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    float randomizza(){
        float r = randomUniform();
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




	void step() override;

    json_t *toJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "stepMode", json_integer((int) stepMode));
    json_object_set_new(rootJ, "range", json_integer((int) range));
    return rootJ;
	}
	void fromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "stepMode");
		if (sumJ)
			stepMode = json_integer_value(sumJ);

		json_t *extJ = json_object_get(rootJ, "range");
		if (extJ)
			range = json_integer_value(extJ);
	}
    void reset() override {
		range = 0;
		stepMode = false;
	}
};

void Detuner::step()
{
    float amount = clamp(params[DETUNE_PARAM].value + (inputs[DETUNE_CV].value/10.f), 0.f, 1.f);
    float currentPitch = inputs[INPUT_1].value;
    if (stepMode == true){
        if (currentPitch != oldPitch){
            r1 = randomizza();
            r2 = randomizza();
            r3 = randomizza();
            r4 = randomizza();
            oldPitch = currentPitch;
        }
    }
    else{
        r1 = randomizza();
        r2 = randomizza();
        r3 = randomizza();
        r4 = randomizza();
        oldPitch = currentPitch;
    }
	outputs[OUTPUT_1].value  = currentPitch + (r1 * pow(amount,3));
	outputs[OUTPUT_2].value  = currentPitch + (r2 * pow(amount,3));
	outputs[OUTPUT_3].value  = currentPitch + (r3 * pow(amount,3));
	outputs[OUTPUT_4].value  = currentPitch + (r4 * pow(amount,3));
}

struct DetunerWidget : ModuleWidget
{
	DetunerWidget(Detuner *module) : ModuleWidget(module)
	{
		setPanel(SVG::load(assetPlugin(plugin, "res/Detuner.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(Port::create<aPJackRosso>(Vec(33, 52.6), Port::INPUT, module, Detuner::INPUT_1));

        addParam(ParamWidget::create<aPKnob>(Vec(27.4, 103.9), module, Detuner::DETUNE_PARAM, 0.0, 1.0, 0.0));
        addInput(Port::create<aPJackArancione>(Vec(33, 146.5), Port::INPUT, module, Detuner::DETUNE_CV));

		addOutput(Port::create<aPJackRosa>(Vec(33, 194), Port::OUTPUT, module, Detuner::OUTPUT_1));
		addOutput(Port::create<aPJackFux>(Vec(33, 226.2), Port::OUTPUT, module, Detuner::OUTPUT_2));        
		addOutput(Port::create<aPJackViola>(Vec(33, 258.3), Port::OUTPUT, module, Detuner::OUTPUT_3));
		addOutput(Port::create<aPJackBlu>(Vec(33, 290.5), Port::OUTPUT, module, Detuner::OUTPUT_4));
        
	}

    Menu *createContextMenu() override;
};



struct SteppedModeMenuItem : MenuItem {
	Detuner *deTuner;
	void onAction(EventAction &e) override {
		deTuner->stepMode = !deTuner->stepMode;
	}
	void step() override {
		rightText = (deTuner->stepMode) ? "✔" : "";
	}
};

struct Range0MenuItem : MenuItem {
	Detuner *deTuner;
	void onAction(EventAction &e) override {
		deTuner->range = 0;
	}
	void step() override {
        if (deTuner->range == 0){
		rightText = "✔";
        }
        else {
        rightText = ""; 
        }
	}
};

struct Range1MenuItem : MenuItem {
	Detuner *deTuner;
	void onAction(EventAction &e) override {
		deTuner->range = 1;
	}
	void step() override {
        if (deTuner->range == 1){
		rightText = "✔";
        }
        else {
        rightText = ""; 
        }
	}
};

struct Range2MenuItem : MenuItem {
	Detuner *deTuner;
	void onAction(EventAction &e) override {
		deTuner->range = 2;
	}
	void step() override {
        if (deTuner->range == 2){
		rightText = "✔";
        }
        else {
        rightText = ""; 
        }
	}
};

Menu *DetunerWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	Detuner *deTuner = dynamic_cast<Detuner*>(module);
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


	return menu;
}

Model *modelDetuner = Model::create<Detuner, DetunerWidget>("aP-Modules", "4x Random Detuner", "Detuner - 4x Random Detuner", UTILITY_TAG);
