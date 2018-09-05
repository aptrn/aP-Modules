#include <string.h>
#include "Template.hpp"
#include "JWResizableHandle.hpp"
#include "dsp/digital.hpp"
#include <string>
#include <iostream>
#include <random>


#define BUFFER_SIZE 512

struct hpMeter : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};



	hpMeter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;


};



void hpMeter::step() {
	
}

struct hpMeterDisplay : TransparentWidget {
	hpMeter *module;
	int frame = 0;
	float rot = 0;
	std::shared_ptr<Font> font;

	struct Stats {
		float vrms, vpp, vmin, vmax;
		void calculate(float *values) {
			vrms = 0.0;
			vmax = -INFINITY;
			vmin = INFINITY;
			for (int i = 0; i < BUFFER_SIZE; i++) {
				float v = values[i];
				vrms += v*v;
				vmax = fmaxf(vmax, v);
				vmin = fminf(vmin, v);
			}
			vrms = sqrtf(vrms / BUFFER_SIZE);
			vpp = vmax - vmin;
		}
	};
	Stats statsX, statsY;
    void draw(NVGcontext *vg) { 
    /*
        def text(self, float x, float y, const char* txt, const char* end=NULL){
        '''
        draws text string (txt) at specified location
        if end is specified, only the sun-string up to the end is drawn
        '''
        return nvg.nvgText(self.ctx, x, y, txt, end)    
        }
    */
    //char text = box.size.x;

    //std::string text = std::to_string(box.size.x);


	/*
	nvgBeginPath(vg);
	nvgMoveTo(vg, 0, 0);
	nvgLineTo(vg, box.size.x/3, (box.size.y/3)*2);
	nvgLineTo(vg, box.size.x/2, box.size.y);
	nvgClosePath(vg);
	nvgStrokeWidth(vg, 0.5);
	nvgStrokeColor(vg, nvgRGBAf(0.,0.,0.,1.));
	nvgStroke(vg);
	*/
    int hp = box.size.x/15;                         //Pixel/15 = HP
    int u = box.size.y/(380/3);                     //Pixel/380 = 3U, Pixel/(380/3) = 1U


    std::string hpcount = std::to_string(hp);
    std::string hpcost("HP");                       //SCRITTA HP
    hpcount.append (hpcost);
    const char *hpchar = hpcount.c_str();           //HP NUM TEXT


    std::string ucount = std::to_string(u);
    std::string ucost("U");                         //SCRITTA HP
    ucount.append (ucost);
    const char *uchar = ucount.c_str();             //HP NUM TEXT


    const char *end = NULL;                         //LINE END

	int fontsize = min(max(hp, 20), 100);
	
	int minus = fontsize*3;
	int xpos = box.size.x-(minus);
	if (xpos < 5){
		xpos = 5;
	}

	nvgCreateImage(vg, "res/hpMeter.svg", NVG_IMAGE_REPEATX);
	//nvgCreateFont(vg, "Champagne & Limousines", "Champagne & Limousines.ttf");
	nvgFindFont(vg,"Champagne & Limousines");
  //  nvgFontFace(vg, "Champagne & Limousines");
    nvgFillColor(vg, nvgRGBAf(0.,0.,0.,1.));
 
    nvgFontSize(vg, fontsize);
    nvgText(vg, xpos, box.size.y - (fontsize*2), hpchar, end);
    nvgText(vg, xpos, box.size.y - fontsize, uchar, end);

    //nvgText(vg, box.size.x/2+20, box.size.y/2, pcost, end);
    //nvgText(vg, box.size.x/2+23, box.size.y/2, pconst, end);
    

		for(int i = 0; i < box.size.x; i++){
		//int thp = box.size.x/15;
		if (i % 15 == 0){
			if (i % 75 == 0){
				if (i % 150 == 0){
					nvgBeginPath(vg);
					nvgMoveTo(vg, i, 0);
					nvgLineTo(vg, i, 50);
					nvgClosePath(vg);
					nvgStrokeWidth(vg, 1);
					nvgStrokeColor(vg, nvgRGBAf(0.,0.,0.,1.));
					nvgStroke(vg);
					int hpnow = i / 15;
					std::string hpm = std::to_string(hpnow);
					const char *hpmchar = hpm.c_str();
					nvgFontSize(vg, 20);
					nvgText(vg, i - 10, 50 + 20, hpmchar, end);
				}
				else {
					nvgBeginPath(vg);
					nvgMoveTo(vg, i, 0);
					nvgLineTo(vg, i, 40);
					nvgClosePath(vg);
					nvgStrokeWidth(vg, 0.75);
					nvgStrokeColor(vg, nvgRGBAf(0.,0.,0.,1.));
					nvgStroke(vg);
					int hpnow1 = i / 15;
					std::string hpm1 = std::to_string(hpnow1);
					const char *hpm1char = hpm1.c_str();
					nvgFontSize(vg, 10);
					nvgText(vg, i - 5, 40 + 10, hpm1char, end);
				}
			}
			else {
				nvgBeginPath(vg);
				nvgMoveTo(vg, i, 0);
				nvgLineTo(vg, i, 30);
				nvgClosePath(vg);
				nvgStrokeWidth(vg, 0.5);
				nvgStrokeColor(vg, nvgRGBAf(0.,0.,0.,1.));
				nvgStroke(vg);
			}
		}
	}
	

}
};

