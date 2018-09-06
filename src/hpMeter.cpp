#include "aP.hpp"
#include "JWResizableHandle.hpp"

//Colourful resizable blank to know how much space you actually use
//Based on code taken from	JW-Modules Full Scope by jeremywen ---  https://github.com/jeremywen/JW-Modules

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
	std::shared_ptr<Font> font;

    void draw(NVGcontext *vg) { 
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

	font = Font::load(assetPlugin(plugin, "res/Champagne & Limousines.ttf"));
	nvgCreateImage(vg, "res/hpMeter.svg", NVG_IMAGE_REPEATX);
    nvgFillColor(vg, nvgRGBAf(0.,0.,0.,1.));
	nvgFontFaceId(vg, font->handle);
    nvgFontSize(vg, fontsize);
    nvgText(vg, xpos, box.size.y - (fontsize*2), hpchar, end);
    nvgText(vg, xpos, box.size.y - fontsize, uchar, end);    

	for(int i = 0; i < box.size.x; i++){								//Ruler Draw
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
};

hpMeterWidget::hpMeterWidget(hpMeter *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*3, RACK_GRID_HEIGHT);
	int r = static_cast<int>(box.size.x/15) % 255;
	int g = static_cast<int>(box.size.y) % 255;
	int b = static_cast<int>((box.size.x /15)+ box.size.y) % 255;	
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
