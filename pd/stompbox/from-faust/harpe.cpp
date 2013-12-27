//-----------------------------------------------------
// name: "Harpe"
// author: "Grame"
//
// Code generated with Faust 0.9.58 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
#include <cmath>
template <int N> inline float faustpower(float x) 		{ return powf(x,N); } 
template <int N> inline double faustpower(double x) 	{ return pow(x,N); }
template <int N> inline int faustpower(int x) 			{ return faustpower<N/2>(x) * faustpower<N-N/2>(x); } 
template <> 	 inline int faustpower<0>(int x) 		{ return 1; }
template <> 	 inline int faustpower<1>(int x) 		{ return x; }
/************************************************************************
 ************************************************************************
    FAUST Architecture File
	Copyright (C) 2006-2011 Albert Graef <Dr.Graef@t-online.de>
    ---------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as 
	published by the Free Software Foundation; either version 2.1 of the 
	License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
 	License along with the GNU C Library; if not, write to the Free
  	Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  	02111-1307 USA. 
 ************************************************************************
 ************************************************************************/

/* Pd architecture file, written by Albert Graef <Dr.Graef@t-online.de>.
   This was derived from minimal.cpp included in the Faust distribution.
   Please note that this is to be compiled as a shared library, which is
   then loaded dynamically by Pd as an external. */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>

using namespace std;

// On Intel set FZ (Flush to Zero) and DAZ (Denormals Are Zero)
// flags to avoid costly denormals
#ifdef __SSE__
    #include <xmmintrin.h>
    #ifdef __SSE2__
        #define AVOIDDENORMALS _mm_setcsr(_mm_getcsr() | 0x8040)
    #else
        #define AVOIDDENORMALS _mm_setcsr(_mm_getcsr() | 0x8000)
    #endif
#else
  #warning *** puredata.cpp: NO SSE FLAG (denormals may slow things down) ***
  #define AVOIDDENORMALS
#endif

struct Meta 
{
    void declare (const char* key, const char* value) {}
};


// abs is now predefined
//template<typename T> T abs (T a)			{ return (a<T(0)) ? -a : a; }


inline int		lsr (int x, int n)			{ return int(((unsigned int)x) >> n); }

/******************************************************************************
*******************************************************************************

							       VECTOR INTRINSICS

*******************************************************************************
*******************************************************************************/

//inline void *aligned_calloc(size_t nmemb, size_t size) { return (void*)((unsigned)(calloc((nmemb*size)+15,sizeof(char)))+15 & 0xfffffff0); }
//inline void *aligned_calloc(size_t nmemb, size_t size) { return (void*)((size_t)(calloc((nmemb*size)+15,sizeof(char)))+15 & ~15); }


/******************************************************************************
*******************************************************************************

			ABSTRACT USER INTERFACE

*******************************************************************************
*******************************************************************************/

class UI
{
  bool	fStopped;
public:
		
  UI() : fStopped(false) {}
  virtual ~UI() {}
	
  virtual void addButton(const char* label, float* zone) = 0;
  virtual void addCheckButton(const char* label, float* zone) = 0;
  virtual void addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step) = 0;
  virtual void addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step) = 0;
  virtual void addNumEntry(const char* label, float* zone, float init, float min, float max, float step) = 0;

  virtual void addHorizontalBargraph(const char* label, float* zone, float min, float max) = 0;
  virtual void addVerticalBargraph(const char* label, float* zone, float min, float max) = 0;
	
  virtual void openTabBox(const char* label) = 0;
  virtual void openHorizontalBox(const char* label) = 0;
  virtual void openVerticalBox(const char* label) = 0;
  virtual void closeBox() = 0;
	
  virtual void run() = 0;
	
  void stop()	{ fStopped = true; }
  bool stopped() 	{ return fStopped; }

  virtual void declare(float* zone, const char* key, const char* value) {}
};

/***************************************************************************
   Pd UI interface
 ***************************************************************************/

enum ui_elem_type_t {
  UI_BUTTON, UI_CHECK_BUTTON,
  UI_V_SLIDER, UI_H_SLIDER, UI_NUM_ENTRY,
  UI_V_BARGRAPH, UI_H_BARGRAPH,
  UI_END_GROUP, UI_V_GROUP, UI_H_GROUP, UI_T_GROUP
};

struct ui_elem_t {
  ui_elem_type_t type;
  char *label;
  float *zone;
  float init, min, max, step;
};

class PdUI : public UI
{
public:
  int nelems;
  ui_elem_t *elems;
		
