#include "aP.hpp"
#include "dsp/digital.hpp"


//Record CVs into clocked buffer

struct VoltageRecorder : Module
{
	enum ParamIds
	{
        REC_BUTTON,
        STEPS_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
        REC_CV,
		CLOCK_INPUT,
        CV_INPUT,
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
		NUM_LIGHTS
	};

	VoltageRecorder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    SchmittTrigger clock_input;
    SchmittTrigger rec_trig;
    SchmittTrigger rec_cv_trig;
    PulseGenerator main_pulse;
    int currentStep = 1;
    float buffer[32];
    bool recording = false;
    float out;
	void step() override;

    void reset() override {
		recording = false;
        for (int i = 0; i < 32; i++){
		buffer[i] = false;
        }
	}
};

void VoltageRecorder::step()
{
    int steps = params[STEPS_PARAM].value + params[STEPS_CV].value/10;
    steps = clamp(steps, 1, 32);

    if (!inputs[REC_CV].active){
        if (rec_trig.process(params[REC_BUTTON].value)){
            recording = !recording;
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
            if (inputs[CV_INPUT].active){
                if (recording){
                    buffer[currentStep] = inputs[CV_INPUT].value;
                }
            }
            if (outputs[MAIN_OUTPUT].active){
                if (buffer[currentStep]){
                    out = buffer[currentStep];
                }
            }
        }
    }

    lights[REC_LED].value = recording ? 1.0f : 0.0f;
    outputs[REC_OUTPUT].value = recording ? 10.0f : 0.0f;
    outputs[MAIN_OUTPUT].value = out;
}

struct VoltageRecorderWidget : ModuleWidget
{
	VoltageRecorderWidget(VoltageRecorder *module) : ModuleWidget(module)
	{
		setPanel(SVG::load(assetPlugin(plugin, "res/VoltageRecorder.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		
		addInput(Port::create<aPJackRosso>(Vec(33, 53.1), Port::INPUT, module, VoltageRecorder::CLOCK_INPUT));
        addParam(ParamWidget::create<aPLedButton>(Vec(34.4, 100), module, VoltageRecorder::REC_BUTTON, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(37.1, 102.2), module, VoltageRecorder::REC_LED));
		addInput(Port::create<aPJackGiallo>(Vec(12.5, 131), Port::INPUT, module, VoltageRecorder::REC_CV));
 		addOutput(Port::create<aPJackArancione>(Vec(54.5, 131), Port::OUTPUT, module, VoltageRecorder::REC_OUTPUT));
 		addInput(Port::create<aPJackVerde>(Vec(33, 182), Port::INPUT, module, VoltageRecorder::CV_INPUT));
		addParam(ParamWidget::create<aPKnob>(Vec(9.3, 229.5), module, VoltageRecorder::STEPS_PARAM, 0.0, 32.0, 32.0));
 		addInput(Port::create<aPJackViola>(Vec(54.5, 237.7), Port::INPUT, module, VoltageRecorder::STEPS_CV));       
        addOutput(Port::create<aPJackBlu>(Vec(33, 297.5), Port::OUTPUT, module, VoltageRecorder::MAIN_OUTPUT));
	}
};

Model *modelVoltageRecorder = Model::create<VoltageRecorder, VoltageRecorderWidget>("aP-Modules", "VoltageRecorder", "Voltage Recorder - Clocked CVs Recorder", SAMPLER_TAG, UTILITY_TAG);
