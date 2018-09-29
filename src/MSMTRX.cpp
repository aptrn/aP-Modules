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

	MSMTRX() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void MSMTRX::step()
{

	//ENCODER
	float left_in  = inputs[LEFT_INPUT].value;
	float right_in = inputs[RIGHT_INPUT].value;
	float mid_out  = (left_in + right_in)/sqrt(2);
	float side_out = (left_in - right_in)/sqrt(2);
	outputs[MID_OUTPUT].value   = mid_out;
	outputs[SIDE_OUTPUT].value  = side_out;



	//DECODER
	float mid_in   = inputs[MID_INPUT].value;
	float side_in  = inputs[SIDE_INPUT].value;
	float left_out  = (mid_in + side_in)/sqrt(2);
	float right_out = (mid_in - side_in)/sqrt(2);
	outputs[LEFT_OUTPUT].value  = left_out;
	outputs[RIGHT_OUTPUT].value = right_out;
}

struct MSMTRXWidget : ModuleWidget
{
	MSMTRXWidget(MSMTRX *module) : ModuleWidget(module)
	{
		setPanel(SVG::load(assetPlugin(plugin, "res/MSMTRX.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	
		//ENCODER
		addInput(Port::create<aPJackGiallo>(Vec(16.7, 113.2), Port::INPUT, module, MSMTRX::LEFT_INPUT));
		addInput(Port::create<aPJackArancione>(Vec(79.7, 113.2), Port::INPUT, module, MSMTRX::RIGHT_INPUT));
	    addOutput(Port::create<aPJackTurchese>(Vec(16.7, 167.2), Port::OUTPUT, module, MSMTRX::MID_OUTPUT));
		addOutput(Port::create<aPJackBlu>(Vec(79.7, 167.2), Port::OUTPUT, module, MSMTRX::SIDE_OUTPUT));


		//DECODER
		addInput(Port::create<aPJackTurchese>(Vec(16.7, 247.9), Port::INPUT, module, MSMTRX::MID_INPUT));
		addInput(Port::create<aPJackBlu>(Vec(79.7, 247.9), Port::INPUT, module, MSMTRX::SIDE_INPUT));
		addOutput(Port::create<aPJackVerde>(Vec(16.7, 305), Port::OUTPUT, module, MSMTRX::LEFT_OUTPUT));
		addOutput(Port::create<aPJackRosso>(Vec(79.7, 305), Port::OUTPUT, module, MSMTRX::RIGHT_OUTPUT));
	}
};

Model *modelMSMTRX = Model::create<MSMTRX, MSMTRXWidget>("aP-Modules", "MS Matrix", "Mid/Side Matrix - LR to MS and MS to LR encoder/decoder", UTILITY_TAG);