  PdUI();
  PdUI(const char *s);
  virtual ~PdUI();

protected:
  string path;
  void add_elem(ui_elem_type_t type, const char *label = NULL);
  void add_elem(ui_elem_type_t type, const char *label, float *zone);
  void add_elem(ui_elem_type_t type, const char *label, float *zone,
		float init, float min, float max, float step);
  void add_elem(ui_elem_type_t type, const char *label, float *zone,
		float min, float max);

public:
  virtual void addButton(const char* label, float* zone);
  virtual void addCheckButton(const char* label, float* zone);
  virtual void addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step);
  virtual void addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step);
  virtual void addNumEntry(const char* label, float* zone, float init, float min, float max, float step);

  virtual void addHorizontalBargraph(const char* label, float* zone, float min, float max);
  virtual void addVerticalBargraph(const char* label, float* zone, float min, float max);
  
  virtual void openTabBox(const char* label);
  virtual void openHorizontalBox(const char* label);
  virtual void openVerticalBox(const char* label);
  virtual void closeBox();
	
  virtual void run();
};

static string mangle(const char *s)
{
  const char *s0 = s;
  string t = "";
  if (!s) return t;
  while (*s)
    if (isalnum(*s))
      t += *(s++);
    else {
      const char *s1 = s;
      while (*s && !isalnum(*s)) ++s;
      if (s1 != s0 && *s) t += "-";
    }
  return t;
}

static string normpath(string path)
{
  path = string("/")+path;
  int pos = path.find("//");
  while (pos >= 0) {
    path.erase(pos, 1);
    pos = path.find("//");
  }
  return path;
}

static string pathcat(string path, string label)
{
  if (path.empty())
    return normpath(label);
  else if (label.empty())
    return normpath(path);
  else
    return normpath(path+"/"+label);
}

PdUI::PdUI()
{
  nelems = 0;
  elems = NULL;
  path = "";
}

PdUI::PdUI(const char *s)
{
  nelems = 0;
  elems = NULL;
  path = s?s:"";
}

PdUI::~PdUI()
{
  if (elems) {
    for (int i = 0; i < nelems; i++)
      if (elems[i].label)
	free(elems[i].label);
    free(elems);
  }
}

inline void PdUI::add_elem(ui_elem_type_t type, const char *label)
{
  ui_elem_t *elems1 = (ui_elem_t*)realloc(elems, (nelems+1)*sizeof(ui_elem_t));
  if (elems1)
    elems = elems1;
  else
    return;
  string s = pathcat(path, mangle(label));
  elems[nelems].type = type;
  elems[nelems].label = strdup(s.c_str());
  elems[nelems].zone = NULL;
  elems[nelems].init = 0.0;
  elems[nelems].min = 0.0;
  elems[nelems].max = 0.0;
  elems[nelems].step = 0.0;
  nelems++;
}

inline void PdUI::add_elem(ui_elem_type_t type, const char *label, float *zone)
{
  ui_elem_t *elems1 = (ui_elem_t*)realloc(elems, (nelems+1)*sizeof(ui_elem_t));
  if (elems1)
    elems = elems1;
  else
    return;
  string s = pathcat(path, mangle(label));
  elems[nelems].type = type;
  elems[nelems].label = strdup(s.c_str());
  elems[nelems].zone = zone;
  elems[nelems].init = 0.0;
  elems[nelems].min = 0.0;
  elems[nelems].max = 1.0;
  elems[nelems].step = 1.0;
  nelems++;
}

inline void PdUI::add_elem(ui_elem_type_t type, const char *label, float *zone,
			  float init, float min, float max, float step)
{
  ui_elem_t *elems1 = (ui_elem_t*)realloc(elems, (nelems+1)*sizeof(ui_elem_t));
  if (elems1)
    elems = elems1;
  else
    return;
  string s = pathcat(path, mangle(label));
  elems[nelems].type = type;
  elems[nelems].label = strdup(s.c_str());
  elems[nelems].zone = zone;
  elems[nelems].init = init;
  elems[nelems].min = min;
  elems[nelems].max = max;
  elems[nelems].step = step;
  nelems++;
}

inline void PdUI::add_elem(ui_elem_type_t type, const char *label, float *zone,
			  float min, float max)
{
  ui_elem_t *elems1 = (ui_elem_t*)realloc(elems, (nelems+1)*sizeof(ui_elem_t));
  if (elems1)
    elems = elems1;
  else
    return;
  string s = pathcat(path, mangle(label));
  elems[nelems].type = type;
  elems[nelems].label = strdup(s.c_str());
  elems[nelems].zone = zone;
  elems[nelems].init = 0.0;
  elems[nelems].min = min;
  elems[nelems].max = max;
  elems[nelems].step = 0.0;
  nelems++;
}

void PdUI::addButton(const char* label, float* zone)
{ add_elem(UI_BUTTON, label, zone); }
void PdUI::addCheckButton(const char* label, float* zone)
{ add_elem(UI_CHECK_BUTTON, label, zone); }
void PdUI::addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step)
{ add_elem(UI_V_SLIDER, label, zone, init, min, max, step); }
void PdUI::addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step)
{ add_elem(UI_H_SLIDER, label, zone, init, min, max, step); }
void PdUI::addNumEntry(const char* label, float* zone, float init, float min, float max, float step)
{ add_elem(UI_NUM_ENTRY, label, zone, init, min, max, step); }

