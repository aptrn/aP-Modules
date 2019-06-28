#include "aP.hpp"
#include "dsp/digital.hpp"


//BETA
//Triple clock multiplier/divider with chance of skipping per pulse

struct Tempo : Module
{
	enum ParamIds
	{	
		RATE1_PARAM,
		RATE2_PARAM,
		RATE3_PARAM,

		SKIP1_PARAM,
		SKIP2_PARAM,
		SKIP3_PARAM,

		MUTE1_BTN,
		MUTE2_BTN,
		MUTE3_BTN,

		NUM_PARAMS
	};
	enum InputIds
	{
		CLK_INPUT,

		RATE1_CV,
		RATE2_CV,
		RATE3_CV,

		SKIP1_CV,
		SKIP2_CV,
		SKIP3_CV,

		NUM_INPUTS
	};
	enum OutputIds
	{
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		
		NUM_OUTPUTS
	};
	enum LightIds
	{
		SYNC_LED,

		MUTE1_LED,
		MUTE2_LED,
		MUTE3_LED,

		NUM_LIGHTS
	};
	dsp::SchmittTrigger clock_input;
	int ratearr [11] = {6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6};

	dsp::SchmittTrigger mute1_trig;
	dsp::SchmittTrigger mute2_trig;
	dsp::SchmittTrigger mute3_trig;

	dsp::PulseGenerator pulse_1;
	dsp::PulseGenerator pulse_2;
	dsp::PulseGenerator pulse_3;


	float pulse_time = 0;
	int currentStep = 0;
	int prevStep = 0;
	int delta = 0;
	bool sync = false;

	int expStep1, expStep2, expStep3;
	int target1, target2, target3;
	int timer1, timer2, timer3;




	int rate1, rate2, rate3;
	int index1, index2, index3;
	bool isMult1, isMult2, isMult3;
	bool mute1, mute2, mute3;



	Tempo() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);}
	void process(const ProcessArgs &args) override;


};

bool skip_fun(float knob, float cv){
	float r = random::uniform();
	float threshold = clamp(knob + (cv/10.f), 0.f, 1.f);
	return r < threshold;
}

