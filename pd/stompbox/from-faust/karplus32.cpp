//-----------------------------------------------------
// name: "karplus32"
// version: "1.0"
// author: "Grame"
// license: "BSD"
// copyright: "(c)GRAME 2006"
//
// Code generated with Faust 0.9.58 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
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
	FAUSTFLOAT 	fslider2;
	FAUSTFLOAT 	fslider3;
	int 	iRec1[2];
	FAUSTFLOAT 	fbutton0;
	float 	fVec0[2];
	FAUSTFLOAT 	fslider4;
	float 	fRec2[2];
	int 	IOTA;
	float 	fVec1[512];
	FAUSTFLOAT 	fslider5;
	float 	fRec0[3];
	float 	fVec2[2048];
	FAUSTFLOAT 	fslider6;
	float 	fRec3[3];
	float 	fVec3[4096];
	float 	fRec4[3];
	float 	fVec4[4096];
	float 	fRec5[3];
	float 	fVec5[4096];
	float 	fRec6[3];
	float 	fVec6[4096];
	float 	fRec7[3];
	float 	fVec7[4096];
	float 	fRec8[3];
	float 	fVec8[4096];
	float 	fRec9[3];
	float 	fVec9[4096];
	float 	fRec10[3];
	float 	fVec10[4096];
	float 	fRec11[3];
	float 	fVec11[4096];
	float 	fRec12[3];
	float 	fVec12[4096];
	float 	fRec13[3];
	float 	fVec13[4096];
	float 	fRec14[3];
	float 	fVec14[4096];
	float 	fRec15[3];
	float 	fVec15[4096];
	float 	fRec16[3];
	float 	fVec16[4096];
	float 	fRec17[3];
	float 	fVec17[1024];
	float 	fRec18[3];
	float 	fVec18[2048];
	float 	fRec19[3];
	float 	fVec19[4096];
	float 	fRec20[3];
	float 	fVec20[4096];
	float 	fRec21[3];
	float 	fVec21[4096];
	float 	fRec22[3];
	float 	fVec22[4096];
	float 	fRec23[3];
	float 	fVec23[4096];
	float 	fRec24[3];
	float 	fVec24[4096];
	float 	fRec25[3];
	float 	fVec25[4096];
	float 	fRec26[3];
	float 	fVec26[4096];
	float 	fRec27[3];
	float 	fVec27[4096];
	float 	fRec28[3];
	float 	fVec28[4096];
	float 	fRec29[3];
	float 	fVec29[4096];
	float 	fRec30[3];
	float 	fVec30[4096];
	float 	fRec31[3];
	float 	fVec31[4096];
	float 	fRec32[3];
	float 	fVec32[4096];
	float 	fRec33[3];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "karplus32");
		m->declare("version", "1.0");
		m->declare("author", "Grame");
		m->declare("license", "BSD");
		m->declare("copyright", "(c)GRAME 2006");
		m->declare("music.lib/name", "Music Library");
		m->declare("music.lib/author", "GRAME");
		m->declare("music.lib/copyright", "GRAME");
		m->declare("music.lib/version", "1.0");
		m->declare("music.lib/license", "LGPL with exception");
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL with exception");
	}

	virtual int getNumInputs() 	{ return 0; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 0.5f;
		fslider1 = 1.0f;
		fslider2 = 0.1f;
		fslider3 = 0.5f;
		for (int i=0; i<2; i++) iRec1[i] = 0;
		fbutton0 = 0.0;
		for (int i=0; i<2; i++) fVec0[i] = 0;
		fslider4 = 128.0f;
		for (int i=0; i<2; i++) fRec2[i] = 0;
		IOTA = 0;
		for (int i=0; i<512; i++) fVec1[i] = 0;
		fslider5 = 128.0f;
		for (int i=0; i<3; i++) fRec0[i] = 0;
		for (int i=0; i<2048; i++) fVec2[i] = 0;
		fslider6 = 32.0f;
		for (int i=0; i<3; i++) fRec3[i] = 0;
		for (int i=0; i<4096; i++) fVec3[i] = 0;
		for (int i=0; i<3; i++) fRec4[i] = 0;
		for (int i=0; i<4096; i++) fVec4[i] = 0;
		for (int i=0; i<3; i++) fRec5[i] = 0;
		for (int i=0; i<4096; i++) fVec5[i] = 0;
		for (int i=0; i<3; i++) fRec6[i] = 0;
		for (int i=0; i<4096; i++) fVec6[i] = 0;
		for (int i=0; i<3; i++) fRec7[i] = 0;
		for (int i=0; i<4096; i++) fVec7[i] = 0;
		for (int i=0; i<3; i++) fRec8[i] = 0;
		for (int i=0; i<4096; i++) fVec8[i] = 0;
		for (int i=0; i<3; i++) fRec9[i] = 0;
		for (int i=0; i<4096; i++) fVec9[i] = 0;
		for (int i=0; i<3; i++) fRec10[i] = 0;
		for (int i=0; i<4096; i++) fVec10[i] = 0;
		for (int i=0; i<3; i++) fRec11[i] = 0;
		for (int i=0; i<4096; i++) fVec11[i] = 0;
		for (int i=0; i<3; i++) fRec12[i] = 0;
		for (int i=0; i<4096; i++) fVec12[i] = 0;
		for (int i=0; i<3; i++) fRec13[i] = 0;
		for (int i=0; i<4096; i++) fVec13[i] = 0;
		for (int i=0; i<3; i++) fRec14[i] = 0;
		for (int i=0; i<4096; i++) fVec14[i] = 0;
		for (int i=0; i<3; i++) fRec15[i] = 0;
		for (int i=0; i<4096; i++) fVec15[i] = 0;
		for (int i=0; i<3; i++) fRec16[i] = 0;
		for (int i=0; i<4096; i++) fVec16[i] = 0;
		for (int i=0; i<3; i++) fRec17[i] = 0;
		for (int i=0; i<1024; i++) fVec17[i] = 0;
		for (int i=0; i<3; i++) fRec18[i] = 0;
		for (int i=0; i<2048; i++) fVec18[i] = 0;
		for (int i=0; i<3; i++) fRec19[i] = 0;
		for (int i=0; i<4096; i++) fVec19[i] = 0;
		for (int i=0; i<3; i++) fRec20[i] = 0;
		for (int i=0; i<4096; i++) fVec20[i] = 0;
		for (int i=0; i<3; i++) fRec21[i] = 0;
		for (int i=0; i<4096; i++) fVec21[i] = 0;
		for (int i=0; i<3; i++) fRec22[i] = 0;
		for (int i=0; i<4096; i++) fVec22[i] = 0;
		for (int i=0; i<3; i++) fRec23[i] = 0;
		for (int i=0; i<4096; i++) fVec23[i] = 0;
		for (int i=0; i<3; i++) fRec24[i] = 0;
		for (int i=0; i<4096; i++) fVec24[i] = 0;
		for (int i=0; i<3; i++) fRec25[i] = 0;
		for (int i=0; i<4096; i++) fVec25[i] = 0;
		for (int i=0; i<3; i++) fRec26[i] = 0;
		for (int i=0; i<4096; i++) fVec26[i] = 0;
		for (int i=0; i<3; i++) fRec27[i] = 0;
		for (int i=0; i<4096; i++) fVec27[i] = 0;
		for (int i=0; i<3; i++) fRec28[i] = 0;
		for (int i=0; i<4096; i++) fVec28[i] = 0;
		for (int i=0; i<3; i++) fRec29[i] = 0;
		for (int i=0; i<4096; i++) fVec29[i] = 0;
		for (int i=0; i<3; i++) fRec30[i] = 0;
		for (int i=0; i<4096; i++) fVec30[i] = 0;
		for (int i=0; i<3; i++) fRec31[i] = 0;
		for (int i=0; i<4096; i++) fVec31[i] = 0;
		for (int i=0; i<3; i++) fRec32[i] = 0;
		for (int i=0; i<4096; i++) fVec32[i] = 0;
		for (int i=0; i<3; i++) fRec33[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("karplus32");
		interface->openVerticalBox("excitator");
		interface->addHorizontalSlider("excitation (samples)", &fslider4, 128.0f, 2.0f, 512.0f, 1.0f);
		interface->addButton("play", &fbutton0);
		interface->closeBox();
		interface->openVerticalBox("noise generator");
		interface->addHorizontalSlider("level", &fslider3, 0.5f, 0.0f, 1.0f, 0.1f);
		interface->closeBox();
		interface->addHorizontalSlider("output volume", &fslider0, 0.5f, 0.0f, 1.0f, 0.1f);
		interface->openVerticalBox("resonator x32");
		interface->addHorizontalSlider("attenuation", &fslider2, 0.1f, 0.0f, 1.0f, 0.01f);
		interface->addHorizontalSlider("detune", &fslider6, 32.0f, 0.0f, 512.0f, 1.0f);
		interface->addHorizontalSlider("duration (samples)", &fslider5, 128.0f, 2.0f, 512.0f, 1.0f);
		interface->addHorizontalSlider("polyphony", &fslider1, 1.0f, 0.0f, 32.0f, 1.0f);
		interface->closeBox();
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fslider0;
		float 	fSlow1 = fslider1;
		int 	iSlow2 = (fSlow1 > 0);
		float 	fSlow3 = (0.5f * (1.0f - fslider2));
		float 	fSlow4 = (4.656612875245797e-10f * fslider3);
		float 	fSlow5 = fbutton0;
		float 	fSlow6 = (1.0f / fslider4);
		float 	fSlow7 = fslider5;
		int 	iSlow8 = int((int((fSlow7 - 1.5f)) & 4095));
		int 	iSlow9 = (fSlow1 > 2);
		float 	fSlow10 = fslider6;
		int 	iSlow11 = int((int(((fSlow7 + (2 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow12 = (fSlow1 > 4);
		int 	iSlow13 = int((int(((fSlow7 + (4 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow14 = (fSlow1 > 6);
		int 	iSlow15 = int((int(((fSlow7 + (6 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow16 = (fSlow1 > 8);
		int 	iSlow17 = int((int(((fSlow7 + (8 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow18 = (fSlow1 > 10);
		int 	iSlow19 = int((int(((fSlow7 + (10 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow20 = (fSlow1 > 12);
		int 	iSlow21 = int((int(((fSlow7 + (12 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow22 = (fSlow1 > 14);
		int 	iSlow23 = int((int(((fSlow7 + (14 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow24 = (fSlow1 > 16);
		int 	iSlow25 = int((int(((fSlow7 + (16 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow26 = (fSlow1 > 18);
		int 	iSlow27 = int((int(((fSlow7 + (18 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow28 = (fSlow1 > 20);
		int 	iSlow29 = int((int(((fSlow7 + (20 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow30 = (fSlow1 > 22);
		int 	iSlow31 = int((int(((fSlow7 + (22 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow32 = (fSlow1 > 24);
		int 	iSlow33 = int((int(((fSlow7 + (24 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow34 = (fSlow1 > 26);
		int 	iSlow35 = int((int(((fSlow7 + (26 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow36 = (fSlow1 > 28);
		int 	iSlow37 = int((int(((fSlow7 + (28 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow38 = (fSlow1 > 30);
		int 	iSlow39 = int((int(((fSlow7 + (30 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow40 = (fSlow1 > 1);
		int 	iSlow41 = int((int(((fSlow7 + fSlow10) - 1.5f)) & 4095));
		int 	iSlow42 = (fSlow1 > 3);
		int 	iSlow43 = int((int(((fSlow7 + (3 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow44 = (fSlow1 > 5);
		int 	iSlow45 = int((int(((fSlow7 + (5 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow46 = (fSlow1 > 7);
		int 	iSlow47 = int((int(((fSlow7 + (7 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow48 = (fSlow1 > 9);
		int 	iSlow49 = int((int(((fSlow7 + (9 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow50 = (fSlow1 > 11);
		int 	iSlow51 = int((int(((fSlow7 + (11 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow52 = (fSlow1 > 13);
		int 	iSlow53 = int((int(((fSlow7 + (13 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow54 = (fSlow1 > 15);
		int 	iSlow55 = int((int(((fSlow7 + (15 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow56 = (fSlow1 > 17);
		int 	iSlow57 = int((int(((fSlow7 + (17 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow58 = (fSlow1 > 19);
		int 	iSlow59 = int((int(((fSlow7 + (19 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow60 = (fSlow1 > 21);
		int 	iSlow61 = int((int(((fSlow7 + (21 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow62 = (fSlow1 > 23);
		int 	iSlow63 = int((int(((fSlow7 + (23 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow64 = (fSlow1 > 25);
		int 	iSlow65 = int((int(((fSlow7 + (25 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow66 = (fSlow1 > 27);
		int 	iSlow67 = int((int(((fSlow7 + (27 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow68 = (fSlow1 > 29);
		int 	iSlow69 = int((int(((fSlow7 + (29 * fSlow10)) - 1.5f)) & 4095));
		int 	iSlow70 = (fSlow1 > 31);
		int 	iSlow71 = int((int(((fSlow7 + (31 * fSlow10)) - 1.5f)) & 4095));
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			iRec1[0] = (12345 + (1103515245 * iRec1[1]));
			fVec0[0] = fSlow5;
			fRec2[0] = ((fRec2[1] + ((fSlow5 - fVec0[1]) > 0.0f)) - (fSlow6 * (fRec2[1] > 0)));
			float fTemp0 = (fSlow4 * (iRec1[0] * ((fRec2[0] > 0.0f) + 1.52587890625e-05f)));
			fVec1[IOTA&511] = ((fSlow3 * (fRec0[1] + fRec0[2])) + fTemp0);
			fRec0[0] = fVec1[(IOTA-iSlow8)&511];
			fVec2[IOTA&2047] = (fTemp0 + (fSlow3 * (fRec3[1] + fRec3[2])));
			fRec3[0] = fVec2[(IOTA-iSlow11)&2047];
			fVec3[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec4[1] + fRec4[2])));
			fRec4[0] = fVec3[(IOTA-iSlow13)&4095];
			fVec4[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec5[1] + fRec5[2])));
			fRec5[0] = fVec4[(IOTA-iSlow15)&4095];
			fVec5[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec6[1] + fRec6[2])));
			fRec6[0] = fVec5[(IOTA-iSlow17)&4095];
			fVec6[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec7[1] + fRec7[2])));
			fRec7[0] = fVec6[(IOTA-iSlow19)&4095];
			fVec7[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec8[1] + fRec8[2])));
			fRec8[0] = fVec7[(IOTA-iSlow21)&4095];
			fVec8[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec9[1] + fRec9[2])));
			fRec9[0] = fVec8[(IOTA-iSlow23)&4095];
			fVec9[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec10[1] + fRec10[2])));
			fRec10[0] = fVec9[(IOTA-iSlow25)&4095];
			fVec10[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec11[1] + fRec11[2])));
			fRec11[0] = fVec10[(IOTA-iSlow27)&4095];
			fVec11[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec12[1] + fRec12[2])));
			fRec12[0] = fVec11[(IOTA-iSlow29)&4095];
			fVec12[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec13[1] + fRec13[2])));
			fRec13[0] = fVec12[(IOTA-iSlow31)&4095];
			fVec13[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec14[1] + fRec14[2])));
			fRec14[0] = fVec13[(IOTA-iSlow33)&4095];
			fVec14[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec15[1] + fRec15[2])));
			fRec15[0] = fVec14[(IOTA-iSlow35)&4095];
			fVec15[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec16[1] + fRec16[2])));
			fRec16[0] = fVec15[(IOTA-iSlow37)&4095];
			fVec16[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec17[1] + fRec17[2])));
			fRec17[0] = fVec16[(IOTA-iSlow39)&4095];
			output0[i] = (FAUSTFLOAT)(fSlow0 * ((((((((((((((((iSlow2 * fRec0[0]) + (iSlow9 * fRec3[0])) + (iSlow12 * fRec4[0])) + (iSlow14 * fRec5[0])) + (iSlow16 * fRec6[0])) + (iSlow18 * fRec7[0])) + (iSlow20 * fRec8[0])) + (iSlow22 * fRec9[0])) + (iSlow24 * fRec10[0])) + (iSlow26 * fRec11[0])) + (iSlow28 * fRec12[0])) + (iSlow30 * fRec13[0])) + (iSlow32 * fRec14[0])) + (iSlow34 * fRec15[0])) + (iSlow36 * fRec16[0])) + (iSlow38 * fRec17[0])));
			fVec17[IOTA&1023] = (fTemp0 + (fSlow3 * (fRec18[1] + fRec18[2])));
			fRec18[0] = fVec17[(IOTA-iSlow41)&1023];
			fVec18[IOTA&2047] = (fTemp0 + (fSlow3 * (fRec19[1] + fRec19[2])));
			fRec19[0] = fVec18[(IOTA-iSlow43)&2047];
			fVec19[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec20[1] + fRec20[2])));
			fRec20[0] = fVec19[(IOTA-iSlow45)&4095];
			fVec20[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec21[1] + fRec21[2])));
			fRec21[0] = fVec20[(IOTA-iSlow47)&4095];
			fVec21[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec22[1] + fRec22[2])));
			fRec22[0] = fVec21[(IOTA-iSlow49)&4095];
			fVec22[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec23[1] + fRec23[2])));
			fRec23[0] = fVec22[(IOTA-iSlow51)&4095];
			fVec23[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec24[1] + fRec24[2])));
			fRec24[0] = fVec23[(IOTA-iSlow53)&4095];
			fVec24[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec25[1] + fRec25[2])));
			fRec25[0] = fVec24[(IOTA-iSlow55)&4095];
			fVec25[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec26[1] + fRec26[2])));
			fRec26[0] = fVec25[(IOTA-iSlow57)&4095];
			fVec26[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec27[1] + fRec27[2])));
			fRec27[0] = fVec26[(IOTA-iSlow59)&4095];
			fVec27[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec28[1] + fRec28[2])));
			fRec28[0] = fVec27[(IOTA-iSlow61)&4095];
			fVec28[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec29[1] + fRec29[2])));
			fRec29[0] = fVec28[(IOTA-iSlow63)&4095];
			fVec29[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec30[1] + fRec30[2])));
			fRec30[0] = fVec29[(IOTA-iSlow65)&4095];
			fVec30[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec31[1] + fRec31[2])));
			fRec31[0] = fVec30[(IOTA-iSlow67)&4095];
			fVec31[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec32[1] + fRec32[2])));
			fRec32[0] = fVec31[(IOTA-iSlow69)&4095];
			fVec32[IOTA&4095] = (fTemp0 + (fSlow3 * (fRec33[1] + fRec33[2])));
			fRec33[0] = fVec32[(IOTA-iSlow71)&4095];
			output1[i] = (FAUSTFLOAT)(fSlow0 * ((((((((((((((((iSlow40 * fRec18[0]) + (iSlow42 * fRec19[0])) + (iSlow44 * fRec20[0])) + (iSlow46 * fRec21[0])) + (iSlow48 * fRec22[0])) + (iSlow50 * fRec23[0])) + (iSlow52 * fRec24[0])) + (iSlow54 * fRec25[0])) + (iSlow56 * fRec26[0])) + (iSlow58 * fRec27[0])) + (iSlow60 * fRec28[0])) + (iSlow62 * fRec29[0])) + (iSlow64 * fRec30[0])) + (iSlow66 * fRec31[0])) + (iSlow68 * fRec32[0])) + (iSlow70 * fRec33[0])));
			// post processing
			fRec33[2] = fRec33[1]; fRec33[1] = fRec33[0];
			fRec32[2] = fRec32[1]; fRec32[1] = fRec32[0];
			fRec31[2] = fRec31[1]; fRec31[1] = fRec31[0];
			fRec30[2] = fRec30[1]; fRec30[1] = fRec30[0];
			fRec29[2] = fRec29[1]; fRec29[1] = fRec29[0];
			fRec28[2] = fRec28[1]; fRec28[1] = fRec28[0];
			fRec27[2] = fRec27[1]; fRec27[1] = fRec27[0];
			fRec26[2] = fRec26[1]; fRec26[1] = fRec26[0];
			fRec25[2] = fRec25[1]; fRec25[1] = fRec25[0];
			fRec24[2] = fRec24[1]; fRec24[1] = fRec24[0];
			fRec23[2] = fRec23[1]; fRec23[1] = fRec23[0];
			fRec22[2] = fRec22[1]; fRec22[1] = fRec22[0];
			fRec21[2] = fRec21[1]; fRec21[1] = fRec21[0];
			fRec20[2] = fRec20[1]; fRec20[1] = fRec20[0];
			fRec19[2] = fRec19[1]; fRec19[1] = fRec19[0];
			fRec18[2] = fRec18[1]; fRec18[1] = fRec18[0];
			fRec17[2] = fRec17[1]; fRec17[1] = fRec17[0];
			fRec16[2] = fRec16[1]; fRec16[1] = fRec16[0];
			fRec15[2] = fRec15[1]; fRec15[1] = fRec15[0];
			fRec14[2] = fRec14[1]; fRec14[1] = fRec14[0];
			fRec13[2] = fRec13[1]; fRec13[1] = fRec13[0];
			fRec12[2] = fRec12[1]; fRec12[1] = fRec12[0];
			fRec11[2] = fRec11[1]; fRec11[1] = fRec11[0];
			fRec10[2] = fRec10[1]; fRec10[1] = fRec10[0];
			fRec9[2] = fRec9[1]; fRec9[1] = fRec9[0];
			fRec8[2] = fRec8[1]; fRec8[1] = fRec8[0];
			fRec7[2] = fRec7[1]; fRec7[1] = fRec7[0];
			fRec6[2] = fRec6[1]; fRec6[1] = fRec6[0];
			fRec5[2] = fRec5[1]; fRec5[1] = fRec5[0];
			fRec4[2] = fRec4[1]; fRec4[1] = fRec4[0];
			fRec3[2] = fRec3[1]; fRec3[1] = fRec3[0];
			fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
			IOTA = IOTA+1;
			fRec2[1] = fRec2[0];
			fVec0[1] = fVec0[0];
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