void PdUI::addHorizontalBargraph(const char* label, float* zone, float min, float max)
{ add_elem(UI_H_BARGRAPH, label, zone, min, max); }
void PdUI::addVerticalBargraph(const char* label, float* zone, float min, float max)
{ add_elem(UI_V_BARGRAPH, label, zone, min, max); }

void PdUI::openTabBox(const char* label)
{
  if (!path.empty()) path += "/";
  path += mangle(label);
}
void PdUI::openHorizontalBox(const char* label)
{
  if (!path.empty()) path += "/";
  path += mangle(label);
}
void PdUI::openVerticalBox(const char* label)
{
  if (!path.empty()) path += "/";
  path += mangle(label);
}
void PdUI::closeBox()
{
  int pos = path.rfind("/");
  if (pos < 0) pos = 0;
  path.erase(pos);
}

void PdUI::run() {}

/******************************************************************************
*******************************************************************************

			    FAUST DSP

*******************************************************************************
*******************************************************************************/



//----------------------------------------------------------------
//  abstract definition of a signal processor
//----------------------------------------------------------------
			
class dsp {
 protected:
	int fSamplingFreq;
 public:
	dsp() {}
	virtual ~dsp() {}
	virtual int getNumInputs() = 0;
	virtual int getNumOutputs() = 0;
	virtual void buildUserInterface(UI* interface) = 0;
	virtual void init(int samplingRate) = 0;
 	virtual void compute(int len, float** inputs, float** outputs) = 0;
};

//----------------------------------------------------------------------------
//  FAUST generated signal processor
//----------------------------------------------------------------------------
		

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;

#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

