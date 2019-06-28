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

	VoltageRecorder() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);}

    dsp::SchmittTrigger clock_input;
    dsp::SchmittTrigger rec_trig;
    dsp::SchmittTrigger rec_cv_trig;
    dsp::PulseGenerator main_pulse;
    int currentStep = 1;
    float buffer[32];
    bool recording = false;
    float out;
	void process(const ProcessArgs &args) override;

    void reset() {
		recording = false;
        for (int i = 0; i < 32; i++){
		buffer[i] = false;
        }
	}
};

void VoltageRecorder::process(const ProcessArgs &args)
{
    int steps = params[STEPS_PARAM].getValue() + params[STEPS_CV].getValue()/10;
    steps = clamp(steps, 1, 32);

    if (!inputs[REC_CV].isConnected()){
        if (rec_trig.process(params[REC_BUTTON].getValue())){
            recording = !recording;
        }
    }
    else {
        recording = inputs[REC_CV].getVoltage() > 0.2;
    }

	if(inputs[CLOCK_INPUT].isConnected()){
        if (clock_input.process(rescale(inputs[CLOCK_INPUT].getVoltage(), 0.2f, 1.7f, 0.0f, 1.0f))){
            currentStep++;
            if (currentStep >= steps){
            currentStep = 1;
            }
            if (inputs[CV_INPUT].isConnected()){
                if (recording){
                    buffer[currentStep] = inputs[CV_INPUT].getVoltage();
                }
            }
            if (outputs[MAIN_OUTPUT].isConnected()){
                if (buffer[currentStep]){
                    out = buffer[currentStep];
                }
            }
        }
    }

    lights[REC_LED].value = recording ? 1.0f : 0.0f;
    outputs[REC_OUTPUT].setVoltage(recording ? 10.0f : 0.0f);
    outputs[MAIN_OUTPUT].setVoltage(out);
}

struct VoltageRecorderWidget : ModuleWidget
{
	VoltageRecorderWidget(VoltageRecorder *module) : ModuleWidget(module)
	{
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/VoltageRecorder.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		
		addInput(createInput<aPJackRosso>(Vec(33, 53.1), module, VoltageRecorder::CLOCK_INPUT));
        addParam(createParam<aPLedButton>(Vec(34.4, 100), module, VoltageRecorder::REC_BUTTON));
		addChild(createLight<LedLight<RedLight>>(Vec(37.1, 102.2), module, VoltageRecorder::REC_LED));
		addInput(createInput<aPJackGiallo>(Vec(12.5, 131), module, VoltageRecorder::REC_CV));
 		addOutput(createOutput<aPJackArancione>(Vec(54.5, 131), module, VoltageRecorder::REC_OUTPUT));
 		addInput(createInput<aPJackVerde>(Vec(33, 182), module, VoltageRecorder::CV_INPUT));
		addParam(createParam<aPKnob>(Vec(9.3, 229.5), module, VoltageRecorder::STEPS_PARAM));
 		addInput(createInput<aPJackViola>(Vec(54.5, 237.7), module, VoltageRecorder::STEPS_CV));       
        addOutput(createOutput<aPJackBlu>(Vec(33, 297.5), module, VoltageRecorder::MAIN_OUTPUT));
	}
};

Model *modelVoltageRecorder = createModel<VoltageRecorder, VoltageRecorderWidget>("Voltage-Recorder");
