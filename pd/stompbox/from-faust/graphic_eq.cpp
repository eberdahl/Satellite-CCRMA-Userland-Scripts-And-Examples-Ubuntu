//-----------------------------------------------------
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
	FAUSTFLOAT 	fslider0;
	int 	iConst0;
	float 	fConst1;
	FAUSTFLOAT 	fslider1;
	float 	fConst2;
	float 	fConst3;
	float 	fConst4;
	float 	fConst5;
	float 	fConst6;
	float 	fConst7;
	float 	fConst8;
	float 	fConst9;
	float 	fConst10;
	float 	fConst11;
	float 	fConst12;
	float 	fConst13;
	FAUSTFLOAT 	fcheckbox0;
	FAUSTFLOAT 	fslider2;
	float 	fRec4[2];
	FAUSTFLOAT 	fcheckbox1;
	FAUSTFLOAT 	fcheckbox2;
	int 	iRec6[2];
	float 	fRec5[4];
	float 	fConst14;
	float 	fVec0[2];
	float 	fConst15;
	float 	fConst16;
	FAUSTFLOAT 	fslider3;
	FAUSTFLOAT 	fslider4;
	float 	fRec7[2];
	float 	fRec8[2];
	float 	fVec1[2];
	FAUSTFLOAT 	fslider5;
	float 	fRec9[2];
	float 	fVec2[2];
	FAUSTFLOAT 	fslider6;
	float 	fRec10[2];
	float 	fVec3[2];
	float 	fConst17;
	float 	fConst18;
	float 	fConst19;
	float 	fConst20;
	float 	fConst21;
	float 	fConst22;
	float 	fConst23;
	float 	fVec4[2];
	float 	fConst24;
	float 	fRec21[2];
	float 	fConst25;
	float 	fConst26;
	float 	fRec20[3];
	float 	fConst27;
	float 	fConst28;
	float 	fRec19[3];
	float 	fConst29;
	float 	fConst30;
	float 	fConst31;
	float 	fConst32;
	float 	fConst33;
	float 	fConst34;
	float 	fConst35;
	float 	fRec18[3];
	float 	fConst36;
	float 	fConst37;
	float 	fConst38;
	float 	fConst39;
	float 	fConst40;
	float 	fConst41;
	float 	fRec17[3];
	float 	fConst42;
	float 	fConst43;
	float 	fConst44;
	float 	fConst45;
	float 	fConst46;
	float 	fConst47;
	float 	fConst48;
	float 	fRec16[3];
	float 	fConst49;
	float 	fConst50;
	float 	fConst51;
	float 	fConst52;
	float 	fConst53;
	float 	fConst54;
	float 	fRec15[3];
	float 	fConst55;
	float 	fConst56;
	float 	fConst57;
	float 	fConst58;
	float 	fConst59;
	float 	fConst60;
	float 	fConst61;
	float 	fRec14[3];
	float 	fConst62;
	float 	fConst63;
	float 	fConst64;
	float 	fConst65;
	float 	fConst66;
	float 	fConst67;
	float 	fRec13[3];
	float 	fConst68;
	float 	fConst69;
	float 	fConst70;
	float 	fConst71;
	float 	fConst72;
	float 	fConst73;
	float 	fConst74;
	float 	fRec12[3];
	float 	fConst75;
	float 	fConst76;
	float 	fConst77;
	float 	fConst78;
	float 	fConst79;
	float 	fConst80;
	float 	fRec11[3];
	FAUSTFLOAT 	fslider7;
	float 	fRec22[2];
	float 	fConst81;
	float 	fConst82;
	float 	fConst83;
	float 	fConst84;
	float 	fConst85;
	float 	fConst86;
	float 	fRec35[2];
	float 	fRec34[3];
	float 	fRec33[3];
	float 	fVec5[2];
	float 	fConst87;
	float 	fRec32[2];
	float 	fConst88;
	float 	fRec31[3];
	float 	fConst89;
	float 	fConst90;
	float 	fRec30[3];
	float 	fRec29[3];
	float 	fRec28[3];
	float 	fRec27[3];
	float 	fRec26[3];
	float 	fRec25[3];
	float 	fRec24[3];
	float 	fRec23[3];
	FAUSTFLOAT 	fslider8;
	float 	fRec36[2];
	float 	fConst91;
	float 	fConst92;
	float 	fConst93;
	float 	fConst94;
	float 	fConst95;
	float 	fConst96;
	float 	fRec48[2];
	float 	fRec47[3];
	float 	fRec46[3];
	float 	fVec6[2];
	float 	fConst97;
	float 	fRec45[2];
	float 	fConst98;
	float 	fRec44[3];
	float 	fConst99;
	float 	fConst100;
	float 	fRec43[3];
	float 	fRec42[3];
	float 	fRec41[3];
	float 	fRec40[3];
	float 	fRec39[3];
	float 	fRec38[3];
	float 	fRec37[3];
	FAUSTFLOAT 	fslider9;
	float 	fRec49[2];
	float 	fConst101;
	float 	fConst102;
	float 	fConst103;
	float 	fConst104;
	float 	fConst105;
	float 	fConst106;
	float 	fRec60[2];
	float 	fRec59[3];
	float 	fRec58[3];
	float 	fVec7[2];
	float 	fConst107;
	float 	fRec57[2];
	float 	fConst108;
	float 	fRec56[3];
	float 	fConst109;
	float 	fConst110;
	float 	fRec55[3];
	float 	fRec54[3];
	float 	fRec53[3];
	float 	fRec52[3];
	float 	fRec51[3];
	float 	fRec50[3];
	FAUSTFLOAT 	fslider10;
	float 	fRec61[2];
	float 	fConst111;
	float 	fConst112;
	float 	fConst113;
	float 	fConst114;
	float 	fConst115;
	float 	fConst116;
	float 	fRec71[2];
	float 	fRec70[3];
	float 	fRec69[3];
	float 	fVec8[2];
	float 	fConst117;
	float 	fRec68[2];
	float 	fConst118;
	float 	fRec67[3];
	float 	fConst119;
	float 	fConst120;
	float 	fRec66[3];
	float 	fRec65[3];
	float 	fRec64[3];
	float 	fRec63[3];
	float 	fRec62[3];
	FAUSTFLOAT 	fslider11;
	float 	fRec72[2];
	float 	fConst121;
	float 	fConst122;
	float 	fConst123;
	float 	fConst124;
	float 	fConst125;
	float 	fConst126;
	float 	fRec81[2];
	float 	fRec80[3];
	float 	fRec79[3];
	float 	fVec9[2];
	float 	fConst127;
	float 	fRec78[2];
	float 	fConst128;
	float 	fRec77[3];
	float 	fConst129;
	float 	fConst130;
	float 	fRec76[3];
	float 	fRec75[3];
	float 	fRec74[3];
	float 	fRec73[3];
	FAUSTFLOAT 	fslider12;
	float 	fRec82[2];
	float 	fConst131;
	float 	fConst132;
	float 	fConst133;
	float 	fConst134;
	float 	fConst135;
	float 	fConst136;
	float 	fRec90[2];
	float 	fRec89[3];
	float 	fRec88[3];
	float 	fVec10[2];
	float 	fConst137;
	float 	fRec87[2];
	float 	fConst138;
	float 	fRec86[3];
	float 	fConst139;
	float 	fConst140;
	float 	fRec85[3];
	float 	fRec84[3];
	float 	fRec83[3];
	FAUSTFLOAT 	fslider13;
	float 	fRec91[2];
	float 	fConst141;
	float 	fConst142;
	float 	fConst143;
	float 	fConst144;
	float 	fConst145;
	float 	fConst146;
	float 	fRec98[2];
	float 	fRec97[3];
	float 	fRec96[3];
	float 	fVec11[2];
	float 	fConst147;
	float 	fRec95[2];
	float 	fConst148;
	float 	fRec94[3];
	float 	fConst149;
	float 	fConst150;
	float 	fRec93[3];
	float 	fRec92[3];
	FAUSTFLOAT 	fslider14;
	float 	fRec99[2];
	float 	fConst151;
	float 	fConst152;
	float 	fConst153;
	float 	fConst154;
	float 	fConst155;
	float 	fRec105[2];
	float 	fRec104[3];
	float 	fRec103[3];
	float 	fVec12[2];
	float 	fConst156;
	float 	fRec102[2];
	float 	fConst157;
	float 	fRec101[3];
	float 	fConst158;
	float 	fConst159;
	float 	fRec100[3];
	FAUSTFLOAT 	fslider15;
	float 	fRec106[2];
	float 	fRec109[2];
	float 	fRec108[3];
	float 	fRec107[3];
	FAUSTFLOAT 	fslider16;
	float 	fRec110[2];
	float 	fConst160;
	float 	fConst161;
	float 	fRec3[3];
	float 	fConst162;
	float 	fConst163;
	float 	fConst164;
	float 	fRec2[3];
	float 	fConst165;
	float 	fConst166;
	float 	fConst167;
	float 	fRec1[3];
	float 	fConst168;
	float 	fRec0[2];
	FAUSTFLOAT 	fbargraph0;
	float 	fConst169;
	float 	fConst170;
	float 	fConst171;
	float 	fConst172;
	float 	fConst173;
	float 	fConst174;
	float 	fConst175;
	float 	fConst176;
	float 	fConst177;
	float 	fConst178;
	float 	fConst179;
	float 	fConst180;
	float 	fConst181;
	float 	fConst182;
	float 	fConst183;
	float 	fConst184;
	float 	fConst185;
	float 	fConst186;
	float 	fConst187;
	float 	fConst188;
	float 	fConst189;
	float 	fRec117[3];
	float 	fConst190;
	float 	fConst191;
	float 	fConst192;
	float 	fRec116[3];
	float 	fConst193;
	float 	fConst194;
	float 	fConst195;
	float 	fRec115[3];
	float 	fConst196;
	float 	fConst197;
	float 	fConst198;
	float 	fConst199;
	float 	fRec114[3];
	float 	fConst200;
	float 	fConst201;
	float 	fConst202;
	float 	fRec113[3];
	float 	fConst203;
	float 	fConst204;
	float 	fConst205;
	float 	fRec112[3];
	float 	fConst206;
	float 	fRec111[2];
	FAUSTFLOAT 	fbargraph1;
	float 	fConst207;
	float 	fConst208;
	float 	fConst209;
	float 	fConst210;
	float 	fConst211;
	float 	fConst212;
	float 	fConst213;
	float 	fConst214;
	float 	fConst215;
	float 	fConst216;
	float 	fConst217;
	float 	fConst218;
	float 	fConst219;
	float 	fConst220;
	float 	fConst221;
	float 	fConst222;
	float 	fConst223;
	float 	fConst224;
	float 	fConst225;
	float 	fConst226;
	float 	fConst227;
	float 	fRec124[3];
	float 	fConst228;
	float 	fConst229;
	float 	fConst230;
	float 	fRec123[3];
	float 	fConst231;
	float 	fConst232;
	float 	fConst233;
	float 	fRec122[3];
	float 	fConst234;
	float 	fConst235;
	float 	fConst236;
	float 	fConst237;
	float 	fRec121[3];
	float 	fConst238;
	float 	fConst239;
	float 	fConst240;
	float 	fRec120[3];
	float 	fConst241;
	float 	fConst242;
	float 	fConst243;
	float 	fRec119[3];
	float 	fConst244;
	float 	fRec118[2];
	FAUSTFLOAT 	fbargraph2;
	float 	fConst245;
	float 	fConst246;
	float 	fConst247;
	float 	fConst248;
	float 	fConst249;
	float 	fConst250;
	float 	fConst251;
	float 	fConst252;
	float 	fConst253;
	float 	fConst254;
	float 	fConst255;
	float 	fConst256;
	float 	fConst257;
	float 	fConst258;
	float 	fConst259;
	float 	fConst260;
	float 	fConst261;
	float 	fConst262;
	float 	fRec131[3];
	float 	fConst263;
	float 	fConst264;
	float 	fConst265;
	float 	fRec130[3];
	float 	fConst266;
	float 	fConst267;
	float 	fConst268;
	float 	fRec129[3];
	float 	fConst269;
	float 	fConst270;
	float 	fConst271;
	float 	fRec128[3];
	float 	fConst272;
	float 	fConst273;
	float 	fConst274;
	float 	fRec127[3];
	float 	fConst275;
	float 	fConst276;
	float 	fConst277;
	float 	fRec126[3];
	float 	fConst278;
	float 	fRec125[2];
	FAUSTFLOAT 	fbargraph3;
	float 	fConst279;
	float 	fConst280;
	float 	fConst281;
	float 	fConst282;
	float 	fConst283;
	float 	fConst284;
	float 	fConst285;
	float 	fConst286;
	float 	fConst287;
	float 	fConst288;
	float 	fConst289;
	float 	fConst290;
	float 	fConst291;
	float 	fConst292;
	float 	fConst293;
	float 	fConst294;
	float 	fConst295;
	float 	fConst296;
	float 	fConst297;
	float 	fConst298;
	float 	fConst299;
	float 	fRec138[3];
	float 	fConst300;
	float 	fConst301;
	float 	fConst302;
	float 	fRec137[3];
	float 	fConst303;
	float 	fConst304;
	float 	fConst305;
	float 	fRec136[3];
	float 	fConst306;
	float 	fConst307;
	float 	fConst308;
	float 	fConst309;
	float 	fRec135[3];
	float 	fConst310;
	float 	fConst311;
	float 	fConst312;
	float 	fRec134[3];
	float 	fConst313;
	float 	fConst314;
	float 	fConst315;
	float 	fRec133[3];
	float 	fConst316;
	float 	fRec132[2];
	FAUSTFLOAT 	fbargraph4;
	float 	fConst317;
	float 	fConst318;
	float 	fConst319;
	float 	fConst320;
	float 	fConst321;
	float 	fConst322;
	float 	fConst323;
	float 	fConst324;
	float 	fConst325;
	float 	fConst326;
	float 	fConst327;
	float 	fConst328;
	float 	fConst329;
	float 	fConst330;
	float 	fConst331;
	float 	fConst332;
	float 	fConst333;
	float 	fConst334;
	float 	fConst335;
	float 	fConst336;
	float 	fConst337;
	float 	fRec145[3];
	float 	fConst338;
	float 	fConst339;
	float 	fConst340;
	float 	fRec144[3];
	float 	fConst341;
	float 	fConst342;
	float 	fConst343;
	float 	fRec143[3];
	float 	fConst344;
	float 	fConst345;
	float 	fConst346;
	float 	fConst347;
	float 	fRec142[3];
	float 	fConst348;
	float 	fConst349;
	float 	fConst350;
	float 	fRec141[3];
	float 	fConst351;
	float 	fConst352;
	float 	fConst353;
	float 	fRec140[3];
	float 	fConst354;
	float 	fRec139[2];
	FAUSTFLOAT 	fbargraph5;
	float 	fConst355;
	float 	fConst356;
	float 	fConst357;
	float 	fConst358;
	float 	fConst359;
	float 	fConst360;
	float 	fConst361;
	float 	fConst362;
	float 	fConst363;
	float 	fConst364;
	float 	fConst365;
	float 	fConst366;
	float 	fConst367;
	float 	fConst368;
	float 	fConst369;
	float 	fConst370;
	float 	fConst371;
	float 	fConst372;
	float 	fRec152[3];
	float 	fConst373;
	float 	fConst374;
	float 	fConst375;
	float 	fRec151[3];
	float 	fConst376;
	float 	fConst377;
	float 	fConst378;
	float 	fRec150[3];
	float 	fConst379;
	float 	fConst380;
	float 	fConst381;
	float 	fRec149[3];
	float 	fConst382;
	float 	fConst383;
	float 	fConst384;
	float 	fRec148[3];
	float 	fConst385;
	float 	fConst386;
	float 	fConst387;
	float 	fRec147[3];
	float 	fConst388;
	float 	fRec146[2];
	FAUSTFLOAT 	fbargraph6;
	float 	fConst389;
	float 	fConst390;
	float 	fConst391;
	float 	fConst392;
	float 	fConst393;
	float 	fConst394;
	float 	fConst395;
	float 	fConst396;
	float 	fConst397;
	float 	fConst398;
	float 	fConst399;
	float 	fConst400;
	float 	fConst401;
	float 	fConst402;
	float 	fConst403;
	float 	fConst404;
	float 	fConst405;
	float 	fConst406;
	float 	fConst407;
	float 	fConst408;
	float 	fConst409;
	float 	fRec159[3];
	float 	fConst410;
	float 	fConst411;
	float 	fConst412;
	float 	fRec158[3];
	float 	fConst413;
	float 	fConst414;
	float 	fConst415;
	float 	fRec157[3];
	float 	fConst416;
	float 	fConst417;
	float 	fConst418;
	float 	fConst419;
	float 	fRec156[3];
	float 	fConst420;
	float 	fConst421;
	float 	fConst422;
	float 	fRec155[3];
	float 	fConst423;
	float 	fConst424;
	float 	fConst425;
	float 	fRec154[3];
	float 	fConst426;
	float 	fRec153[2];
	FAUSTFLOAT 	fbargraph7;
	float 	fConst427;
	float 	fConst428;
	float 	fConst429;
	float 	fConst430;
	float 	fConst431;
	float 	fConst432;
	float 	fConst433;
	float 	fConst434;
	float 	fConst435;
	float 	fConst436;
	float 	fConst437;
	float 	fConst438;
	float 	fConst439;
	float 	fConst440;
	float 	fConst441;
	float 	fConst442;
	float 	fConst443;
	float 	fConst444;
	float 	fConst445;
	float 	fConst446;
	float 	fConst447;
	float 	fRec166[3];
	float 	fConst448;
	float 	fConst449;
	float 	fConst450;
	float 	fRec165[3];
	float 	fConst451;
	float 	fConst452;
	float 	fConst453;
	float 	fRec164[3];
	float 	fConst454;
	float 	fConst455;
	float 	fConst456;
	float 	fConst457;
	float 	fRec163[3];
	float 	fConst458;
	float 	fConst459;
	float 	fConst460;
	float 	fRec162[3];
	float 	fConst461;
	float 	fConst462;
	float 	fConst463;
	float 	fRec161[3];
	float 	fConst464;
	float 	fRec160[2];
	FAUSTFLOAT 	fbargraph8;
	float 	fConst465;
	float 	fConst466;
	float 	fConst467;
	float 	fConst468;
	float 	fConst469;
	float 	fConst470;
	float 	fConst471;
	float 	fConst472;
	float 	fConst473;
	float 	fConst474;
	float 	fConst475;
	float 	fConst476;
	float 	fConst477;
	float 	fConst478;
	float 	fConst479;
	float 	fConst480;
	float 	fConst481;
	float 	fConst482;
	float 	fRec173[3];
	float 	fConst483;
	float 	fConst484;
	float 	fConst485;
	float 	fRec172[3];
	float 	fConst486;
	float 	fConst487;
	float 	fConst488;
	float 	fRec171[3];
	float 	fConst489;
	float 	fConst490;
	float 	fConst491;
	float 	fRec170[3];
	float 	fConst492;
	float 	fConst493;
	float 	fConst494;
	float 	fRec169[3];
	float 	fConst495;
	float 	fConst496;
	float 	fConst497;
	float 	fRec168[3];
	float 	fConst498;
	float 	fRec167[2];
	FAUSTFLOAT 	fbargraph9;
	float 	fConst499;
	float 	fConst500;
	float 	fConst501;
	float 	fConst502;
	float 	fConst503;
	float 	fConst504;
	float 	fConst505;
	float 	fConst506;
	float 	fConst507;
	float 	fConst508;
	float 	fConst509;
	float 	fConst510;
	float 	fConst511;
	float 	fConst512;
	float 	fConst513;
	float 	fConst514;
	float 	fConst515;
	float 	fConst516;
	float 	fConst517;
	float 	fConst518;
	float 	fConst519;
	float 	fRec180[3];
	float 	fConst520;
	float 	fConst521;
	float 	fConst522;
	float 	fRec179[3];
	float 	fConst523;
	float 	fConst524;
	float 	fConst525;
	float 	fRec178[3];
	float 	fConst526;
	float 	fConst527;
	float 	fConst528;
	float 	fConst529;
	float 	fRec177[3];
	float 	fConst530;
	float 	fConst531;
	float 	fConst532;
	float 	fRec176[3];
	float 	fConst533;
	float 	fConst534;
	float 	fConst535;
	float 	fRec175[3];
	float 	fConst536;
	float 	fRec174[2];
	FAUSTFLOAT 	fbargraph10;
	float 	fConst537;
	float 	fConst538;
	float 	fConst539;
	float 	fConst540;
	float 	fConst541;
	float 	fConst542;
	float 	fConst543;
	float 	fConst544;
	float 	fConst545;
	float 	fConst546;
	float 	fConst547;
	float 	fConst548;
	float 	fConst549;
	float 	fConst550;
	float 	fConst551;
	float 	fConst552;
	float 	fConst553;
	float 	fConst554;
	float 	fConst555;
	float 	fConst556;
	float 	fConst557;
	float 	fRec187[3];
	float 	fConst558;
	float 	fConst559;
	float 	fConst560;
	float 	fRec186[3];
	float 	fConst561;
	float 	fConst562;
	float 	fConst563;
	float 	fRec185[3];
	float 	fConst564;
	float 	fConst565;
	float 	fConst566;
	float 	fConst567;
	float 	fRec184[3];
	float 	fConst568;
	float 	fConst569;
	float 	fConst570;
	float 	fRec183[3];
	float 	fConst571;
	float 	fConst572;
	float 	fConst573;
	float 	fRec182[3];
	float 	fConst574;
	float 	fRec181[2];
	FAUSTFLOAT 	fbargraph11;
	float 	fConst575;
	float 	fConst576;
	float 	fConst577;
	float 	fConst578;
	float 	fConst579;
	float 	fConst580;
	float 	fConst581;
	float 	fConst582;
	float 	fConst583;
	float 	fConst584;
	float 	fConst585;
	float 	fConst586;
	float 	fConst587;
	float 	fConst588;
	float 	fConst589;
	float 	fConst590;
	float 	fConst591;
	float 	fConst592;
	float 	fRec194[3];
	float 	fConst593;
	float 	fConst594;
	float 	fConst595;
	float 	fRec193[3];
	float 	fConst596;
	float 	fConst597;
	float 	fConst598;
	float 	fRec192[3];
	float 	fConst599;
	float 	fConst600;
	float 	fConst601;
	float 	fRec191[3];
	float 	fConst602;
	float 	fConst603;
	float 	fConst604;
	float 	fRec190[3];
	float 	fConst605;
	float 	fConst606;
	float 	fConst607;
	float 	fRec189[3];
	float 	fConst608;
	float 	fRec188[2];
	FAUSTFLOAT 	fbargraph12;
	float 	fConst609;
	float 	fConst610;
	float 	fConst611;
	float 	fConst612;
	float 	fConst613;
	float 	fConst614;
	float 	fConst615;
	float 	fConst616;
	float 	fConst617;
	float 	fConst618;
	float 	fConst619;
	float 	fConst620;
	float 	fConst621;
	float 	fConst622;
	float 	fConst623;
	float 	fConst624;
	float 	fConst625;
	float 	fConst626;
	float 	fConst627;
	float 	fConst628;
	float 	fConst629;
	float 	fRec201[3];
	float 	fConst630;
	float 	fConst631;
	float 	fConst632;
	float 	fRec200[3];
	float 	fConst633;
	float 	fConst634;
	float 	fConst635;
	float 	fRec199[3];
	float 	fConst636;
	float 	fConst637;
	float 	fConst638;
	float 	fConst639;
	float 	fRec198[3];
	float 	fConst640;
	float 	fConst641;
	float 	fConst642;
	float 	fRec197[3];
	float 	fConst643;
	float 	fConst644;
	float 	fConst645;
	float 	fRec196[3];
	float 	fConst646;
	float 	fRec195[2];
	FAUSTFLOAT 	fbargraph13;
	float 	fConst647;
	float 	fConst648;
	float 	fConst649;
	float 	fConst650;
	float 	fConst651;
	float 	fConst652;
	float 	fConst653;
	float 	fConst654;
	float 	fConst655;
	float 	fRec205[3];
	float 	fConst656;
	float 	fConst657;
	float 	fConst658;
	float 	fRec204[3];
	float 	fConst659;
	float 	fConst660;
	float 	fConst661;
	float 	fRec203[3];
	float 	fConst662;
	float 	fRec202[2];
	FAUSTFLOAT 	fbargraph14;
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("oscillator.lib/name", "Faust Oscillator Library");
		m->declare("oscillator.lib/author", "Julius O. Smith (jos at ccrma.stanford.edu)");
		m->declare("oscillator.lib/copyright", "Julius O. Smith III");
		m->declare("oscillator.lib/version", "1.11");
		m->declare("oscillator.lib/license", "STK-4.3");
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
		m->declare("filter.lib/name", "Faust Filter Library");
		m->declare("filter.lib/author", "Julius O. Smith (jos at ccrma.stanford.edu)");
		m->declare("filter.lib/copyright", "Julius O. Smith III");
		m->declare("filter.lib/version", "1.29");
		m->declare("filter.lib/license", "STK-4.3");
		m->declare("filter.lib/reference", "https://ccrma.stanford.edu/~jos/filters/");
		m->declare("effect.lib/name", "Faust Audio Effect Library");
		m->declare("effect.lib/author", "Julius O. Smith (jos at ccrma.stanford.edu)");
		m->declare("effect.lib/copyright", "Julius O. Smith III");
		m->declare("effect.lib/version", "1.33");
		m->declare("effect.lib/license", "STK-4.3");
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 5e+01f;
		iConst0 = min(192000, max(1, fSamplingFreq));
		fConst1 = (1.0f / float(iConst0));
		fslider1 = 0.1f;
		fConst2 = tanf((31415.926535897932f / float(iConst0)));
		fConst3 = (1.0f / fConst2);
		fConst4 = (1.0f / (0.9351401670315425f + ((fConst3 + 0.157482159302087f) / fConst2)));
		fConst5 = faustpower<2>(fConst2);
		fConst6 = (50.063807016150385f / fConst5);
		fConst7 = (0.9351401670315425f + fConst6);
		fConst8 = (1.0f / (1.450071084655647f + ((fConst3 + 0.7431304601070396f) / fConst2)));
		fConst9 = (11.052052171507189f / fConst5);
		fConst10 = (1.450071084655647f + fConst9);
		fConst11 = (1.0f / (4.076781969643807f + ((fConst3 + 3.1897274020965583f) / fConst2)));
		fConst12 = (0.0017661728399818856f / fConst5);
		fConst13 = (0.00040767818495825777f + fConst12);
		fcheckbox0 = 0.0;
		fslider2 = -2e+01f;
		for (int i=0; i<2; i++) fRec4[i] = 0;
		fcheckbox1 = 0.0;
		fcheckbox2 = 0.0;
		for (int i=0; i<2; i++) iRec6[i] = 0;
		for (int i=0; i<4; i++) fRec5[i] = 0;
		fConst14 = (0.3333333333333333f * iConst0);
		for (int i=0; i<2; i++) fVec0[i] = 0;
		fConst15 = float(iConst0);
		fConst16 = (2.0f / fConst15);
		fslider3 = 0.1f;
		fslider4 = 49.0f;
		for (int i=0; i<2; i++) fRec7[i] = 0;
		for (int i=0; i<2; i++) fRec8[i] = 0;
		for (int i=0; i<2; i++) fVec1[i] = 0;
		fslider5 = -0.1f;
		for (int i=0; i<2; i++) fRec9[i] = 0;
		for (int i=0; i<2; i++) fVec2[i] = 0;
		fslider6 = 0.1f;
		for (int i=0; i<2; i++) fRec10[i] = 0;
		for (int i=0; i<2; i++) fVec3[i] = 0;
		fConst17 = (1 + ((fConst3 + 0.6180339887498947f) / fConst2));
		fConst18 = (1.0f / fConst17);
		fConst19 = (1.0f / fConst5);
		fConst20 = (1.0f / (1 + ((fConst3 + 1.6180339887498947f) / fConst2)));
		fConst21 = (1 + fConst3);
		fConst22 = (0 - ((1 - fConst3) / fConst21));
		fConst23 = (1.0f / fConst21);
		for (int i=0; i<2; i++) fVec4[i] = 0;
		fConst24 = (0 - fConst3);
		for (int i=0; i<2; i++) fRec21[i] = 0;
		fConst25 = (1 + ((fConst3 - 1.6180339887498947f) / fConst2));
		fConst26 = (2 * (1 - fConst19));
		for (int i=0; i<3; i++) fRec20[i] = 0;
		fConst27 = (2 * (0 - fConst19));
		fConst28 = (1 + ((fConst3 - 0.6180339887498947f) / fConst2));
		for (int i=0; i<3; i++) fRec19[i] = 0;
		fConst29 = tanf((122.7184630308513f / float(iConst0)));
		fConst30 = (1.0f / fConst29);
		fConst31 = (1.0f / (1 + ((1.618033988749895f + fConst30) / fConst29)));
		fConst32 = (1 + ((fConst30 - 1.618033988749895f) / fConst29));
		fConst33 = faustpower<2>(fConst29);
		fConst34 = (1.0f / fConst33);
		fConst35 = (2 * (1 - fConst34));
		for (int i=0; i<3; i++) fRec18[i] = 0;
		fConst36 = tanf((245.4369260617026f / float(iConst0)));
		fConst37 = (1.0f / fConst36);
		fConst38 = (1.0f / (1 + ((1.618033988749895f + fConst37) / fConst36)));
		fConst39 = (1 + ((fConst37 - 1.618033988749895f) / fConst36));
		fConst40 = (1.0f / faustpower<2>(fConst36));
		fConst41 = (2 * (1 - fConst40));
		for (int i=0; i<3; i++) fRec17[i] = 0;
		fConst42 = tanf((490.8738521234052f / float(iConst0)));
		fConst43 = (1.0f / fConst42);
		fConst44 = (1.0f / (1 + ((1.618033988749895f + fConst43) / fConst42)));
		fConst45 = (1 + ((fConst43 - 1.618033988749895f) / fConst42));
		fConst46 = faustpower<2>(fConst42);
		fConst47 = (1.0f / fConst46);
		fConst48 = (2 * (1 - fConst47));
		for (int i=0; i<3; i++) fRec16[i] = 0;
		fConst49 = tanf((981.7477042468104f / float(iConst0)));
		fConst50 = (1.0f / fConst49);
		fConst51 = (1.0f / (1 + ((1.618033988749895f + fConst50) / fConst49)));
		fConst52 = (1 + ((fConst50 - 1.618033988749895f) / fConst49));
		fConst53 = (1.0f / faustpower<2>(fConst49));
		fConst54 = (2 * (1 - fConst53));
		for (int i=0; i<3; i++) fRec15[i] = 0;
		fConst55 = tanf((1963.4954084936207f / float(iConst0)));
		fConst56 = (1.0f / fConst55);
		fConst57 = (1.0f / (1 + ((1.618033988749895f + fConst56) / fConst55)));
		fConst58 = (1 + ((fConst56 - 1.618033988749895f) / fConst55));
		fConst59 = faustpower<2>(fConst55);
		fConst60 = (1.0f / fConst59);
		fConst61 = (2 * (1 - fConst60));
		for (int i=0; i<3; i++) fRec14[i] = 0;
		fConst62 = tanf((3926.9908169872415f / float(iConst0)));
		fConst63 = (1.0f / fConst62);
		fConst64 = (1.0f / (1 + ((1.618033988749895f + fConst63) / fConst62)));
		fConst65 = (1 + ((fConst63 - 1.618033988749895f) / fConst62));
		fConst66 = (1.0f / faustpower<2>(fConst62));
		fConst67 = (2 * (1 - fConst66));
		for (int i=0; i<3; i++) fRec13[i] = 0;
		fConst68 = tanf((7853.981633974483f / float(iConst0)));
		fConst69 = (1.0f / fConst68);
		fConst70 = (1.0f / (1 + ((1.618033988749895f + fConst69) / fConst68)));
		fConst71 = (1 + ((fConst69 - 1.618033988749895f) / fConst68));
		fConst72 = faustpower<2>(fConst68);
		fConst73 = (1.0f / fConst72);
		fConst74 = (2 * (1 - fConst73));
		for (int i=0; i<3; i++) fRec12[i] = 0;
		fConst75 = tanf((15707.963267948966f / float(iConst0)));
		fConst76 = (1.0f / fConst75);
		fConst77 = (1.0f / (1 + ((1.618033988749895f + fConst76) / fConst75)));
		fConst78 = (1 + ((fConst76 - 1.618033988749895f) / fConst75));
		fConst79 = (1.0f / faustpower<2>(fConst75));
		fConst80 = (2 * (1 - fConst79));
		for (int i=0; i<3; i++) fRec11[i] = 0;
		fslider7 = -1e+01f;
		for (int i=0; i<2; i++) fRec22[i] = 0;
		fConst81 = (1 + ((0.6180339887498947f + fConst76) / fConst75));
		fConst82 = (1.0f / fConst81);
		fConst83 = (1.0f / (1 + ((1.6180339887498947f + fConst76) / fConst75)));
		fConst84 = (1 + fConst76);
		fConst85 = (0 - ((1 - fConst76) / fConst84));
		fConst86 = (1.0f / (fConst17 * fConst84));
		for (int i=0; i<2; i++) fRec35[i] = 0;
		for (int i=0; i<3; i++) fRec34[i] = 0;
		for (int i=0; i<3; i++) fRec33[i] = 0;
		for (int i=0; i<2; i++) fVec5[i] = 0;
		fConst87 = (0 - fConst76);
		for (int i=0; i<2; i++) fRec32[i] = 0;
		fConst88 = (1 + ((fConst76 - 1.6180339887498947f) / fConst75));
		for (int i=0; i<3; i++) fRec31[i] = 0;
		fConst89 = (2 * (0 - fConst79));
		fConst90 = (1 + ((fConst76 - 0.6180339887498947f) / fConst75));
		for (int i=0; i<3; i++) fRec30[i] = 0;
		for (int i=0; i<3; i++) fRec29[i] = 0;
		for (int i=0; i<3; i++) fRec28[i] = 0;
		for (int i=0; i<3; i++) fRec27[i] = 0;
		for (int i=0; i<3; i++) fRec26[i] = 0;
		for (int i=0; i<3; i++) fRec25[i] = 0;
		for (int i=0; i<3; i++) fRec24[i] = 0;
		for (int i=0; i<3; i++) fRec23[i] = 0;
		fslider8 = -1e+01f;
		for (int i=0; i<2; i++) fRec36[i] = 0;
		fConst91 = (1 + ((0.6180339887498947f + fConst69) / fConst68));
		fConst92 = (1.0f / fConst91);
		fConst93 = (1.0f / (1 + ((1.6180339887498947f + fConst69) / fConst68)));
		fConst94 = (1 + fConst69);
		fConst95 = (0 - ((1 - fConst69) / fConst94));
		fConst96 = (1.0f / (fConst81 * fConst94));
		for (int i=0; i<2; i++) fRec48[i] = 0;
		for (int i=0; i<3; i++) fRec47[i] = 0;
		for (int i=0; i<3; i++) fRec46[i] = 0;
		for (int i=0; i<2; i++) fVec6[i] = 0;
		fConst97 = (0 - fConst69);
		for (int i=0; i<2; i++) fRec45[i] = 0;
		fConst98 = (1 + ((fConst69 - 1.6180339887498947f) / fConst68));
		for (int i=0; i<3; i++) fRec44[i] = 0;
		fConst99 = (2 * (0 - fConst73));
		fConst100 = (1 + ((fConst69 - 0.6180339887498947f) / fConst68));
		for (int i=0; i<3; i++) fRec43[i] = 0;
		for (int i=0; i<3; i++) fRec42[i] = 0;
		for (int i=0; i<3; i++) fRec41[i] = 0;
		for (int i=0; i<3; i++) fRec40[i] = 0;
		for (int i=0; i<3; i++) fRec39[i] = 0;
		for (int i=0; i<3; i++) fRec38[i] = 0;
		for (int i=0; i<3; i++) fRec37[i] = 0;
		fslider9 = -1e+01f;
		for (int i=0; i<2; i++) fRec49[i] = 0;
		fConst101 = (1 + ((0.6180339887498947f + fConst63) / fConst62));
		fConst102 = (1.0f / fConst101);
		fConst103 = (1.0f / (1 + ((1.6180339887498947f + fConst63) / fConst62)));
		fConst104 = (1 + fConst63);
		fConst105 = (0 - ((1 - fConst63) / fConst104));
		fConst106 = (1.0f / (fConst91 * fConst104));
		for (int i=0; i<2; i++) fRec60[i] = 0;
		for (int i=0; i<3; i++) fRec59[i] = 0;
		for (int i=0; i<3; i++) fRec58[i] = 0;
		for (int i=0; i<2; i++) fVec7[i] = 0;
		fConst107 = (0 - fConst63);
		for (int i=0; i<2; i++) fRec57[i] = 0;
		fConst108 = (1 + ((fConst63 - 1.6180339887498947f) / fConst62));
		for (int i=0; i<3; i++) fRec56[i] = 0;
		fConst109 = (2 * (0 - fConst66));
		fConst110 = (1 + ((fConst63 - 0.6180339887498947f) / fConst62));
		for (int i=0; i<3; i++) fRec55[i] = 0;
		for (int i=0; i<3; i++) fRec54[i] = 0;
		for (int i=0; i<3; i++) fRec53[i] = 0;
		for (int i=0; i<3; i++) fRec52[i] = 0;
		for (int i=0; i<3; i++) fRec51[i] = 0;
		for (int i=0; i<3; i++) fRec50[i] = 0;
		fslider10 = -1e+01f;
		for (int i=0; i<2; i++) fRec61[i] = 0;
		fConst111 = (1 + ((0.6180339887498947f + fConst56) / fConst55));
		fConst112 = (1.0f / fConst111);
		fConst113 = (1.0f / (1 + ((1.6180339887498947f + fConst56) / fConst55)));
		fConst114 = (1 + fConst56);
		fConst115 = (0 - ((1 - fConst56) / fConst114));
		fConst116 = (1.0f / (fConst101 * fConst114));
		for (int i=0; i<2; i++) fRec71[i] = 0;
		for (int i=0; i<3; i++) fRec70[i] = 0;
		for (int i=0; i<3; i++) fRec69[i] = 0;
		for (int i=0; i<2; i++) fVec8[i] = 0;
		fConst117 = (0 - fConst56);
		for (int i=0; i<2; i++) fRec68[i] = 0;
		fConst118 = (1 + ((fConst56 - 1.6180339887498947f) / fConst55));
		for (int i=0; i<3; i++) fRec67[i] = 0;
		fConst119 = (2 * (0 - fConst60));
		fConst120 = (1 + ((fConst56 - 0.6180339887498947f) / fConst55));
		for (int i=0; i<3; i++) fRec66[i] = 0;
		for (int i=0; i<3; i++) fRec65[i] = 0;
		for (int i=0; i<3; i++) fRec64[i] = 0;
		for (int i=0; i<3; i++) fRec63[i] = 0;
		for (int i=0; i<3; i++) fRec62[i] = 0;
		fslider11 = -1e+01f;
		for (int i=0; i<2; i++) fRec72[i] = 0;
		fConst121 = (1 + ((0.6180339887498947f + fConst50) / fConst49));
		fConst122 = (1.0f / fConst121);
		fConst123 = (1.0f / (1 + ((1.6180339887498947f + fConst50) / fConst49)));
		fConst124 = (1 + fConst50);
		fConst125 = (0 - ((1 - fConst50) / fConst124));
		fConst126 = (1.0f / (fConst111 * fConst124));
		for (int i=0; i<2; i++) fRec81[i] = 0;
		for (int i=0; i<3; i++) fRec80[i] = 0;
		for (int i=0; i<3; i++) fRec79[i] = 0;
		for (int i=0; i<2; i++) fVec9[i] = 0;
		fConst127 = (0 - fConst50);
		for (int i=0; i<2; i++) fRec78[i] = 0;
		fConst128 = (1 + ((fConst50 - 1.6180339887498947f) / fConst49));
		for (int i=0; i<3; i++) fRec77[i] = 0;
		fConst129 = (2 * (0 - fConst53));
		fConst130 = (1 + ((fConst50 - 0.6180339887498947f) / fConst49));
		for (int i=0; i<3; i++) fRec76[i] = 0;
		for (int i=0; i<3; i++) fRec75[i] = 0;
		for (int i=0; i<3; i++) fRec74[i] = 0;
		for (int i=0; i<3; i++) fRec73[i] = 0;
		fslider12 = -1e+01f;
		for (int i=0; i<2; i++) fRec82[i] = 0;
		fConst131 = (1 + ((0.6180339887498947f + fConst43) / fConst42));
		fConst132 = (1.0f / fConst131);
		fConst133 = (1.0f / (1 + ((1.6180339887498947f + fConst43) / fConst42)));
		fConst134 = (1 + fConst43);
		fConst135 = (0 - ((1 - fConst43) / fConst134));
		fConst136 = (1.0f / (fConst121 * fConst134));
		for (int i=0; i<2; i++) fRec90[i] = 0;
		for (int i=0; i<3; i++) fRec89[i] = 0;
		for (int i=0; i<3; i++) fRec88[i] = 0;
		for (int i=0; i<2; i++) fVec10[i] = 0;
		fConst137 = (0 - fConst43);
		for (int i=0; i<2; i++) fRec87[i] = 0;
		fConst138 = (1 + ((fConst43 - 1.6180339887498947f) / fConst42));
		for (int i=0; i<3; i++) fRec86[i] = 0;
		fConst139 = (2 * (0 - fConst47));
		fConst140 = (1 + ((fConst43 - 0.6180339887498947f) / fConst42));
		for (int i=0; i<3; i++) fRec85[i] = 0;
		for (int i=0; i<3; i++) fRec84[i] = 0;
		for (int i=0; i<3; i++) fRec83[i] = 0;
		fslider13 = -1e+01f;
		for (int i=0; i<2; i++) fRec91[i] = 0;
		fConst141 = (1 + ((0.6180339887498947f + fConst37) / fConst36));
		fConst142 = (1.0f / fConst141);
		fConst143 = (1.0f / (1 + ((1.6180339887498947f + fConst37) / fConst36)));
		fConst144 = (1 + fConst37);
		fConst145 = (0 - ((1 - fConst37) / fConst144));
		fConst146 = (1.0f / (fConst131 * fConst144));
		for (int i=0; i<2; i++) fRec98[i] = 0;
		for (int i=0; i<3; i++) fRec97[i] = 0;
		for (int i=0; i<3; i++) fRec96[i] = 0;
		for (int i=0; i<2; i++) fVec11[i] = 0;
		fConst147 = (0 - fConst37);
		for (int i=0; i<2; i++) fRec95[i] = 0;
		fConst148 = (1 + ((fConst37 - 1.6180339887498947f) / fConst36));
		for (int i=0; i<3; i++) fRec94[i] = 0;
		fConst149 = (2 * (0 - fConst40));
		fConst150 = (1 + ((fConst37 - 0.6180339887498947f) / fConst36));
		for (int i=0; i<3; i++) fRec93[i] = 0;
		for (int i=0; i<3; i++) fRec92[i] = 0;
		fslider14 = -1e+01f;
		for (int i=0; i<2; i++) fRec99[i] = 0;
		fConst151 = (1.0f / (1 + ((0.6180339887498947f + fConst30) / fConst29)));
		fConst152 = (1.0f / (1 + ((1.6180339887498947f + fConst30) / fConst29)));
		fConst153 = (1 + fConst30);
		fConst154 = (0 - ((1 - fConst30) / fConst153));
		fConst155 = (1.0f / (fConst141 * fConst153));
		for (int i=0; i<2; i++) fRec105[i] = 0;
		for (int i=0; i<3; i++) fRec104[i] = 0;
		for (int i=0; i<3; i++) fRec103[i] = 0;
		for (int i=0; i<2; i++) fVec12[i] = 0;
		fConst156 = (0 - fConst30);
		for (int i=0; i<2; i++) fRec102[i] = 0;
		fConst157 = (1 + ((fConst30 - 1.6180339887498947f) / fConst29));
		for (int i=0; i<3; i++) fRec101[i] = 0;
		fConst158 = (2 * (0 - fConst34));
		fConst159 = (1 + ((fConst30 - 0.6180339887498947f) / fConst29));
		for (int i=0; i<3; i++) fRec100[i] = 0;
		fslider15 = -1e+01f;
		for (int i=0; i<2; i++) fRec106[i] = 0;
		for (int i=0; i<2; i++) fRec109[i] = 0;
		for (int i=0; i<3; i++) fRec108[i] = 0;
		for (int i=0; i<3; i++) fRec107[i] = 0;
		fslider16 = -1e+01f;
		for (int i=0; i<2; i++) fRec110[i] = 0;
		fConst160 = (4.076781969643807f + ((fConst3 - 3.1897274020965583f) / fConst2));
		fConst161 = (2 * (4.076781969643807f - fConst19));
		for (int i=0; i<3; i++) fRec3[i] = 0;
		fConst162 = (2 * (0.00040767818495825777f - fConst12));
		fConst163 = (1.450071084655647f + ((fConst3 - 0.7431304601070396f) / fConst2));
		fConst164 = (2 * (1.450071084655647f - fConst19));
		for (int i=0; i<3; i++) fRec2[i] = 0;
		fConst165 = (2 * (1.450071084655647f - fConst9));
		fConst166 = (0.9351401670315425f + ((fConst3 - 0.157482159302087f) / fConst2));
		fConst167 = (2 * (0.9351401670315425f - fConst19));
		for (int i=0; i<3; i++) fRec1[i] = 0;
		fConst168 = (2 * (0.9351401670315425f - fConst6));
		for (int i=0; i<2; i++) fRec0[i] = 0;
		fConst169 = tanf((19790.793572264363f / float(iConst0)));
		fConst170 = (1.0f / fConst169);
		fConst171 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst170) / fConst169)));
		fConst172 = faustpower<2>(fConst169);
		fConst173 = (50.063807016150385f / fConst172);
		fConst174 = (0.9351401670315425f + fConst173);
		fConst175 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst170) / fConst169)));
		fConst176 = (11.052052171507189f / fConst172);
		fConst177 = (1.450071084655647f + fConst176);
		fConst178 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst170) / fConst169)));
		fConst179 = (0.0017661728399818856f / fConst172);
		fConst180 = (0.00040767818495825777f + fConst179);
		fConst181 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst3) / fConst2)));
		fConst182 = (53.53615295455673f + fConst19);
		fConst183 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst3) / fConst2)));
		fConst184 = (7.621731298870603f + fConst19);
		fConst185 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst3) / fConst2)));
		fConst186 = (9.9999997055e-05f / fConst5);
		fConst187 = (0.000433227200555f + fConst186);
		fConst188 = (0.24529150870616f + ((fConst3 - 0.782413046821645f) / fConst2));
		fConst189 = (2 * (0.24529150870616f - fConst19));
		for (int i=0; i<3; i++) fRec117[i] = 0;
		fConst190 = (2 * (0.000433227200555f - fConst186));
		fConst191 = (0.689621364484675f + ((fConst3 - 0.512478641889141f) / fConst2));
		fConst192 = (2 * (0.689621364484675f - fConst19));
		for (int i=0; i<3; i++) fRec116[i] = 0;
		fConst193 = (2 * (7.621731298870603f - fConst19));
		fConst194 = (1.069358407707312f + ((fConst3 - 0.168404871113589f) / fConst2));
		fConst195 = (2 * (1.069358407707312f - fConst19));
		for (int i=0; i<3; i++) fRec115[i] = 0;
		fConst196 = (2 * (53.53615295455673f - fConst19));
		fConst197 = (4.076781969643807f + ((fConst170 - 3.1897274020965583f) / fConst169));
		fConst198 = (1.0f / fConst172);
		fConst199 = (2 * (4.076781969643807f - fConst198));
		for (int i=0; i<3; i++) fRec114[i] = 0;
		fConst200 = (2 * (0.00040767818495825777f - fConst179));
		fConst201 = (1.450071084655647f + ((fConst170 - 0.7431304601070396f) / fConst169));
		fConst202 = (2 * (1.450071084655647f - fConst198));
		for (int i=0; i<3; i++) fRec113[i] = 0;
		fConst203 = (2 * (1.450071084655647f - fConst176));
		fConst204 = (0.9351401670315425f + ((fConst170 - 0.157482159302087f) / fConst169));
		fConst205 = (2 * (0.9351401670315425f - fConst198));
		for (int i=0; i<3; i++) fRec112[i] = 0;
		fConst206 = (2 * (0.9351401670315425f - fConst173));
		for (int i=0; i<2; i++) fRec111[i] = 0;
		fConst207 = tanf((12467.418707910012f / float(iConst0)));
		fConst208 = (1.0f / fConst207);
		fConst209 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst208) / fConst207)));
		fConst210 = faustpower<2>(fConst207);
		fConst211 = (50.063807016150385f / fConst210);
		fConst212 = (0.9351401670315425f + fConst211);
		fConst213 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst208) / fConst207)));
		fConst214 = (11.052052171507189f / fConst210);
		fConst215 = (1.450071084655647f + fConst214);
		fConst216 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst208) / fConst207)));
		fConst217 = (0.0017661728399818856f / fConst210);
		fConst218 = (0.00040767818495825777f + fConst217);
		fConst219 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst170) / fConst169)));
		fConst220 = (53.53615295455673f + fConst198);
		fConst221 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst170) / fConst169)));
		fConst222 = (7.621731298870603f + fConst198);
		fConst223 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst170) / fConst169)));
		fConst224 = (9.9999997055e-05f / fConst172);
		fConst225 = (0.000433227200555f + fConst224);
		fConst226 = (0.24529150870616f + ((fConst170 - 0.782413046821645f) / fConst169));
		fConst227 = (2 * (0.24529150870616f - fConst198));
		for (int i=0; i<3; i++) fRec124[i] = 0;
		fConst228 = (2 * (0.000433227200555f - fConst224));
		fConst229 = (0.689621364484675f + ((fConst170 - 0.512478641889141f) / fConst169));
		fConst230 = (2 * (0.689621364484675f - fConst198));
		for (int i=0; i<3; i++) fRec123[i] = 0;
		fConst231 = (2 * (7.621731298870603f - fConst198));
		fConst232 = (1.069358407707312f + ((fConst170 - 0.168404871113589f) / fConst169));
		fConst233 = (2 * (1.069358407707312f - fConst198));
		for (int i=0; i<3; i++) fRec122[i] = 0;
		fConst234 = (2 * (53.53615295455673f - fConst198));
		fConst235 = (4.076781969643807f + ((fConst208 - 3.1897274020965583f) / fConst207));
		fConst236 = (1.0f / fConst210);
		fConst237 = (2 * (4.076781969643807f - fConst236));
		for (int i=0; i<3; i++) fRec121[i] = 0;
		fConst238 = (2 * (0.00040767818495825777f - fConst217));
		fConst239 = (1.450071084655647f + ((fConst208 - 0.7431304601070396f) / fConst207));
		fConst240 = (2 * (1.450071084655647f - fConst236));
		for (int i=0; i<3; i++) fRec120[i] = 0;
		fConst241 = (2 * (1.450071084655647f - fConst214));
		fConst242 = (0.9351401670315425f + ((fConst208 - 0.157482159302087f) / fConst207));
		fConst243 = (2 * (0.9351401670315425f - fConst236));
		for (int i=0; i<3; i++) fRec119[i] = 0;
		fConst244 = (2 * (0.9351401670315425f - fConst211));
		for (int i=0; i<2; i++) fRec118[i] = 0;
		fConst245 = (1.0f / (0.9351401670315425f + ((fConst69 + 0.157482159302087f) / fConst68)));
		fConst246 = (50.063807016150385f / fConst72);
		fConst247 = (0.9351401670315425f + fConst246);
		fConst248 = (1.0f / (1.450071084655647f + ((fConst69 + 0.7431304601070396f) / fConst68)));
		fConst249 = (11.052052171507189f / fConst72);
		fConst250 = (1.450071084655647f + fConst249);
		fConst251 = (1.0f / (4.076781969643807f + ((fConst69 + 3.1897274020965583f) / fConst68)));
		fConst252 = (0.0017661728399818856f / fConst72);
		fConst253 = (0.00040767818495825777f + fConst252);
		fConst254 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst208) / fConst207)));
		fConst255 = (53.53615295455673f + fConst236);
		fConst256 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst208) / fConst207)));
		fConst257 = (7.621731298870603f + fConst236);
		fConst258 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst208) / fConst207)));
		fConst259 = (9.9999997055e-05f / fConst210);
		fConst260 = (0.000433227200555f + fConst259);
		fConst261 = (0.24529150870616f + ((fConst208 - 0.782413046821645f) / fConst207));
		fConst262 = (2 * (0.24529150870616f - fConst236));
		for (int i=0; i<3; i++) fRec131[i] = 0;
		fConst263 = (2 * (0.000433227200555f - fConst259));
		fConst264 = (0.689621364484675f + ((fConst208 - 0.512478641889141f) / fConst207));
		fConst265 = (2 * (0.689621364484675f - fConst236));
		for (int i=0; i<3; i++) fRec130[i] = 0;
		fConst266 = (2 * (7.621731298870603f - fConst236));
		fConst267 = (1.069358407707312f + ((fConst208 - 0.168404871113589f) / fConst207));
		fConst268 = (2 * (1.069358407707312f - fConst236));
		for (int i=0; i<3; i++) fRec129[i] = 0;
		fConst269 = (2 * (53.53615295455673f - fConst236));
		fConst270 = (4.076781969643807f + ((fConst69 - 3.1897274020965583f) / fConst68));
		fConst271 = (2 * (4.076781969643807f - fConst73));
		for (int i=0; i<3; i++) fRec128[i] = 0;
		fConst272 = (2 * (0.00040767818495825777f - fConst252));
		fConst273 = (1.450071084655647f + ((fConst69 - 0.7431304601070396f) / fConst68));
		fConst274 = (2 * (1.450071084655647f - fConst73));
		for (int i=0; i<3; i++) fRec127[i] = 0;
		fConst275 = (2 * (1.450071084655647f - fConst249));
		fConst276 = (0.9351401670315425f + ((fConst69 - 0.157482159302087f) / fConst68));
		fConst277 = (2 * (0.9351401670315425f - fConst73));
		for (int i=0; i<3; i++) fRec126[i] = 0;
		fConst278 = (2 * (0.9351401670315425f - fConst246));
		for (int i=0; i<2; i++) fRec125[i] = 0;
		fConst279 = tanf((4947.698393066091f / float(iConst0)));
		fConst280 = (1.0f / fConst279);
		fConst281 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst280) / fConst279)));
		fConst282 = faustpower<2>(fConst279);
		fConst283 = (50.063807016150385f / fConst282);
		fConst284 = (0.9351401670315425f + fConst283);
		fConst285 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst280) / fConst279)));
		fConst286 = (11.052052171507189f / fConst282);
		fConst287 = (1.450071084655647f + fConst286);
		fConst288 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst280) / fConst279)));
		fConst289 = (0.0017661728399818856f / fConst282);
		fConst290 = (0.00040767818495825777f + fConst289);
		fConst291 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst69) / fConst68)));
		fConst292 = (53.53615295455673f + fConst73);
		fConst293 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst69) / fConst68)));
		fConst294 = (7.621731298870603f + fConst73);
		fConst295 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst69) / fConst68)));
		fConst296 = (9.9999997055e-05f / fConst72);
		fConst297 = (0.000433227200555f + fConst296);
		fConst298 = (0.24529150870616f + ((fConst69 - 0.782413046821645f) / fConst68));
		fConst299 = (2 * (0.24529150870616f - fConst73));
		for (int i=0; i<3; i++) fRec138[i] = 0;
		fConst300 = (2 * (0.000433227200555f - fConst296));
		fConst301 = (0.689621364484675f + ((fConst69 - 0.512478641889141f) / fConst68));
		fConst302 = (2 * (0.689621364484675f - fConst73));
		for (int i=0; i<3; i++) fRec137[i] = 0;
		fConst303 = (2 * (7.621731298870603f - fConst73));
		fConst304 = (1.069358407707312f + ((fConst69 - 0.168404871113589f) / fConst68));
		fConst305 = (2 * (1.069358407707312f - fConst73));
		for (int i=0; i<3; i++) fRec136[i] = 0;
		fConst306 = (2 * (53.53615295455673f - fConst73));
		fConst307 = (4.076781969643807f + ((fConst280 - 3.1897274020965583f) / fConst279));
		fConst308 = (1.0f / fConst282);
		fConst309 = (2 * (4.076781969643807f - fConst308));
		for (int i=0; i<3; i++) fRec135[i] = 0;
		fConst310 = (2 * (0.00040767818495825777f - fConst289));
		fConst311 = (1.450071084655647f + ((fConst280 - 0.7431304601070396f) / fConst279));
		fConst312 = (2 * (1.450071084655647f - fConst308));
		for (int i=0; i<3; i++) fRec134[i] = 0;
		fConst313 = (2 * (1.450071084655647f - fConst286));
		fConst314 = (0.9351401670315425f + ((fConst280 - 0.157482159302087f) / fConst279));
		fConst315 = (2 * (0.9351401670315425f - fConst308));
		for (int i=0; i<3; i++) fRec133[i] = 0;
		fConst316 = (2 * (0.9351401670315425f - fConst283));
		for (int i=0; i<2; i++) fRec132[i] = 0;
		fConst317 = tanf((3116.8546769775025f / float(iConst0)));
		fConst318 = (1.0f / fConst317);
		fConst319 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst318) / fConst317)));
		fConst320 = faustpower<2>(fConst317);
		fConst321 = (50.063807016150385f / fConst320);
		fConst322 = (0.9351401670315425f + fConst321);
		fConst323 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst318) / fConst317)));
		fConst324 = (11.052052171507189f / fConst320);
		fConst325 = (1.450071084655647f + fConst324);
		fConst326 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst318) / fConst317)));
		fConst327 = (0.0017661728399818856f / fConst320);
		fConst328 = (0.00040767818495825777f + fConst327);
		fConst329 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst280) / fConst279)));
		fConst330 = (53.53615295455673f + fConst308);
		fConst331 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst280) / fConst279)));
		fConst332 = (7.621731298870603f + fConst308);
		fConst333 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst280) / fConst279)));
		fConst334 = (9.9999997055e-05f / fConst282);
		fConst335 = (0.000433227200555f + fConst334);
		fConst336 = (0.24529150870616f + ((fConst280 - 0.782413046821645f) / fConst279));
		fConst337 = (2 * (0.24529150870616f - fConst308));
		for (int i=0; i<3; i++) fRec145[i] = 0;
		fConst338 = (2 * (0.000433227200555f - fConst334));
		fConst339 = (0.689621364484675f + ((fConst280 - 0.512478641889141f) / fConst279));
		fConst340 = (2 * (0.689621364484675f - fConst308));
		for (int i=0; i<3; i++) fRec144[i] = 0;
		fConst341 = (2 * (7.621731298870603f - fConst308));
		fConst342 = (1.069358407707312f + ((fConst280 - 0.168404871113589f) / fConst279));
		fConst343 = (2 * (1.069358407707312f - fConst308));
		for (int i=0; i<3; i++) fRec143[i] = 0;
		fConst344 = (2 * (53.53615295455673f - fConst308));
		fConst345 = (4.076781969643807f + ((fConst318 - 3.1897274020965583f) / fConst317));
		fConst346 = (1.0f / fConst320);
		fConst347 = (2 * (4.076781969643807f - fConst346));
		for (int i=0; i<3; i++) fRec142[i] = 0;
		fConst348 = (2 * (0.00040767818495825777f - fConst327));
		fConst349 = (1.450071084655647f + ((fConst318 - 0.7431304601070396f) / fConst317));
		fConst350 = (2 * (1.450071084655647f - fConst346));
		for (int i=0; i<3; i++) fRec141[i] = 0;
		fConst351 = (2 * (1.450071084655647f - fConst324));
		fConst352 = (0.9351401670315425f + ((fConst318 - 0.157482159302087f) / fConst317));
		fConst353 = (2 * (0.9351401670315425f - fConst346));
		for (int i=0; i<3; i++) fRec140[i] = 0;
		fConst354 = (2 * (0.9351401670315425f - fConst321));
		for (int i=0; i<2; i++) fRec139[i] = 0;
		fConst355 = (1.0f / (0.9351401670315425f + ((fConst56 + 0.157482159302087f) / fConst55)));
		fConst356 = (50.063807016150385f / fConst59);
		fConst357 = (0.9351401670315425f + fConst356);
		fConst358 = (1.0f / (1.450071084655647f + ((fConst56 + 0.7431304601070396f) / fConst55)));
		fConst359 = (11.052052171507189f / fConst59);
		fConst360 = (1.450071084655647f + fConst359);
		fConst361 = (1.0f / (4.076781969643807f + ((fConst56 + 3.1897274020965583f) / fConst55)));
		fConst362 = (0.0017661728399818856f / fConst59);
		fConst363 = (0.00040767818495825777f + fConst362);
		fConst364 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst318) / fConst317)));
		fConst365 = (53.53615295455673f + fConst346);
		fConst366 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst318) / fConst317)));
		fConst367 = (7.621731298870603f + fConst346);
		fConst368 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst318) / fConst317)));
		fConst369 = (9.9999997055e-05f / fConst320);
		fConst370 = (0.000433227200555f + fConst369);
		fConst371 = (0.24529150870616f + ((fConst318 - 0.782413046821645f) / fConst317));
		fConst372 = (2 * (0.24529150870616f - fConst346));
		for (int i=0; i<3; i++) fRec152[i] = 0;
		fConst373 = (2 * (0.000433227200555f - fConst369));
		fConst374 = (0.689621364484675f + ((fConst318 - 0.512478641889141f) / fConst317));
		fConst375 = (2 * (0.689621364484675f - fConst346));
		for (int i=0; i<3; i++) fRec151[i] = 0;
		fConst376 = (2 * (7.621731298870603f - fConst346));
		fConst377 = (1.069358407707312f + ((fConst318 - 0.168404871113589f) / fConst317));
		fConst378 = (2 * (1.069358407707312f - fConst346));
		for (int i=0; i<3; i++) fRec150[i] = 0;
		fConst379 = (2 * (53.53615295455673f - fConst346));
		fConst380 = (4.076781969643807f + ((fConst56 - 3.1897274020965583f) / fConst55));
		fConst381 = (2 * (4.076781969643807f - fConst60));
		for (int i=0; i<3; i++) fRec149[i] = 0;
		fConst382 = (2 * (0.00040767818495825777f - fConst362));
		fConst383 = (1.450071084655647f + ((fConst56 - 0.7431304601070396f) / fConst55));
		fConst384 = (2 * (1.450071084655647f - fConst60));
		for (int i=0; i<3; i++) fRec148[i] = 0;
		fConst385 = (2 * (1.450071084655647f - fConst359));
		fConst386 = (0.9351401670315425f + ((fConst56 - 0.157482159302087f) / fConst55));
		fConst387 = (2 * (0.9351401670315425f - fConst60));
		for (int i=0; i<3; i++) fRec147[i] = 0;
		fConst388 = (2 * (0.9351401670315425f - fConst356));
		for (int i=0; i<2; i++) fRec146[i] = 0;
		fConst389 = tanf((1236.9245982665225f / float(iConst0)));
		fConst390 = (1.0f / fConst389);
		fConst391 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst390) / fConst389)));
		fConst392 = faustpower<2>(fConst389);
		fConst393 = (50.063807016150385f / fConst392);
		fConst394 = (0.9351401670315425f + fConst393);
		fConst395 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst390) / fConst389)));
		fConst396 = (11.052052171507189f / fConst392);
		fConst397 = (1.450071084655647f + fConst396);
		fConst398 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst390) / fConst389)));
		fConst399 = (0.0017661728399818856f / fConst392);
		fConst400 = (0.00040767818495825777f + fConst399);
		fConst401 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst56) / fConst55)));
		fConst402 = (53.53615295455673f + fConst60);
		fConst403 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst56) / fConst55)));
		fConst404 = (7.621731298870603f + fConst60);
		fConst405 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst56) / fConst55)));
		fConst406 = (9.9999997055e-05f / fConst59);
		fConst407 = (0.000433227200555f + fConst406);
		fConst408 = (0.24529150870616f + ((fConst56 - 0.782413046821645f) / fConst55));
		fConst409 = (2 * (0.24529150870616f - fConst60));
		for (int i=0; i<3; i++) fRec159[i] = 0;
		fConst410 = (2 * (0.000433227200555f - fConst406));
		fConst411 = (0.689621364484675f + ((fConst56 - 0.512478641889141f) / fConst55));
		fConst412 = (2 * (0.689621364484675f - fConst60));
		for (int i=0; i<3; i++) fRec158[i] = 0;
		fConst413 = (2 * (7.621731298870603f - fConst60));
		fConst414 = (1.069358407707312f + ((fConst56 - 0.168404871113589f) / fConst55));
		fConst415 = (2 * (1.069358407707312f - fConst60));
		for (int i=0; i<3; i++) fRec157[i] = 0;
		fConst416 = (2 * (53.53615295455673f - fConst60));
		fConst417 = (4.076781969643807f + ((fConst390 - 3.1897274020965583f) / fConst389));
		fConst418 = (1.0f / fConst392);
		fConst419 = (2 * (4.076781969643807f - fConst418));
		for (int i=0; i<3; i++) fRec156[i] = 0;
		fConst420 = (2 * (0.00040767818495825777f - fConst399));
		fConst421 = (1.450071084655647f + ((fConst390 - 0.7431304601070396f) / fConst389));
		fConst422 = (2 * (1.450071084655647f - fConst418));
		for (int i=0; i<3; i++) fRec155[i] = 0;
		fConst423 = (2 * (1.450071084655647f - fConst396));
		fConst424 = (0.9351401670315425f + ((fConst390 - 0.157482159302087f) / fConst389));
		fConst425 = (2 * (0.9351401670315425f - fConst418));
		for (int i=0; i<3; i++) fRec154[i] = 0;
		fConst426 = (2 * (0.9351401670315425f - fConst393));
		for (int i=0; i<2; i++) fRec153[i] = 0;
		fConst427 = tanf((779.213669244376f / float(iConst0)));
		fConst428 = (1.0f / fConst427);
		fConst429 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst428) / fConst427)));
		fConst430 = faustpower<2>(fConst427);
		fConst431 = (50.063807016150385f / fConst430);
		fConst432 = (0.9351401670315425f + fConst431);
		fConst433 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst428) / fConst427)));
		fConst434 = (11.052052171507189f / fConst430);
		fConst435 = (1.450071084655647f + fConst434);
		fConst436 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst428) / fConst427)));
		fConst437 = (0.0017661728399818856f / fConst430);
		fConst438 = (0.00040767818495825777f + fConst437);
		fConst439 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst390) / fConst389)));
		fConst440 = (53.53615295455673f + fConst418);
		fConst441 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst390) / fConst389)));
		fConst442 = (7.621731298870603f + fConst418);
		fConst443 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst390) / fConst389)));
		fConst444 = (9.9999997055e-05f / fConst392);
		fConst445 = (0.000433227200555f + fConst444);
		fConst446 = (0.24529150870616f + ((fConst390 - 0.782413046821645f) / fConst389));
		fConst447 = (2 * (0.24529150870616f - fConst418));
		for (int i=0; i<3; i++) fRec166[i] = 0;
		fConst448 = (2 * (0.000433227200555f - fConst444));
		fConst449 = (0.689621364484675f + ((fConst390 - 0.512478641889141f) / fConst389));
		fConst450 = (2 * (0.689621364484675f - fConst418));
		for (int i=0; i<3; i++) fRec165[i] = 0;
		fConst451 = (2 * (7.621731298870603f - fConst418));
		fConst452 = (1.069358407707312f + ((fConst390 - 0.168404871113589f) / fConst389));
		fConst453 = (2 * (1.069358407707312f - fConst418));
		for (int i=0; i<3; i++) fRec164[i] = 0;
		fConst454 = (2 * (53.53615295455673f - fConst418));
		fConst455 = (4.076781969643807f + ((fConst428 - 3.1897274020965583f) / fConst427));
		fConst456 = (1.0f / fConst430);
		fConst457 = (2 * (4.076781969643807f - fConst456));
		for (int i=0; i<3; i++) fRec163[i] = 0;
		fConst458 = (2 * (0.00040767818495825777f - fConst437));
		fConst459 = (1.450071084655647f + ((fConst428 - 0.7431304601070396f) / fConst427));
		fConst460 = (2 * (1.450071084655647f - fConst456));
		for (int i=0; i<3; i++) fRec162[i] = 0;
		fConst461 = (2 * (1.450071084655647f - fConst434));
		fConst462 = (0.9351401670315425f + ((fConst428 - 0.157482159302087f) / fConst427));
		fConst463 = (2 * (0.9351401670315425f - fConst456));
		for (int i=0; i<3; i++) fRec161[i] = 0;
		fConst464 = (2 * (0.9351401670315425f - fConst431));
		for (int i=0; i<2; i++) fRec160[i] = 0;
		fConst465 = (1.0f / (0.9351401670315425f + ((fConst43 + 0.157482159302087f) / fConst42)));
		fConst466 = (50.063807016150385f / fConst46);
		fConst467 = (0.9351401670315425f + fConst466);
		fConst468 = (1.0f / (1.450071084655647f + ((fConst43 + 0.7431304601070396f) / fConst42)));
		fConst469 = (11.052052171507189f / fConst46);
		fConst470 = (1.450071084655647f + fConst469);
		fConst471 = (1.0f / (4.076781969643807f + ((fConst43 + 3.1897274020965583f) / fConst42)));
		fConst472 = (0.0017661728399818856f / fConst46);
		fConst473 = (0.00040767818495825777f + fConst472);
		fConst474 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst428) / fConst427)));
		fConst475 = (53.53615295455673f + fConst456);
		fConst476 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst428) / fConst427)));
		fConst477 = (7.621731298870603f + fConst456);
		fConst478 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst428) / fConst427)));
		fConst479 = (9.9999997055e-05f / fConst430);
		fConst480 = (0.000433227200555f + fConst479);
		fConst481 = (0.24529150870616f + ((fConst428 - 0.782413046821645f) / fConst427));
		fConst482 = (2 * (0.24529150870616f - fConst456));
		for (int i=0; i<3; i++) fRec173[i] = 0;
		fConst483 = (2 * (0.000433227200555f - fConst479));
		fConst484 = (0.689621364484675f + ((fConst428 - 0.512478641889141f) / fConst427));
		fConst485 = (2 * (0.689621364484675f - fConst456));
		for (int i=0; i<3; i++) fRec172[i] = 0;
		fConst486 = (2 * (7.621731298870603f - fConst456));
		fConst487 = (1.069358407707312f + ((fConst428 - 0.168404871113589f) / fConst427));
		fConst488 = (2 * (1.069358407707312f - fConst456));
		for (int i=0; i<3; i++) fRec171[i] = 0;
		fConst489 = (2 * (53.53615295455673f - fConst456));
		fConst490 = (4.076781969643807f + ((fConst43 - 3.1897274020965583f) / fConst42));
		fConst491 = (2 * (4.076781969643807f - fConst47));
		for (int i=0; i<3; i++) fRec170[i] = 0;
		fConst492 = (2 * (0.00040767818495825777f - fConst472));
		fConst493 = (1.450071084655647f + ((fConst43 - 0.7431304601070396f) / fConst42));
		fConst494 = (2 * (1.450071084655647f - fConst47));
		for (int i=0; i<3; i++) fRec169[i] = 0;
		fConst495 = (2 * (1.450071084655647f - fConst469));
		fConst496 = (0.9351401670315425f + ((fConst43 - 0.157482159302087f) / fConst42));
		fConst497 = (2 * (0.9351401670315425f - fConst47));
		for (int i=0; i<3; i++) fRec168[i] = 0;
		fConst498 = (2 * (0.9351401670315425f - fConst466));
		for (int i=0; i<2; i++) fRec167[i] = 0;
		fConst499 = tanf((309.2311495666306f / float(iConst0)));
		fConst500 = (1.0f / fConst499);
		fConst501 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst500) / fConst499)));
		fConst502 = faustpower<2>(fConst499);
		fConst503 = (50.063807016150385f / fConst502);
		fConst504 = (0.9351401670315425f + fConst503);
		fConst505 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst500) / fConst499)));
		fConst506 = (11.052052171507189f / fConst502);
		fConst507 = (1.450071084655647f + fConst506);
		fConst508 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst500) / fConst499)));
		fConst509 = (0.0017661728399818856f / fConst502);
		fConst510 = (0.00040767818495825777f + fConst509);
		fConst511 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst43) / fConst42)));
		fConst512 = (53.53615295455673f + fConst47);
		fConst513 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst43) / fConst42)));
		fConst514 = (7.621731298870603f + fConst47);
		fConst515 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst43) / fConst42)));
		fConst516 = (9.9999997055e-05f / fConst46);
		fConst517 = (0.000433227200555f + fConst516);
		fConst518 = (0.24529150870616f + ((fConst43 - 0.782413046821645f) / fConst42));
		fConst519 = (2 * (0.24529150870616f - fConst47));
		for (int i=0; i<3; i++) fRec180[i] = 0;
		fConst520 = (2 * (0.000433227200555f - fConst516));
		fConst521 = (0.689621364484675f + ((fConst43 - 0.512478641889141f) / fConst42));
		fConst522 = (2 * (0.689621364484675f - fConst47));
		for (int i=0; i<3; i++) fRec179[i] = 0;
		fConst523 = (2 * (7.621731298870603f - fConst47));
		fConst524 = (1.069358407707312f + ((fConst43 - 0.168404871113589f) / fConst42));
		fConst525 = (2 * (1.069358407707312f - fConst47));
		for (int i=0; i<3; i++) fRec178[i] = 0;
		fConst526 = (2 * (53.53615295455673f - fConst47));
		fConst527 = (4.076781969643807f + ((fConst500 - 3.1897274020965583f) / fConst499));
		fConst528 = (1.0f / fConst502);
		fConst529 = (2 * (4.076781969643807f - fConst528));
		for (int i=0; i<3; i++) fRec177[i] = 0;
		fConst530 = (2 * (0.00040767818495825777f - fConst509));
		fConst531 = (1.450071084655647f + ((fConst500 - 0.7431304601070396f) / fConst499));
		fConst532 = (2 * (1.450071084655647f - fConst528));
		for (int i=0; i<3; i++) fRec176[i] = 0;
		fConst533 = (2 * (1.450071084655647f - fConst506));
		fConst534 = (0.9351401670315425f + ((fConst500 - 0.157482159302087f) / fConst499));
		fConst535 = (2 * (0.9351401670315425f - fConst528));
		for (int i=0; i<3; i++) fRec175[i] = 0;
		fConst536 = (2 * (0.9351401670315425f - fConst503));
		for (int i=0; i<2; i++) fRec174[i] = 0;
		fConst537 = tanf((194.803417311094f / float(iConst0)));
		fConst538 = (1.0f / fConst537);
		fConst539 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst538) / fConst537)));
		fConst540 = faustpower<2>(fConst537);
		fConst541 = (50.063807016150385f / fConst540);
		fConst542 = (0.9351401670315425f + fConst541);
		fConst543 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst538) / fConst537)));
		fConst544 = (11.052052171507189f / fConst540);
		fConst545 = (1.450071084655647f + fConst544);
		fConst546 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst538) / fConst537)));
		fConst547 = (0.0017661728399818856f / fConst540);
		fConst548 = (0.00040767818495825777f + fConst547);
		fConst549 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst500) / fConst499)));
		fConst550 = (53.53615295455673f + fConst528);
		fConst551 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst500) / fConst499)));
		fConst552 = (7.621731298870603f + fConst528);
		fConst553 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst500) / fConst499)));
		fConst554 = (9.9999997055e-05f / fConst502);
		fConst555 = (0.000433227200555f + fConst554);
		fConst556 = (0.24529150870616f + ((fConst500 - 0.782413046821645f) / fConst499));
		fConst557 = (2 * (0.24529150870616f - fConst528));
		for (int i=0; i<3; i++) fRec187[i] = 0;
		fConst558 = (2 * (0.000433227200555f - fConst554));
		fConst559 = (0.689621364484675f + ((fConst500 - 0.512478641889141f) / fConst499));
		fConst560 = (2 * (0.689621364484675f - fConst528));
		for (int i=0; i<3; i++) fRec186[i] = 0;
		fConst561 = (2 * (7.621731298870603f - fConst528));
		fConst562 = (1.069358407707312f + ((fConst500 - 0.168404871113589f) / fConst499));
		fConst563 = (2 * (1.069358407707312f - fConst528));
		for (int i=0; i<3; i++) fRec185[i] = 0;
		fConst564 = (2 * (53.53615295455673f - fConst528));
		fConst565 = (4.076781969643807f + ((fConst538 - 3.1897274020965583f) / fConst537));
		fConst566 = (1.0f / fConst540);
		fConst567 = (2 * (4.076781969643807f - fConst566));
		for (int i=0; i<3; i++) fRec184[i] = 0;
		fConst568 = (2 * (0.00040767818495825777f - fConst547));
		fConst569 = (1.450071084655647f + ((fConst538 - 0.7431304601070396f) / fConst537));
		fConst570 = (2 * (1.450071084655647f - fConst566));
		for (int i=0; i<3; i++) fRec183[i] = 0;
		fConst571 = (2 * (1.450071084655647f - fConst544));
		fConst572 = (0.9351401670315425f + ((fConst538 - 0.157482159302087f) / fConst537));
		fConst573 = (2 * (0.9351401670315425f - fConst566));
		for (int i=0; i<3; i++) fRec182[i] = 0;
		fConst574 = (2 * (0.9351401670315425f - fConst541));
		for (int i=0; i<2; i++) fRec181[i] = 0;
		fConst575 = (1.0f / (0.9351401670315425f + ((fConst30 + 0.157482159302087f) / fConst29)));
		fConst576 = (50.063807016150385f / fConst33);
		fConst577 = (0.9351401670315425f + fConst576);
		fConst578 = (1.0f / (1.450071084655647f + ((fConst30 + 0.7431304601070396f) / fConst29)));
		fConst579 = (11.052052171507189f / fConst33);
		fConst580 = (1.450071084655647f + fConst579);
		fConst581 = (1.0f / (4.076781969643807f + ((fConst30 + 3.1897274020965583f) / fConst29)));
		fConst582 = (0.0017661728399818856f / fConst33);
		fConst583 = (0.00040767818495825777f + fConst582);
		fConst584 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst538) / fConst537)));
		fConst585 = (53.53615295455673f + fConst566);
		fConst586 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst538) / fConst537)));
		fConst587 = (7.621731298870603f + fConst566);
		fConst588 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst538) / fConst537)));
		fConst589 = (9.9999997055e-05f / fConst540);
		fConst590 = (0.000433227200555f + fConst589);
		fConst591 = (0.24529150870616f + ((fConst538 - 0.782413046821645f) / fConst537));
		fConst592 = (2 * (0.24529150870616f - fConst566));
		for (int i=0; i<3; i++) fRec194[i] = 0;
		fConst593 = (2 * (0.000433227200555f - fConst589));
		fConst594 = (0.689621364484675f + ((fConst538 - 0.512478641889141f) / fConst537));
		fConst595 = (2 * (0.689621364484675f - fConst566));
		for (int i=0; i<3; i++) fRec193[i] = 0;
		fConst596 = (2 * (7.621731298870603f - fConst566));
		fConst597 = (1.069358407707312f + ((fConst538 - 0.168404871113589f) / fConst537));
		fConst598 = (2 * (1.069358407707312f - fConst566));
		for (int i=0; i<3; i++) fRec192[i] = 0;
		fConst599 = (2 * (53.53615295455673f - fConst566));
		fConst600 = (4.076781969643807f + ((fConst30 - 3.1897274020965583f) / fConst29));
		fConst601 = (2 * (4.076781969643807f - fConst34));
		for (int i=0; i<3; i++) fRec191[i] = 0;
		fConst602 = (2 * (0.00040767818495825777f - fConst582));
		fConst603 = (1.450071084655647f + ((fConst30 - 0.7431304601070396f) / fConst29));
		fConst604 = (2 * (1.450071084655647f - fConst34));
		for (int i=0; i<3; i++) fRec190[i] = 0;
		fConst605 = (2 * (1.450071084655647f - fConst579));
		fConst606 = (0.9351401670315425f + ((fConst30 - 0.157482159302087f) / fConst29));
		fConst607 = (2 * (0.9351401670315425f - fConst34));
		for (int i=0; i<3; i++) fRec189[i] = 0;
		fConst608 = (2 * (0.9351401670315425f - fConst576));
		for (int i=0; i<2; i++) fRec188[i] = 0;
		fConst609 = tanf((77.3077873916577f / float(iConst0)));
		fConst610 = (1.0f / fConst609);
		fConst611 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst610) / fConst609)));
		fConst612 = faustpower<2>(fConst609);
		fConst613 = (50.063807016150385f / fConst612);
		fConst614 = (0.9351401670315425f + fConst613);
		fConst615 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst610) / fConst609)));
		fConst616 = (11.052052171507189f / fConst612);
		fConst617 = (1.450071084655647f + fConst616);
		fConst618 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst610) / fConst609)));
		fConst619 = (0.0017661728399818856f / fConst612);
		fConst620 = (0.00040767818495825777f + fConst619);
		fConst621 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst30) / fConst29)));
		fConst622 = (53.53615295455673f + fConst34);
		fConst623 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst30) / fConst29)));
		fConst624 = (7.621731298870603f + fConst34);
		fConst625 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst30) / fConst29)));
		fConst626 = (9.9999997055e-05f / fConst33);
		fConst627 = (0.000433227200555f + fConst626);
		fConst628 = (0.24529150870616f + ((fConst30 - 0.782413046821645f) / fConst29));
		fConst629 = (2 * (0.24529150870616f - fConst34));
		for (int i=0; i<3; i++) fRec201[i] = 0;
		fConst630 = (2 * (0.000433227200555f - fConst626));
		fConst631 = (0.689621364484675f + ((fConst30 - 0.512478641889141f) / fConst29));
		fConst632 = (2 * (0.689621364484675f - fConst34));
		for (int i=0; i<3; i++) fRec200[i] = 0;
		fConst633 = (2 * (7.621731298870603f - fConst34));
		fConst634 = (1.069358407707312f + ((fConst30 - 0.168404871113589f) / fConst29));
		fConst635 = (2 * (1.069358407707312f - fConst34));
		for (int i=0; i<3; i++) fRec199[i] = 0;
		fConst636 = (2 * (53.53615295455673f - fConst34));
		fConst637 = (4.076781969643807f + ((fConst610 - 3.1897274020965583f) / fConst609));
		fConst638 = (1.0f / fConst612);
		fConst639 = (2 * (4.076781969643807f - fConst638));
		for (int i=0; i<3; i++) fRec198[i] = 0;
		fConst640 = (2 * (0.00040767818495825777f - fConst619));
		fConst641 = (1.450071084655647f + ((fConst610 - 0.7431304601070396f) / fConst609));
		fConst642 = (2 * (1.450071084655647f - fConst638));
		for (int i=0; i<3; i++) fRec197[i] = 0;
		fConst643 = (2 * (1.450071084655647f - fConst616));
		fConst644 = (0.9351401670315425f + ((fConst610 - 0.157482159302087f) / fConst609));
		fConst645 = (2 * (0.9351401670315425f - fConst638));
		for (int i=0; i<3; i++) fRec196[i] = 0;
		fConst646 = (2 * (0.9351401670315425f - fConst613));
		for (int i=0; i<2; i++) fRec195[i] = 0;
		fConst647 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst610) / fConst609)));
		fConst648 = (53.53615295455673f + fConst638);
		fConst649 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst610) / fConst609)));
		fConst650 = (7.621731298870603f + fConst638);
		fConst651 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst610) / fConst609)));
		fConst652 = (9.9999997055e-05f / fConst612);
		fConst653 = (0.000433227200555f + fConst652);
		fConst654 = (0.24529150870616f + ((fConst610 - 0.782413046821645f) / fConst609));
		fConst655 = (2 * (0.24529150870616f - fConst638));
		for (int i=0; i<3; i++) fRec205[i] = 0;
		fConst656 = (2 * (0.000433227200555f - fConst652));
		fConst657 = (0.689621364484675f + ((fConst610 - 0.512478641889141f) / fConst609));
		fConst658 = (2 * (0.689621364484675f - fConst638));
		for (int i=0; i<3; i++) fRec204[i] = 0;
		fConst659 = (2 * (7.621731298870603f - fConst638));
		fConst660 = (1.069358407707312f + ((fConst610 - 0.168404871113589f) / fConst609));
		fConst661 = (2 * (1.069358407707312f - fConst638));
		for (int i=0; i<3; i++) fRec203[i] = 0;
		fConst662 = (2 * (53.53615295455673f - fConst638));
		for (int i=0; i<2; i++) fRec202[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("graphic_eq");
		interface->declare(0, "1", "");
		interface->openVerticalBox("");
		interface->declare(0, "0", "");
		interface->declare(0, "tooltip", "See Faust's oscillator.lib for documentation and references");
		interface->openVerticalBox("SAWTOOTH OSCILLATOR");
		interface->declare(0, "1", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider2, "1", "");
		interface->declare(&fslider2, "style", "knob");
		interface->declare(&fslider2, "tooltip", "Sawtooth waveform amplitude");
		interface->declare(&fslider2, "unit", "dB");
		interface->addVerticalSlider("Amplitude", &fslider2, -2e+01f, -1.2e+02f, 1e+01f, 0.1f);
		interface->declare(&fslider4, "2", "");
		interface->declare(&fslider4, "style", "knob");
		interface->declare(&fslider4, "tooltip", "Sawtooth frequency as a Piano Key (PK) number (A440 = key 49)");
		interface->declare(&fslider4, "unit", "PK");
		interface->addVerticalSlider("Frequency", &fslider4, 49.0f, 1.0f, 88.0f, 0.01f);
		interface->declare(&fslider5, "3", "");
		interface->declare(&fslider5, "style", "knob");
		interface->declare(&fslider5, "tooltip", "Percentange frequency-shift up or down for second oscillator");
		interface->declare(&fslider5, "unit", "%%");
		interface->addVerticalSlider("Detuning 1", &fslider5, -0.1f, -1e+01f, 1e+01f, 0.01f);
		interface->declare(&fslider6, "4", "");
		interface->declare(&fslider6, "style", "knob");
		interface->declare(&fslider6, "tooltip", "Percentange frequency-shift up or down for third detuned oscillator");
		interface->declare(&fslider6, "unit", "%%");
		interface->addVerticalSlider("Detuning 2", &fslider6, 0.1f, -1e+01f, 1e+01f, 0.01f);
		interface->declare(&fslider3, "5", "");
		interface->declare(&fslider3, "style", "knob");
		interface->declare(&fslider3, "tooltip", "Portamento (frequency-glide) time-constant in seconds");
		interface->declare(&fslider3, "unit", "sec");
		interface->addVerticalSlider("Portamento", &fslider3, 0.1f, 0.01f, 1.0f, 0.001f);
		interface->declare(0, "6", "");
		interface->openVerticalBox("Alternate Signals");
		interface->declare(&fcheckbox2, "0", "");
		interface->declare(&fcheckbox2, "tooltip", "Pink Noise (or 1/f noise) is Constant-Q Noise, meaning that it has the same total power in every octave");
		interface->addCheckButton("Pink Noise Instead (uses only Amplitude control on the left)", &fcheckbox2);
		interface->declare(&fcheckbox1, "1", "");
		interface->addCheckButton("External Input Instead (overrides Sawtooth/Noise selection above)", &fcheckbox1);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "2", "");
		interface->openVerticalBox("");
		interface->declare(0, "tooltip", "See Faust's filter.lib for documentation and references");
		interface->openVerticalBox("CONSTANT-Q FILTER BANK (Butterworth dyadic tree)");
		interface->declare(0, "0", "");
		interface->openHorizontalBox("");
		interface->declare(&fcheckbox0, "0", "");
		interface->declare(&fcheckbox0, "tooltip", "When this is checked, the filter-bank has no effect");
		interface->addCheckButton("Bypass", &fcheckbox0);
		interface->closeBox();
		interface->declare(0, "1", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider16, "1", "");
		interface->declare(&fslider16, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider16, "unit", "dB");
		interface->addVerticalSlider("", &fslider16, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->declare(&fslider15, "2", "");
		interface->declare(&fslider15, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider15, "unit", "dB");
		interface->addVerticalSlider("", &fslider15, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->declare(&fslider14, "3", "");
		interface->declare(&fslider14, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider14, "unit", "dB");
		interface->addVerticalSlider("", &fslider14, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->declare(&fslider13, "4", "");
		interface->declare(&fslider13, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider13, "unit", "dB");
		interface->addVerticalSlider("", &fslider13, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->declare(&fslider12, "5", "");
		interface->declare(&fslider12, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider12, "unit", "dB");
		interface->addVerticalSlider("", &fslider12, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->declare(&fslider11, "6", "");
		interface->declare(&fslider11, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider11, "unit", "dB");
		interface->addVerticalSlider("", &fslider11, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->declare(&fslider10, "7", "");
		interface->declare(&fslider10, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider10, "unit", "dB");
		interface->addVerticalSlider("", &fslider10, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->declare(&fslider9, "8", "");
		interface->declare(&fslider9, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider9, "unit", "dB");
		interface->addVerticalSlider("", &fslider9, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->declare(&fslider8, "9", "");
		interface->declare(&fslider8, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider8, "unit", "dB");
		interface->addVerticalSlider("", &fslider8, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->declare(&fslider7, "10", "");
		interface->declare(&fslider7, "tooltip", "Bandpass filter gain in dB");
		interface->declare(&fslider7, "unit", "dB");
		interface->addVerticalSlider("", &fslider7, -1e+01f, -7e+01f, 1e+01f, 0.1f);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "3", "");
		interface->openVerticalBox("");
		interface->declare(0, "0", "");
		interface->declare(0, "tooltip", "See Faust's filter.lib for documentation and references");
		interface->openHorizontalBox("CONSTANT-Q SPECTRUM ANALYZER (6E)");
		interface->declare(&fbargraph14, "0", "");
		interface->declare(&fbargraph14, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph14, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph14, -5e+01f, 1e+01f);
		interface->declare(&fbargraph13, "1", "");
		interface->declare(&fbargraph13, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph13, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph13, -5e+01f, 1e+01f);
		interface->declare(&fbargraph12, "2", "");
		interface->declare(&fbargraph12, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph12, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph12, -5e+01f, 1e+01f);
		interface->declare(&fbargraph11, "3", "");
		interface->declare(&fbargraph11, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph11, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph11, -5e+01f, 1e+01f);
		interface->declare(&fbargraph10, "4", "");
		interface->declare(&fbargraph10, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph10, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph10, -5e+01f, 1e+01f);
		interface->declare(&fbargraph9, "5", "");
		interface->declare(&fbargraph9, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph9, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph9, -5e+01f, 1e+01f);
		interface->declare(&fbargraph8, "6", "");
		interface->declare(&fbargraph8, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph8, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph8, -5e+01f, 1e+01f);
		interface->declare(&fbargraph7, "7", "");
		interface->declare(&fbargraph7, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph7, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph7, -5e+01f, 1e+01f);
		interface->declare(&fbargraph6, "8", "");
		interface->declare(&fbargraph6, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph6, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph6, -5e+01f, 1e+01f);
		interface->declare(&fbargraph5, "9", "");
		interface->declare(&fbargraph5, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph5, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph5, -5e+01f, 1e+01f);
		interface->declare(&fbargraph4, "10", "");
		interface->declare(&fbargraph4, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph4, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph4, -5e+01f, 1e+01f);
		interface->declare(&fbargraph3, "11", "");
		interface->declare(&fbargraph3, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph3, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph3, -5e+01f, 1e+01f);
		interface->declare(&fbargraph2, "12", "");
		interface->declare(&fbargraph2, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph2, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph2, -5e+01f, 1e+01f);
		interface->declare(&fbargraph1, "13", "");
		interface->declare(&fbargraph1, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph1, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph1, -5e+01f, 1e+01f);
		interface->declare(&fbargraph0, "14", "");
		interface->declare(&fbargraph0, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph0, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph0, -5e+01f, 1e+01f);
		interface->closeBox();
		interface->declare(0, "1", "");
		interface->openHorizontalBox("SPECTRUM ANALYZER CONTROLS");
		interface->declare(&fslider1, "0", "");
		interface->declare(&fslider1, "tooltip", "band-level averaging time in seconds");
		interface->declare(&fslider1, "unit", "sec");
		interface->addHorizontalSlider("Level Averaging Time", &fslider1, 0.1f, 0.0f, 1.0f, 0.01f);
		interface->declare(&fslider0, "1", "");
		interface->declare(&fslider0, "tooltip", "Level offset in decibels");
		interface->declare(&fslider0, "unit", "dB");
		interface->addHorizontalSlider("Level dB Offset", &fslider0, 5e+01f, 0.0f, 1e+02f, 1.0f);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fslider0;
		float 	fSlow1 = expf((0 - (fConst1 / fslider1)));
		float 	fSlow2 = (1.0f - fSlow1);
		int 	iSlow3 = int(fcheckbox0);
		float 	fSlow4 = (0.0010000000000000009f * powf(10,(0.05f * fslider2)));
		int 	iSlow5 = int(fcheckbox1);
		int 	iSlow6 = int(fcheckbox2);
		float 	fSlow7 = expf((0 - (fConst1 / fslider3)));
		float 	fSlow8 = (4.4e+02f * (powf(2.0f,(0.08333333333333333f * (fslider4 - 49.0f))) * (1.0f - fSlow7)));
		float 	fSlow9 = (1 + (0.01f * fslider5));
		float 	fSlow10 = (1.0f / fSlow9);
		float 	fSlow11 = (fConst16 * fSlow9);
		float 	fSlow12 = (fConst15 / fSlow9);
		float 	fSlow13 = (1 + (0.01f * fslider6));
		float 	fSlow14 = (1.0f / fSlow13);
		float 	fSlow15 = (fConst16 * fSlow13);
		float 	fSlow16 = (fConst15 / fSlow13);
		float 	fSlow17 = (0.0010000000000000009f * fslider7);
		float 	fSlow18 = (0.0010000000000000009f * fslider8);
		float 	fSlow19 = (0.0010000000000000009f * fslider9);
		float 	fSlow20 = (0.0010000000000000009f * fslider10);
		float 	fSlow21 = (0.0010000000000000009f * fslider11);
		float 	fSlow22 = (0.0010000000000000009f * fslider12);
		float 	fSlow23 = (0.0010000000000000009f * fslider13);
		float 	fSlow24 = (0.0010000000000000009f * fslider14);
		float 	fSlow25 = (0.0010000000000000009f * fslider15);
		float 	fSlow26 = (0.0010000000000000009f * fslider16);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			fRec4[0] = ((0.999f * fRec4[1]) + fSlow4);
			iRec6[0] = (12345 + (1103515245 * iRec6[1]));
			fRec5[0] = (((0.5221894f * fRec5[3]) + ((4.656612875245797e-10f * iRec6[0]) + (2.494956002f * fRec5[1]))) - (2.017265875f * fRec5[2]));
			fVec0[0] = 0.25f;
			fRec7[0] = ((fSlow7 * fRec7[1]) + fSlow8);
			fRec8[0] = fmodf((1.0f + fRec8[1]),(fConst15 / fRec7[0]));
			float fTemp0 = faustpower<2>(((fConst16 * (fRec7[0] * fRec8[0])) - 1.0f));
			fVec1[0] = fTemp0;
			fRec9[0] = fmodf((1.0f + fRec9[1]),(fSlow12 / fRec7[0]));
			float fTemp1 = faustpower<2>(((fSlow11 * (fRec7[0] * fRec9[0])) - 1.0f));
			fVec2[0] = fTemp1;
			fRec10[0] = fmodf((1.0f + fRec10[1]),(fSlow16 / fRec7[0]));
			float fTemp2 = faustpower<2>(((fSlow15 * (fRec7[0] * fRec10[0])) - 1.0f));
			fVec3[0] = fTemp2;
			float fTemp3 = (fRec4[0] * ((iSlow5)?(float)input0[i]:((iSlow6)?(((0.049922035f * fRec5[0]) + (0.050612699f * fRec5[2])) - ((0.095993537f * fRec5[1]) + (0.004408786f * fRec5[3]))):(fConst14 * (((fRec4[0] * fVec0[1]) * (((fVec1[0] - fVec1[1]) + (fSlow10 * (fVec2[0] - fVec2[1]))) + (fSlow14 * (fVec3[0] - fVec3[1])))) / fRec7[0])))));
			float fTemp4 = ((iSlow3)?0:fTemp3);
			fVec4[0] = fTemp4;
			fRec21[0] = ((fConst22 * fRec21[1]) + (fConst23 * ((fConst3 * fVec4[0]) + (fConst24 * fVec4[1]))));
			fRec20[0] = (fRec21[0] - (fConst20 * ((fConst25 * fRec20[2]) + (fConst26 * fRec20[1]))));
			fRec19[0] = ((fConst20 * (((fConst19 * fRec20[0]) + (fConst27 * fRec20[1])) + (fConst19 * fRec20[2]))) - (fConst18 * ((fConst28 * fRec19[2]) + (fConst26 * fRec19[1]))));
			float fTemp5 = (fConst35 * fRec18[1]);
			fRec18[0] = ((fConst18 * (((fConst19 * fRec19[0]) + (fConst27 * fRec19[1])) + (fConst19 * fRec19[2]))) - (fConst31 * ((fConst32 * fRec18[2]) + fTemp5)));
			float fTemp6 = (fConst41 * fRec17[1]);
			fRec17[0] = ((fRec18[2] + (fConst31 * (fTemp5 + (fConst32 * fRec18[0])))) - (fConst38 * ((fConst39 * fRec17[2]) + fTemp6)));
			float fTemp7 = (fConst48 * fRec16[1]);
			fRec16[0] = ((fRec17[2] + (fConst38 * (fTemp6 + (fConst39 * fRec17[0])))) - (fConst44 * ((fConst45 * fRec16[2]) + fTemp7)));
			float fTemp8 = (fConst54 * fRec15[1]);
			fRec15[0] = ((fRec16[2] + (fConst44 * (fTemp7 + (fConst45 * fRec16[0])))) - (fConst51 * ((fConst52 * fRec15[2]) + fTemp8)));
			float fTemp9 = (fConst61 * fRec14[1]);
			fRec14[0] = ((fRec15[2] + (fConst51 * (fTemp8 + (fConst52 * fRec15[0])))) - (fConst57 * ((fConst58 * fRec14[2]) + fTemp9)));
			float fTemp10 = (fConst67 * fRec13[1]);
			fRec13[0] = ((fRec14[2] + (fConst57 * (fTemp9 + (fConst58 * fRec14[0])))) - (fConst64 * ((fConst65 * fRec13[2]) + fTemp10)));
			float fTemp11 = (fConst74 * fRec12[1]);
			fRec12[0] = ((fRec13[2] + (fConst64 * (fTemp10 + (fConst65 * fRec13[0])))) - (fConst70 * ((fConst71 * fRec12[2]) + fTemp11)));
			float fTemp12 = (fConst80 * fRec11[1]);
			fRec11[0] = ((fRec12[2] + (fConst70 * (fTemp11 + (fConst71 * fRec12[0])))) - (fConst77 * ((fConst78 * fRec11[2]) + fTemp12)));
			fRec22[0] = ((0.999f * fRec22[1]) + fSlow17);
			fRec35[0] = ((fConst22 * fRec35[1]) + (fConst23 * (fVec4[0] + fVec4[1])));
			fRec34[0] = (fRec35[0] - (fConst20 * ((fConst25 * fRec34[2]) + (fConst26 * fRec34[1]))));
			fRec33[0] = ((fConst20 * (fRec34[2] + (fRec34[0] + (2 * fRec34[1])))) - (fConst18 * ((fConst28 * fRec33[2]) + (fConst26 * fRec33[1]))));
			float fTemp13 = (fRec33[2] + (fRec33[0] + (2 * fRec33[1])));
			fVec5[0] = fTemp13;
			fRec32[0] = ((fConst85 * fRec32[1]) + (fConst86 * ((fConst76 * fVec5[0]) + (fConst87 * fVec5[1]))));
			fRec31[0] = (fRec32[0] - (fConst83 * ((fConst88 * fRec31[2]) + (fConst80 * fRec31[1]))));
			fRec30[0] = ((fConst83 * (((fConst79 * fRec31[0]) + (fConst89 * fRec31[1])) + (fConst79 * fRec31[2]))) - (fConst82 * ((fConst90 * fRec30[2]) + (fConst80 * fRec30[1]))));
			float fTemp14 = (fConst35 * fRec29[1]);
			fRec29[0] = ((fConst82 * (((fConst79 * fRec30[0]) + (fConst89 * fRec30[1])) + (fConst79 * fRec30[2]))) - (fConst31 * ((fConst32 * fRec29[2]) + fTemp14)));
			float fTemp15 = (fConst41 * fRec28[1]);
			fRec28[0] = ((fRec29[2] + (fConst31 * (fTemp14 + (fConst32 * fRec29[0])))) - (fConst38 * ((fConst39 * fRec28[2]) + fTemp15)));
			float fTemp16 = (fConst48 * fRec27[1]);
			fRec27[0] = ((fRec28[2] + (fConst38 * (fTemp15 + (fConst39 * fRec28[0])))) - (fConst44 * ((fConst45 * fRec27[2]) + fTemp16)));
			float fTemp17 = (fConst54 * fRec26[1]);
			fRec26[0] = ((fRec27[2] + (fConst44 * (fTemp16 + (fConst45 * fRec27[0])))) - (fConst51 * ((fConst52 * fRec26[2]) + fTemp17)));
			float fTemp18 = (fConst61 * fRec25[1]);
			fRec25[0] = ((fRec26[2] + (fConst51 * (fTemp17 + (fConst52 * fRec26[0])))) - (fConst57 * ((fConst58 * fRec25[2]) + fTemp18)));
			float fTemp19 = (fConst67 * fRec24[1]);
			fRec24[0] = ((fRec25[2] + (fConst57 * (fTemp18 + (fConst58 * fRec25[0])))) - (fConst64 * ((fConst65 * fRec24[2]) + fTemp19)));
			float fTemp20 = (fConst74 * fRec23[1]);
			fRec23[0] = ((fRec24[2] + (fConst64 * (fTemp19 + (fConst65 * fRec24[0])))) - (fConst70 * ((fConst71 * fRec23[2]) + fTemp20)));
			fRec36[0] = ((0.999f * fRec36[1]) + fSlow18);
			fRec48[0] = ((fConst85 * fRec48[1]) + (fConst86 * (fVec5[0] + fVec5[1])));
			fRec47[0] = (fRec48[0] - (fConst83 * ((fConst88 * fRec47[2]) + (fConst80 * fRec47[1]))));
			fRec46[0] = ((fConst83 * (fRec47[2] + (fRec47[0] + (2 * fRec47[1])))) - (fConst82 * ((fConst90 * fRec46[2]) + (fConst80 * fRec46[1]))));
			float fTemp21 = (fRec46[2] + (fRec46[0] + (2 * fRec46[1])));
			fVec6[0] = fTemp21;
			fRec45[0] = ((fConst95 * fRec45[1]) + (fConst96 * ((fConst69 * fVec6[0]) + (fConst97 * fVec6[1]))));
			fRec44[0] = (fRec45[0] - (fConst93 * ((fConst98 * fRec44[2]) + (fConst74 * fRec44[1]))));
			fRec43[0] = ((fConst93 * (((fConst73 * fRec44[0]) + (fConst99 * fRec44[1])) + (fConst73 * fRec44[2]))) - (fConst92 * ((fConst100 * fRec43[2]) + (fConst74 * fRec43[1]))));
			float fTemp22 = (fConst35 * fRec42[1]);
			fRec42[0] = ((fConst92 * (((fConst73 * fRec43[0]) + (fConst99 * fRec43[1])) + (fConst73 * fRec43[2]))) - (fConst31 * ((fConst32 * fRec42[2]) + fTemp22)));
			float fTemp23 = (fConst41 * fRec41[1]);
			fRec41[0] = ((fRec42[2] + (fConst31 * (fTemp22 + (fConst32 * fRec42[0])))) - (fConst38 * ((fConst39 * fRec41[2]) + fTemp23)));
			float fTemp24 = (fConst48 * fRec40[1]);
			fRec40[0] = ((fRec41[2] + (fConst38 * (fTemp23 + (fConst39 * fRec41[0])))) - (fConst44 * ((fConst45 * fRec40[2]) + fTemp24)));
			float fTemp25 = (fConst54 * fRec39[1]);
			fRec39[0] = ((fRec40[2] + (fConst44 * (fTemp24 + (fConst45 * fRec40[0])))) - (fConst51 * ((fConst52 * fRec39[2]) + fTemp25)));
			float fTemp26 = (fConst61 * fRec38[1]);
			fRec38[0] = ((fRec39[2] + (fConst51 * (fTemp25 + (fConst52 * fRec39[0])))) - (fConst57 * ((fConst58 * fRec38[2]) + fTemp26)));
			float fTemp27 = (fConst67 * fRec37[1]);
			fRec37[0] = ((fRec38[2] + (fConst57 * (fTemp26 + (fConst58 * fRec38[0])))) - (fConst64 * ((fConst65 * fRec37[2]) + fTemp27)));
			fRec49[0] = ((0.999f * fRec49[1]) + fSlow19);
			fRec60[0] = ((fConst95 * fRec60[1]) + (fConst96 * (fVec6[0] + fVec6[1])));
			fRec59[0] = (fRec60[0] - (fConst93 * ((fConst98 * fRec59[2]) + (fConst74 * fRec59[1]))));
			fRec58[0] = ((fConst93 * (fRec59[2] + (fRec59[0] + (2 * fRec59[1])))) - (fConst92 * ((fConst100 * fRec58[2]) + (fConst74 * fRec58[1]))));
			float fTemp28 = (fRec58[2] + (fRec58[0] + (2 * fRec58[1])));
			fVec7[0] = fTemp28;
			fRec57[0] = ((fConst105 * fRec57[1]) + (fConst106 * ((fConst63 * fVec7[0]) + (fConst107 * fVec7[1]))));
			fRec56[0] = (fRec57[0] - (fConst103 * ((fConst108 * fRec56[2]) + (fConst67 * fRec56[1]))));
			fRec55[0] = ((fConst103 * (((fConst66 * fRec56[0]) + (fConst109 * fRec56[1])) + (fConst66 * fRec56[2]))) - (fConst102 * ((fConst110 * fRec55[2]) + (fConst67 * fRec55[1]))));
			float fTemp29 = (fConst35 * fRec54[1]);
			fRec54[0] = ((fConst102 * (((fConst66 * fRec55[0]) + (fConst109 * fRec55[1])) + (fConst66 * fRec55[2]))) - (fConst31 * ((fConst32 * fRec54[2]) + fTemp29)));
			float fTemp30 = (fConst41 * fRec53[1]);
			fRec53[0] = ((fRec54[2] + (fConst31 * (fTemp29 + (fConst32 * fRec54[0])))) - (fConst38 * ((fConst39 * fRec53[2]) + fTemp30)));
			float fTemp31 = (fConst48 * fRec52[1]);
			fRec52[0] = ((fRec53[2] + (fConst38 * (fTemp30 + (fConst39 * fRec53[0])))) - (fConst44 * ((fConst45 * fRec52[2]) + fTemp31)));
			float fTemp32 = (fConst54 * fRec51[1]);
			fRec51[0] = ((fRec52[2] + (fConst44 * (fTemp31 + (fConst45 * fRec52[0])))) - (fConst51 * ((fConst52 * fRec51[2]) + fTemp32)));
			float fTemp33 = (fConst61 * fRec50[1]);
			fRec50[0] = ((fRec51[2] + (fConst51 * (fTemp32 + (fConst52 * fRec51[0])))) - (fConst57 * ((fConst58 * fRec50[2]) + fTemp33)));
			fRec61[0] = ((0.999f * fRec61[1]) + fSlow20);
			fRec71[0] = ((fConst105 * fRec71[1]) + (fConst106 * (fVec7[0] + fVec7[1])));
			fRec70[0] = (fRec71[0] - (fConst103 * ((fConst108 * fRec70[2]) + (fConst67 * fRec70[1]))));
			fRec69[0] = ((fConst103 * (fRec70[2] + (fRec70[0] + (2 * fRec70[1])))) - (fConst102 * ((fConst110 * fRec69[2]) + (fConst67 * fRec69[1]))));
			float fTemp34 = (fRec69[2] + (fRec69[0] + (2 * fRec69[1])));
			fVec8[0] = fTemp34;
			fRec68[0] = ((fConst115 * fRec68[1]) + (fConst116 * ((fConst56 * fVec8[0]) + (fConst117 * fVec8[1]))));
			fRec67[0] = (fRec68[0] - (fConst113 * ((fConst118 * fRec67[2]) + (fConst61 * fRec67[1]))));
			fRec66[0] = ((fConst113 * (((fConst60 * fRec67[0]) + (fConst119 * fRec67[1])) + (fConst60 * fRec67[2]))) - (fConst112 * ((fConst120 * fRec66[2]) + (fConst61 * fRec66[1]))));
			float fTemp35 = (fConst35 * fRec65[1]);
			fRec65[0] = ((fConst112 * (((fConst60 * fRec66[0]) + (fConst119 * fRec66[1])) + (fConst60 * fRec66[2]))) - (fConst31 * ((fConst32 * fRec65[2]) + fTemp35)));
			float fTemp36 = (fConst41 * fRec64[1]);
			fRec64[0] = ((fRec65[2] + (fConst31 * (fTemp35 + (fConst32 * fRec65[0])))) - (fConst38 * ((fConst39 * fRec64[2]) + fTemp36)));
			float fTemp37 = (fConst48 * fRec63[1]);
			fRec63[0] = ((fRec64[2] + (fConst38 * (fTemp36 + (fConst39 * fRec64[0])))) - (fConst44 * ((fConst45 * fRec63[2]) + fTemp37)));
			float fTemp38 = (fConst54 * fRec62[1]);
			fRec62[0] = ((fRec63[2] + (fConst44 * (fTemp37 + (fConst45 * fRec63[0])))) - (fConst51 * ((fConst52 * fRec62[2]) + fTemp38)));
			fRec72[0] = ((0.999f * fRec72[1]) + fSlow21);
			fRec81[0] = ((fConst115 * fRec81[1]) + (fConst116 * (fVec8[0] + fVec8[1])));
			fRec80[0] = (fRec81[0] - (fConst113 * ((fConst118 * fRec80[2]) + (fConst61 * fRec80[1]))));
			fRec79[0] = ((fConst113 * (fRec80[2] + (fRec80[0] + (2 * fRec80[1])))) - (fConst112 * ((fConst120 * fRec79[2]) + (fConst61 * fRec79[1]))));
			float fTemp39 = (fRec79[2] + (fRec79[0] + (2 * fRec79[1])));
			fVec9[0] = fTemp39;
			fRec78[0] = ((fConst125 * fRec78[1]) + (fConst126 * ((fConst50 * fVec9[0]) + (fConst127 * fVec9[1]))));
			fRec77[0] = (fRec78[0] - (fConst123 * ((fConst128 * fRec77[2]) + (fConst54 * fRec77[1]))));
			fRec76[0] = ((fConst123 * (((fConst53 * fRec77[0]) + (fConst129 * fRec77[1])) + (fConst53 * fRec77[2]))) - (fConst122 * ((fConst130 * fRec76[2]) + (fConst54 * fRec76[1]))));
			float fTemp40 = (fConst35 * fRec75[1]);
			fRec75[0] = ((fConst122 * (((fConst53 * fRec76[0]) + (fConst129 * fRec76[1])) + (fConst53 * fRec76[2]))) - (fConst31 * ((fConst32 * fRec75[2]) + fTemp40)));
			float fTemp41 = (fConst41 * fRec74[1]);
			fRec74[0] = ((fRec75[2] + (fConst31 * (fTemp40 + (fConst32 * fRec75[0])))) - (fConst38 * ((fConst39 * fRec74[2]) + fTemp41)));
			float fTemp42 = (fConst48 * fRec73[1]);
			fRec73[0] = ((fRec74[2] + (fConst38 * (fTemp41 + (fConst39 * fRec74[0])))) - (fConst44 * ((fConst45 * fRec73[2]) + fTemp42)));
			fRec82[0] = ((0.999f * fRec82[1]) + fSlow22);
			fRec90[0] = ((fConst125 * fRec90[1]) + (fConst126 * (fVec9[0] + fVec9[1])));
			fRec89[0] = (fRec90[0] - (fConst123 * ((fConst128 * fRec89[2]) + (fConst54 * fRec89[1]))));
			fRec88[0] = ((fConst123 * (fRec89[2] + (fRec89[0] + (2 * fRec89[1])))) - (fConst122 * ((fConst130 * fRec88[2]) + (fConst54 * fRec88[1]))));
			float fTemp43 = (fRec88[2] + (fRec88[0] + (2 * fRec88[1])));
			fVec10[0] = fTemp43;
			fRec87[0] = ((fConst135 * fRec87[1]) + (fConst136 * ((fConst43 * fVec10[0]) + (fConst137 * fVec10[1]))));
			fRec86[0] = (fRec87[0] - (fConst133 * ((fConst138 * fRec86[2]) + (fConst48 * fRec86[1]))));
			fRec85[0] = ((fConst133 * (((fConst47 * fRec86[0]) + (fConst139 * fRec86[1])) + (fConst47 * fRec86[2]))) - (fConst132 * ((fConst140 * fRec85[2]) + (fConst48 * fRec85[1]))));
			float fTemp44 = (fConst35 * fRec84[1]);
			fRec84[0] = ((fConst132 * (((fConst47 * fRec85[0]) + (fConst139 * fRec85[1])) + (fConst47 * fRec85[2]))) - (fConst31 * ((fConst32 * fRec84[2]) + fTemp44)));
			float fTemp45 = (fConst41 * fRec83[1]);
			fRec83[0] = ((fRec84[2] + (fConst31 * (fTemp44 + (fConst32 * fRec84[0])))) - (fConst38 * ((fConst39 * fRec83[2]) + fTemp45)));
			fRec91[0] = ((0.999f * fRec91[1]) + fSlow23);
			fRec98[0] = ((fConst135 * fRec98[1]) + (fConst136 * (fVec10[0] + fVec10[1])));
			fRec97[0] = (fRec98[0] - (fConst133 * ((fConst138 * fRec97[2]) + (fConst48 * fRec97[1]))));
			fRec96[0] = ((fConst133 * (fRec97[2] + (fRec97[0] + (2 * fRec97[1])))) - (fConst132 * ((fConst140 * fRec96[2]) + (fConst48 * fRec96[1]))));
			float fTemp46 = (fRec96[2] + (fRec96[0] + (2 * fRec96[1])));
			fVec11[0] = fTemp46;
			fRec95[0] = ((fConst145 * fRec95[1]) + (fConst146 * ((fConst37 * fVec11[0]) + (fConst147 * fVec11[1]))));
			fRec94[0] = (fRec95[0] - (fConst143 * ((fConst148 * fRec94[2]) + (fConst41 * fRec94[1]))));
			fRec93[0] = ((fConst143 * (((fConst40 * fRec94[0]) + (fConst149 * fRec94[1])) + (fConst40 * fRec94[2]))) - (fConst142 * ((fConst150 * fRec93[2]) + (fConst41 * fRec93[1]))));
			float fTemp47 = (fConst35 * fRec92[1]);
			fRec92[0] = ((fConst142 * (((fConst40 * fRec93[0]) + (fConst149 * fRec93[1])) + (fConst40 * fRec93[2]))) - (fConst31 * ((fConst32 * fRec92[2]) + fTemp47)));
			fRec99[0] = ((0.999f * fRec99[1]) + fSlow24);
			fRec105[0] = ((fConst145 * fRec105[1]) + (fConst146 * (fVec11[0] + fVec11[1])));
			fRec104[0] = (fRec105[0] - (fConst143 * ((fConst148 * fRec104[2]) + (fConst41 * fRec104[1]))));
			fRec103[0] = ((fConst143 * (fRec104[2] + (fRec104[0] + (2 * fRec104[1])))) - (fConst142 * ((fConst150 * fRec103[2]) + (fConst41 * fRec103[1]))));
			float fTemp48 = (fRec103[2] + (fRec103[0] + (2 * fRec103[1])));
			fVec12[0] = fTemp48;
			fRec102[0] = ((fConst154 * fRec102[1]) + (fConst155 * ((fConst30 * fVec12[0]) + (fConst156 * fVec12[1]))));
			fRec101[0] = (fRec102[0] - (fConst152 * ((fConst157 * fRec101[2]) + (fConst35 * fRec101[1]))));
			fRec100[0] = ((fConst152 * (((fConst34 * fRec101[0]) + (fConst158 * fRec101[1])) + (fConst34 * fRec101[2]))) - (fConst151 * ((fConst159 * fRec100[2]) + (fConst35 * fRec100[1]))));
			fRec106[0] = ((0.999f * fRec106[1]) + fSlow25);
			fRec109[0] = ((fConst154 * fRec109[1]) + (fConst155 * (fVec12[0] + fVec12[1])));
			fRec108[0] = (fRec109[0] - (fConst152 * ((fConst157 * fRec108[2]) + (fConst35 * fRec108[1]))));
			fRec107[0] = ((fConst152 * (fRec108[2] + (fRec108[0] + (2 * fRec108[1])))) - (fConst151 * ((fConst159 * fRec107[2]) + (fConst35 * fRec107[1]))));
			fRec110[0] = ((0.999f * fRec110[1]) + fSlow26);
			float fTemp49 = ((iSlow3)?fTemp3:((((((((((fRec11[2] + (fConst77 * (fTemp12 + (fConst78 * fRec11[0])))) * powf(10,(0.05f * fRec22[0]))) + ((fRec23[2] + (fConst70 * (fTemp20 + (fConst71 * fRec23[0])))) * powf(10,(0.05f * fRec36[0])))) + ((fRec37[2] + (fConst64 * (fTemp27 + (fConst65 * fRec37[0])))) * powf(10,(0.05f * fRec49[0])))) + ((fRec50[2] + (fConst57 * (fTemp33 + (fConst58 * fRec50[0])))) * powf(10,(0.05f * fRec61[0])))) + ((fRec62[2] + (fConst51 * (fTemp38 + (fConst52 * fRec62[0])))) * powf(10,(0.05f * fRec72[0])))) + ((fRec73[2] + (fConst44 * (fTemp42 + (fConst45 * fRec73[0])))) * powf(10,(0.05f * fRec82[0])))) + ((fRec83[2] + (fConst38 * (fTemp45 + (fConst39 * fRec83[0])))) * powf(10,(0.05f * fRec91[0])))) + ((fRec92[2] + (fConst31 * (fTemp47 + (fConst32 * fRec92[0])))) * powf(10,(0.05f * fRec99[0])))) + (fConst151 * (((((fConst34 * fRec100[0]) + (fConst158 * fRec100[1])) + (fConst34 * fRec100[2])) * powf(10,(0.05f * fRec106[0]))) + ((fRec107[2] + (fRec107[0] + (2 * fRec107[1]))) * powf(10,(0.05f * fRec110[0])))))));
			fRec3[0] = (fTemp49 - (fConst11 * ((fConst160 * fRec3[2]) + (fConst161 * fRec3[1]))));
			fRec2[0] = ((fConst11 * (((fConst13 * fRec3[0]) + (fConst162 * fRec3[1])) + (fConst13 * fRec3[2]))) - (fConst8 * ((fConst163 * fRec2[2]) + (fConst164 * fRec2[1]))));
			fRec1[0] = ((fConst8 * (((fConst10 * fRec2[0]) + (fConst165 * fRec2[1])) + (fConst10 * fRec2[2]))) - (fConst4 * ((fConst166 * fRec1[2]) + (fConst167 * fRec1[1]))));
			fRec0[0] = ((fSlow1 * fRec0[1]) + (fSlow2 * fabsf((fConst4 * (((fConst7 * fRec1[0]) + (fConst168 * fRec1[1])) + (fConst7 * fRec1[2]))))));
			fbargraph0 = (fSlow0 + (20 * log10f(fRec0[0])));
			fRec117[0] = (fTemp49 - (fConst185 * ((fConst188 * fRec117[2]) + (fConst189 * fRec117[1]))));
			fRec116[0] = ((fConst185 * (((fConst187 * fRec117[0]) + (fConst190 * fRec117[1])) + (fConst187 * fRec117[2]))) - (fConst183 * ((fConst191 * fRec116[2]) + (fConst192 * fRec116[1]))));
			fRec115[0] = ((fConst183 * (((fConst184 * fRec116[0]) + (fConst193 * fRec116[1])) + (fConst184 * fRec116[2]))) - (fConst181 * ((fConst194 * fRec115[2]) + (fConst195 * fRec115[1]))));
			float fTemp50 = (fConst181 * (((fConst182 * fRec115[0]) + (fConst196 * fRec115[1])) + (fConst182 * fRec115[2])));
			fRec114[0] = (fTemp50 - (fConst178 * ((fConst197 * fRec114[2]) + (fConst199 * fRec114[1]))));
			fRec113[0] = ((fConst178 * (((fConst180 * fRec114[0]) + (fConst200 * fRec114[1])) + (fConst180 * fRec114[2]))) - (fConst175 * ((fConst201 * fRec113[2]) + (fConst202 * fRec113[1]))));
			fRec112[0] = ((fConst175 * (((fConst177 * fRec113[0]) + (fConst203 * fRec113[1])) + (fConst177 * fRec113[2]))) - (fConst171 * ((fConst204 * fRec112[2]) + (fConst205 * fRec112[1]))));
			fRec111[0] = ((fSlow1 * fRec111[1]) + (fSlow2 * fabsf((fConst171 * (((fConst174 * fRec112[0]) + (fConst206 * fRec112[1])) + (fConst174 * fRec112[2]))))));
			fbargraph1 = (fSlow0 + (20 * log10f(fRec111[0])));
			fRec124[0] = (fTemp50 - (fConst223 * ((fConst226 * fRec124[2]) + (fConst227 * fRec124[1]))));
			fRec123[0] = ((fConst223 * (((fConst225 * fRec124[0]) + (fConst228 * fRec124[1])) + (fConst225 * fRec124[2]))) - (fConst221 * ((fConst229 * fRec123[2]) + (fConst230 * fRec123[1]))));
			fRec122[0] = ((fConst221 * (((fConst222 * fRec123[0]) + (fConst231 * fRec123[1])) + (fConst222 * fRec123[2]))) - (fConst219 * ((fConst232 * fRec122[2]) + (fConst233 * fRec122[1]))));
			float fTemp51 = (fConst219 * (((fConst220 * fRec122[0]) + (fConst234 * fRec122[1])) + (fConst220 * fRec122[2])));
			fRec121[0] = (fTemp51 - (fConst216 * ((fConst235 * fRec121[2]) + (fConst237 * fRec121[1]))));
			fRec120[0] = ((fConst216 * (((fConst218 * fRec121[0]) + (fConst238 * fRec121[1])) + (fConst218 * fRec121[2]))) - (fConst213 * ((fConst239 * fRec120[2]) + (fConst240 * fRec120[1]))));
			fRec119[0] = ((fConst213 * (((fConst215 * fRec120[0]) + (fConst241 * fRec120[1])) + (fConst215 * fRec120[2]))) - (fConst209 * ((fConst242 * fRec119[2]) + (fConst243 * fRec119[1]))));
			fRec118[0] = ((fSlow1 * fRec118[1]) + (fSlow2 * fabsf((fConst209 * (((fConst212 * fRec119[0]) + (fConst244 * fRec119[1])) + (fConst212 * fRec119[2]))))));
			fbargraph2 = (fSlow0 + (20 * log10f(fRec118[0])));
			fRec131[0] = (fTemp51 - (fConst258 * ((fConst261 * fRec131[2]) + (fConst262 * fRec131[1]))));
			fRec130[0] = ((fConst258 * (((fConst260 * fRec131[0]) + (fConst263 * fRec131[1])) + (fConst260 * fRec131[2]))) - (fConst256 * ((fConst264 * fRec130[2]) + (fConst265 * fRec130[1]))));
			fRec129[0] = ((fConst256 * (((fConst257 * fRec130[0]) + (fConst266 * fRec130[1])) + (fConst257 * fRec130[2]))) - (fConst254 * ((fConst267 * fRec129[2]) + (fConst268 * fRec129[1]))));
			float fTemp52 = (fConst254 * (((fConst255 * fRec129[0]) + (fConst269 * fRec129[1])) + (fConst255 * fRec129[2])));
			fRec128[0] = (fTemp52 - (fConst251 * ((fConst270 * fRec128[2]) + (fConst271 * fRec128[1]))));
			fRec127[0] = ((fConst251 * (((fConst253 * fRec128[0]) + (fConst272 * fRec128[1])) + (fConst253 * fRec128[2]))) - (fConst248 * ((fConst273 * fRec127[2]) + (fConst274 * fRec127[1]))));
			fRec126[0] = ((fConst248 * (((fConst250 * fRec127[0]) + (fConst275 * fRec127[1])) + (fConst250 * fRec127[2]))) - (fConst245 * ((fConst276 * fRec126[2]) + (fConst277 * fRec126[1]))));
			fRec125[0] = ((fSlow1 * fRec125[1]) + (fSlow2 * fabsf((fConst245 * (((fConst247 * fRec126[0]) + (fConst278 * fRec126[1])) + (fConst247 * fRec126[2]))))));
			fbargraph3 = (fSlow0 + (20 * log10f(fRec125[0])));
			fRec138[0] = (fTemp52 - (fConst295 * ((fConst298 * fRec138[2]) + (fConst299 * fRec138[1]))));
			fRec137[0] = ((fConst295 * (((fConst297 * fRec138[0]) + (fConst300 * fRec138[1])) + (fConst297 * fRec138[2]))) - (fConst293 * ((fConst301 * fRec137[2]) + (fConst302 * fRec137[1]))));
			fRec136[0] = ((fConst293 * (((fConst294 * fRec137[0]) + (fConst303 * fRec137[1])) + (fConst294 * fRec137[2]))) - (fConst291 * ((fConst304 * fRec136[2]) + (fConst305 * fRec136[1]))));
			float fTemp53 = (fConst291 * (((fConst292 * fRec136[0]) + (fConst306 * fRec136[1])) + (fConst292 * fRec136[2])));
			fRec135[0] = (fTemp53 - (fConst288 * ((fConst307 * fRec135[2]) + (fConst309 * fRec135[1]))));
			fRec134[0] = ((fConst288 * (((fConst290 * fRec135[0]) + (fConst310 * fRec135[1])) + (fConst290 * fRec135[2]))) - (fConst285 * ((fConst311 * fRec134[2]) + (fConst312 * fRec134[1]))));
			fRec133[0] = ((fConst285 * (((fConst287 * fRec134[0]) + (fConst313 * fRec134[1])) + (fConst287 * fRec134[2]))) - (fConst281 * ((fConst314 * fRec133[2]) + (fConst315 * fRec133[1]))));
			fRec132[0] = ((fSlow1 * fRec132[1]) + (fSlow2 * fabsf((fConst281 * (((fConst284 * fRec133[0]) + (fConst316 * fRec133[1])) + (fConst284 * fRec133[2]))))));
			fbargraph4 = (fSlow0 + (20 * log10f(fRec132[0])));
			fRec145[0] = (fTemp53 - (fConst333 * ((fConst336 * fRec145[2]) + (fConst337 * fRec145[1]))));
			fRec144[0] = ((fConst333 * (((fConst335 * fRec145[0]) + (fConst338 * fRec145[1])) + (fConst335 * fRec145[2]))) - (fConst331 * ((fConst339 * fRec144[2]) + (fConst340 * fRec144[1]))));
			fRec143[0] = ((fConst331 * (((fConst332 * fRec144[0]) + (fConst341 * fRec144[1])) + (fConst332 * fRec144[2]))) - (fConst329 * ((fConst342 * fRec143[2]) + (fConst343 * fRec143[1]))));
			float fTemp54 = (fConst329 * (((fConst330 * fRec143[0]) + (fConst344 * fRec143[1])) + (fConst330 * fRec143[2])));
			fRec142[0] = (fTemp54 - (fConst326 * ((fConst345 * fRec142[2]) + (fConst347 * fRec142[1]))));
			fRec141[0] = ((fConst326 * (((fConst328 * fRec142[0]) + (fConst348 * fRec142[1])) + (fConst328 * fRec142[2]))) - (fConst323 * ((fConst349 * fRec141[2]) + (fConst350 * fRec141[1]))));
			fRec140[0] = ((fConst323 * (((fConst325 * fRec141[0]) + (fConst351 * fRec141[1])) + (fConst325 * fRec141[2]))) - (fConst319 * ((fConst352 * fRec140[2]) + (fConst353 * fRec140[1]))));
			fRec139[0] = ((fSlow1 * fRec139[1]) + (fSlow2 * fabsf((fConst319 * (((fConst322 * fRec140[0]) + (fConst354 * fRec140[1])) + (fConst322 * fRec140[2]))))));
			fbargraph5 = (fSlow0 + (20 * log10f(fRec139[0])));
			fRec152[0] = (fTemp54 - (fConst368 * ((fConst371 * fRec152[2]) + (fConst372 * fRec152[1]))));
			fRec151[0] = ((fConst368 * (((fConst370 * fRec152[0]) + (fConst373 * fRec152[1])) + (fConst370 * fRec152[2]))) - (fConst366 * ((fConst374 * fRec151[2]) + (fConst375 * fRec151[1]))));
			fRec150[0] = ((fConst366 * (((fConst367 * fRec151[0]) + (fConst376 * fRec151[1])) + (fConst367 * fRec151[2]))) - (fConst364 * ((fConst377 * fRec150[2]) + (fConst378 * fRec150[1]))));
			float fTemp55 = (fConst364 * (((fConst365 * fRec150[0]) + (fConst379 * fRec150[1])) + (fConst365 * fRec150[2])));
			fRec149[0] = (fTemp55 - (fConst361 * ((fConst380 * fRec149[2]) + (fConst381 * fRec149[1]))));
			fRec148[0] = ((fConst361 * (((fConst363 * fRec149[0]) + (fConst382 * fRec149[1])) + (fConst363 * fRec149[2]))) - (fConst358 * ((fConst383 * fRec148[2]) + (fConst384 * fRec148[1]))));
			fRec147[0] = ((fConst358 * (((fConst360 * fRec148[0]) + (fConst385 * fRec148[1])) + (fConst360 * fRec148[2]))) - (fConst355 * ((fConst386 * fRec147[2]) + (fConst387 * fRec147[1]))));
			fRec146[0] = ((fSlow1 * fRec146[1]) + (fSlow2 * fabsf((fConst355 * (((fConst357 * fRec147[0]) + (fConst388 * fRec147[1])) + (fConst357 * fRec147[2]))))));
			fbargraph6 = (fSlow0 + (20 * log10f(fRec146[0])));
			fRec159[0] = (fTemp55 - (fConst405 * ((fConst408 * fRec159[2]) + (fConst409 * fRec159[1]))));
			fRec158[0] = ((fConst405 * (((fConst407 * fRec159[0]) + (fConst410 * fRec159[1])) + (fConst407 * fRec159[2]))) - (fConst403 * ((fConst411 * fRec158[2]) + (fConst412 * fRec158[1]))));
			fRec157[0] = ((fConst403 * (((fConst404 * fRec158[0]) + (fConst413 * fRec158[1])) + (fConst404 * fRec158[2]))) - (fConst401 * ((fConst414 * fRec157[2]) + (fConst415 * fRec157[1]))));
			float fTemp56 = (fConst401 * (((fConst402 * fRec157[0]) + (fConst416 * fRec157[1])) + (fConst402 * fRec157[2])));
			fRec156[0] = (fTemp56 - (fConst398 * ((fConst417 * fRec156[2]) + (fConst419 * fRec156[1]))));
			fRec155[0] = ((fConst398 * (((fConst400 * fRec156[0]) + (fConst420 * fRec156[1])) + (fConst400 * fRec156[2]))) - (fConst395 * ((fConst421 * fRec155[2]) + (fConst422 * fRec155[1]))));
			fRec154[0] = ((fConst395 * (((fConst397 * fRec155[0]) + (fConst423 * fRec155[1])) + (fConst397 * fRec155[2]))) - (fConst391 * ((fConst424 * fRec154[2]) + (fConst425 * fRec154[1]))));
			fRec153[0] = ((fSlow1 * fRec153[1]) + (fSlow2 * fabsf((fConst391 * (((fConst394 * fRec154[0]) + (fConst426 * fRec154[1])) + (fConst394 * fRec154[2]))))));
			fbargraph7 = (fSlow0 + (20 * log10f(fRec153[0])));
			fRec166[0] = (fTemp56 - (fConst443 * ((fConst446 * fRec166[2]) + (fConst447 * fRec166[1]))));
			fRec165[0] = ((fConst443 * (((fConst445 * fRec166[0]) + (fConst448 * fRec166[1])) + (fConst445 * fRec166[2]))) - (fConst441 * ((fConst449 * fRec165[2]) + (fConst450 * fRec165[1]))));
			fRec164[0] = ((fConst441 * (((fConst442 * fRec165[0]) + (fConst451 * fRec165[1])) + (fConst442 * fRec165[2]))) - (fConst439 * ((fConst452 * fRec164[2]) + (fConst453 * fRec164[1]))));
			float fTemp57 = (fConst439 * (((fConst440 * fRec164[0]) + (fConst454 * fRec164[1])) + (fConst440 * fRec164[2])));
			fRec163[0] = (fTemp57 - (fConst436 * ((fConst455 * fRec163[2]) + (fConst457 * fRec163[1]))));
			fRec162[0] = ((fConst436 * (((fConst438 * fRec163[0]) + (fConst458 * fRec163[1])) + (fConst438 * fRec163[2]))) - (fConst433 * ((fConst459 * fRec162[2]) + (fConst460 * fRec162[1]))));
			fRec161[0] = ((fConst433 * (((fConst435 * fRec162[0]) + (fConst461 * fRec162[1])) + (fConst435 * fRec162[2]))) - (fConst429 * ((fConst462 * fRec161[2]) + (fConst463 * fRec161[1]))));
			fRec160[0] = ((fSlow1 * fRec160[1]) + (fSlow2 * fabsf((fConst429 * (((fConst432 * fRec161[0]) + (fConst464 * fRec161[1])) + (fConst432 * fRec161[2]))))));
			fbargraph8 = (fSlow0 + (20 * log10f(fRec160[0])));
			fRec173[0] = (fTemp57 - (fConst478 * ((fConst481 * fRec173[2]) + (fConst482 * fRec173[1]))));
			fRec172[0] = ((fConst478 * (((fConst480 * fRec173[0]) + (fConst483 * fRec173[1])) + (fConst480 * fRec173[2]))) - (fConst476 * ((fConst484 * fRec172[2]) + (fConst485 * fRec172[1]))));
			fRec171[0] = ((fConst476 * (((fConst477 * fRec172[0]) + (fConst486 * fRec172[1])) + (fConst477 * fRec172[2]))) - (fConst474 * ((fConst487 * fRec171[2]) + (fConst488 * fRec171[1]))));
			float fTemp58 = (fConst474 * (((fConst475 * fRec171[0]) + (fConst489 * fRec171[1])) + (fConst475 * fRec171[2])));
			fRec170[0] = (fTemp58 - (fConst471 * ((fConst490 * fRec170[2]) + (fConst491 * fRec170[1]))));
			fRec169[0] = ((fConst471 * (((fConst473 * fRec170[0]) + (fConst492 * fRec170[1])) + (fConst473 * fRec170[2]))) - (fConst468 * ((fConst493 * fRec169[2]) + (fConst494 * fRec169[1]))));
			fRec168[0] = ((fConst468 * (((fConst470 * fRec169[0]) + (fConst495 * fRec169[1])) + (fConst470 * fRec169[2]))) - (fConst465 * ((fConst496 * fRec168[2]) + (fConst497 * fRec168[1]))));
			fRec167[0] = ((fSlow1 * fRec167[1]) + (fSlow2 * fabsf((fConst465 * (((fConst467 * fRec168[0]) + (fConst498 * fRec168[1])) + (fConst467 * fRec168[2]))))));
			fbargraph9 = (fSlow0 + (20 * log10f(fRec167[0])));
			fRec180[0] = (fTemp58 - (fConst515 * ((fConst518 * fRec180[2]) + (fConst519 * fRec180[1]))));
			fRec179[0] = ((fConst515 * (((fConst517 * fRec180[0]) + (fConst520 * fRec180[1])) + (fConst517 * fRec180[2]))) - (fConst513 * ((fConst521 * fRec179[2]) + (fConst522 * fRec179[1]))));
			fRec178[0] = ((fConst513 * (((fConst514 * fRec179[0]) + (fConst523 * fRec179[1])) + (fConst514 * fRec179[2]))) - (fConst511 * ((fConst524 * fRec178[2]) + (fConst525 * fRec178[1]))));
			float fTemp59 = (fConst511 * (((fConst512 * fRec178[0]) + (fConst526 * fRec178[1])) + (fConst512 * fRec178[2])));
			fRec177[0] = (fTemp59 - (fConst508 * ((fConst527 * fRec177[2]) + (fConst529 * fRec177[1]))));
			fRec176[0] = ((fConst508 * (((fConst510 * fRec177[0]) + (fConst530 * fRec177[1])) + (fConst510 * fRec177[2]))) - (fConst505 * ((fConst531 * fRec176[2]) + (fConst532 * fRec176[1]))));
			fRec175[0] = ((fConst505 * (((fConst507 * fRec176[0]) + (fConst533 * fRec176[1])) + (fConst507 * fRec176[2]))) - (fConst501 * ((fConst534 * fRec175[2]) + (fConst535 * fRec175[1]))));
			fRec174[0] = ((fSlow1 * fRec174[1]) + (fSlow2 * fabsf((fConst501 * (((fConst504 * fRec175[0]) + (fConst536 * fRec175[1])) + (fConst504 * fRec175[2]))))));
			fbargraph10 = (fSlow0 + (20 * log10f(fRec174[0])));
			fRec187[0] = (fTemp59 - (fConst553 * ((fConst556 * fRec187[2]) + (fConst557 * fRec187[1]))));
			fRec186[0] = ((fConst553 * (((fConst555 * fRec187[0]) + (fConst558 * fRec187[1])) + (fConst555 * fRec187[2]))) - (fConst551 * ((fConst559 * fRec186[2]) + (fConst560 * fRec186[1]))));
			fRec185[0] = ((fConst551 * (((fConst552 * fRec186[0]) + (fConst561 * fRec186[1])) + (fConst552 * fRec186[2]))) - (fConst549 * ((fConst562 * fRec185[2]) + (fConst563 * fRec185[1]))));
			float fTemp60 = (fConst549 * (((fConst550 * fRec185[0]) + (fConst564 * fRec185[1])) + (fConst550 * fRec185[2])));
			fRec184[0] = (fTemp60 - (fConst546 * ((fConst565 * fRec184[2]) + (fConst567 * fRec184[1]))));
			fRec183[0] = ((fConst546 * (((fConst548 * fRec184[0]) + (fConst568 * fRec184[1])) + (fConst548 * fRec184[2]))) - (fConst543 * ((fConst569 * fRec183[2]) + (fConst570 * fRec183[1]))));
			fRec182[0] = ((fConst543 * (((fConst545 * fRec183[0]) + (fConst571 * fRec183[1])) + (fConst545 * fRec183[2]))) - (fConst539 * ((fConst572 * fRec182[2]) + (fConst573 * fRec182[1]))));
			fRec181[0] = ((fSlow1 * fRec181[1]) + (fSlow2 * fabsf((fConst539 * (((fConst542 * fRec182[0]) + (fConst574 * fRec182[1])) + (fConst542 * fRec182[2]))))));
			fbargraph11 = (fSlow0 + (20 * log10f(fRec181[0])));
			fRec194[0] = (fTemp60 - (fConst588 * ((fConst591 * fRec194[2]) + (fConst592 * fRec194[1]))));
			fRec193[0] = ((fConst588 * (((fConst590 * fRec194[0]) + (fConst593 * fRec194[1])) + (fConst590 * fRec194[2]))) - (fConst586 * ((fConst594 * fRec193[2]) + (fConst595 * fRec193[1]))));
			fRec192[0] = ((fConst586 * (((fConst587 * fRec193[0]) + (fConst596 * fRec193[1])) + (fConst587 * fRec193[2]))) - (fConst584 * ((fConst597 * fRec192[2]) + (fConst598 * fRec192[1]))));
			float fTemp61 = (fConst584 * (((fConst585 * fRec192[0]) + (fConst599 * fRec192[1])) + (fConst585 * fRec192[2])));
			fRec191[0] = (fTemp61 - (fConst581 * ((fConst600 * fRec191[2]) + (fConst601 * fRec191[1]))));
			fRec190[0] = ((fConst581 * (((fConst583 * fRec191[0]) + (fConst602 * fRec191[1])) + (fConst583 * fRec191[2]))) - (fConst578 * ((fConst603 * fRec190[2]) + (fConst604 * fRec190[1]))));
			fRec189[0] = ((fConst578 * (((fConst580 * fRec190[0]) + (fConst605 * fRec190[1])) + (fConst580 * fRec190[2]))) - (fConst575 * ((fConst606 * fRec189[2]) + (fConst607 * fRec189[1]))));
			fRec188[0] = ((fSlow1 * fRec188[1]) + (fSlow2 * fabsf((fConst575 * (((fConst577 * fRec189[0]) + (fConst608 * fRec189[1])) + (fConst577 * fRec189[2]))))));
			fbargraph12 = (fSlow0 + (20 * log10f(fRec188[0])));
			fRec201[0] = (fTemp61 - (fConst625 * ((fConst628 * fRec201[2]) + (fConst629 * fRec201[1]))));
			fRec200[0] = ((fConst625 * (((fConst627 * fRec201[0]) + (fConst630 * fRec201[1])) + (fConst627 * fRec201[2]))) - (fConst623 * ((fConst631 * fRec200[2]) + (fConst632 * fRec200[1]))));
			fRec199[0] = ((fConst623 * (((fConst624 * fRec200[0]) + (fConst633 * fRec200[1])) + (fConst624 * fRec200[2]))) - (fConst621 * ((fConst634 * fRec199[2]) + (fConst635 * fRec199[1]))));
			float fTemp62 = (fConst621 * (((fConst622 * fRec199[0]) + (fConst636 * fRec199[1])) + (fConst622 * fRec199[2])));
			fRec198[0] = (fTemp62 - (fConst618 * ((fConst637 * fRec198[2]) + (fConst639 * fRec198[1]))));
			fRec197[0] = ((fConst618 * (((fConst620 * fRec198[0]) + (fConst640 * fRec198[1])) + (fConst620 * fRec198[2]))) - (fConst615 * ((fConst641 * fRec197[2]) + (fConst642 * fRec197[1]))));
			fRec196[0] = ((fConst615 * (((fConst617 * fRec197[0]) + (fConst643 * fRec197[1])) + (fConst617 * fRec197[2]))) - (fConst611 * ((fConst644 * fRec196[2]) + (fConst645 * fRec196[1]))));
			fRec195[0] = ((fSlow1 * fRec195[1]) + (fSlow2 * fabsf((fConst611 * (((fConst614 * fRec196[0]) + (fConst646 * fRec196[1])) + (fConst614 * fRec196[2]))))));
			fbargraph13 = (fSlow0 + (20 * log10f(fRec195[0])));
			fRec205[0] = (fTemp62 - (fConst651 * ((fConst654 * fRec205[2]) + (fConst655 * fRec205[1]))));
			fRec204[0] = ((fConst651 * (((fConst653 * fRec205[0]) + (fConst656 * fRec205[1])) + (fConst653 * fRec205[2]))) - (fConst649 * ((fConst657 * fRec204[2]) + (fConst658 * fRec204[1]))));
			fRec203[0] = ((fConst649 * (((fConst650 * fRec204[0]) + (fConst659 * fRec204[1])) + (fConst650 * fRec204[2]))) - (fConst647 * ((fConst660 * fRec203[2]) + (fConst661 * fRec203[1]))));
			fRec202[0] = ((fSlow1 * fRec202[1]) + (fSlow2 * fabsf((fConst647 * (((fConst648 * fRec203[0]) + (fConst662 * fRec203[1])) + (fConst648 * fRec203[2]))))));
			fbargraph14 = (fSlow0 + (20 * log10f(fRec202[0])));
			float fTemp63 = fTemp49;
			output0[i] = (FAUSTFLOAT)fTemp63;
			output1[i] = (FAUSTFLOAT)fTemp63;
			// post processing
			fRec202[1] = fRec202[0];
			fRec203[2] = fRec203[1]; fRec203[1] = fRec203[0];
			fRec204[2] = fRec204[1]; fRec204[1] = fRec204[0];
			fRec205[2] = fRec205[1]; fRec205[1] = fRec205[0];
			fRec195[1] = fRec195[0];
			fRec196[2] = fRec196[1]; fRec196[1] = fRec196[0];
			fRec197[2] = fRec197[1]; fRec197[1] = fRec197[0];
			fRec198[2] = fRec198[1]; fRec198[1] = fRec198[0];
			fRec199[2] = fRec199[1]; fRec199[1] = fRec199[0];
			fRec200[2] = fRec200[1]; fRec200[1] = fRec200[0];
			fRec201[2] = fRec201[1]; fRec201[1] = fRec201[0];
			fRec188[1] = fRec188[0];
			fRec189[2] = fRec189[1]; fRec189[1] = fRec189[0];
			fRec190[2] = fRec190[1]; fRec190[1] = fRec190[0];
			fRec191[2] = fRec191[1]; fRec191[1] = fRec191[0];
			fRec192[2] = fRec192[1]; fRec192[1] = fRec192[0];
			fRec193[2] = fRec193[1]; fRec193[1] = fRec193[0];
			fRec194[2] = fRec194[1]; fRec194[1] = fRec194[0];
			fRec181[1] = fRec181[0];
			fRec182[2] = fRec182[1]; fRec182[1] = fRec182[0];
			fRec183[2] = fRec183[1]; fRec183[1] = fRec183[0];
			fRec184[2] = fRec184[1]; fRec184[1] = fRec184[0];
			fRec185[2] = fRec185[1]; fRec185[1] = fRec185[0];
			fRec186[2] = fRec186[1]; fRec186[1] = fRec186[0];
			fRec187[2] = fRec187[1]; fRec187[1] = fRec187[0];
			fRec174[1] = fRec174[0];
			fRec175[2] = fRec175[1]; fRec175[1] = fRec175[0];
			fRec176[2] = fRec176[1]; fRec176[1] = fRec176[0];
			fRec177[2] = fRec177[1]; fRec177[1] = fRec177[0];
			fRec178[2] = fRec178[1]; fRec178[1] = fRec178[0];
			fRec179[2] = fRec179[1]; fRec179[1] = fRec179[0];
			fRec180[2] = fRec180[1]; fRec180[1] = fRec180[0];
			fRec167[1] = fRec167[0];
			fRec168[2] = fRec168[1]; fRec168[1] = fRec168[0];
			fRec169[2] = fRec169[1]; fRec169[1] = fRec169[0];
			fRec170[2] = fRec170[1]; fRec170[1] = fRec170[0];
			fRec171[2] = fRec171[1]; fRec171[1] = fRec171[0];
			fRec172[2] = fRec172[1]; fRec172[1] = fRec172[0];
			fRec173[2] = fRec173[1]; fRec173[1] = fRec173[0];
			fRec160[1] = fRec160[0];
			fRec161[2] = fRec161[1]; fRec161[1] = fRec161[0];
			fRec162[2] = fRec162[1]; fRec162[1] = fRec162[0];
			fRec163[2] = fRec163[1]; fRec163[1] = fRec163[0];
			fRec164[2] = fRec164[1]; fRec164[1] = fRec164[0];
			fRec165[2] = fRec165[1]; fRec165[1] = fRec165[0];
			fRec166[2] = fRec166[1]; fRec166[1] = fRec166[0];
			fRec153[1] = fRec153[0];
			fRec154[2] = fRec154[1]; fRec154[1] = fRec154[0];
			fRec155[2] = fRec155[1]; fRec155[1] = fRec155[0];
			fRec156[2] = fRec156[1]; fRec156[1] = fRec156[0];
			fRec157[2] = fRec157[1]; fRec157[1] = fRec157[0];
			fRec158[2] = fRec158[1]; fRec158[1] = fRec158[0];
			fRec159[2] = fRec159[1]; fRec159[1] = fRec159[0];
			fRec146[1] = fRec146[0];
			fRec147[2] = fRec147[1]; fRec147[1] = fRec147[0];
			fRec148[2] = fRec148[1]; fRec148[1] = fRec148[0];
			fRec149[2] = fRec149[1]; fRec149[1] = fRec149[0];
			fRec150[2] = fRec150[1]; fRec150[1] = fRec150[0];
			fRec151[2] = fRec151[1]; fRec151[1] = fRec151[0];
			fRec152[2] = fRec152[1]; fRec152[1] = fRec152[0];
			fRec139[1] = fRec139[0];
			fRec140[2] = fRec140[1]; fRec140[1] = fRec140[0];
			fRec141[2] = fRec141[1]; fRec141[1] = fRec141[0];
			fRec142[2] = fRec142[1]; fRec142[1] = fRec142[0];
			fRec143[2] = fRec143[1]; fRec143[1] = fRec143[0];
			fRec144[2] = fRec144[1]; fRec144[1] = fRec144[0];
			fRec145[2] = fRec145[1]; fRec145[1] = fRec145[0];
			fRec132[1] = fRec132[0];
			fRec133[2] = fRec133[1]; fRec133[1] = fRec133[0];
			fRec134[2] = fRec134[1]; fRec134[1] = fRec134[0];
			fRec135[2] = fRec135[1]; fRec135[1] = fRec135[0];
			fRec136[2] = fRec136[1]; fRec136[1] = fRec136[0];
			fRec137[2] = fRec137[1]; fRec137[1] = fRec137[0];
			fRec138[2] = fRec138[1]; fRec138[1] = fRec138[0];
			fRec125[1] = fRec125[0];
			fRec126[2] = fRec126[1]; fRec126[1] = fRec126[0];
			fRec127[2] = fRec127[1]; fRec127[1] = fRec127[0];
			fRec128[2] = fRec128[1]; fRec128[1] = fRec128[0];
			fRec129[2] = fRec129[1]; fRec129[1] = fRec129[0];
			fRec130[2] = fRec130[1]; fRec130[1] = fRec130[0];
			fRec131[2] = fRec131[1]; fRec131[1] = fRec131[0];
			fRec118[1] = fRec118[0];
			fRec119[2] = fRec119[1]; fRec119[1] = fRec119[0];
			fRec120[2] = fRec120[1]; fRec120[1] = fRec120[0];
			fRec121[2] = fRec121[1]; fRec121[1] = fRec121[0];
			fRec122[2] = fRec122[1]; fRec122[1] = fRec122[0];
			fRec123[2] = fRec123[1]; fRec123[1] = fRec123[0];
			fRec124[2] = fRec124[1]; fRec124[1] = fRec124[0];
			fRec111[1] = fRec111[0];
			fRec112[2] = fRec112[1]; fRec112[1] = fRec112[0];
			fRec113[2] = fRec113[1]; fRec113[1] = fRec113[0];
			fRec114[2] = fRec114[1]; fRec114[1] = fRec114[0];
			fRec115[2] = fRec115[1]; fRec115[1] = fRec115[0];
			fRec116[2] = fRec116[1]; fRec116[1] = fRec116[0];
			fRec117[2] = fRec117[1]; fRec117[1] = fRec117[0];
			fRec0[1] = fRec0[0];
			fRec1[2] = fRec1[1]; fRec1[1] = fRec1[0];
			fRec2[2] = fRec2[1]; fRec2[1] = fRec2[0];
			fRec3[2] = fRec3[1]; fRec3[1] = fRec3[0];
			fRec110[1] = fRec110[0];
			fRec107[2] = fRec107[1]; fRec107[1] = fRec107[0];
			fRec108[2] = fRec108[1]; fRec108[1] = fRec108[0];
			fRec109[1] = fRec109[0];
			fRec106[1] = fRec106[0];
			fRec100[2] = fRec100[1]; fRec100[1] = fRec100[0];
			fRec101[2] = fRec101[1]; fRec101[1] = fRec101[0];
			fRec102[1] = fRec102[0];
			fVec12[1] = fVec12[0];
			fRec103[2] = fRec103[1]; fRec103[1] = fRec103[0];
			fRec104[2] = fRec104[1]; fRec104[1] = fRec104[0];
			fRec105[1] = fRec105[0];
			fRec99[1] = fRec99[0];
			fRec92[2] = fRec92[1]; fRec92[1] = fRec92[0];
			fRec93[2] = fRec93[1]; fRec93[1] = fRec93[0];
			fRec94[2] = fRec94[1]; fRec94[1] = fRec94[0];
			fRec95[1] = fRec95[0];
			fVec11[1] = fVec11[0];
			fRec96[2] = fRec96[1]; fRec96[1] = fRec96[0];
			fRec97[2] = fRec97[1]; fRec97[1] = fRec97[0];
			fRec98[1] = fRec98[0];
			fRec91[1] = fRec91[0];
			fRec83[2] = fRec83[1]; fRec83[1] = fRec83[0];
			fRec84[2] = fRec84[1]; fRec84[1] = fRec84[0];
			fRec85[2] = fRec85[1]; fRec85[1] = fRec85[0];
			fRec86[2] = fRec86[1]; fRec86[1] = fRec86[0];
			fRec87[1] = fRec87[0];
			fVec10[1] = fVec10[0];
			fRec88[2] = fRec88[1]; fRec88[1] = fRec88[0];
			fRec89[2] = fRec89[1]; fRec89[1] = fRec89[0];
			fRec90[1] = fRec90[0];
			fRec82[1] = fRec82[0];
			fRec73[2] = fRec73[1]; fRec73[1] = fRec73[0];
			fRec74[2] = fRec74[1]; fRec74[1] = fRec74[0];
			fRec75[2] = fRec75[1]; fRec75[1] = fRec75[0];
			fRec76[2] = fRec76[1]; fRec76[1] = fRec76[0];
			fRec77[2] = fRec77[1]; fRec77[1] = fRec77[0];
			fRec78[1] = fRec78[0];
			fVec9[1] = fVec9[0];
			fRec79[2] = fRec79[1]; fRec79[1] = fRec79[0];
			fRec80[2] = fRec80[1]; fRec80[1] = fRec80[0];
			fRec81[1] = fRec81[0];
			fRec72[1] = fRec72[0];
			fRec62[2] = fRec62[1]; fRec62[1] = fRec62[0];
			fRec63[2] = fRec63[1]; fRec63[1] = fRec63[0];
			fRec64[2] = fRec64[1]; fRec64[1] = fRec64[0];
			fRec65[2] = fRec65[1]; fRec65[1] = fRec65[0];
			fRec66[2] = fRec66[1]; fRec66[1] = fRec66[0];
			fRec67[2] = fRec67[1]; fRec67[1] = fRec67[0];
			fRec68[1] = fRec68[0];
			fVec8[1] = fVec8[0];
			fRec69[2] = fRec69[1]; fRec69[1] = fRec69[0];
			fRec70[2] = fRec70[1]; fRec70[1] = fRec70[0];
			fRec71[1] = fRec71[0];
			fRec61[1] = fRec61[0];
			fRec50[2] = fRec50[1]; fRec50[1] = fRec50[0];
			fRec51[2] = fRec51[1]; fRec51[1] = fRec51[0];
			fRec52[2] = fRec52[1]; fRec52[1] = fRec52[0];
			fRec53[2] = fRec53[1]; fRec53[1] = fRec53[0];
			fRec54[2] = fRec54[1]; fRec54[1] = fRec54[0];
			fRec55[2] = fRec55[1]; fRec55[1] = fRec55[0];
			fRec56[2] = fRec56[1]; fRec56[1] = fRec56[0];
			fRec57[1] = fRec57[0];
			fVec7[1] = fVec7[0];
			fRec58[2] = fRec58[1]; fRec58[1] = fRec58[0];
			fRec59[2] = fRec59[1]; fRec59[1] = fRec59[0];
			fRec60[1] = fRec60[0];
			fRec49[1] = fRec49[0];
			fRec37[2] = fRec37[1]; fRec37[1] = fRec37[0];
			fRec38[2] = fRec38[1]; fRec38[1] = fRec38[0];
			fRec39[2] = fRec39[1]; fRec39[1] = fRec39[0];
			fRec40[2] = fRec40[1]; fRec40[1] = fRec40[0];
			fRec41[2] = fRec41[1]; fRec41[1] = fRec41[0];
			fRec42[2] = fRec42[1]; fRec42[1] = fRec42[0];
			fRec43[2] = fRec43[1]; fRec43[1] = fRec43[0];
			fRec44[2] = fRec44[1]; fRec44[1] = fRec44[0];
			fRec45[1] = fRec45[0];
			fVec6[1] = fVec6[0];
			fRec46[2] = fRec46[1]; fRec46[1] = fRec46[0];
			fRec47[2] = fRec47[1]; fRec47[1] = fRec47[0];
			fRec48[1] = fRec48[0];
			fRec36[1] = fRec36[0];
			fRec23[2] = fRec23[1]; fRec23[1] = fRec23[0];
			fRec24[2] = fRec24[1]; fRec24[1] = fRec24[0];
			fRec25[2] = fRec25[1]; fRec25[1] = fRec25[0];
			fRec26[2] = fRec26[1]; fRec26[1] = fRec26[0];
			fRec27[2] = fRec27[1]; fRec27[1] = fRec27[0];
			fRec28[2] = fRec28[1]; fRec28[1] = fRec28[0];
			fRec29[2] = fRec29[1]; fRec29[1] = fRec29[0];
			fRec30[2] = fRec30[1]; fRec30[1] = fRec30[0];
			fRec31[2] = fRec31[1]; fRec31[1] = fRec31[0];
			fRec32[1] = fRec32[0];
			fVec5[1] = fVec5[0];
			fRec33[2] = fRec33[1]; fRec33[1] = fRec33[0];
			fRec34[2] = fRec34[1]; fRec34[1] = fRec34[0];
			fRec35[1] = fRec35[0];
			fRec22[1] = fRec22[0];
			fRec11[2] = fRec11[1]; fRec11[1] = fRec11[0];
			fRec12[2] = fRec12[1]; fRec12[1] = fRec12[0];
			fRec13[2] = fRec13[1]; fRec13[1] = fRec13[0];
			fRec14[2] = fRec14[1]; fRec14[1] = fRec14[0];
			fRec15[2] = fRec15[1]; fRec15[1] = fRec15[0];
			fRec16[2] = fRec16[1]; fRec16[1] = fRec16[0];
			fRec17[2] = fRec17[1]; fRec17[1] = fRec17[0];
			fRec18[2] = fRec18[1]; fRec18[1] = fRec18[0];
			fRec19[2] = fRec19[1]; fRec19[1] = fRec19[0];
			fRec20[2] = fRec20[1]; fRec20[1] = fRec20[0];
			fRec21[1] = fRec21[0];
			fVec4[1] = fVec4[0];
			fVec3[1] = fVec3[0];
			fRec10[1] = fRec10[0];
			fVec2[1] = fVec2[0];
			fRec9[1] = fRec9[0];
			fVec1[1] = fVec1[0];
			fRec8[1] = fRec8[0];
			fRec7[1] = fRec7[0];
			fVec0[1] = fVec0[0];
			for (int i=3; i>0; i--) fRec5[i] = fRec5[i-1];
			iRec6[1] = iRec6[0];
			fRec4[1] = fRec4[0];
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