class mydsp : public dsp {
  private:
	FAUSTFLOAT 	fslider0;
	FAUSTFLOAT 	fslider1;
	int 	iRec1[2];
	FAUSTFLOAT 	fslider2;
	float 	fRec3[2];
	int 	iVec0[2];
	float 	fRec2[2];
	int 	IOTA;
	float 	fVec1[128];
	float 	fRec0[3];
	int 	iVec2[2];
	float 	fRec5[2];
	float 	fVec3[128];
	float 	fRec4[3];
	int 	iVec4[2];
	float 	fRec7[2];
	float 	fVec5[128];
	float 	fRec6[3];
	int 	iVec6[2];
	float 	fRec9[2];
	float 	fVec7[128];
	float 	fRec8[3];
	int 	iVec8[2];
	float 	fRec11[2];
	float 	fVec9[64];
	float 	fRec10[3];
	int 	iVec10[2];
	float 	fRec13[2];
	float 	fVec11[64];
	float 	fRec12[3];
	int 	iVec12[2];
	float 	fRec15[2];
	float 	fVec13[64];
	float 	fRec14[3];
	int 	iVec14[2];
	float 	fRec17[2];
	float 	fVec15[64];
	float 	fRec16[3];
	int 	iVec16[2];
	float 	fRec19[2];
	float 	fVec17[64];
	float 	fRec18[3];
	int 	iVec18[2];
	float 	fRec21[2];
	float 	fVec19[32];
	float 	fRec20[3];
	int 	iVec20[2];
	float 	fRec23[2];
	float 	fVec21[32];
	float 	fRec22[3];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "Harpe");
		m->declare("author", "Grame");
	}

	virtual int getNumInputs() 	{ return 0; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 0.005f;
		fslider1 = 0.5f;
		for (int i=0; i<2; i++) iRec1[i] = 0;
		fslider2 = 0.0f;
		for (int i=0; i<2; i++) fRec3[i] = 0;
		for (int i=0; i<2; i++) iVec0[i] = 0;
		for (int i=0; i<2; i++) fRec2[i] = 0;
		IOTA = 0;
		for (int i=0; i<128; i++) fVec1[i] = 0;
		for (int i=0; i<3; i++) fRec0[i] = 0;
		for (int i=0; i<2; i++) iVec2[i] = 0;
		for (int i=0; i<2; i++) fRec5[i] = 0;
		for (int i=0; i<128; i++) fVec3[i] = 0;
		for (int i=0; i<3; i++) fRec4[i] = 0;
		for (int i=0; i<2; i++) iVec4[i] = 0;
		for (int i=0; i<2; i++) fRec7[i] = 0;
		for (int i=0; i<128; i++) fVec5[i] = 0;
		for (int i=0; i<3; i++) fRec6[i] = 0;
		for (int i=0; i<2; i++) iVec6[i] = 0;
		for (int i=0; i<2; i++) fRec9[i] = 0;
		for (int i=0; i<128; i++) fVec7[i] = 0;
		for (int i=0; i<3; i++) fRec8[i] = 0;
		for (int i=0; i<2; i++) iVec8[i] = 0;
		for (int i=0; i<2; i++) fRec11[i] = 0;
		for (int i=0; i<64; i++) fVec9[i] = 0;
		for (int i=0; i<3; i++) fRec10[i] = 0;
		for (int i=0; i<2; i++) iVec10[i] = 0;
		for (int i=0; i<2; i++) fRec13[i] = 0;
		for (int i=0; i<64; i++) fVec11[i] = 0;
		for (int i=0; i<3; i++) fRec12[i] = 0;
		for (int i=0; i<2; i++) iVec12[i] = 0;
		for (int i=0; i<2; i++) fRec15[i] = 0;
		for (int i=0; i<64; i++) fVec13[i] = 0;
		for (int i=0; i<3; i++) fRec14[i] = 0;
		for (int i=0; i<2; i++) iVec14[i] = 0;
		for (int i=0; i<2; i++) fRec17[i] = 0;
		for (int i=0; i<64; i++) fVec15[i] = 0;
		for (int i=0; i<3; i++) fRec16[i] = 0;
		for (int i=0; i<2; i++) iVec16[i] = 0;
		for (int i=0; i<2; i++) fRec19[i] = 0;
		for (int i=0; i<64; i++) fVec17[i] = 0;
		for (int i=0; i<3; i++) fRec18[i] = 0;
		for (int i=0; i<2; i++) iVec18[i] = 0;
		for (int i=0; i<2; i++) fRec21[i] = 0;
		for (int i=0; i<32; i++) fVec19[i] = 0;
		for (int i=0; i<3; i++) fRec20[i] = 0;
		for (int i=0; i<2; i++) iVec20[i] = 0;
		for (int i=0; i<2; i++) fRec23[i] = 0;
		for (int i=0; i<32; i++) fVec21[i] = 0;
		for (int i=0; i<3; i++) fRec22[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("harpe");
		interface->declare(&fslider0, "osc", "/1/fader3");
		interface->addHorizontalSlider("attenuation", &fslider0, 0.005f, 0.0f, 0.01f, 0.001f);
		interface->declare(&fslider2, "osc", "/accxyz/1 -10 10");
		interface->addHorizontalSlider("hand", &fslider2, 0.0f, 0.0f, 1.0f, 0.01f);
		interface->declare(&fslider1, "osc", "/accxyz/0 -10 10");
		interface->declare(&fslider1, "unit", "f");
		interface->addHorizontalSlider("level", &fslider1, 0.5f, 0.0f, 1.0f, 0.01f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = (0.5f * (1.0f - fslider0));
		float 	fSlow1 = (4.656612875245797e-10f * faustpower<2>(fslider1));
		float 	fSlow2 = (0.09999999999999998f * fslider2);
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			iRec1[0] = (12345 + (1103515245 * iRec1[1]));
			fRec3[0] = ((0.9f * fRec3[1]) + fSlow2);
			float fTemp0 = min(fRec3[0], fRec3[1]);
			float fTemp1 = max(fRec3[0], fRec3[1]);
			int iTemp2 = ((fTemp0 < 0.045454545454545456f) & (0.045454545454545456f < fTemp1));
			iVec0[0] = iTemp2;
			fRec2[0] = ((fRec2[1] + ((iVec0[0] - iVec0[1]) > 0.0f)) - (0.009977324263038548f * (fRec2[1] > 0.0f)));
			fVec1[IOTA&127] = ((fSlow0 * (fRec0[1] + fRec0[2])) + (fSlow1 * (iRec1[0] * (fRec2[0] > 0.0f))));
			fRec0[0] = fVec1[(IOTA-99)&127];
			int iTemp3 = ((fTemp0 < 0.13636363636363635f) & (0.13636363636363635f < fTemp1));
			iVec2[0] = iTemp3;
			fRec5[0] = ((fRec5[1] + ((iVec2[0] - iVec2[1]) > 0.0f)) - (0.011460935968224386f * (fRec5[1] > 0.0f)));
			fVec3[IOTA&127] = ((fSlow0 * (fRec4[1] + fRec4[2])) + (fSlow1 * (iRec1[0] * (fRec5[0] > 0.0f))));
			fRec4[0] = fVec3[(IOTA-86)&127];
			int iTemp4 = ((fTemp0 < 0.22727272727272727f) & (0.22727272727272727f < fTemp1));
			iVec4[0] = iTemp4;
			fRec7[0] = ((fRec7[1] + ((iVec4[0] - iVec4[1]) > 0.0f)) - (0.013165158293425702f * (fRec7[1] > 0.0f)));
			fVec5[IOTA&127] = ((fSlow0 * (fRec6[1] + fRec6[2])) + (fSlow1 * (iRec1[0] * (fRec7[0] > 0.0f))));
			fRec6[0] = fVec5[(IOTA-74)&127];
			int iTemp5 = ((fTemp0 < 0.3181818181818182f) & (0.3181818181818182f < fTemp1));
			iVec6[0] = iTemp5;
			fRec9[0] = ((fRec9[1] + ((iVec6[0] - iVec6[1]) > 0.0f)) - (0.015122795674933676f * (fRec9[1] > 0.0f)));
			fVec7[IOTA&127] = ((fSlow0 * (fRec8[1] + fRec8[2])) + (fSlow1 * (iRec1[0] * (fRec9[0] > 0.0f))));
			fRec8[0] = fVec7[(IOTA-65)&127];
			int iTemp6 = ((fTemp0 < 0.4090909090909091f) & (0.4090909090909091f < fTemp1));
			iVec8[0] = iTemp6;
			fRec11[0] = ((fRec11[1] + ((iVec8[0] - iVec8[1]) > 0.0f)) - (0.01737153051475259f * (fRec11[1] > 0.0f)));
			fVec9[IOTA&63] = ((fSlow0 * (fRec10[1] + fRec10[2])) + (fSlow1 * ((fRec11[0] > 0.0f) * iRec1[0])));
			fRec10[0] = fVec9[(IOTA-56)&63];
			int iTemp7 = ((fTemp0 < 0.5f) & (0.5f < fTemp1));
			iVec10[0] = iTemp7;
			fRec13[0] = ((fRec13[1] + ((iVec10[0] - iVec10[1]) > 0.0f)) - (0.019954648526077097f * (fRec13[1] > 0.0f)));
			fVec11[IOTA&63] = ((fSlow0 * (fRec12[1] + fRec12[2])) + (fSlow1 * (iRec1[0] * (fRec13[0] > 0.0f))));
			fRec12[0] = fVec11[(IOTA-49)&63];
			float fTemp8 = (0.7071067811865476f * fRec12[0]);
			int iTemp9 = ((fTemp0 < 0.5909090909090909f) & (0.5909090909090909f < fTemp1));
			iVec12[0] = iTemp9;
			fRec15[0] = ((fRec15[1] + ((iVec12[0] - iVec12[1]) > 0.0f)) - (0.02292187193644877f * (fRec15[1] > 0.0f)));
			fVec13[IOTA&63] = ((fSlow0 * (fRec14[1] + fRec14[2])) + (fSlow1 * (iRec1[0] * (fRec15[0] > 0.0f))));
			fRec14[0] = fVec13[(IOTA-42)&63];
			int iTemp10 = ((fTemp0 < 0.6818181818181818f) & (0.6818181818181818f < fTemp1));
			iVec14[0] = iTemp10;
			fRec17[0] = ((fRec17[1] + ((iVec14[0] - iVec14[1]) > 0.0f)) - (0.026330316586851404f * (fRec17[1] > 0.0f)));
			fVec15[IOTA&63] = ((fSlow0 * (fRec16[1] + fRec16[2])) + (fSlow1 * (iRec1[0] * (fRec17[0] > 0.0f))));
			fRec16[0] = fVec15[(IOTA-36)&63];
			int iTemp11 = ((fTemp0 < 0.7727272727272727f) & (0.7727272727272727f < fTemp1));
			iVec16[0] = iTemp11;
			fRec19[0] = ((fRec19[1] + ((iVec16[0] - iVec16[1]) > 0.0f)) - (0.03024559134986736f * (fRec19[1] > 0.0f)));
			fVec17[IOTA&63] = ((fSlow0 * (fRec18[1] + fRec18[2])) + (fSlow1 * (iRec1[0] * (fRec19[0] > 0.0f))));
			fRec18[0] = fVec17[(IOTA-32)&63];
			int iTemp12 = ((fTemp0 < 0.8636363636363636f) & (0.8636363636363636f < fTemp1));
			iVec18[0] = iTemp12;
			fRec21[0] = ((fRec21[1] + ((iVec18[0] - iVec18[1]) > 0.0f)) - (0.03474306102950518f * (fRec21[1] > 0.0f)));
			fVec19[IOTA&31] = ((fSlow0 * (fRec20[1] + fRec20[2])) + (fSlow1 * (iRec1[0] * (fRec21[0] > 0.0f))));
			fRec20[0] = fVec19[(IOTA-27)&31];
			int iTemp13 = ((fTemp0 < 0.9545454545454546f) & (0.9545454545454546f < fTemp1));
			iVec20[0] = iTemp13;
			fRec23[0] = ((fRec23[1] + ((iVec20[0] - iVec20[1]) > 0.0f)) - (0.039909297052154194f * (fRec23[1] > 0.0f)));
			fVec21[IOTA&31] = ((fSlow0 * (fRec22[1] + fRec22[2])) + (fSlow1 * (iRec1[0] * (fRec23[0] > 0.0f))));
			fRec22[0] = fVec21[(IOTA-24)&31];
			output0[i] = (FAUSTFLOAT)(((((((((((0.9770084209183945f * fRec0[0]) + (0.9293203772845852f * fRec4[0])) + (0.8790490729915326f * fRec6[0])) + (0.8257228238447705f * fRec8[0])) + (0.7687061147858073f * fRec10[0])) + fTemp8) + (0.6396021490668313f * fRec14[0])) + (0.5640760748177662f * fRec16[0])) + (0.4767312946227962f * fRec18[0])) + (0.3692744729379982f * fRec20[0])) + (0.21320071635561033f * fRec22[0]));
			output1[i] = (FAUSTFLOAT)((((((fTemp8 + (((((0.21320071635561044f * fRec0[0]) + (0.3692744729379982f * fRec4[0])) + (0.4767312946227962f * fRec6[0])) + (0.5640760748177662f * fRec8[0])) + (0.6396021490668313f * fRec10[0]))) + (0.7687061147858074f * fRec14[0])) + (0.8257228238447705f * fRec16[0])) + (0.8790490729915326f * fRec18[0])) + (0.9293203772845852f * fRec20[0])) + (0.9770084209183945f * fRec22[0]));
			// post processing
			fRec22[2] = fRec22[1]; fRec22[1] = fRec22[0];
			fRec23[1] = fRec23[0];
			iVec20[1] = iVec20[0];
			fRec20[2] = fRec20[1]; fRec20[1] = fRec20[0];
			fRec21[1] = fRec21[0];
			iVec18[1] = iVec18[0];
			fRec18[2] = fRec18[1]; fRec18[1] = fRec18[0];
			fRec19[1] = fRec19[0];
			iVec16[1] = iVec16[0];
			fRec16[2] = fRec16[1]; fRec16[1] = fRec16[0];
			fRec17[1] = fRec17[0];
			iVec14[1] = iVec14[0];
			fRec14[2] = fRec14[1]; fRec14[1] = fRec14[0];
			fRec15[1] = fRec15[0];
			iVec12[1] = iVec12[0];
			fRec12[2] = fRec12[1]; fRec12[1] = fRec12[0];
			fRec13[1] = fRec13[0];
			iVec10[1] = iVec10[0];
			fRec10[2] = fRec10[1]; fRec10[1] = fRec10[0];
			fRec11[1] = fRec11[0];
			iVec8[1] = iVec8[0];
			fRec8[2] = fRec8[1]; fRec8[1] = fRec8[0];
			fRec9[1] = fRec9[0];
			iVec6[1] = iVec6[0];
			fRec6[2] = fRec6[1]; fRec6[1] = fRec6[0];
			fRec7[1] = fRec7[0];
			iVec4[1] = iVec4[0];
			fRec4[2] = fRec4[1]; fRec4[1] = fRec4[0];
			fRec5[1] = fRec5[0];
			iVec2[1] = iVec2[0];
			fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
			IOTA = IOTA+1;
			fRec2[1] = fRec2[0];
			iVec0[1] = iVec0[0];
			fRec3[1] = fRec3[0];
			iRec1[1] = iRec1[0];
		}
	}
};