struct hpMeterWidget : ModuleWidget {
	Panel *panel;
	JWModuleResizeHandle *leftHandle;
	JWModuleResizeHandle *rightHandle;
	TransparentWidget *display;
	hpMeterWidget(hpMeter *module);
	void step() override;
   // void draw() override;

};

hpMeterWidget::hpMeterWidget(hpMeter *module) : ModuleWidget(module) {



	box.size = Vec(RACK_GRID_WIDTH*3, RACK_GRID_HEIGHT);
	int r = static_cast<int>(box.size.x/15) % 255;
	int g = static_cast<int>(box.size.y) % 255;
	int b = static_cast<int>((box.size.x /15)+ box.size.y) % 255;	
	//setPanel(SVG::load(assetPlugin(plugin, "res/hpMeter.svg")));
	{
		panel = new Panel();
		panel->backgroundColor = nvgRGB(r, g, b);
		panel->box.size = box.size;
		addChild(panel);
	}
	panel->backgroundColor = nvgRGB(r, g, b);
	
	leftHandle = new JWModuleResizeHandle(45);
	rightHandle = new JWModuleResizeHandle(45);
	rightHandle->right = true;
    leftHandle->right = true;
	addChild(leftHandle);
	addChild(rightHandle);

	{
		hpMeterDisplay *display = new hpMeterDisplay();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, box.size.y);
		addChild(display);
		this->display = display;
	}


}



void hpMeterWidget::step() {
	panel->box.size = box.size;
	int x = static_cast<int>(box.size.x);
	int y = static_cast<int>(box.size.y);

	int r = rescale(abs(x + 2*y) % 240, 0, 240, 150, 240);
	int g = rescale(abs(2*y - x) % 240, 0, 240, 150, 240);
	int b = rescale(abs(2*x % y) % 240, 0, 240, 150, 240);

	

	
	panel->backgroundColor = nvgRGB(r, g, b);
	display->box.size = Vec(box.size.x, box.size.y);
	rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
	rightHandle->box.pos.y = box.size.y - rightHandle->box.size.y;
	leftHandle->box.pos.y = box.size.y - leftHandle->box.size.y;
    leftHandle->box.pos.x = box.size.x - leftHandle->box.size.x;
	ModuleWidget::step();
}


Model *modelhpMeter = Model::create<hpMeter, hpMeterWidget>("aP", "hpMeter", "HP Meter", VISUAL_TAG);
