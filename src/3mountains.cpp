#include "plugin.hpp"

///////// THINGS TO DO NEXT SESSION
// 0. chain 1v/oct inputs to first input
// 1.

struct tripleOSC {
	// deltatime = time difference for loops, sync to main clock
	float deltaTime[3];
	float pitch[3];
	float freq[3];
	float sine[3];
	float saw[3];
	float square[3];
	float triangle[3];
	float phase[3];
	float sine_tri[3];
	float saw_sqr[3];
	float pwm_width[3]; //sets maximum & minimum cv values
	float pwm_amount[3]; //adds cv to pwm
	void step (	float pitch_param_value[], 
		float pitch_input_value[], 
		float fine_param_value[], 
		float pulsewidth_param_value[], 
		float pwm_amount_value[], 
		float pwm_cv_input_value[], 
		float freqmod_param_value[], 
		float freqmod_input_value[]) {
		//add function parameters for frequency modulation
		for (int i = 0; i < 3; i++) {
		// set deltatime to last step
			deltaTime[i] = APP->engine->getSampleTime();
		// set pitch parameter to knob value + cv
			pitch[i] = pitch_param_value[i] + pitch_input_value[i];


		//take 2 inputs, add new float & add freqmod to pitch
		//float freqmod_param_value, float freqmod_input_value
		//printf("freqmod: %f\n", freqmod_input_value * freqmod_param_value);
		//printf("freqmod param: %f\n", freqmod_param_value);
		//printf("freqmod input: %f\n", freqmod_input_value);
			pitch[i] += freqmod_input_value[i] * freqmod_param_value[i];




		// calculate pulsewidth value
			pwm_amount[i] = (1.0f - (pulsewidth_param_value[i] - 0.5f) / 0.49);
		//printf("pwk amount: %f\n", pulsewidth_param_value);
		// clamp cv input to range
		float clamped_cv = clamp(pwm_cv_input_value[i], -5.0f, 5.0f); ///CHANGE THIS INPUT GANGES FROM 0.0 TO 10.0!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// divide clamp to find percentage
		float cv_percentage = clamped_cv / 5.0f;
		// apply percentage to pwm_amount_value
		float offset = pwm_amount_value[i] * cv_percentage;
		
		//printf("offset: %f\n", offset);
		// add to pwm_amount
		pwm_amount[i] += offset;
		pwm_amount[i] = clamp(pwm_amount[i], 0.0f, 1.0f);
		// The default pitch is C3
		//printf("Pitch: %f\n", pitch);
		
		//increases/decreases coarse pitch + cv
		freq[i] = 130.812f * powf(2.0f, pitch[i]);  //C4 = 261.626f
		
		//adjusts between semitone
		freq[i] = freq[i] + (freq[i] * powf(2.0f, fine_param_value[i]/12));


		phase[i] += freq[i] * deltaTime[i];
		//printf("F*dT: %f\n", freq * deltaTime);
		//printf("Phase: %f\n", phase);
		if (phase[i] >= 1.0f) {
			phase[i] -= 1.0f;
		}

		// printf("%f\n", freq);
		// printf("%f\n", deltaTime);
		//printf("%f\n", phase);

		sine[i] = sinf((2.0f * M_PI * phase[i])-(0.5f * M_PI));
		saw[i] = 1 - (2 * phase[i]);
		square[i] = 0.0f;
		triangle[i] = -1.0f;
		
		//sine wave
		//modify sine output apply triangle morph
		//first calculate pure triangle value
		if ( phase[i] <= 0.5f) {
			sine_tri[i] = phase[i];

		} else {
			sine_tri[i] = (1.0f - phase[i]);
		}
		sine_tri[i] *= 4.0f;
		sine_tri[i] -= 1.0f;

		float smallSegmentPW = pulsewidth_param_value[i];
		float sine_pulsewidth = pulsewidth_param_value[i];
		bool is_left = true;
		if (pulsewidth_param_value[i] > 0.5f) {
			is_left = false;
			smallSegmentPW = 1.0f - pulsewidth_param_value[i];
		}

		//printf("pwm_amount_value: %f\n", pwm_amount_value);
		float sine_pwm_amount_copy = 0.0f;

		if (pwm_cv_input_value[i] != 0.0f) {

			if (smallSegmentPW < pwm_amount_value[i] * 0.5f) {

				float sine_difference = (pwm_amount_value[i] * 0.5f) - smallSegmentPW;
				if (sine_pulsewidth > 0.5f) {
					sine_pulsewidth -= sine_difference;
				} else {
					sine_pulsewidth += sine_difference;
				}
			}	
		}


		//printf("smallSegmentPW: %f\n", smallSegmentPW);
		
		float pwm_range = (pwm_amount_value[i]) * 0.5f;
		//float pwm_range = (pwm_amount_value);
		float pwm_offset = pwm_range * cv_percentage;

		//printf("sine pulsewidth %f\n", sine_pulsewidth);
		sine_pulsewidth += pwm_offset;
		printf("sine pulsewidth with CV %f\n", pwm_cv_input_value);
		//sine = sine_tri + ((sine - sine_tri) * (sine_pulsewidth));//(pwm_amount - 0.5f) / 0.49));
		sine[i] = sine[i] + ((sine_tri[i] - sine[i]) * (sine_pulsewidth));//(pwm_amount - 0.5f) / 0.49));


		//square wave
		float sqrpulse = sine_pulsewidth;

		
		//sqrpulse = clamp(sqrpulse, 0.5f, 0.99);
		sqrpulse = (sqrpulse / 2.0f) + 0.5f;
		//printf("sqrpulse: %f\n", sqrpulse);

		if (phase[i] >= sqrpulse) {
			square[i] = 1.0f;
		} else {
			square[i] = -1.0f;
		}

		//saw wave
		//interpolation wave
		float saw_pulse = sine_pulsewidth;
		if (phase[i] <= 0.5) {
			saw_sqr[i] = 1.0f;
		} else {
			saw_sqr[i] = -1.0f;
		}
		//saw = sinf(phase) * pow(fabs(phase), ((1.0f - pwm_amount) / 2.0f));
		saw[i] = saw_sqr[i] + ((saw[i] - saw_sqr[i]) * (1.0f - saw_pulse));//(1.0f - (pwm_amount - 0.5f) / 0.49));

	 	// //triangle wave
	 	// float tripulse = pulsewidth_param_value;

		// tripulse += offset * 0.5f;
		// tripulse = clamp(tripulse, 0.5f, 0.99);

		// if (tripulse >= phase) {
		// 	triangle = phase / (tripulse / 2.0f);

		// } else {
		// 	triangle = 1 - (phase / (tripulse / 2.0f));
		// }
		// triangle -= 1.0f;

///////////////////////////////////////// new work on tri-saw output
		
		float tripulse = sine_pulsewidth;
		tripulse /= 2.0f;
		tripulse += 0.5f;


		if (phase[i] < tripulse) {
			//phase needs to relate to tripulse
			//find percentage of phase to tripulse and use same percentage agains one half
			float percentageToTripulse = 0.5f / tripulse;
			float valueToUSe = percentageToTripulse * phase[i];
			triangle[i] = (valueToUSe * 4) - 1;
		} else {
			triangle[i] =  - ((phase[i] * 4) - 1) + 2;
		}


		float multiplier = 5.0f;
		sine[i] *= multiplier; 
		saw[i] *= multiplier;
		square[i] *= multiplier;
		triangle[i] *= multiplier; 
	}
}


float getsine(int index) {
	return sine[index];
}
float getsaw(int index) {
	return saw[index];
}
float getsquare(int index) {
	return square[index];
}
float gettriangle(int index) {
	return triangle[index];
}

};

