#include "aP.hpp"
#include "dsp/digital.hpp"

//Record gates into clocked buffer

struct GrooveRecorder : Module
{
	enum ParamIds
	{
        REC_BUTTON,
        MANUAL_BUTTON,
        STEPS_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
        REC_CV,
		CLOCK_INPUT,
        GATE_INPUT,
        STEPS_CV,
		NUM_INPUTS
	};
	enum OutputIds
	{
		MAIN_OUTPUT,
        REC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
        REC_LED,
        MANUAL_LED,
		NUM_LIGHTS
	};
    
    SchmittTrigger clock_input;
    SchmittTrigger manual_trig;
    SchmittTrigger gate_input;
    SchmittTrigger rec_trig;
    SchmittTrigger rec_cv_trig;
    PulseGenerator main_pulse;
    PulseGenerator led_pulse;

    int currentStep = 1;
    bool buffer[32];
    bool recording = false;
    bool manual;
	void step() override;

	GrooveRecorder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    void reset() override {
		recording = false;
        for (int i = 0; i < 32; i++){
		buffer[i] = false;
        }
	}
};

void GrooveRecorder::step()
{   
    int steps = params[STEPS_PARAM].value + params[STEPS_CV].value/10;
    steps = clamp(steps, 1, 32);

    if (!inputs[REC_CV].active){
        if (rec_trig.process(params[REC_BUTTON].value)){
            recording = !recording;
            if (recording == true){
                currentStep = 1;
            }
        }
    }
    else {
        recording = inputs[REC_CV].value > 0.2;
    }

	if(inputs[CLOCK_INPUT].active){
        if (clock_input.process(rescale(inputs[CLOCK_INPUT].value, 0.2f, 1.7f, 0.0f, 1.0f))){
            currentStep++;
            if (currentStep >= steps){
                currentStep = 1;
            }
            if (inputs[GATE_INPUT].active){
                if (recording == true){
                    if (gate_input.process(rescale(inputs[GATE_INPUT].value, 0.2f, 1.7f, 0.0f, 1.0f))){
                       buffer[currentStep] = true;
                    }
                    else if (inputs[GATE_INPUT].value > 0){
                       buffer[currentStep] = true;
                    }
                    else {
                       buffer[currentStep] = false;
                    }
                }
            }
            if (recording == true){
                if (params[MANUAL_BUTTON].value > 0){
                    buffer[currentStep] = true;
                }
            }
            if (buffer[currentStep]){
                main_pulse.trigger(0.0001);
                led_pulse.trigger(0.05);
            }
        }
    }

    lights[REC_LED].value = recording ? 1.0f : 0.0f;
    outputs[REC_OUTPUT].value = recording ? 10.0f : 0.0f;



    float out = main_pulse.process(1.0 / engineGetSampleRate()); 
    outputs[MAIN_OUTPUT].value = (out ? 10.0f : 0.0f);
    lights[MANUAL_LED].value = led_pulse.process(1.0 / engineGetSampleRate())  ? 1.0f : 0.0f;
}

struct GrooveRecorderWidget : ModuleWidget
{
	GrooveRecorderWidget(GrooveRecorder *module) : ModuleWidget(module)
	{
		setPanel(SVG::load(assetPlugin(plugin, "res/GrooveRecorder.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(ParamWidget::create<aPLedButton>(Vec(55.7, 183), module, GrooveRecorder::MANUAL_BUTTON, 0.0, 1.0, 0.0));
        addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(57.4, 185.2), module, GrooveRecorder::MANUAL_LED));

		addInput(Port::create<aPJackRosso>(Vec(33, 52.6), Port::INPUT, module, GrooveRecorder::CLOCK_INPUT));
        addParam(ParamWidget::create<aPLedButton>(Vec(34.4, 100), module, GrooveRecorder::REC_BUTTON, 0.0, 1.0, 0.0));  
		addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(37.1, 102.2), module, GrooveRecorder::REC_LED));
		addInput(Port::create<aPJackGiallo>(Vec(12.5, 131), Port::INPUT, module, GrooveRecorder::REC_CV));
 		addOutput(Port::create<aPJackArancione>(Vec(54.5, 131.5), Port::OUTPUT, module, GrooveRecorder::REC_OUTPUT));
 		addInput(Port::create<aPJackVerde>(Vec(12.8, 182), Port::INPUT, module, GrooveRecorder::GATE_INPUT));
		addParam(ParamWidget::create<aPKnob>(Vec(9.3, 229.5), module, GrooveRecorder::STEPS_PARAM, 0.0, 32.0, 32.0));
 		addInput(Port::create<aPJackViola>(Vec(54.5, 237.7), Port::INPUT, module, GrooveRecorder::STEPS_CV));       
        addOutput(Port::create<aPJackBlu>(Vec(33.5, 297.5), Port::OUTPUT, module, GrooveRecorder::MAIN_OUTPUT));
	}
};

Model *modelGrooveRecorder = Model::create<GrooveRecorder, GrooveRecorderWidget>("aP", "Groove Recorder", "Groove Recorder - Clocked Gates Recorder", SAMPLER_TAG, UTILITY_TAG);