#include <stdio.h>
#include <string.h>
#include "m_pd.h"

#define faust_setup(name) xfaust_setup(name)
#define xfaust_setup(name) name ## _tilde_setup(void)
#define sym(name) xsym(name)
#define xsym(name) #name

// time for "active" toggle xfades in secs
#define XFADE_TIME 0.1f

static t_class *faust_class;

struct t_faust {
  t_object x_obj;
#ifdef __MINGW32__
  /* This seems to be necessary as some as yet undetermined Pd routine seems
     to write past the end of x_obj on Windows. */
  int fence; /* dummy field (not used) */
#endif
  mydsp *dsp;
  PdUI *ui;
  string *label;
  int active, xfade, n_xfade, rate, n_in, n_out;
  t_sample **inputs, **outputs, **buf;
  t_outlet *out;
  t_sample f;
};

static t_symbol *s_button, *s_checkbox, *s_vslider, *s_hslider, *s_nentry,
  *s_vbargraph, *s_hbargraph;

static inline void zero_samples(int k, int n, t_sample **out)
{
  for (int i = 0; i < k; i++)
#ifdef __STDC_IEC_559__
    /* IEC 559 a.k.a. IEEE 754 floats can be initialized faster like this */
    memset(out[i], 0, n*sizeof(t_sample));
#else
    for (int j = 0; j < n; j++)
      out[i][j] = 0.0f;
#endif
}

