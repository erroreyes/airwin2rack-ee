/* ========================================
 *  Chamber - Chamber.h
 *  Copyright (c) 2016 airwindows, Airwindows uses the MIT license
 * ======================================== */

#ifndef __Chamber_H
#include "Chamber.h"
#endif
#include <cmath>
#include <algorithm>
namespace airwinconsolidated::Chamber {

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {return new Chamber(audioMaster);}

Chamber::Chamber(audioMasterCallback audioMaster) :
    AudioEffectX(audioMaster, kNumPrograms, kNumParameters)
{
	A = 0.35;
	B = 0.35;
	C = 0.35;
	D = 0.35;
	E = 0.35;
	
	iirAL = 0.0; iirAR = 0.0;
	iirBL = 0.0; iirBR = 0.0;
	iirCL = 0.0; iirCR = 0.0;
	
	for(int count = 0; count < 19999; count++) {aEL[count] = 0.0;aER[count] = 0.0;}
	for(int count = 0; count < 12360; count++) {aFL[count] = 0.0;aFR[count] = 0.0;}
	for(int count = 0; count < 7639; count++) {aGL[count] = 0.0;aGR[count] = 0.0;}
	for(int count = 0; count < 4721; count++) {aHL[count] = 0.0;aHR[count] = 0.0;}
	for(int count = 0; count < 2915; count++) {aAL[count] = 0.0;aAR[count] = 0.0;}
	for(int count = 0; count < 1803; count++) {aBL[count] = 0.0;aBR[count] = 0.0;}
	for(int count = 0; count < 1114; count++) {aCL[count] = 0.0;aCR[count] = 0.0;}
	for(int count = 0; count < 688; count++) {aDL[count] = 0.0;aDR[count] = 0.0;}
	for(int count = 0; count < 425; count++) {aIL[count] = 0.0;aIR[count] = 0.0;}
	for(int count = 0; count < 263; count++) {aJL[count] = 0.0;aJR[count] = 0.0;}
	for(int count = 0; count < 162; count++) {aKL[count] = 0.0;aKR[count] = 0.0;}
	for(int count = 0; count < 100; count++) {aLL[count] = 0.0;aLR[count] = 0.0;}
	
	feedbackAL = 0.0; feedbackAR = 0.0;
	feedbackBL = 0.0; feedbackBR = 0.0;
	feedbackCL = 0.0; feedbackCR = 0.0;
	feedbackDL = 0.0; feedbackDR = 0.0;
	previousAL = 0.0; previousAR = 0.0;
	previousBL = 0.0; previousBR = 0.0;
	previousCL = 0.0; previousCR = 0.0;
	previousDL = 0.0; previousDR = 0.0;
	
	for(int count = 0; count < 9; count++) {lastRefL[count] = 0.0;lastRefR[count] = 0.0;}
	
	countI = 1;
	countJ = 1;
	countK = 1;
	countL = 1;
	
	countA = 1;
	countB = 1;
	countC = 1;
	countD = 1;	
	
	countE = 1;
	countF = 1;
	countG = 1;
	countH = 1;
	cycle = 0;
	
	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
	//this is reset: values being initialized only once. Startup values, whatever they are.
	
    _canDo.insert("plugAsChannelInsert"); // plug-in can be used as a channel insert effect.
    _canDo.insert("plugAsSend"); // plug-in can be used as a send effect.
    _canDo.insert("x2in2out"); 
    setNumInputs(kNumInputs);
    setNumOutputs(kNumOutputs);
    setUniqueID(kUniqueId);
    canProcessReplacing();     // supports output replacing
    canDoubleReplacing();      // supports double precision processing
	programsAreChunks(true);
    vst_strncpy (_programName, "Default", kVstMaxProgNameLen); // default program name
}

Chamber::~Chamber() {}
VstInt32 Chamber::getVendorVersion () {return 1000;}
void Chamber::setProgramName(char *name) {vst_strncpy (_programName, name, kVstMaxProgNameLen);}
void Chamber::getProgramName(char *name) {vst_strncpy (name, _programName, kVstMaxProgNameLen);}
//airwindows likes to ignore this stuff. Make your own programs, and make a different plugin rather than
//trying to do versioning and preventing people from using older versions. Maybe they like the old one!

static float pinParameter(float data)
{
	if (data < 0.0f) return 0.0f;
	if (data > 1.0f) return 1.0f;
	return data;
}

void Chamber::setParameter(VstInt32 index, float value) {
    switch (index) {
        case kParamA: A = value; break;
        case kParamB: B = value; break;
        case kParamC: C = value; break;
        case kParamD: D = value; break;
        case kParamE: E = value; break;
        default: break; // unknown parameter, shouldn't happen!
    }
}

float Chamber::getParameter(VstInt32 index) {
    switch (index) {
        case kParamA: return A; break;
        case kParamB: return B; break;
        case kParamC: return C; break;
        case kParamD: return D; break;
        case kParamE: return E; break;
        default: break; // unknown parameter, shouldn't happen!
    } return 0.0; //we only need to update the relevant name, this is simple to manage
}

void Chamber::getParameterName(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "Bigness", kVstMaxParamStrLen); break;
		case kParamB: vst_strncpy (text, "Longness", kVstMaxParamStrLen); break;
		case kParamC: vst_strncpy (text, "Liteness", kVstMaxParamStrLen); break;
		case kParamD: vst_strncpy (text, "Darkness", kVstMaxParamStrLen); break;
		case kParamE: vst_strncpy (text, "Wetness", kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
    } //this is our labels for displaying in the VST host
}

void Chamber::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: float2string (A, text, kVstMaxParamStrLen); break;
        case kParamB: float2string (B, text, kVstMaxParamStrLen); break;
        case kParamC: float2string (C, text, kVstMaxParamStrLen); break;
        case kParamD: float2string (D, text, kVstMaxParamStrLen); break;
        case kParamE: float2string (E, text, kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
	} //this displays the values and handles 'popups' where it's discrete choices
}

void Chamber::getParameterLabel(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamB: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamC: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamD: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamE: vst_strncpy (text, "", kVstMaxParamStrLen); break;
		default: break; // unknown parameter, shouldn't happen!
    }
}

VstInt32 Chamber::canDo(char *text) 
{ return (_canDo.find(text) == _canDo.end()) ? -1: 1; } // 1 = yes, -1 = no, 0 = don't know

bool Chamber::getEffectName(char* name) {
    vst_strncpy(name, "Chamber", kVstMaxProductStrLen); return true;
}

VstPlugCategory Chamber::getPlugCategory() {return kPlugCategEffect;}

bool Chamber::getProductString(char* text) {
  	vst_strncpy (text, "airwindows Chamber", kVstMaxProductStrLen); return true;
}

bool Chamber::getVendorString(char* text) {
  	vst_strncpy (text, "airwindows", kVstMaxVendorStrLen); return true;
}
bool Chamber::parameterTextToValue(VstInt32 index, const char *text, float &value) {
    switch(index) {
    case kParamA: { auto b = string2float(text, value); return b; break; }
    case kParamB: { auto b = string2float(text, value); return b; break; }
    case kParamC: { auto b = string2float(text, value); return b; break; }
    case kParamD: { auto b = string2float(text, value); return b; break; }
    case kParamE: { auto b = string2float(text, value); return b; break; }

    }
    return false;
}
bool Chamber::canConvertParameterTextToValue(VstInt32 index) {
    switch(index) {
        case kParamA: return true;
        case kParamB: return true;
        case kParamC: return true;
        case kParamD: return true;
        case kParamE: return true;

    }
    return false;
}
} // end namespace
