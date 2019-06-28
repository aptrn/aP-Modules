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
    
    dsp::SchmittTrigger clock_input;
    dsp::SchmittTrigger manual_trig;
    dsp::SchmittTrigger gate_input;
    dsp::SchmittTrigger rec_trig;
    dsp::SchmittTrigger rec_cv_trig;
    dsp::PulseGenerator main_pulse;
    dsp::PulseGenerator led_pulse;

    int currentStep = 1;
    bool buffer[32];
    bool recording = false;
    bool manual;
	void process(const ProcessArgs &args) override;

	GrooveRecorder() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);}


    void reset() {
		recording = false;
        for (int i = 0; i < 32; i++){
		buffer[i] = false;
        }
	}
};

void GrooveRecorder::process(const ProcessArgs &args)
{   
    int steps = params[STEPS_PARAM].getValue() + params[STEPS_CV].getValue()/10;
    steps = clamp(steps, 1, 32);

    if (!inputs[REC_CV].isConnected()){
        if (rec_trig.process(params[REC_BUTTON].getValue())){
            recording = !recording;
            if (recording == true){
                currentStep = 1;
            }
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
            if (inputs[GATE_INPUT].isConnected()){
                if (recording == true){
                    if (gate_input.process(rescale(inputs[GATE_INPUT].getVoltage(), 0.2f, 1.7f, 0.0f, 1.0f))){
                       buffer[currentStep] = true;
                    }
                    else if (inputs[GATE_INPUT].getVoltage() > 0){
                       buffer[currentStep] = true;
                    }
                    else {
                       buffer[currentStep] = false;
                    }
                }
            }
            if (recording == true){
                if (params[MANUAL_BUTTON].getValue() > 0){
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
    outputs[REC_OUTPUT].setVoltage(recording ? 10.0f : 0.0f);



    float out = main_pulse.process(1.0 / args.sampleRate); 
    outputs[MAIN_OUTPUT].setVoltage((out ? 10.0f : 0.0f));
    lights[MANUAL_LED].value = led_pulse.process(1.0 / args.sampleRate)  ? 1.0f : 0.0f;
}

struct GrooveRecorderWidget : ModuleWidget
{
	GrooveRecorderWidget(GrooveRecorder *module) : ModuleWidget(module)
	{
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/GrooveRecorder.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<aPLedButton>(Vec(55.7, 183), module, GrooveRecorder::MANUAL_BUTTON));
        addChild(createLight<LedLight<RedLight>>(Vec(57.4, 185.2), module, GrooveRecorder::MANUAL_LED));

		addInput(createInput<aPJackRosso>(Vec(33, 52.6), module, GrooveRecorder::CLOCK_INPUT));
        addParam(createParam<aPLedButton>(Vec(34.4, 100), module, GrooveRecorder::REC_BUTTON));  
		addChild(createLight<LedLight<RedLight>>(Vec(37.1, 102.2), module, GrooveRecorder::REC_LED));
		addInput(createInput<aPJackGiallo>(Vec(12.5, 131), module, GrooveRecorder::REC_CV));
 		addOutput(createOutput<aPJackArancione>(Vec(54.5, 131.5), module, GrooveRecorder::REC_OUTPUT));
 		addInput(createInput<aPJackVerde>(Vec(12.8, 182), module, GrooveRecorder::GATE_INPUT));
		addParam(createParam<aPKnob>(Vec(9.3, 229.5), module, GrooveRecorder::STEPS_PARAM));
 		addInput(createInput<aPJackViola>(Vec(54.5, 237.7), module, GrooveRecorder::STEPS_CV));       
        addOutput(createOutput<aPJackBlu>(Vec(33.5, 297.5), module, GrooveRecorder::MAIN_OUTPUT));
	}
};

Model *modelGrooveRecorder = createModel<GrooveRecorder, GrooveRecorderWidget>("Groove-Recorder");
