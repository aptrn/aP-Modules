/*


#include "aP.hpp"
#include "dsp/digital.hpp"

struct GrooveRecorder : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
		CLOCK_INPUT,
        GATE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		MAIN_OUTPUT,
        EOC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
        SYNC_LED,
		NUM_LIGHTS
	};

	GrooveRecorder() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);}

    dsp::SchmittTrigger clock_input;
    dsp::PulseGenerator main_pulse;
    int currentStep = 0;
    int prevStep = 0;
    bool sync;
    int delta;
    
	void process(const ProcessArgs &args) override;
};

void GrooveRecorder::process(const ProcessArgs &args)
{
currentStep++;
	if(inputs[CLOCK_INPUT].isConnected()){
        if (clock_input.process(rescale(inputs[CLOCK_INPUT].getVoltage(), 0.2f, 1.7f, 0.0f, 1.0f))){
            if (sync){
                if (delta - currentStep != prevStep){
                    sync = false;
                }
                else {
                    prevStep = currentStep;
                    main_pulse.trigger(0.0001f);
                }
            }
            else{
                if (prevStep < 1){
                    prevStep = currentStep; 
                }
                else{
                    sync = true;
                    delta = currentStep - prevStep;
                }
            }
        }
    }
    lights[SYNC_LED].value = sync ? 1.0f : 0.0f;
	outputs[MAIN_OUTPUT].setVoltage(main_pulse.process(1.0 / args.sampleRate));
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

		addChild(createLight<LedLight<RedLight>>(Vec(20.9, 280.9), module, GrooveRecorder::SYNC_LED));
		addInput(createInput<PJ301MPort>(Vec(17, 114), module, GrooveRecorder::CLOCK_INPUT));
		addOutput(createOutput<PJ301MPort>(Vec(80, 310), module, GrooveRecorder::MAIN_OUTPUT));
	}
};

Model *modelGrooveRecorder = createModel<GrooveRecorder, GrooveRecorderWidget>("aP", "GrooveRecorder", "Groove Recorder", UTILITY_TAG);


*/