void Tempo::process(const ProcessArgs &args)
{

	/////////////////////////////////////////////////////////// INITIALIZATION


	//Create Skips
	bool skip1 = skip_fun(params[SKIP1_PARAM].getValue(), inputs[SKIP1_CV].getVoltage());
	bool skip2 = skip_fun(params[SKIP2_PARAM].getValue(), inputs[SKIP2_CV].getVoltage());
	bool skip3 = skip_fun(params[SKIP3_PARAM].getValue(), inputs[SKIP3_CV].getVoltage());



	//Check Mutes and relative LEDs
	if (mute1_trig.process(params[MUTE1_BTN].getValue())){
		mute1 = !mute1;
	}
	lights[MUTE1_LED].value = mute1 ? 1.0f : 0.0f;
	
	if (mute2_trig.process(params[MUTE2_BTN].getValue())){
		mute2 = !mute2;
	}
	lights[MUTE2_LED].value = mute2 ? 1.0f : 0.0f;

	if (mute3_trig.process(params[MUTE3_BTN].getValue())){
		mute3 = !mute3;
	}
	lights[MUTE3_LED].value = mute3 ? 1.0f : 0.0f;




	//Check Rates (knobs and CVs) and consult division/multi array
	int index1, index2, index3;


	if (inputs[RATE1_CV].isConnected()){
		float rate1_cv_val = rescale(inputs[RATE1_CV].getVoltage(), -10.0f, 10.f, -1.0f, 1.0f);
		float rate1_val = rescale(params[RATE1_PARAM].getValue(), 0.0f, 1.0f, -1.0f, 1.0f);
		index1 = rescale(rate1_cv_val*rate1_val, -1.0f, 1.0f, 0, 11);
	}
	else {
		float rate1_val = params[RATE1_PARAM].getValue();
		index1 = rescale(rate1_val, 0.0f, 1.0f, 0, 11);
	}
	isMult1 = index1 > 5;
	rate1 = ratearr[index1];


	if (inputs[RATE2_CV].isConnected()){
		float rate2_cv_val = rescale(inputs[RATE2_CV].getVoltage(), -10.0f, 10.f, -1.0f, 1.0f);
		float rate2_val = rescale(params[RATE2_PARAM].getValue(), 0.0f, 1.0f, -1.0f, 1.0f);
		index2 = rescale(rate2_cv_val*rate2_val, -1.0f, 1.0f, 0, 11);
	}
	else {
		float rate2_val = params[RATE2_PARAM].getValue();
		index2 = rescale(rate2_val, 0.0f, 1.0f, 0, 11);
	}
	isMult2 = index2 > 5;
	rate2 = ratearr[index2];


	if (inputs[RATE3_CV].isConnected()){
	float rate3_cv_val = rescale(inputs[RATE3_CV].getVoltage(), -10.0f, 10.f, -1.0f, 1.0f);
	float rate3_val = rescale(params[RATE3_PARAM].getValue(), 0.0f, 1.0f, -1.0f, 1.0f);
	index3 = rescale(rate3_cv_val*rate3_val, -1.0f, 1.0f, 0, 11);
	}
	else {
		float rate3_val = params[RATE3_PARAM].getValue();
		index3 = rescale(rate3_val, 0.0f, 1.0f, 0, 11);
	}
	isMult3 = index3 > 5;
	rate3 = ratearr[index3];


	
	/////////////////////////////////////////////////////////// END INITIALIZATION



	/////////////////////////////////////////////////////////// CLOCK DETECTION
	currentStep++;				
	
	if (inputs[CLK_INPUT].isConnected()){				
		if (clock_input.process(rescale(inputs[CLK_INPUT].getVoltage(), 0.2f, 1.7f, 0.0f, 1.0f))){
			if (sync == true){											//Se credi di essere syncato
				int verifica = delta + prevStep;											//dovresti essere a verifica
				if (currentStep < verifica+ 5 && currentStep > verifica - 5){							//se sei fra verifica +/- 5
					sync = true;																		//sei syncato	

					if (isMult1 == false && rate1 != 1){													//se il rate non è 1 o un clock divider
						timer1 = currentStep;														//aggiorna timer
						if (skip1 == false){														//se non skippi
							pulse_1.trigger(0.0001f);												//suona
						}
					}	
					if (isMult2 == false && rate2 != 1){													//così per i 3 canali
						timer2 = currentStep;
						if (skip2 == false){
							pulse_2.trigger(0.0001f);
						}
					}	
					if (isMult3 == false && rate3 != 1){
						timer3 = currentStep;
						if (skip3 == false){
							pulse_3.trigger(0.0001f);
						}
					}

					prevStep = currentStep;											//Aggiorna il prevStep
				}
				else{																					//Se non sei fra verifica +/- 5
					sync = false;																		//non sei syncato
					prevStep = currentStep;													//Azzera il prevStep
				}
			}										
			else if (sync == false){										//Se non sei syncato
				if (prevStep == 0){														//Se è il primo giro
					prevStep = currentStep;														//Aggiorna il prevStep
					sync = false;																//Sei ancora non syncato
				}
				else {																	//Se non è il primo giro
					delta = currentStep - prevStep;												//calcola il delta con il prevStep
					sync = true;																//sei syncato
					timer1 = currentStep;														//aggiorna i timer
					timer2 = currentStep;														//
					timer3 = currentStep;
					prevStep = currentStep;											//Aggiorna il prevStep
				}
			}
		}
	}
	else {										//Se non c'è clock azzera tutto, sei fuori sync
		sync = false;
		prevStep = 0;
	}


	/////////////////////////////////////////////////////////// END CLOCK DETECTION



	/////////////////////////////////////////////////////////// TRIGGERING TRIGGERS


	if (sync == true){													//Se sei syncato

		if(isMult1){						//CLOCK DIVIDER
			target1 = delta * rate1;									//Calcola i target
		}
		else {								//CLOCK MULTIPLIER
			target1 = delta / rate1;		
		}
		expStep1 = target1 + timer1;									//Calcola in che step dovresti essere
		if (currentStep == expStep1 && skip1 == false){					//Se sei nello step giusto e non skippi
			pulse_1.trigger(0.0001f);									//Suona
			timer1 = currentStep;										//Aggiorna il timer
		}																//così per i 3 canali

		if(isMult2){
			target2 = delta * rate2;		
		}
		else {
			target2 = delta / rate2;		
		}
		expStep2 = target2 + timer2;
		if (currentStep == expStep2 && skip2 == false){
			pulse_2.trigger(0.0001f);	
			timer2 = currentStep;
		}

		if(isMult3){
			target3 = delta * rate3;	
		}
		else {
			target3 = delta / rate3;	
		}
		expStep3 = target3 + timer3;
		if (currentStep == expStep3 && skip3 == false){
			pulse_3.trigger(0.0001f);	
			timer3 = currentStep;
		}
	}
	/////////////////////////////////////////////////////////// END TRIGGERING TRIGGERS


	/////////////////////////////////////////////////////////// TRIG OUTPUT

	bool out1 = pulse_1.process(1.0 / args.sampleRate);  
	if (mute1 == false){
  	outputs[OUTPUT_1].setVoltage(rescale(out1, 0.0f, 1.0f, 0.0f, 10.0f));
	}

	bool out2 = pulse_2.process(1.0 / args.sampleRate);  
	if (mute2 == false){
  	outputs[OUTPUT_2].setVoltage(rescale(out2, 0.0f, 1.0f, 0.0f, 10.0f));
	}

	bool out3 = pulse_3.process(1.0 / args.sampleRate);  
	if (mute3 == false){
  	outputs[OUTPUT_3].setVoltage(rescale(out3, 0.0f, 1.0f, 0.0f, 10.0f));
	}

	// LED OUTPUT
	lights[SYNC_LED].value = sync ? 1.0f : 0.0f;
}


