//-----------------------------------------------------
// name: "mixer"
// version: "1.0"
// author: "Grame"
// license: "BSD"
// copyright: "(c)GRAME 2006"
//
// Code generated with Faust 0.9.58 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
#include <math.h>
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
	FAUSTFLOAT 	fslider0;
	float 	fRec1[2];
	FAUSTFLOAT 	fentry0;
	FAUSTFLOAT 	fcheckbox0;
	FAUSTFLOAT 	fslider1;
	float 	fRec3[2];
	float 	fRec2[2];
	FAUSTFLOAT 	fbargraph0;
	FAUSTFLOAT 	fentry1;
	FAUSTFLOAT 	fcheckbox1;
	FAUSTFLOAT 	fslider2;
	float 	fRec5[2];
	float 	fRec4[2];
	FAUSTFLOAT 	fbargraph1;
	FAUSTFLOAT 	fentry2;
	FAUSTFLOAT 	fcheckbox2;
	FAUSTFLOAT 	fslider3;
	float 	fRec7[2];
	float 	fRec6[2];
	FAUSTFLOAT 	fbargraph2;
	FAUSTFLOAT 	fentry3;
	FAUSTFLOAT 	fcheckbox3;
	FAUSTFLOAT 	fslider4;
	float 	fRec9[2];
	float 	fRec8[2];
	FAUSTFLOAT 	fbargraph3;
	FAUSTFLOAT 	fentry4;
	FAUSTFLOAT 	fcheckbox4;
	FAUSTFLOAT 	fslider5;
	float 	fRec11[2];
	float 	fRec10[2];
	FAUSTFLOAT 	fbargraph4;
	FAUSTFLOAT 	fentry5;
	FAUSTFLOAT 	fcheckbox5;
	FAUSTFLOAT 	fslider6;
	float 	fRec13[2];
	float 	fRec12[2];
	FAUSTFLOAT 	fbargraph5;
	FAUSTFLOAT 	fentry6;
	FAUSTFLOAT 	fcheckbox6;
	FAUSTFLOAT 	fslider7;
	float 	fRec15[2];
	float 	fRec14[2];
	FAUSTFLOAT 	fbargraph6;
	FAUSTFLOAT 	fentry7;
	FAUSTFLOAT 	fcheckbox7;
	FAUSTFLOAT 	fslider8;
	float 	fRec17[2];
	float 	fRec16[2];
	FAUSTFLOAT 	fbargraph7;
	float 	fRec0[2];
	FAUSTFLOAT 	fbargraph8;
	float 	fRec18[2];
	FAUSTFLOAT 	fbargraph9;
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "mixer");
		m->declare("version", "1.0");
		m->declare("author", "Grame");
		m->declare("license", "BSD");
		m->declare("copyright", "(c)GRAME 2006");
		m->declare("volume.dsp/name", "volume");
		m->declare("volume.dsp/version", "1.0");
		m->declare("volume.dsp/author", "Grame");
		m->declare("volume.dsp/license", "BSD");
		m->declare("volume.dsp/copyright", "(c)GRAME 2006");
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
		m->declare("vumeter.dsp/name", "vumeter");
		m->declare("vumeter.dsp/version", "1.0");
		m->declare("vumeter.dsp/author", "Grame");
		m->declare("vumeter.dsp/license", "BSD");
		m->declare("vumeter.dsp/copyright", "(c)GRAME 2006");
		m->declare("panpot.dsp/name", "panpot");
		m->declare("panpot.dsp/version", "1.0");
		m->declare("panpot.dsp/author", "Grame");
		m->declare("panpot.dsp/license", "BSD");
		m->declare("panpot.dsp/copyright", "(c)GRAME 2006");
	}

	virtual int getNumInputs() 	{ return 8; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fConst0 = (1.0f / float(min(192000, max(1, fSamplingFreq))));
		fslider0 = 0.0f;
		for (int i=0; i<2; i++) fRec1[i] = 0;
		fentry0 = 0.0f;
		fcheckbox0 = 0.0;
		fslider1 = 0.0f;
		for (int i=0; i<2; i++) fRec3[i] = 0;
		for (int i=0; i<2; i++) fRec2[i] = 0;
		fentry1 = 0.0f;
		fcheckbox1 = 0.0;
		fslider2 = 0.0f;
		for (int i=0; i<2; i++) fRec5[i] = 0;
		for (int i=0; i<2; i++) fRec4[i] = 0;
		fentry2 = 0.0f;
		fcheckbox2 = 0.0;
		fslider3 = 0.0f;
		for (int i=0; i<2; i++) fRec7[i] = 0;
		for (int i=0; i<2; i++) fRec6[i] = 0;
		fentry3 = 0.0f;
		fcheckbox3 = 0.0;
		fslider4 = 0.0f;
		for (int i=0; i<2; i++) fRec9[i] = 0;
		for (int i=0; i<2; i++) fRec8[i] = 0;
		fentry4 = 0.0f;
		fcheckbox4 = 0.0;
		fslider5 = 0.0f;
		for (int i=0; i<2; i++) fRec11[i] = 0;
		for (int i=0; i<2; i++) fRec10[i] = 0;
		fentry5 = 0.0f;
		fcheckbox5 = 0.0;
		fslider6 = 0.0f;
		for (int i=0; i<2; i++) fRec13[i] = 0;
		for (int i=0; i<2; i++) fRec12[i] = 0;
		fentry6 = 0.0f;
		fcheckbox6 = 0.0;
		fslider7 = 0.0f;
		for (int i=0; i<2; i++) fRec15[i] = 0;
		for (int i=0; i<2; i++) fRec14[i] = 0;
		fentry7 = 0.0f;
		fcheckbox7 = 0.0;
		fslider8 = 0.0f;
		for (int i=0; i<2; i++) fRec17[i] = 0;
		for (int i=0; i<2; i++) fRec16[i] = 0;
		for (int i=0; i<2; i++) fRec0[i] = 0;
		for (int i=0; i<2; i++) fRec18[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openHorizontalBox("mixer");
		interface->openVerticalBox("Ch 0");
		interface->declare(&fentry0, "1", "");
		interface->declare(&fentry0, "style", "knob");
		interface->addNumEntry("pan", &fentry0, 0.0f, -9e+01f, 9e+01f, 1.0f);
		interface->declare(0, "2", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider1, "1", "");
		interface->addVerticalSlider("", &fslider1, 0.0f, -7e+01f, 4.0f, 0.1f);
		interface->declare(&fbargraph0, "2", "");
		interface->declare(&fbargraph0, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph0, -7e+01f, 5.0f);
		interface->closeBox();
		interface->addCheckButton("mute", &fcheckbox0);
		interface->closeBox();
		interface->openVerticalBox("Ch 1");
		interface->declare(&fentry1, "1", "");
		interface->declare(&fentry1, "style", "knob");
		interface->addNumEntry("pan", &fentry1, 0.0f, -9e+01f, 9e+01f, 1.0f);
		interface->declare(0, "2", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider2, "1", "");
		interface->addVerticalSlider("", &fslider2, 0.0f, -7e+01f, 4.0f, 0.1f);
		interface->declare(&fbargraph1, "2", "");
		interface->declare(&fbargraph1, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph1, -7e+01f, 5.0f);
		interface->closeBox();
		interface->addCheckButton("mute", &fcheckbox1);
		interface->closeBox();
		interface->openVerticalBox("Ch 2");
		interface->declare(&fentry2, "1", "");
		interface->declare(&fentry2, "style", "knob");
		interface->addNumEntry("pan", &fentry2, 0.0f, -9e+01f, 9e+01f, 1.0f);
		interface->declare(0, "2", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider3, "1", "");
		interface->addVerticalSlider("", &fslider3, 0.0f, -7e+01f, 4.0f, 0.1f);
		interface->declare(&fbargraph2, "2", "");
		interface->declare(&fbargraph2, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph2, -7e+01f, 5.0f);
		interface->closeBox();
		interface->addCheckButton("mute", &fcheckbox2);
		interface->closeBox();
		interface->openVerticalBox("Ch 3");
		interface->declare(&fentry3, "1", "");
		interface->declare(&fentry3, "style", "knob");
		interface->addNumEntry("pan", &fentry3, 0.0f, -9e+01f, 9e+01f, 1.0f);
		interface->declare(0, "2", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider4, "1", "");
		interface->addVerticalSlider("", &fslider4, 0.0f, -7e+01f, 4.0f, 0.1f);
		interface->declare(&fbargraph3, "2", "");
		interface->declare(&fbargraph3, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph3, -7e+01f, 5.0f);
		interface->closeBox();
		interface->addCheckButton("mute", &fcheckbox3);
		interface->closeBox();
		interface->openVerticalBox("Ch 4");
		interface->declare(&fentry4, "1", "");
		interface->declare(&fentry4, "style", "knob");
		interface->addNumEntry("pan", &fentry4, 0.0f, -9e+01f, 9e+01f, 1.0f);
		interface->declare(0, "2", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider5, "1", "");
		interface->addVerticalSlider("", &fslider5, 0.0f, -7e+01f, 4.0f, 0.1f);
		interface->declare(&fbargraph4, "2", "");
		interface->declare(&fbargraph4, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph4, -7e+01f, 5.0f);
		interface->closeBox();
		interface->addCheckButton("mute", &fcheckbox4);
		interface->closeBox();
		interface->openVerticalBox("Ch 5");
		interface->declare(&fentry5, "1", "");
		interface->declare(&fentry5, "style", "knob");
		interface->addNumEntry("pan", &fentry5, 0.0f, -9e+01f, 9e+01f, 1.0f);
		interface->declare(0, "2", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider6, "1", "");
		interface->addVerticalSlider("", &fslider6, 0.0f, -7e+01f, 4.0f, 0.1f);
		interface->declare(&fbargraph5, "2", "");
		interface->declare(&fbargraph5, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph5, -7e+01f, 5.0f);
		interface->closeBox();
		interface->addCheckButton("mute", &fcheckbox5);
		interface->closeBox();
		interface->openVerticalBox("Ch 6");
		interface->declare(&fentry6, "1", "");
		interface->declare(&fentry6, "style", "knob");
		interface->addNumEntry("pan", &fentry6, 0.0f, -9e+01f, 9e+01f, 1.0f);
		interface->declare(0, "2", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider7, "1", "");
		interface->addVerticalSlider("", &fslider7, 0.0f, -7e+01f, 4.0f, 0.1f);
		interface->declare(&fbargraph6, "2", "");
		interface->declare(&fbargraph6, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph6, -7e+01f, 5.0f);
		interface->closeBox();
		interface->addCheckButton("mute", &fcheckbox6);
		interface->closeBox();
		interface->openVerticalBox("Ch 7");
		interface->declare(&fentry7, "1", "");
		interface->declare(&fentry7, "style", "knob");
		interface->addNumEntry("pan", &fentry7, 0.0f, -9e+01f, 9e+01f, 1.0f);
		interface->declare(0, "2", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider8, "1", "");
		interface->addVerticalSlider("", &fslider8, 0.0f, -7e+01f, 4.0f, 0.1f);
		interface->declare(&fbargraph7, "2", "");
		interface->declare(&fbargraph7, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph7, -7e+01f, 5.0f);
		interface->closeBox();
		interface->addCheckButton("mute", &fcheckbox7);
		interface->closeBox();
		interface->openHorizontalBox("stereo out");
		interface->openVerticalBox("L");
		interface->declare(&fbargraph8, "2", "");
		interface->declare(&fbargraph8, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph8, -7e+01f, 5.0f);
		interface->closeBox();
		interface->openVerticalBox("R");
		interface->declare(&fbargraph9, "2", "");
		interface->declare(&fbargraph9, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph9, -7e+01f, 5.0f);
		interface->closeBox();
		interface->declare(&fslider0, "1", "");
		interface->addVerticalSlider("", &fslider0, 0.0f, -7e+01f, 4.0f, 0.1f);
		interface->closeBox();
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = (0.0010000000000000009f * powf(10,(0.05f * fslider0)));
		float 	fSlow1 = (0.005555555555555556f * (fentry0 - 9e+01f));
		float 	fSlow2 = sqrtf((0 - fSlow1));
		float 	fSlow3 = (1 - fcheckbox0);
		float 	fSlow4 = (0.0010000000000000009f * powf(10,(0.05f * fslider1)));
		float 	fSlow5 = (0.005555555555555556f * (fentry1 - 9e+01f));
		float 	fSlow6 = sqrtf((0 - fSlow5));
		float 	fSlow7 = (1 - fcheckbox1);
		float 	fSlow8 = (0.0010000000000000009f * powf(10,(0.05f * fslider2)));
		float 	fSlow9 = (0.005555555555555556f * (fentry2 - 9e+01f));
		float 	fSlow10 = sqrtf((0 - fSlow9));
		float 	fSlow11 = (1 - fcheckbox2);
		float 	fSlow12 = (0.0010000000000000009f * powf(10,(0.05f * fslider3)));
		float 	fSlow13 = (0.005555555555555556f * (fentry3 - 9e+01f));
		float 	fSlow14 = sqrtf((0 - fSlow13));
		float 	fSlow15 = (1 - fcheckbox3);
		float 	fSlow16 = (0.0010000000000000009f * powf(10,(0.05f * fslider4)));
		float 	fSlow17 = (0.005555555555555556f * (fentry4 - 9e+01f));
		float 	fSlow18 = sqrtf((0 - fSlow17));
		float 	fSlow19 = (1 - fcheckbox4);
		float 	fSlow20 = (0.0010000000000000009f * powf(10,(0.05f * fslider5)));
		float 	fSlow21 = (0.005555555555555556f * (fentry5 - 9e+01f));
		float 	fSlow22 = sqrtf((0 - fSlow21));
		float 	fSlow23 = (1 - fcheckbox5);
		float 	fSlow24 = (0.0010000000000000009f * powf(10,(0.05f * fslider6)));
		float 	fSlow25 = (0.005555555555555556f * (fentry6 - 9e+01f));
		float 	fSlow26 = sqrtf((0 - fSlow25));
		float 	fSlow27 = (1 - fcheckbox6);
		float 	fSlow28 = (0.0010000000000000009f * powf(10,(0.05f * fslider7)));
		float 	fSlow29 = (0.005555555555555556f * (fentry7 - 9e+01f));
		float 	fSlow30 = sqrtf((0 - fSlow29));
		float 	fSlow31 = (1 - fcheckbox7);
		float 	fSlow32 = (0.0010000000000000009f * powf(10,(0.05f * fslider8)));
		float 	fSlow33 = sqrtf((1 + fSlow1));
		float 	fSlow34 = sqrtf((1 + fSlow5));
		float 	fSlow35 = sqrtf((1 + fSlow9));
		float 	fSlow36 = sqrtf((1 + fSlow13));
		float 	fSlow37 = sqrtf((1 + fSlow17));
		float 	fSlow38 = sqrtf((1 + fSlow21));
		float 	fSlow39 = sqrtf((1 + fSlow25));
		float 	fSlow40 = sqrtf((1 + fSlow29));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* input2 = input[2];
		FAUSTFLOAT* input3 = input[3];
		FAUSTFLOAT* input4 = input[4];
		FAUSTFLOAT* input5 = input[5];
		FAUSTFLOAT* input6 = input[6];
		FAUSTFLOAT* input7 = input[7];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			fRec1[0] = ((0.999f * fRec1[1]) + fSlow0);
			fRec3[0] = ((0.999f * fRec3[1]) + fSlow4);
			float fTemp0 = (fSlow3 * ((float)input0[i] * fRec3[0]));
			fRec2[0] = max((fRec2[1] - fConst0), fabsf(fTemp0));
			fbargraph0 = (20 * log10f(max(0.00031622776601683794f, fRec2[0])));
			float fTemp1 = fTemp0;
			fRec5[0] = ((0.999f * fRec5[1]) + fSlow8);
			float fTemp2 = (fSlow7 * ((float)input1[i] * fRec5[0]));
			fRec4[0] = max((fRec4[1] - fConst0), fabsf(fTemp2));
			fbargraph1 = (20 * log10f(max(0.00031622776601683794f, fRec4[0])));
			float fTemp3 = fTemp2;
			fRec7[0] = ((0.999f * fRec7[1]) + fSlow12);
			float fTemp4 = (fSlow11 * ((float)input2[i] * fRec7[0]));
			fRec6[0] = max((fRec6[1] - fConst0), fabsf(fTemp4));
			fbargraph2 = (20 * log10f(max(0.00031622776601683794f, fRec6[0])));
			float fTemp5 = fTemp4;
			fRec9[0] = ((0.999f * fRec9[1]) + fSlow16);
			float fTemp6 = (fSlow15 * ((float)input3[i] * fRec9[0]));
			fRec8[0] = max((fRec8[1] - fConst0), fabsf(fTemp6));
			fbargraph3 = (20 * log10f(max(0.00031622776601683794f, fRec8[0])));
			float fTemp7 = fTemp6;
			fRec11[0] = ((0.999f * fRec11[1]) + fSlow20);
			float fTemp8 = (fSlow19 * ((float)input4[i] * fRec11[0]));
			fRec10[0] = max((fRec10[1] - fConst0), fabsf(fTemp8));
			fbargraph4 = (20 * log10f(max(0.00031622776601683794f, fRec10[0])));
			float fTemp9 = fTemp8;
			fRec13[0] = ((0.999f * fRec13[1]) + fSlow24);
			float fTemp10 = (fSlow23 * ((float)input5[i] * fRec13[0]));
			fRec12[0] = max((fRec12[1] - fConst0), fabsf(fTemp10));
			fbargraph5 = (20 * log10f(max(0.00031622776601683794f, fRec12[0])));
			float fTemp11 = fTemp10;
			fRec15[0] = ((0.999f * fRec15[1]) + fSlow28);
			float fTemp12 = (fSlow27 * ((float)input6[i] * fRec15[0]));
			fRec14[0] = max((fRec14[1] - fConst0), fabsf(fTemp12));
			fbargraph6 = (20 * log10f(max(0.00031622776601683794f, fRec14[0])));
			float fTemp13 = fTemp12;
			fRec17[0] = ((0.999f * fRec17[1]) + fSlow32);
			float fTemp14 = (fSlow31 * ((float)input7[i] * fRec17[0]));
			fRec16[0] = max((fRec16[1] - fConst0), fabsf(fTemp14));
			fbargraph7 = (20 * log10f(max(0.00031622776601683794f, fRec16[0])));
			float fTemp15 = fTemp14;
			float fTemp16 = (fRec1[0] * ((((((((fSlow2 * fTemp1) + (fSlow6 * fTemp3)) + (fSlow10 * fTemp5)) + (fSlow14 * fTemp7)) + (fSlow18 * fTemp9)) + (fSlow22 * fTemp11)) + (fSlow26 * fTemp13)) + (fSlow30 * fTemp15)));
			fRec0[0] = max((fRec0[1] - fConst0), fabsf(fTemp16));
			fbargraph8 = (20 * log10f(max(0.00031622776601683794f, fRec0[0])));
			output0[i] = (FAUSTFLOAT)fTemp16;
			float fTemp17 = (fRec1[0] * ((((((((fSlow33 * fTemp1) + (fSlow34 * fTemp3)) + (fSlow35 * fTemp5)) + (fSlow36 * fTemp7)) + (fSlow37 * fTemp9)) + (fSlow38 * fTemp11)) + (fSlow39 * fTemp13)) + (fSlow40 * fTemp15)));
			fRec18[0] = max((fRec18[1] - fConst0), fabsf(fTemp17));
			fbargraph9 = (20 * log10f(max(0.00031622776601683794f, fRec18[0])));
			output1[i] = (FAUSTFLOAT)fTemp17;
			// post processing
			fRec18[1] = fRec18[0];
			fRec0[1] = fRec0[0];
			fRec16[1] = fRec16[0];
			fRec17[1] = fRec17[0];
			fRec14[1] = fRec14[0];
			fRec15[1] = fRec15[0];
			fRec12[1] = fRec12[0];
			fRec13[1] = fRec13[0];
			fRec10[1] = fRec10[0];
			fRec11[1] = fRec11[0];
			fRec8[1] = fRec8[0];
			fRec9[1] = fRec9[0];
			fRec6[1] = fRec6[0];
			fRec7[1] = fRec7[0];
			fRec4[1] = fRec4[0];
			fRec5[1] = fRec5[0];
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
			fRec1[1] = fRec1[0];
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