struct _3mountains : Module {
	enum ParamIds {
		PITCH_ONE_PARAM,
		CV_ONE_PARAM,
		PWM_ONE_PARAM,
		FINE_ONE_PARAM,
		PULSEWIDTH_ONE_PARAM,
		PITCH_TWO_PARAM,
		PITCH_THREE_PARAM,
		CV_TWO_PARAM,
		CV_THREE_PARAM,
		PWM_TWO_PARAM,
		PWM_THREE_PARAM,
		FINE_TWO_PARAM,
		FINE_THREE_PARAM,
		PULSEWIDTH_TWO_PARAM,
		PULSEWIDTH_THREE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PWM_IN_ONE_INPUT,
		CV_IN_ONE_INPUT,
		_1VOCT_ONE_INPUT,
		PWM_IN_TWO_INPUT,
		PWM_IN_THREE_INPUT,
		CV_IN_TWO_INPUT,
		CV_IN_THREE_INPUT,
		_1VOCT_TWO_INPUT,
		_1VOCT_THREE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SQUARE_ONE_OUTPUT,
		SINE_ONE_OUTPUT,
		SAW_ONE_OUTPUT,
		TRIANGLE_ONE_OUTPUT,
		SQUARE_TWO_OUTPUT,
		SQUARE_THREE_OUTPUT,
		SINE_TWO_OUTPUT,
		SINE_THREE_OUTPUT,
		SAW_TWO_OUTPUT,
		SAW_THREE_OUTPUT,
		TRIANGLE_TWO_OUTPUT,
		TRIANGLE_THREE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	_3mountains() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCH_ONE_PARAM, -2.0, 2.0, 0.0, "");
		configParam(CV_ONE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PWM_ONE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FINE_ONE_PARAM, -1.0, 1.0, 0.0, "");
		configParam(PULSEWIDTH_ONE_PARAM, 0.0, 1.0, 0.0, "");
		configParam(PITCH_TWO_PARAM, -2.0, 2.0, 0.0, "");
		configParam(PITCH_THREE_PARAM, -2.0, 2.0, 0.0, "");
		configParam(CV_TWO_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CV_THREE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PWM_TWO_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PWM_THREE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FINE_TWO_PARAM, -1.0, 1.0, 0.0, "");
		configParam(FINE_THREE_PARAM, -1.0, 1.0, 0.0, "");
		configParam(PULSEWIDTH_TWO_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PULSEWIDTH_THREE_PARAM, 0.f, 1.f, 0.f, "");
	}

	tripleOSC osc1;
	//tripleOSC osc2;
	//tripleOSC osc3;

	void process(const ProcessArgs &args) override {
		float pitch_param[3] = {
			params[PITCH_ONE_PARAM].value,
			params[PITCH_TWO_PARAM].value,
			params[PITCH_THREE_PARAM].value
		};

		float onevoct_input[3] = {
			inputs[_1VOCT_ONE_INPUT].value,
			inputs[_1VOCT_TWO_INPUT].value,
			inputs[_1VOCT_THREE_INPUT].value
		};

		float fine_param[3] = {
			params[FINE_ONE_PARAM].value,
			params[FINE_TWO_PARAM].value,
			params[FINE_THREE_PARAM].value
		};

		float pulsewidth_param[3] = {
			params[PULSEWIDTH_ONE_PARAM].value,
			params[PULSEWIDTH_TWO_PARAM].value,
			params[PULSEWIDTH_THREE_PARAM].value
		};

		float pwm_param[3] = {
			params[PWM_ONE_PARAM].value,
			params[PWM_TWO_PARAM].value,
			params[PWM_THREE_PARAM].value
		};

		float pwm_input[3] = {
			inputs[PWM_IN_ONE_INPUT].value,
			inputs[PWM_IN_TWO_INPUT].value,
			inputs[PWM_IN_THREE_INPUT].value
		};

		float cv_param[3] = {
			params[CV_ONE_PARAM].value,
			params[CV_TWO_PARAM].value,
			params[CV_THREE_PARAM].value
		};

		float cv_input[3] = {
			inputs[CV_IN_ONE_INPUT].value,
			inputs[CV_IN_TWO_INPUT].value,
			inputs[CV_IN_THREE_INPUT].value
		};




		// osc1
		osc1.step(
			pitch_param, //pitch_param_value
			onevoct_input, //pitch_input_value
			fine_param, //fine_param_value
			pulsewidth_param, //pulsewidth_param_value
			pwm_param, //pwm_amount_value
			pwm_input, //pwm_cv_input_value
			cv_param, //freqmod_param_value
			cv_input //freqmod_input_value
		);
		outputs[SINE_ONE_OUTPUT].value = osc1.getsine(0); // change output to parameter value
		outputs[SAW_ONE_OUTPUT].value = osc1.getsaw(0);
		outputs[SQUARE_ONE_OUTPUT].value = osc1.getsquare(0);
		outputs[TRIANGLE_ONE_OUTPUT].value = osc1.gettriangle(0);

		outputs[SINE_TWO_OUTPUT].value = osc1.getsine(1); // change output to parameter value
		outputs[SAW_TWO_OUTPUT].value = osc1.getsaw(1);
		outputs[SQUARE_TWO_OUTPUT].value = osc1.getsquare(1);
		outputs[TRIANGLE_TWO_OUTPUT].value = osc1.gettriangle(1);

		outputs[SINE_THREE_OUTPUT].value = osc1.getsine(2); // change output to parameter value
		outputs[SAW_THREE_OUTPUT].value = osc1.getsaw(2);
		outputs[SQUARE_THREE_OUTPUT].value = osc1.getsquare(2);
		outputs[TRIANGLE_THREE_OUTPUT].value = osc1.gettriangle(2);

		//osc2
		// osc2.step(params[PITCH_TWO_PARAM].value, //pitch_param_value
		// inputs[_1VOCT_TWO_INPUT].value, //pitch_input_value
		// params[FINE_TWO_PARAM].value, //fine_param_value
		// params[PULSEWIDTH_TWO_PARAM].value, //pulsewidth_param_value
		// params[PWM_TWO_PARAM].value, //pwm_amount_value
		// inputs[PWM_IN_TWO_INPUT].value, //pwm_cv_input_value
		// params[CV_TWO_PARAM].value, //freqmod_param_value
		// inputs[CV_IN_TWO_INPUT].value //freqmod_input_value
		// );
		// outputs[SINE_TWO_OUTPUT].value = osc2.getsine(); // change output to parameter value
		// outputs[SAW_TWO_OUTPUT].value = osc2.getsaw();
		// outputs[SQUARE_TWO_OUTPUT].value = osc2.getsquare();
		// outputs[TRIANGLE_TWO_OUTPUT].value = osc2.gettriangle();

		//osc3
		// osc3.step(params[PITCH_THREE_PARAM].value, //pitch_param_value
		// inputs[_1VOCT_THREE_INPUT].value, //pitch_input_value
		// params[FINE_THREE_PARAM].value, //fine_param_value
		// params[PULSEWIDTH_THREE_PARAM].value, //pulsewidth_param_value
		// params[PWM_THREE_PARAM].value, //pwm_amount_value
		// inputs[PWM_IN_THREE_INPUT].value, //pwm_cv_input_value
		// params[CV_THREE_PARAM].value, //freqmod_param_value
		// inputs[CV_IN_THREE_INPUT].value //freqmod_input_value
		// );
		// outputs[SINE_THREE_OUTPUT].value = osc3.getsine(); // change output to parameter value
		// outputs[SAW_THREE_OUTPUT].value = osc3.getsaw();
		// outputs[SQUARE_THREE_OUTPUT].value = osc3.getsquare();
		// outputs[TRIANGLE_THREE_OUTPUT].value = osc3.gettriangle();
	}
};


