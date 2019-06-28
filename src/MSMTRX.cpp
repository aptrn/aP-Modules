#include "aP.hpp"

//Mid/Side Matrix, simple LR to MS encoder and MS to LR decoder

struct MSMTRX : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
		LEFT_INPUT,
		RIGHT_INPUT,
		MID_INPUT,
		SIDE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		MID_OUTPUT,
		SIDE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	MSMTRX() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);}
	void process(const ProcessArgs &args) override;
};

void MSMTRX::process(const ProcessArgs &args)
{

	//ENCODER
	float left_in  = inputs[LEFT_INPUT].getVoltage();
	float right_in = inputs[RIGHT_INPUT].getVoltage();
	float mid_out  = (left_in + right_in)/sqrt(2);
	float side_out = (left_in - right_in)/sqrt(2);
	outputs[MID_OUTPUT].value   = mid_out;
	outputs[SIDE_OUTPUT].value  = side_out;



	//DECODER
	float mid_in   = inputs[MID_INPUT].getVoltage();
	float side_in  = inputs[SIDE_INPUT].getVoltage();
	float left_out  = (mid_in + side_in)/sqrt(2);
	float right_out = (mid_in - side_in)/sqrt(2);
	outputs[LEFT_OUTPUT].value  = left_out;
	outputs[RIGHT_OUTPUT].setVoltage(right_out);
}

struct MSMTRXWidget : ModuleWidget
{
	MSMTRXWidget(MSMTRX *module) : ModuleWidget(module)
	{
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MSMTRX.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	
		//ENCODER
		addInput(createInput<aPJackGiallo>(Vec(16.7, 113.2), module, MSMTRX::LEFT_INPUT));
		addInput(createInput<aPJackArancione>(Vec(79.7, 113.2), module, MSMTRX::RIGHT_INPUT));
	    addOutput(createOutput<aPJackTurchese>(Vec(16.7, 167.2), module, MSMTRX::MID_OUTPUT));
		addOutput(createOutput<aPJackBlu>(Vec(79.7, 167.2), module, MSMTRX::SIDE_OUTPUT));


		//DECODER
		addInput(createInput<aPJackTurchese>(Vec(16.7, 247.9), module, MSMTRX::MID_INPUT));
		addInput(createInput<aPJackBlu>(Vec(79.7, 247.9), module, MSMTRX::SIDE_INPUT));
		addOutput(createOutput<aPJackVerde>(Vec(16.7, 305), module, MSMTRX::LEFT_OUTPUT));
		addOutput(createOutput<aPJackRosso>(Vec(79.7, 305), module, MSMTRX::RIGHT_OUTPUT));
	}
};

Model *modelMSMTRX = createModel<MSMTRX, MSMTRXWidget>("MS-Matrix");