static inline void copy_samples(int k, int n, t_sample **out, t_sample **in)
{
  for (int i = 0; i < k; i++)
    memcpy(out[i], in[i], n*sizeof(t_sample));
}

static t_int *faust_perform(t_int *w)
{
  t_faust *x = (t_faust *)(w[1]);
  int n = (int)(w[2]);
  if (!x->dsp || !x->buf) return (w+3);
  AVOIDDENORMALS;
  if (x->xfade > 0) {
    float d = 1.0f/x->n_xfade, f = (x->xfade--)*d;
    d = d/n;
    x->dsp->compute(n, x->inputs, x->buf);
    if (x->active)
      if (x->n_in == x->n_out)
	/* xfade inputs -> buf */
	for (int j = 0; j < n; j++, f -= d)
	  for (int i = 0; i < x->n_out; i++)
	    x->outputs[i][j] = f*x->inputs[i][j]+(1.0f-f)*x->buf[i][j];
      else
	/* xfade 0 -> buf */
	for (int j = 0; j < n; j++, f -= d)
	  for (int i = 0; i < x->n_out; i++)
	    x->outputs[i][j] = (1.0f-f)*x->buf[i][j];
    else
      if (x->n_in == x->n_out)
	/* xfade buf -> inputs */
	for (int j = 0; j < n; j++, f -= d)
	  for (int i = 0; i < x->n_out; i++)
	    x->outputs[i][j] = f*x->buf[i][j]+(1.0f-f)*x->inputs[i][j];
      else
	/* xfade buf -> 0 */
	for (int j = 0; j < n; j++, f -= d)
	  for (int i = 0; i < x->n_out; i++)
	    x->outputs[i][j] = f*x->buf[i][j];
  } else if (x->active) {
    x->dsp->compute(n, x->inputs, x->buf);
    copy_samples(x->n_out, n, x->outputs, x->buf);
  } else if (x->n_in == x->n_out) {
    copy_samples(x->n_out, n, x->buf, x->inputs);
    copy_samples(x->n_out, n, x->outputs, x->buf);
  } else
    zero_samples(x->n_out, n, x->outputs);
  return (w+3);
}

