//-----------------------------------------------------
// name: "multibandfilter"
// version: "1.0"
// author: "Grame"
// license: "BSD"
// copyright: "(c)GRAME 2006"
//
// Code generated with Faust 0.9.58 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
#include <math.h>
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
	float 	fConst0;
	FAUSTFLOAT 	fentry0;
	FAUSTFLOAT 	fslider0;
	FAUSTFLOAT 	fentry1;
	FAUSTFLOAT 	fentry2;
	FAUSTFLOAT 	fslider1;
	FAUSTFLOAT 	fentry3;
	FAUSTFLOAT 	fentry4;
	FAUSTFLOAT 	fslider2;
	FAUSTFLOAT 	fentry5;
	FAUSTFLOAT 	fentry6;
	FAUSTFLOAT 	fslider3;
	FAUSTFLOAT 	fentry7;
	FAUSTFLOAT 	fentry8;
	FAUSTFLOAT 	fslider4;
	FAUSTFLOAT 	fentry9;
	FAUSTFLOAT 	fentry10;
	FAUSTFLOAT 	fslider5;
	FAUSTFLOAT 	fentry11;
	FAUSTFLOAT 	fentry12;
	FAUSTFLOAT 	fslider6;
	FAUSTFLOAT 	fentry13;
	FAUSTFLOAT 	fentry14;
	FAUSTFLOAT 	fslider7;
	FAUSTFLOAT 	fentry15;
	FAUSTFLOAT 	fentry16;
	FAUSTFLOAT 	fslider8;
	FAUSTFLOAT 	fentry17;
	FAUSTFLOAT 	fentry18;
	FAUSTFLOAT 	fslider9;
	FAUSTFLOAT 	fentry19;
	float 	fRec9[3];
	float 	fRec8[3];
	float 	fRec7[3];
	float 	fRec6[3];
	float 	fRec5[3];
	float 	fRec4[3];
	float 	fRec3[3];
	float 	fRec2[3];
	float 	fRec1[3];
	float 	fRec0[3];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "multibandfilter");
		m->declare("version", "1.0");
		m->declare("author", "Grame");
		m->declare("license", "BSD");
		m->declare("copyright", "(c)GRAME 2006");
		m->declare("bandfilter.dsp/name", "bandfilter");
		m->declare("bandfilter.dsp/version", "1.0");
		m->declare("bandfilter.dsp/author", "Grame");
		m->declare("bandfilter.dsp/license", "BSD");
		m->declare("bandfilter.dsp/copyright", "(c)GRAME 2006");
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL with exception");
		m->declare("music.lib/name", "Music Library");
		m->declare("music.lib/author", "GRAME");
		m->declare("music.lib/copyright", "GRAME");
		m->declare("music.lib/version", "1.0");
		m->declare("music.lib/license", "LGPL with exception");
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fConst0 = (3.141592653589793f / float(min(192000, max(1, fSamplingFreq))));
		fentry0 = 1e+04f;
		fslider0 = 0.0f;
		fentry1 = 5e+01f;
		fentry2 = 9e+03f;
		fslider1 = 0.0f;
		fentry3 = 5e+01f;
		fentry4 = 8e+03f;
		fslider2 = 0.0f;
		fentry5 = 5e+01f;
		fentry6 = 7e+03f;
		fslider3 = 0.0f;
		fentry7 = 5e+01f;
		fentry8 = 6e+03f;
		fslider4 = 0.0f;
		fentry9 = 5e+01f;
		fentry10 = 5e+03f;
		fslider5 = 0.0f;
		fentry11 = 5e+01f;
		fentry12 = 4e+03f;
		fslider6 = 0.0f;
		fentry13 = 5e+01f;
		fentry14 = 3e+03f;
		fslider7 = 0.0f;
		fentry15 = 5e+01f;
		fentry16 = 2e+03f;
		fslider8 = 0.0f;
		fentry17 = 5e+01f;
		fentry18 = 1e+03f;
		fslider9 = 0.0f;
		fentry19 = 5e+01f;
		for (int i=0; i<3; i++) fRec9[i] = 0;
		for (int i=0; i<3; i++) fRec8[i] = 0;
		for (int i=0; i<3; i++) fRec7[i] = 0;
		for (int i=0; i<3; i++) fRec6[i] = 0;
		for (int i=0; i<3; i++) fRec5[i] = 0;
		for (int i=0; i<3; i++) fRec4[i] = 0;
		for (int i=0; i<3; i++) fRec3[i] = 0;
		for (int i=0; i<3; i++) fRec2[i] = 0;
		for (int i=0; i<3; i++) fRec1[i] = 0;
		for (int i=0; i<3; i++) fRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openHorizontalBox("Multi Band Filter");
		interface->openVerticalBox("peak 0");
		interface->declare(&fentry19, "style", "knob");
		interface->addNumEntry("Q factor", &fentry19, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry18, "style", "knob");
		interface->declare(&fentry18, "unit", "Hz");
		interface->addNumEntry("freq", &fentry18, 1e+03f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider9, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider9, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->openVerticalBox("peak 1");
		interface->declare(&fentry17, "style", "knob");
		interface->addNumEntry("Q factor", &fentry17, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry16, "style", "knob");
		interface->declare(&fentry16, "unit", "Hz");
		interface->addNumEntry("freq", &fentry16, 2e+03f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider8, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider8, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->openVerticalBox("peak 2");
		interface->declare(&fentry15, "style", "knob");
		interface->addNumEntry("Q factor", &fentry15, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry14, "style", "knob");
		interface->declare(&fentry14, "unit", "Hz");
		interface->addNumEntry("freq", &fentry14, 3e+03f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider7, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider7, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->openVerticalBox("peak 3");
		interface->declare(&fentry13, "style", "knob");
		interface->addNumEntry("Q factor", &fentry13, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry12, "style", "knob");
		interface->declare(&fentry12, "unit", "Hz");
		interface->addNumEntry("freq", &fentry12, 4e+03f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider6, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider6, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->openVerticalBox("peak 4");
		interface->declare(&fentry11, "style", "knob");
		interface->addNumEntry("Q factor", &fentry11, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry10, "style", "knob");
		interface->declare(&fentry10, "unit", "Hz");
		interface->addNumEntry("freq", &fentry10, 5e+03f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider5, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider5, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->openVerticalBox("peak 5");
		interface->declare(&fentry9, "style", "knob");
		interface->addNumEntry("Q factor", &fentry9, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry8, "style", "knob");
		interface->declare(&fentry8, "unit", "Hz");
		interface->addNumEntry("freq", &fentry8, 6e+03f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider4, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider4, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->openVerticalBox("peak 6");
		interface->declare(&fentry7, "style", "knob");
		interface->addNumEntry("Q factor", &fentry7, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry6, "style", "knob");
		interface->declare(&fentry6, "unit", "Hz");
		interface->addNumEntry("freq", &fentry6, 7e+03f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider3, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider3, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->openVerticalBox("peak 7");
		interface->declare(&fentry5, "style", "knob");
		interface->addNumEntry("Q factor", &fentry5, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry4, "style", "knob");
		interface->declare(&fentry4, "unit", "Hz");
		interface->addNumEntry("freq", &fentry4, 8e+03f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider2, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider2, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->openVerticalBox("peak 8");
		interface->declare(&fentry3, "style", "knob");
		interface->addNumEntry("Q factor", &fentry3, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry2, "style", "knob");
		interface->declare(&fentry2, "unit", "Hz");
		interface->addNumEntry("freq", &fentry2, 9e+03f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider1, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider1, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->openVerticalBox("peak 9");
		interface->declare(&fentry1, "style", "knob");
		interface->addNumEntry("Q factor", &fentry1, 5e+01f, 0.1f, 1e+02f, 0.1f);
		interface->declare(&fentry0, "style", "knob");
		interface->declare(&fentry0, "unit", "Hz");
		interface->addNumEntry("freq", &fentry0, 1e+04f, 2e+01f, 2e+04f, 1.0f);
		interface->declare(&fslider0, "unit", "dB");
		interface->addVerticalSlider("gain", &fslider0, 0.0f, -5e+01f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = tanf((fConst0 * fentry0));
		float 	fSlow1 = fentry1;
		float 	fSlow2 = (powf(10,(0.05f * (0 - fslider0))) / fSlow1);
		float 	fSlow3 = (1.0f / (1 + (fSlow0 * (fSlow0 + fSlow2))));
		float 	fSlow4 = (2 * (faustpower<2>(fSlow0) - 1));
		float 	fSlow5 = tanf((fConst0 * fentry2));
		float 	fSlow6 = fentry3;
		float 	fSlow7 = (powf(10,(0.05f * (0 - fslider1))) / fSlow6);
		float 	fSlow8 = (1.0f / (1 + (fSlow5 * (fSlow5 + fSlow7))));
		float 	fSlow9 = (2 * (faustpower<2>(fSlow5) - 1));
		float 	fSlow10 = tanf((fConst0 * fentry4));
		float 	fSlow11 = fentry5;
		float 	fSlow12 = (powf(10,(0.05f * (0 - fslider2))) / fSlow11);
		float 	fSlow13 = (1.0f / (1 + (fSlow10 * (fSlow10 + fSlow12))));
		float 	fSlow14 = (2 * (faustpower<2>(fSlow10) - 1));
		float 	fSlow15 = tanf((fConst0 * fentry6));
		float 	fSlow16 = fentry7;
		float 	fSlow17 = (powf(10,(0.05f * (0 - fslider3))) / fSlow16);
		float 	fSlow18 = (1.0f / (1 + (fSlow15 * (fSlow15 + fSlow17))));
		float 	fSlow19 = (2 * (faustpower<2>(fSlow15) - 1));
		float 	fSlow20 = tanf((fConst0 * fentry8));
		float 	fSlow21 = fentry9;
		float 	fSlow22 = (powf(10,(0.05f * (0 - fslider4))) / fSlow21);
		float 	fSlow23 = (1.0f / (1 + (fSlow20 * (fSlow20 + fSlow22))));
		float 	fSlow24 = (2 * (faustpower<2>(fSlow20) - 1));
		float 	fSlow25 = tanf((fConst0 * fentry10));
		float 	fSlow26 = fentry11;
		float 	fSlow27 = (powf(10,(0.05f * (0 - fslider5))) / fSlow26);
		float 	fSlow28 = (1.0f / (1 + (fSlow25 * (fSlow25 + fSlow27))));
		float 	fSlow29 = (2 * (faustpower<2>(fSlow25) - 1));
		float 	fSlow30 = tanf((fConst0 * fentry12));
		float 	fSlow31 = fentry13;
		float 	fSlow32 = (powf(10,(0.05f * (0 - fslider6))) / fSlow31);
		float 	fSlow33 = (1.0f / (1 + (fSlow30 * (fSlow30 + fSlow32))));
		float 	fSlow34 = (2 * (faustpower<2>(fSlow30) - 1));
		float 	fSlow35 = tanf((fConst0 * fentry14));
		float 	fSlow36 = fentry15;
		float 	fSlow37 = (powf(10,(0.05f * (0 - fslider7))) / fSlow36);
		float 	fSlow38 = (1.0f / (1 + (fSlow35 * (fSlow35 + fSlow37))));
		float 	fSlow39 = (2 * (faustpower<2>(fSlow35) - 1));
		float 	fSlow40 = tanf((fConst0 * fentry16));
		float 	fSlow41 = fentry17;
		float 	fSlow42 = (powf(10,(0.05f * (0 - fslider8))) / fSlow41);
		float 	fSlow43 = (1.0f / (1 + (fSlow40 * (fSlow40 + fSlow42))));
		float 	fSlow44 = (2 * (faustpower<2>(fSlow40) - 1));
		float 	fSlow45 = tanf((fConst0 * fentry18));
		float 	fSlow46 = fentry19;
		float 	fSlow47 = (powf(10,(0.05f * (0 - fslider9))) / fSlow46);
		float 	fSlow48 = (1.0f / (1 + (fSlow45 * (fSlow45 + fSlow47))));
		float 	fSlow49 = (2 * (faustpower<2>(fSlow45) - 1));
		float 	fSlow50 = (1 + (fSlow45 * (fSlow45 - fSlow47)));
		float 	fSlow51 = (1.0f / fSlow46);
		float 	fSlow52 = (1 + (fSlow45 * (fSlow45 + fSlow51)));
		float 	fSlow53 = (1 + (fSlow45 * (fSlow45 - fSlow51)));
		float 	fSlow54 = (1 + (fSlow40 * (fSlow40 - fSlow42)));
		float 	fSlow55 = (1.0f / fSlow41);
		float 	fSlow56 = (1 + (fSlow40 * (fSlow40 + fSlow55)));
		float 	fSlow57 = (1 + (fSlow40 * (fSlow40 - fSlow55)));
		float 	fSlow58 = (1 + (fSlow35 * (fSlow35 - fSlow37)));
		float 	fSlow59 = (1.0f / fSlow36);
		float 	fSlow60 = (1 + (fSlow35 * (fSlow35 + fSlow59)));
		float 	fSlow61 = (1 + (fSlow35 * (fSlow35 - fSlow59)));
		float 	fSlow62 = (1 + (fSlow30 * (fSlow30 - fSlow32)));
		float 	fSlow63 = (1.0f / fSlow31);
		float 	fSlow64 = (1 + (fSlow30 * (fSlow30 + fSlow63)));
		float 	fSlow65 = (1 + (fSlow30 * (fSlow30 - fSlow63)));
		float 	fSlow66 = (1 + (fSlow25 * (fSlow25 - fSlow27)));
		float 	fSlow67 = (1.0f / fSlow26);
		float 	fSlow68 = (1 + (fSlow25 * (fSlow67 + fSlow25)));
		float 	fSlow69 = (1 + (fSlow25 * (fSlow25 - fSlow67)));
		float 	fSlow70 = (1 + (fSlow20 * (fSlow20 - fSlow22)));
		float 	fSlow71 = (1.0f / fSlow21);
		float 	fSlow72 = (1 + (fSlow20 * (fSlow20 + fSlow71)));
		float 	fSlow73 = (1 + (fSlow20 * (fSlow20 - fSlow71)));
		float 	fSlow74 = (1 + (fSlow15 * (fSlow15 - fSlow17)));
		float 	fSlow75 = (1.0f / fSlow16);
		float 	fSlow76 = (1 + (fSlow15 * (fSlow15 + fSlow75)));
		float 	fSlow77 = (1 + (fSlow15 * (fSlow15 - fSlow75)));
		float 	fSlow78 = (1 + (fSlow10 * (fSlow10 - fSlow12)));
		float 	fSlow79 = (1.0f / fSlow11);
		float 	fSlow80 = (1 + (fSlow10 * (fSlow10 + fSlow79)));
		float 	fSlow81 = (1 + (fSlow10 * (fSlow10 - fSlow79)));
		float 	fSlow82 = (1 + (fSlow5 * (fSlow5 - fSlow7)));
		float 	fSlow83 = (1.0f / fSlow6);
		float 	fSlow84 = (1 + (fSlow5 * (fSlow5 + fSlow83)));
		float 	fSlow85 = (1 + (fSlow5 * (fSlow5 - fSlow83)));
		float 	fSlow86 = (1 + (fSlow0 * (fSlow0 - fSlow2)));
		float 	fSlow87 = (1.0f / fSlow1);
		float 	fSlow88 = (1 + (fSlow0 * (fSlow0 + fSlow87)));
		float 	fSlow89 = (1 + (fSlow0 * (fSlow0 - fSlow87)));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			float fTemp0 = (fSlow49 * fRec9[1]);
			fRec9[0] = ((float)input0[i] - (fSlow48 * ((fSlow50 * fRec9[2]) + fTemp0)));
			float fTemp1 = (fSlow44 * fRec8[1]);
			fRec8[0] = ((fSlow48 * ((fTemp0 + (fSlow52 * fRec9[0])) + (fSlow53 * fRec9[2]))) - (fSlow43 * ((fSlow54 * fRec8[2]) + fTemp1)));
			float fTemp2 = (fSlow39 * fRec7[1]);
			fRec7[0] = ((fSlow43 * ((fTemp1 + (fSlow56 * fRec8[0])) + (fSlow57 * fRec8[2]))) - (fSlow38 * ((fSlow58 * fRec7[2]) + fTemp2)));
			float fTemp3 = (fSlow34 * fRec6[1]);
			fRec6[0] = ((fSlow38 * ((fTemp2 + (fSlow60 * fRec7[0])) + (fSlow61 * fRec7[2]))) - (fSlow33 * ((fSlow62 * fRec6[2]) + fTemp3)));
			float fTemp4 = (fSlow29 * fRec5[1]);
			fRec5[0] = ((fSlow33 * ((fTemp3 + (fSlow64 * fRec6[0])) + (fSlow65 * fRec6[2]))) - (fSlow28 * ((fSlow66 * fRec5[2]) + fTemp4)));
			float fTemp5 = (fSlow24 * fRec4[1]);
			fRec4[0] = ((fSlow28 * ((fTemp4 + (fSlow68 * fRec5[0])) + (fSlow69 * fRec5[2]))) - (fSlow23 * ((fSlow70 * fRec4[2]) + fTemp5)));
			float fTemp6 = (fSlow19 * fRec3[1]);
			fRec3[0] = ((fSlow23 * ((fTemp5 + (fSlow72 * fRec4[0])) + (fSlow73 * fRec4[2]))) - (fSlow18 * ((fSlow74 * fRec3[2]) + fTemp6)));
			float fTemp7 = (fSlow14 * fRec2[1]);
			fRec2[0] = ((fSlow18 * ((fTemp6 + (fSlow76 * fRec3[0])) + (fSlow77 * fRec3[2]))) - (fSlow13 * ((fSlow78 * fRec2[2]) + fTemp7)));
			float fTemp8 = (fSlow9 * fRec1[1]);
			fRec1[0] = ((fSlow13 * ((fTemp7 + (fSlow80 * fRec2[0])) + (fSlow81 * fRec2[2]))) - (fSlow8 * ((fSlow82 * fRec1[2]) + fTemp8)));
			float fTemp9 = (fSlow4 * fRec0[1]);
			fRec0[0] = ((fSlow8 * ((fTemp8 + (fSlow84 * fRec1[0])) + (fSlow85 * fRec1[2]))) - (fSlow3 * ((fSlow86 * fRec0[2]) + fTemp9)));
			output0[i] = (FAUSTFLOAT)(fSlow3 * ((fTemp9 + (fSlow88 * fRec0[0])) + (fSlow89 * fRec0[2])));
			// post processing
			fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
			fRec1[2] = fRec1[1]; fRec1[1] = fRec1[0];
			fRec2[2] = fRec2[1]; fRec2[1] = fRec2[0];
			fRec3[2] = fRec3[1]; fRec3[1] = fRec3[0];
			fRec4[2] = fRec4[1]; fRec4[1] = fRec4[0];
			fRec5[2] = fRec5[1]; fRec5[1] = fRec5[0];
			fRec6[2] = fRec6[1]; fRec6[1] = fRec6[0];
			fRec7[2] = fRec7[1]; fRec7[1] = fRec7[0];
			fRec8[2] = fRec8[1]; fRec8[1] = fRec8[0];
			fRec9[2] = fRec9[1]; fRec9[1] = fRec9[0];
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