struct _3mountainsWidget : ModuleWidget {
	_3mountainsWidget(_3mountains *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/3mountainsVCVCompatible2.svg")));

		APP->window->loadSvg(asset::plugin(pluginInstance, "res/BigOrangeKnob.svg"));
		APP->window->loadSvg(asset::plugin(pluginInstance, "res/MassiveOrangeKnob.svg"));
		APP->window->loadSvg(asset::plugin(pluginInstance, "res/MassiveWhiteKnob.svg")); 

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<MyMassiveOrangeKnob>(mm2px(Vec(27.995, 24.975)), module, _3mountains::PITCH_ONE_PARAM));
		addParam(createParamCentered<MyMassiveOrangeKnob>(mm2px(Vec(83.772, 24.975)), module, _3mountains::PITCH_TWO_PARAM));
		addParam(createParamCentered<MyMassiveOrangeKnob>(mm2px(Vec(139.497, 24.975)), module, _3mountains::PITCH_THREE_PARAM));
		addParam(createParamCentered<MyBigOrangeKnob>(mm2px(Vec(11.282, 47.991)), module, _3mountains::CV_ONE_PARAM));
		addParam(createParamCentered<MyBigOrangeKnob>(mm2px(Vec(27.946, 47.991)), module, _3mountains::FINE_ONE_PARAM));
		addParam(createParamCentered<MyBigOrangeKnob>(mm2px(Vec(45.328, 47.991)), module, _3mountains::PWM_ONE_PARAM));
		addParam(createParamCentered<MyBigOrangeKnob>(mm2px(Vec(67.059, 47.991)), module, _3mountains::CV_TWO_PARAM));
		addParam(createParamCentered<MyBigOrangeKnob>(mm2px(Vec(83.723, 47.991)), module, _3mountains::FINE_TWO_PARAM));
		addParam(createParamCentered<MyBigOrangeKnob>(mm2px(Vec(100.935, 47.991)), module, _3mountains::PWM_TWO_PARAM));
		addParam(createParamCentered<MyBigOrangeKnob>(mm2px(Vec(121.915, 47.991)), module, _3mountains::CV_THREE_PARAM));
		addParam(createParamCentered<MyBigOrangeKnob>(mm2px(Vec(139.467, 47.991)), module, _3mountains::FINE_THREE_PARAM));
		addParam(createParamCentered<MyBigOrangeKnob>(mm2px(Vec(156.319, 47.991)), module, _3mountains::PWM_THREE_PARAM));
		addParam(createParamCentered<MyMassiveWhiteKnob>(mm2px(Vec(27.932, 92.7)), module, _3mountains::PULSEWIDTH_ONE_PARAM));
		addParam(createParamCentered<MyMassiveWhiteKnob>(mm2px(Vec(83.709, 92.7)), module, _3mountains::PULSEWIDTH_TWO_PARAM));
		addParam(createParamCentered<MyMassiveWhiteKnob>(mm2px(Vec(139.435, 92.7)), module, _3mountains::PULSEWIDTH_THREE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.282, 68.626)), module, _3mountains::CV_IN_ONE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28.118, 68.626)), module, _3mountains::_1VOCT_ONE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.328, 68.626)), module, _3mountains::PWM_IN_ONE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(66.889, 68.626)), module, _3mountains::CV_IN_TWO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(83.725, 68.626)), module, _3mountains::_1VOCT_TWO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(101.105, 68.626)), module, _3mountains::PWM_IN_TWO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(122.085, 68.626)), module, _3mountains::CV_IN_THREE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(139.468, 68.626)), module, _3mountains::_1VOCT_THREE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(156.319, 68.626)), module, _3mountains::PWM_IN_THREE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.956, 112.806)), module, _3mountains::SINE_ONE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.693, 112.806)), module, _3mountains::TRIANGLE_ONE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.447, 112.806)), module, _3mountains::SAW_ONE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(47.134, 112.806)), module, _3mountains::SQUARE_ONE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.733, 112.806)), module, _3mountains::SINE_TWO_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(77.445, 112.806)), module, _3mountains::TRIANGLE_TWO_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(90.279, 112.806)), module, _3mountains::SAW_TWO_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(102.91, 112.806)), module, _3mountains::SQUARE_TWO_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.698, 112.806)), module, _3mountains::SINE_THREE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(134.443, 112.806)), module, _3mountains::TRIANGLE_THREE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(147.184, 112.806)), module, _3mountains::SAW_THREE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(159.777, 112.806)), module, _3mountains::SQUARE_THREE_OUTPUT));



	}
};


Model *model_3mountains = createModel<_3mountains, _3mountainsWidget>("3mountains");