static void faust_dsp(t_faust *x, t_signal **sp)
{
  int n = sp[0]->s_n, sr = (int)sp[0]->s_sr;
  if (x->rate <= 0) {
    /* default sample rate is whatever Pd tells us */
    PdUI *ui = x->ui;
    float *z = NULL;
    if (ui->nelems > 0 &&
	(z = (float*)malloc(ui->nelems*sizeof(float)))) {
      /* save the current control values */
      for (int i = 0; i < ui->nelems; i++)
	if (ui->elems[i].zone)
	  z[i] = *ui->elems[i].zone;
    }
    /* set the proper sample rate; this requires reinitializing the dsp */
    x->rate = sr;
    x->dsp->init(sr);
    if (z) {
      /* restore previous control values */
      for (int i = 0; i < ui->nelems; i++)
	if (ui->elems[i].zone)
	  *ui->elems[i].zone = z[i];
      free(z);
    }
  }
  if (n > 0)
    x->n_xfade = (int)(x->rate*XFADE_TIME/n);
  dsp_add(faust_perform, 2, x, n);
  for (int i = 0; i < x->n_in; i++)
    x->inputs[i] = sp[i+1]->s_vec;
  for (int i = 0; i < x->n_out; i++)
    x->outputs[i] = sp[x->n_in+i+1]->s_vec;
  if (x->buf != NULL)
    for (int i = 0; i < x->n_out; i++) {
      x->buf[i] = (t_sample*)malloc(n*sizeof(t_sample));
      if (x->buf[i] == NULL) {
	for (int j = 0; j < i; j++)
	  free(x->buf[j]);
	free(x->buf);
	x->buf = NULL;
	break;
      }
    }
}

static int pathcmp(const char *s, const char *t)
{
  int n = strlen(s), m = strlen(t);
  if (n == 0 || m == 0)
    return 0;
  else if (t[0] == '/')
    return strcmp(s, t);
  else if (n <= m || s[n-m-1] != '/')
    return strcmp(s+1, t);
  else
    return strcmp(s+n-m, t);
}

static void faust_any(t_faust *x, t_symbol *s, int argc, t_atom *argv)
{
  if (!x->dsp) return;
  PdUI *ui = x->ui;
  if (s == &s_bang) {
    for (int i = 0; i < ui->nelems; i++)
      if (ui->elems[i].label && ui->elems[i].zone) {
	t_atom args[6];
	t_symbol *_s;
	switch (ui->elems[i].type) {
	case UI_BUTTON:
	  _s = s_button;
	  break;
	case UI_CHECK_BUTTON:
	  _s = s_checkbox;
	  break;
	case UI_V_SLIDER:
	  _s = s_vslider;
	  break;
	case UI_H_SLIDER:
	  _s = s_hslider;
	  break;
	case UI_NUM_ENTRY:
	  _s = s_nentry;
	  break;
	case UI_V_BARGRAPH:
	  _s = s_vbargraph;
	  break;
	case UI_H_BARGRAPH:
	  _s = s_hbargraph;
	  break;
	default:
	  continue;
	}
	SETSYMBOL(&args[0], gensym(ui->elems[i].label));
	SETFLOAT(&args[1], *ui->elems[i].zone);
	SETFLOAT(&args[2], ui->elems[i].init);
	SETFLOAT(&args[3], ui->elems[i].min);
	SETFLOAT(&args[4], ui->elems[i].max);
	SETFLOAT(&args[5], ui->elems[i].step);
	outlet_anything(x->out, _s, 6, args);
      }
  } else {
    const char *label = s->s_name;
    int count = 0;
    for (int i = 0; i < ui->nelems; i++)
      if (ui->elems[i].label &&
	  pathcmp(ui->elems[i].label, label) == 0) {
	if (argc == 0) {
	  if (ui->elems[i].zone) {
	    t_atom arg;
	    SETFLOAT(&arg, *ui->elems[i].zone);
	    outlet_anything(x->out, gensym(ui->elems[i].label), 1, &arg);
	  }
	  ++count;
	} else if (argc == 1 &&
		   (argv[0].a_type == A_FLOAT ||
		    argv[0].a_type == A_DEFFLOAT) &&
		   ui->elems[i].zone) {
	  float f = atom_getfloat(argv);
	  *ui->elems[i].zone = f;
	  ++count;
	} else
	  pd_error(x, "[faust] %s: bad control argument: %s",
		   x->label->c_str(), label);
      }
    if (count == 0 && strcmp(label, "active") == 0) {
      if (argc == 0) {
	t_atom arg;
	SETFLOAT(&arg, (float)x->active);
	outlet_anything(x->out, gensym((char*)"active"), 1, &arg);
      } else if (argc == 1 &&
		 (argv[0].a_type == A_FLOAT ||
		  argv[0].a_type == A_DEFFLOAT)) {
	float f = atom_getfloat(argv);
	x->active = (int)f;
	x->xfade = x->n_xfade;
      }
    }
  }
}