struct TempoWidget : ModuleWidget
{
	TempoWidget(Tempo *module) : ModuleWidget(module)
	{
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Tempo.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		//MAIN
		addInput(createInput<aPJackNero>(Vec(18.2, 50.3), module, Tempo::CLK_INPUT));
		addChild(createLight<LedLight<RedLight>>(Vec(111.2, 52.6), module, Tempo::SYNC_LED));

		//RATE
		addParam(createParam<aPKnob>(Vec(15, 105.3), module, Tempo::RATE1_PARAM));
		addParam(createParam<aPKnob>(Vec(57.8, 105.3), module, Tempo::RATE2_PARAM));
		addParam(createParam<aPKnob>(Vec(101, 105.3), module, Tempo::RATE3_PARAM));

		addInput(createInput<aPJackGiallo>(Vec(20.7, 145.5), module, Tempo::RATE1_CV));
		addInput(createInput<aPJackTurchese>(Vec(63.5, 145.5), module, Tempo::RATE2_CV));
		addInput(createInput<aPJackRosa>(Vec(106.7, 145.5), module, Tempo::RATE3_CV));
		//SKIP
		addParam(createParam<aPKnob>(Vec(15, 196.3), module, Tempo::SKIP1_PARAM));
		addParam(createParam<aPKnob>(Vec(57.8, 196.3), module, Tempo::SKIP2_PARAM));
		addParam(createParam<aPKnob>(Vec(101, 196.3), module, Tempo::SKIP3_PARAM));

		addInput(createInput<aPJackArancione>(Vec(20.7, 234.9), module, Tempo::SKIP1_CV));
		addInput(createInput<aPJackAzzurro>(Vec(63.5, 234.9), module, Tempo::SKIP2_CV));
		addInput(createInput<aPJackFux>(Vec(106.7,234.9), module, Tempo::SKIP3_CV));

		//MUTE
		addParam(createParam<aPLedButton>(Vec(21.9, 278.7), module, Tempo::MUTE1_BTN));
		addParam(createParam<aPLedButton>(Vec(64.7, 278.7), module, Tempo::MUTE2_BTN));
		addParam(createParam<aPLedButton>(Vec(107.8, 278.7), module, Tempo::MUTE3_BTN));

		addChild(createLight<LedLight<RedLight>>(Vec(24.6, 280.9), module, Tempo::MUTE1_LED));
		addChild(createLight<LedLight<RedLight>>(Vec(67.4, 280.9), module, Tempo::MUTE2_LED));
		addChild(createLight<LedLight<RedLight>>(Vec(110.5, 280.9), module, Tempo::MUTE3_LED));
		
		//OUT
		//addChild(createLight<SmallLight<RedLight>>(Vec(21.2, 304.4), module, Tempo::MULT_LED));
		addOutput(createOutput<aPJackRosso>(Vec(20.7, 320.5), module, Tempo::OUTPUT_1));
		addOutput(createOutput<aPJackBlu>(Vec(63.5, 320.5), module, Tempo::OUTPUT_2));
		addOutput(createOutput<aPJackViola>(Vec(106.7, 320.5), module, Tempo::OUTPUT_3));
	}
};

Model *modelTempo = createModel<Tempo, TempoWidget>("Tempo");
