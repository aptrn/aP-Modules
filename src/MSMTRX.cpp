#include "Template.hpp"

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

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void MSMTRX::step()
{

	float left_in  = inputs[LEFT_INPUT].value *0.5f;
	float right_in = inputs[RIGHT_INPUT].value *0.5f;

	float mid_out  = (left_in + right_in);
	float side_out = (left_in + (right_in * -1));
	outputs[MID_OUTPUT].value   = mid_out*1.3f;
	outputs[SIDE_OUTPUT].value  = side_out*1.5f;


	float mid_in   = inputs[MID_INPUT].value *0.5f;
	float side_in  = inputs[SIDE_INPUT].value *0.5f;
	float left_out  = (mid_in + side_in);
	float right_out = (mid_in + (side_in * -1));
	outputs[LEFT_OUTPUT].value  = left_out*1.5f;
	outputs[RIGHT_OUTPUT].value = right_out*1.5f;

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

	

		addInput(Port::create<PJ301MPort>(Vec(16, 112), Port::INPUT, module, MSMTRX::LEFT_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(78, 112), Port::INPUT, module, MSMTRX::RIGHT_INPUT));
	    addOutput(Port::create<PJ301MPort>(Vec(16, 180), Port::OUTPUT, module, MSMTRX::MID_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(78, 180), Port::OUTPUT, module, MSMTRX::SIDE_OUTPUT));

		addInput(Port::create<PJ301MPort>(Vec(16, 250), Port::INPUT, module, MSMTRX::MID_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(78, 250), Port::INPUT, module, MSMTRX::SIDE_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(16, 316), Port::OUTPUT, module, MSMTRX::LEFT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(78, 316), Port::OUTPUT, module, MSMTRX::RIGHT_OUTPUT));
	

	}
};

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelMSMTRX = Model::create<MSMTRX, MSMTRXWidget>("aP", "MSMTRX", "Mid/Side Matrix", UTILITY_TAG);