static void faust_free(t_faust *x)
{
  if (x->label) delete x->label;
  if (x->dsp) delete x->dsp;
  if (x->ui) delete x->ui;
  if (x->inputs) free(x->inputs);
  if (x->outputs) free(x->outputs);
  if (x->buf) {
    for (int i = 0; i < x->n_out; i++)
      if (x->buf[i]) free(x->buf[i]);
    free(x->buf);
  }
}

static void *faust_new(t_symbol *s, int argc, t_atom *argv)
{
  t_faust *x = (t_faust*)pd_new(faust_class);
  int sr = -1;
  t_symbol *id = NULL;
  x->active = 1;
  for (int i = 0; i < argc; i++)
    if (argv[i].a_type == A_FLOAT || argv[i].a_type == A_DEFFLOAT)
      sr = (int)argv[i].a_w.w_float;
    else if (argv[i].a_type == A_SYMBOL || argv[i].a_type == A_DEFSYMBOL)
      id = argv[i].a_w.w_symbol;
  x->rate = sr;
  if (sr <= 0) sr = 44100;
  x->xfade = 0; x->n_xfade = (int)(sr*XFADE_TIME/64);
  x->inputs = x->outputs = x->buf = NULL;
  x->label = new string(sym(mydsp) "~");
  x->dsp = new mydsp();
  x->ui = new PdUI(id?id->s_name:NULL);
  if (!x->dsp || !x->ui || !x->label) goto error;
  if (id) {
    *x->label += " ";
    *x->label += id->s_name;
  }
  x->n_in = x->dsp->getNumInputs();
  x->n_out = x->dsp->getNumOutputs();
  if (x->n_in > 0)
    x->inputs = (t_sample**)malloc(x->n_in*sizeof(t_sample*));
  if (x->n_out > 0) {
    x->outputs = (t_sample**)malloc(x->n_out*sizeof(t_sample*));
    x->buf = (t_sample**)malloc(x->n_out*sizeof(t_sample*));
  }
  if ((x->n_in > 0 && x->inputs == NULL) ||
      (x->n_out > 0 && (x->outputs == NULL || x->buf == NULL)))
    goto error;
  for (int i = 0; i < x->n_out; i++)
    x->buf[i] = NULL;
  x->dsp->init(sr);
  x->dsp->buildUserInterface(x->ui);
  for (int i = 0; i < x->n_in; i++)
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->out = outlet_new(&x->x_obj, 0);
  for (int i = 0; i < x->n_out; i++)
    outlet_new(&x->x_obj, &s_signal);
  return (void *)x;
 error:
  faust_free(x);
  x->dsp = NULL; x->ui = NULL;
  x->inputs = x->outputs = x->buf = NULL;
  return (void *)x;
}

extern "C" void faust_setup(mydsp)
{
  t_symbol *s = gensym(sym(mydsp) "~");
  faust_class =
    class_new(s, (t_newmethod)faust_new, (t_method)faust_free,
	      sizeof(t_faust), CLASS_DEFAULT,
	      A_GIMME, A_NULL);
  class_addmethod(faust_class, (t_method)faust_dsp, gensym((char*)"dsp"), A_NULL);
  class_addanything(faust_class, faust_any);
  class_addmethod(faust_class, nullfn, &s_signal, A_NULL);
  s_button = gensym((char*)"button");
  s_checkbox = gensym((char*)"checkbox");
  s_vslider = gensym((char*)"vslider");
  s_hslider = gensym((char*)"hslider");
  s_nentry = gensym((char*)"nentry");
  s_vbargraph = gensym((char*)"vbargraph");
  s_hbargraph = gensym((char*)"hbargrap");
  /* give some indication that we're loaded and ready to go */
  mydsp dsp = mydsp();
  post("[faust] %s: %d inputs, %d outputs", sym(mydsp) "~",
       dsp.getNumInputs(), dsp.getNumOutputs());
}
