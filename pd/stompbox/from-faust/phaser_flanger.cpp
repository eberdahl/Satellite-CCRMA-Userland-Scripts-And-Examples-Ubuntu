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
	int 	iVec0[2];
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
	FAUSTFLOAT 	fcheckbox1;
	FAUSTFLOAT 	fslider2;
	float 	fRec4[2];
	FAUSTFLOAT 	fcheckbox2;
	FAUSTFLOAT 	fcheckbox3;
	int 	iRec6[2];
	float 	fRec5[4];
	float 	fConst14;
	float 	fVec1[2];
	float 	fConst15;
	float 	fConst16;
	FAUSTFLOAT 	fslider3;
	FAUSTFLOAT 	fslider4;
	float 	fRec7[2];
	float 	fRec8[2];
	float 	fVec2[2];
	FAUSTFLOAT 	fslider5;
	float 	fRec9[2];
	float 	fVec3[2];
	FAUSTFLOAT 	fslider6;
	float 	fRec10[2];
	float 	fVec4[2];
	FAUSTFLOAT 	fslider7;
	float 	fConst17;
	FAUSTFLOAT 	fslider8;
	float 	fRec11[2];
	float 	fRec12[2];
	FAUSTFLOAT 	fbargraph0;
	FAUSTFLOAT 	fcheckbox4;
	FAUSTFLOAT 	fslider9;
	FAUSTFLOAT 	fslider10;
	int 	IOTA;
	float 	fVec5[2048];
	FAUSTFLOAT 	fslider11;
	FAUSTFLOAT 	fslider12;
	float 	fRec13[2];
	FAUSTFLOAT 	fslider13;
	FAUSTFLOAT 	fcheckbox5;
	FAUSTFLOAT 	fslider14;
	FAUSTFLOAT 	fslider15;
	FAUSTFLOAT 	fslider16;
	FAUSTFLOAT 	fslider17;
	FAUSTFLOAT 	fslider18;
	FAUSTFLOAT 	fslider19;
	float 	fRec16[2];
	float 	fRec17[2];
	FAUSTFLOAT 	fslider20;
	float 	fRec20[3];
	float 	fRec19[3];
	float 	fRec18[3];
	float 	fRec15[3];
	float 	fRec14[2];
	FAUSTFLOAT 	fcheckbox6;
	float 	fVec6[2048];
	float 	fRec21[2];
	float 	fRec26[3];
	float 	fRec25[3];
	float 	fRec24[3];
	float 	fRec23[3];
	float 	fRec22[2];
	float 	fConst18;
	float 	fConst19;
	float 	fConst20;
	float 	fRec3[3];
	float 	fConst21;
	float 	fConst22;
	float 	fConst23;
	float 	fRec2[3];
	float 	fConst24;
	float 	fConst25;
	float 	fConst26;
	float 	fRec1[3];
	float 	fConst27;
	float 	fRec0[2];
	FAUSTFLOAT 	fbargraph1;
	float 	fConst28;
	float 	fConst29;
	float 	fConst30;
	float 	fConst31;
	float 	fConst32;
	float 	fConst33;
	float 	fConst34;
	float 	fConst35;
	float 	fConst36;
	float 	fConst37;
	float 	fConst38;
	float 	fConst39;
	float 	fConst40;
	float 	fConst41;
	float 	fConst42;
	float 	fConst43;
	float 	fConst44;
	float 	fConst45;
	float 	fConst46;
	float 	fConst47;
	float 	fConst48;
	float 	fRec33[3];
	float 	fConst49;
	float 	fConst50;
	float 	fConst51;
	float 	fRec32[3];
	float 	fConst52;
	float 	fConst53;
	float 	fConst54;
	float 	fRec31[3];
	float 	fConst55;
	float 	fConst56;
	float 	fConst57;
	float 	fConst58;
	float 	fRec30[3];
	float 	fConst59;
	float 	fConst60;
	float 	fConst61;
	float 	fRec29[3];
	float 	fConst62;
	float 	fConst63;
	float 	fConst64;
	float 	fRec28[3];
	float 	fConst65;
	float 	fRec27[2];
	FAUSTFLOAT 	fbargraph2;
	float 	fConst66;
	float 	fConst67;
	float 	fConst68;
	float 	fConst69;
	float 	fConst70;
	float 	fConst71;
	float 	fConst72;
	float 	fConst73;
	float 	fConst74;
	float 	fConst75;
	float 	fConst76;
	float 	fConst77;
	float 	fConst78;
	float 	fConst79;
	float 	fConst80;
	float 	fConst81;
	float 	fConst82;
	float 	fConst83;
	float 	fConst84;
	float 	fConst85;
	float 	fConst86;
	float 	fRec40[3];
	float 	fConst87;
	float 	fConst88;
	float 	fConst89;
	float 	fRec39[3];
	float 	fConst90;
	float 	fConst91;
	float 	fConst92;
	float 	fRec38[3];
	float 	fConst93;
	float 	fConst94;
	float 	fConst95;
	float 	fConst96;
	float 	fRec37[3];
	float 	fConst97;
	float 	fConst98;
	float 	fConst99;
	float 	fRec36[3];
	float 	fConst100;
	float 	fConst101;
	float 	fConst102;
	float 	fRec35[3];
	float 	fConst103;
	float 	fRec34[2];
	FAUSTFLOAT 	fbargraph3;
	float 	fConst104;
	float 	fConst105;
	float 	fConst106;
	float 	fConst107;
	float 	fConst108;
	float 	fConst109;
	float 	fConst110;
	float 	fConst111;
	float 	fConst112;
	float 	fConst113;
	float 	fConst114;
	float 	fConst115;
	float 	fConst116;
	float 	fConst117;
	float 	fConst118;
	float 	fConst119;
	float 	fConst120;
	float 	fConst121;
	float 	fConst122;
	float 	fConst123;
	float 	fConst124;
	float 	fRec47[3];
	float 	fConst125;
	float 	fConst126;
	float 	fConst127;
	float 	fRec46[3];
	float 	fConst128;
	float 	fConst129;
	float 	fConst130;
	float 	fRec45[3];
	float 	fConst131;
	float 	fConst132;
	float 	fConst133;
	float 	fConst134;
	float 	fRec44[3];
	float 	fConst135;
	float 	fConst136;
	float 	fConst137;
	float 	fRec43[3];
	float 	fConst138;
	float 	fConst139;
	float 	fConst140;
	float 	fRec42[3];
	float 	fConst141;
	float 	fRec41[2];
	FAUSTFLOAT 	fbargraph4;
	float 	fConst142;
	float 	fConst143;
	float 	fConst144;
	float 	fConst145;
	float 	fConst146;
	float 	fConst147;
	float 	fConst148;
	float 	fConst149;
	float 	fConst150;
	float 	fConst151;
	float 	fConst152;
	float 	fConst153;
	float 	fConst154;
	float 	fConst155;
	float 	fConst156;
	float 	fConst157;
	float 	fConst158;
	float 	fConst159;
	float 	fConst160;
	float 	fConst161;
	float 	fConst162;
	float 	fRec54[3];
	float 	fConst163;
	float 	fConst164;
	float 	fConst165;
	float 	fRec53[3];
	float 	fConst166;
	float 	fConst167;
	float 	fConst168;
	float 	fRec52[3];
	float 	fConst169;
	float 	fConst170;
	float 	fConst171;
	float 	fConst172;
	float 	fRec51[3];
	float 	fConst173;
	float 	fConst174;
	float 	fConst175;
	float 	fRec50[3];
	float 	fConst176;
	float 	fConst177;
	float 	fConst178;
	float 	fRec49[3];
	float 	fConst179;
	float 	fRec48[2];
	FAUSTFLOAT 	fbargraph5;
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
	float 	fConst190;
	float 	fConst191;
	float 	fConst192;
	float 	fConst193;
	float 	fConst194;
	float 	fConst195;
	float 	fConst196;
	float 	fConst197;
	float 	fConst198;
	float 	fConst199;
	float 	fConst200;
	float 	fRec61[3];
	float 	fConst201;
	float 	fConst202;
	float 	fConst203;
	float 	fRec60[3];
	float 	fConst204;
	float 	fConst205;
	float 	fConst206;
	float 	fRec59[3];
	float 	fConst207;
	float 	fConst208;
	float 	fConst209;
	float 	fConst210;
	float 	fRec58[3];
	float 	fConst211;
	float 	fConst212;
	float 	fConst213;
	float 	fRec57[3];
	float 	fConst214;
	float 	fConst215;
	float 	fConst216;
	float 	fRec56[3];
	float 	fConst217;
	float 	fRec55[2];
	FAUSTFLOAT 	fbargraph6;
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
	float 	fConst228;
	float 	fConst229;
	float 	fConst230;
	float 	fConst231;
	float 	fConst232;
	float 	fConst233;
	float 	fConst234;
	float 	fConst235;
	float 	fConst236;
	float 	fConst237;
	float 	fConst238;
	float 	fRec68[3];
	float 	fConst239;
	float 	fConst240;
	float 	fConst241;
	float 	fRec67[3];
	float 	fConst242;
	float 	fConst243;
	float 	fConst244;
	float 	fRec66[3];
	float 	fConst245;
	float 	fConst246;
	float 	fConst247;
	float 	fConst248;
	float 	fRec65[3];
	float 	fConst249;
	float 	fConst250;
	float 	fConst251;
	float 	fRec64[3];
	float 	fConst252;
	float 	fConst253;
	float 	fConst254;
	float 	fRec63[3];
	float 	fConst255;
	float 	fRec62[2];
	FAUSTFLOAT 	fbargraph7;
	float 	fConst256;
	float 	fConst257;
	float 	fConst258;
	float 	fConst259;
	float 	fConst260;
	float 	fConst261;
	float 	fConst262;
	float 	fConst263;
	float 	fConst264;
	float 	fConst265;
	float 	fConst266;
	float 	fConst267;
	float 	fConst268;
	float 	fConst269;
	float 	fConst270;
	float 	fConst271;
	float 	fConst272;
	float 	fConst273;
	float 	fConst274;
	float 	fConst275;
	float 	fConst276;
	float 	fRec75[3];
	float 	fConst277;
	float 	fConst278;
	float 	fConst279;
	float 	fRec74[3];
	float 	fConst280;
	float 	fConst281;
	float 	fConst282;
	float 	fRec73[3];
	float 	fConst283;
	float 	fConst284;
	float 	fConst285;
	float 	fConst286;
	float 	fRec72[3];
	float 	fConst287;
	float 	fConst288;
	float 	fConst289;
	float 	fRec71[3];
	float 	fConst290;
	float 	fConst291;
	float 	fConst292;
	float 	fRec70[3];
	float 	fConst293;
	float 	fRec69[2];
	FAUSTFLOAT 	fbargraph8;
	float 	fConst294;
	float 	fConst295;
	float 	fConst296;
	float 	fConst297;
	float 	fConst298;
	float 	fConst299;
	float 	fConst300;
	float 	fConst301;
	float 	fConst302;
	float 	fConst303;
	float 	fConst304;
	float 	fConst305;
	float 	fConst306;
	float 	fConst307;
	float 	fConst308;
	float 	fConst309;
	float 	fConst310;
	float 	fConst311;
	float 	fConst312;
	float 	fConst313;
	float 	fConst314;
	float 	fRec82[3];
	float 	fConst315;
	float 	fConst316;
	float 	fConst317;
	float 	fRec81[3];
	float 	fConst318;
	float 	fConst319;
	float 	fConst320;
	float 	fRec80[3];
	float 	fConst321;
	float 	fConst322;
	float 	fConst323;
	float 	fConst324;
	float 	fRec79[3];
	float 	fConst325;
	float 	fConst326;
	float 	fConst327;
	float 	fRec78[3];
	float 	fConst328;
	float 	fConst329;
	float 	fConst330;
	float 	fRec77[3];
	float 	fConst331;
	float 	fRec76[2];
	FAUSTFLOAT 	fbargraph9;
	float 	fConst332;
	float 	fConst333;
	float 	fConst334;
	float 	fConst335;
	float 	fConst336;
	float 	fConst337;
	float 	fConst338;
	float 	fConst339;
	float 	fConst340;
	float 	fConst341;
	float 	fConst342;
	float 	fConst343;
	float 	fConst344;
	float 	fConst345;
	float 	fConst346;
	float 	fConst347;
	float 	fConst348;
	float 	fConst349;
	float 	fConst350;
	float 	fConst351;
	float 	fConst352;
	float 	fRec89[3];
	float 	fConst353;
	float 	fConst354;
	float 	fConst355;
	float 	fRec88[3];
	float 	fConst356;
	float 	fConst357;
	float 	fConst358;
	float 	fRec87[3];
	float 	fConst359;
	float 	fConst360;
	float 	fConst361;
	float 	fConst362;
	float 	fRec86[3];
	float 	fConst363;
	float 	fConst364;
	float 	fConst365;
	float 	fRec85[3];
	float 	fConst366;
	float 	fConst367;
	float 	fConst368;
	float 	fRec84[3];
	float 	fConst369;
	float 	fRec83[2];
	FAUSTFLOAT 	fbargraph10;
	float 	fConst370;
	float 	fConst371;
	float 	fConst372;
	float 	fConst373;
	float 	fConst374;
	float 	fConst375;
	float 	fConst376;
	float 	fConst377;
	float 	fConst378;
	float 	fConst379;
	float 	fConst380;
	float 	fConst381;
	float 	fConst382;
	float 	fConst383;
	float 	fConst384;
	float 	fConst385;
	float 	fConst386;
	float 	fConst387;
	float 	fConst388;
	float 	fConst389;
	float 	fConst390;
	float 	fRec96[3];
	float 	fConst391;
	float 	fConst392;
	float 	fConst393;
	float 	fRec95[3];
	float 	fConst394;
	float 	fConst395;
	float 	fConst396;
	float 	fRec94[3];
	float 	fConst397;
	float 	fConst398;
	float 	fConst399;
	float 	fConst400;
	float 	fRec93[3];
	float 	fConst401;
	float 	fConst402;
	float 	fConst403;
	float 	fRec92[3];
	float 	fConst404;
	float 	fConst405;
	float 	fConst406;
	float 	fRec91[3];
	float 	fConst407;
	float 	fRec90[2];
	FAUSTFLOAT 	fbargraph11;
	float 	fConst408;
	float 	fConst409;
	float 	fConst410;
	float 	fConst411;
	float 	fConst412;
	float 	fConst413;
	float 	fConst414;
	float 	fConst415;
	float 	fConst416;
	float 	fConst417;
	float 	fConst418;
	float 	fConst419;
	float 	fConst420;
	float 	fConst421;
	float 	fConst422;
	float 	fConst423;
	float 	fConst424;
	float 	fConst425;
	float 	fConst426;
	float 	fConst427;
	float 	fConst428;
	float 	fRec103[3];
	float 	fConst429;
	float 	fConst430;
	float 	fConst431;
	float 	fRec102[3];
	float 	fConst432;
	float 	fConst433;
	float 	fConst434;
	float 	fRec101[3];
	float 	fConst435;
	float 	fConst436;
	float 	fConst437;
	float 	fConst438;
	float 	fRec100[3];
	float 	fConst439;
	float 	fConst440;
	float 	fConst441;
	float 	fRec99[3];
	float 	fConst442;
	float 	fConst443;
	float 	fConst444;
	float 	fRec98[3];
	float 	fConst445;
	float 	fRec97[2];
	FAUSTFLOAT 	fbargraph12;
	float 	fConst446;
	float 	fConst447;
	float 	fConst448;
	float 	fConst449;
	float 	fConst450;
	float 	fConst451;
	float 	fConst452;
	float 	fConst453;
	float 	fConst454;
	float 	fConst455;
	float 	fConst456;
	float 	fConst457;
	float 	fConst458;
	float 	fConst459;
	float 	fConst460;
	float 	fConst461;
	float 	fConst462;
	float 	fConst463;
	float 	fConst464;
	float 	fConst465;
	float 	fConst466;
	float 	fRec110[3];
	float 	fConst467;
	float 	fConst468;
	float 	fConst469;
	float 	fRec109[3];
	float 	fConst470;
	float 	fConst471;
	float 	fConst472;
	float 	fRec108[3];
	float 	fConst473;
	float 	fConst474;
	float 	fConst475;
	float 	fConst476;
	float 	fRec107[3];
	float 	fConst477;
	float 	fConst478;
	float 	fConst479;
	float 	fRec106[3];
	float 	fConst480;
	float 	fConst481;
	float 	fConst482;
	float 	fRec105[3];
	float 	fConst483;
	float 	fRec104[2];
	FAUSTFLOAT 	fbargraph13;
	float 	fConst484;
	float 	fConst485;
	float 	fConst486;
	float 	fConst487;
	float 	fConst488;
	float 	fConst489;
	float 	fConst490;
	float 	fConst491;
	float 	fConst492;
	float 	fConst493;
	float 	fConst494;
	float 	fConst495;
	float 	fConst496;
	float 	fConst497;
	float 	fConst498;
	float 	fConst499;
	float 	fConst500;
	float 	fConst501;
	float 	fConst502;
	float 	fConst503;
	float 	fConst504;
	float 	fRec117[3];
	float 	fConst505;
	float 	fConst506;
	float 	fConst507;
	float 	fRec116[3];
	float 	fConst508;
	float 	fConst509;
	float 	fConst510;
	float 	fRec115[3];
	float 	fConst511;
	float 	fConst512;
	float 	fConst513;
	float 	fConst514;
	float 	fRec114[3];
	float 	fConst515;
	float 	fConst516;
	float 	fConst517;
	float 	fRec113[3];
	float 	fConst518;
	float 	fConst519;
	float 	fConst520;
	float 	fRec112[3];
	float 	fConst521;
	float 	fRec111[2];
	FAUSTFLOAT 	fbargraph14;
	float 	fConst522;
	float 	fConst523;
	float 	fConst524;
	float 	fConst525;
	float 	fConst526;
	float 	fConst527;
	float 	fConst528;
	float 	fConst529;
	float 	fConst530;
	float 	fRec121[3];
	float 	fConst531;
	float 	fConst532;
	float 	fConst533;
	float 	fRec120[3];
	float 	fConst534;
	float 	fConst535;
	float 	fConst536;
	float 	fRec119[3];
	float 	fConst537;
	float 	fRec118[2];
	FAUSTFLOAT 	fbargraph15;
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
		for (int i=0; i<2; i++) iVec0[i] = 0;
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
		fConst11 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst3) / fConst2)));
		fConst12 = (0.0017661728399818856f / fConst5);
		fConst13 = (0.00040767818495825777f + fConst12);
		fcheckbox0 = 0.0;
		fcheckbox1 = 0.0;
		fslider2 = -2e+01f;
		for (int i=0; i<2; i++) fRec4[i] = 0;
		fcheckbox2 = 0.0;
		fcheckbox3 = 0.0;
		for (int i=0; i<2; i++) iRec6[i] = 0;
		for (int i=0; i<4; i++) fRec5[i] = 0;
		fConst14 = (0.3333333333333333f * iConst0);
		for (int i=0; i<2; i++) fVec1[i] = 0;
		fConst15 = float(iConst0);
		fConst16 = (2.0f / fConst15);
		fslider3 = 0.1f;
		fslider4 = 49.0f;
		for (int i=0; i<2; i++) fRec7[i] = 0;
		for (int i=0; i<2; i++) fRec8[i] = 0;
		for (int i=0; i<2; i++) fVec2[i] = 0;
		fslider5 = -0.1f;
		for (int i=0; i<2; i++) fRec9[i] = 0;
		for (int i=0; i<2; i++) fVec3[i] = 0;
		fslider6 = 0.1f;
		for (int i=0; i<2; i++) fRec10[i] = 0;
		for (int i=0; i<2; i++) fVec4[i] = 0;
		fslider7 = 0.0f;
		fConst17 = (6.283185307179586f / float(iConst0));
		fslider8 = 0.5f;
		for (int i=0; i<2; i++) fRec11[i] = 0;
		for (int i=0; i<2; i++) fRec12[i] = 0;
		fcheckbox4 = 0.0;
		fslider9 = 1.0f;
		fslider10 = 0.0f;
		IOTA = 0;
		for (int i=0; i<2048; i++) fVec5[i] = 0;
		fslider11 = 1.0f;
		fslider12 = 1e+01f;
		for (int i=0; i<2; i++) fRec13[i] = 0;
		fslider13 = 0.0f;
		fcheckbox5 = 0.0;
		fslider14 = 1.0f;
		fslider15 = 1e+03f;
		fslider16 = 1.5f;
		fslider17 = 1e+02f;
		fslider18 = 8e+02f;
		fslider19 = 0.5f;
		for (int i=0; i<2; i++) fRec16[i] = 0;
		for (int i=0; i<2; i++) fRec17[i] = 0;
		fslider20 = 0.0f;
		for (int i=0; i<3; i++) fRec20[i] = 0;
		for (int i=0; i<3; i++) fRec19[i] = 0;
		for (int i=0; i<3; i++) fRec18[i] = 0;
		for (int i=0; i<3; i++) fRec15[i] = 0;
		for (int i=0; i<2; i++) fRec14[i] = 0;
		fcheckbox6 = 0.0;
		for (int i=0; i<2048; i++) fVec6[i] = 0;
		for (int i=0; i<2; i++) fRec21[i] = 0;
		for (int i=0; i<3; i++) fRec26[i] = 0;
		for (int i=0; i<3; i++) fRec25[i] = 0;
		for (int i=0; i<3; i++) fRec24[i] = 0;
		for (int i=0; i<3; i++) fRec23[i] = 0;
		for (int i=0; i<2; i++) fRec22[i] = 0;
		fConst18 = (4.076781969643807f + ((fConst3 - 3.1897274020965583f) / fConst2));
		fConst19 = (1.0f / fConst5);
		fConst20 = (2 * (4.076781969643807f - fConst19));
		for (int i=0; i<3; i++) fRec3[i] = 0;
		fConst21 = (2 * (0.00040767818495825777f - fConst12));
		fConst22 = (1.450071084655647f + ((fConst3 - 0.7431304601070396f) / fConst2));
		fConst23 = (2 * (1.450071084655647f - fConst19));
		for (int i=0; i<3; i++) fRec2[i] = 0;
		fConst24 = (2 * (1.450071084655647f - fConst9));
		fConst25 = (0.9351401670315425f + ((fConst3 - 0.157482159302087f) / fConst2));
		fConst26 = (2 * (0.9351401670315425f - fConst19));
		for (int i=0; i<3; i++) fRec1[i] = 0;
		fConst27 = (2 * (0.9351401670315425f - fConst6));
		for (int i=0; i<2; i++) fRec0[i] = 0;
		fConst28 = tanf((19790.793572264363f / float(iConst0)));
		fConst29 = (1.0f / fConst28);
		fConst30 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst29) / fConst28)));
		fConst31 = faustpower<2>(fConst28);
		fConst32 = (50.063807016150385f / fConst31);
		fConst33 = (0.9351401670315425f + fConst32);
		fConst34 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst29) / fConst28)));
		fConst35 = (11.052052171507189f / fConst31);
		fConst36 = (1.450071084655647f + fConst35);
		fConst37 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst29) / fConst28)));
		fConst38 = (0.0017661728399818856f / fConst31);
		fConst39 = (0.00040767818495825777f + fConst38);
		fConst40 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst3) / fConst2)));
		fConst41 = (53.53615295455673f + fConst19);
		fConst42 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst3) / fConst2)));
		fConst43 = (7.621731298870603f + fConst19);
		fConst44 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst3) / fConst2)));
		fConst45 = (9.9999997055e-05f / fConst5);
		fConst46 = (0.000433227200555f + fConst45);
		fConst47 = (0.24529150870616f + ((fConst3 - 0.782413046821645f) / fConst2));
		fConst48 = (2 * (0.24529150870616f - fConst19));
		for (int i=0; i<3; i++) fRec33[i] = 0;
		fConst49 = (2 * (0.000433227200555f - fConst45));
		fConst50 = (0.689621364484675f + ((fConst3 - 0.512478641889141f) / fConst2));
		fConst51 = (2 * (0.689621364484675f - fConst19));
		for (int i=0; i<3; i++) fRec32[i] = 0;
		fConst52 = (2 * (7.621731298870603f - fConst19));
		fConst53 = (1.069358407707312f + ((fConst3 - 0.168404871113589f) / fConst2));
		fConst54 = (2 * (1.069358407707312f - fConst19));
		for (int i=0; i<3; i++) fRec31[i] = 0;
		fConst55 = (2 * (53.53615295455673f - fConst19));
		fConst56 = (4.076781969643807f + ((fConst29 - 3.1897274020965583f) / fConst28));
		fConst57 = (1.0f / fConst31);
		fConst58 = (2 * (4.076781969643807f - fConst57));
		for (int i=0; i<3; i++) fRec30[i] = 0;
		fConst59 = (2 * (0.00040767818495825777f - fConst38));
		fConst60 = (1.450071084655647f + ((fConst29 - 0.7431304601070396f) / fConst28));
		fConst61 = (2 * (1.450071084655647f - fConst57));
		for (int i=0; i<3; i++) fRec29[i] = 0;
		fConst62 = (2 * (1.450071084655647f - fConst35));
		fConst63 = (0.9351401670315425f + ((fConst29 - 0.157482159302087f) / fConst28));
		fConst64 = (2 * (0.9351401670315425f - fConst57));
		for (int i=0; i<3; i++) fRec28[i] = 0;
		fConst65 = (2 * (0.9351401670315425f - fConst32));
		for (int i=0; i<2; i++) fRec27[i] = 0;
		fConst66 = tanf((12467.418707910012f / float(iConst0)));
		fConst67 = (1.0f / fConst66);
		fConst68 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst67) / fConst66)));
		fConst69 = faustpower<2>(fConst66);
		fConst70 = (50.063807016150385f / fConst69);
		fConst71 = (0.9351401670315425f + fConst70);
		fConst72 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst67) / fConst66)));
		fConst73 = (11.052052171507189f / fConst69);
		fConst74 = (1.450071084655647f + fConst73);
		fConst75 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst67) / fConst66)));
		fConst76 = (0.0017661728399818856f / fConst69);
		fConst77 = (0.00040767818495825777f + fConst76);
		fConst78 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst29) / fConst28)));
		fConst79 = (53.53615295455673f + fConst57);
		fConst80 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst29) / fConst28)));
		fConst81 = (7.621731298870603f + fConst57);
		fConst82 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst29) / fConst28)));
		fConst83 = (9.9999997055e-05f / fConst31);
		fConst84 = (0.000433227200555f + fConst83);
		fConst85 = (0.24529150870616f + ((fConst29 - 0.782413046821645f) / fConst28));
		fConst86 = (2 * (0.24529150870616f - fConst57));
		for (int i=0; i<3; i++) fRec40[i] = 0;
		fConst87 = (2 * (0.000433227200555f - fConst83));
		fConst88 = (0.689621364484675f + ((fConst29 - 0.512478641889141f) / fConst28));
		fConst89 = (2 * (0.689621364484675f - fConst57));
		for (int i=0; i<3; i++) fRec39[i] = 0;
		fConst90 = (2 * (7.621731298870603f - fConst57));
		fConst91 = (1.069358407707312f + ((fConst29 - 0.168404871113589f) / fConst28));
		fConst92 = (2 * (1.069358407707312f - fConst57));
		for (int i=0; i<3; i++) fRec38[i] = 0;
		fConst93 = (2 * (53.53615295455673f - fConst57));
		fConst94 = (4.076781969643807f + ((fConst67 - 3.1897274020965583f) / fConst66));
		fConst95 = (1.0f / fConst69);
		fConst96 = (2 * (4.076781969643807f - fConst95));
		for (int i=0; i<3; i++) fRec37[i] = 0;
		fConst97 = (2 * (0.00040767818495825777f - fConst76));
		fConst98 = (1.450071084655647f + ((fConst67 - 0.7431304601070396f) / fConst66));
		fConst99 = (2 * (1.450071084655647f - fConst95));
		for (int i=0; i<3; i++) fRec36[i] = 0;
		fConst100 = (2 * (1.450071084655647f - fConst73));
		fConst101 = (0.9351401670315425f + ((fConst67 - 0.157482159302087f) / fConst66));
		fConst102 = (2 * (0.9351401670315425f - fConst95));
		for (int i=0; i<3; i++) fRec35[i] = 0;
		fConst103 = (2 * (0.9351401670315425f - fConst70));
		for (int i=0; i<2; i++) fRec34[i] = 0;
		fConst104 = tanf((7853.981633974483f / float(iConst0)));
		fConst105 = (1.0f / fConst104);
		fConst106 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst105) / fConst104)));
		fConst107 = faustpower<2>(fConst104);
		fConst108 = (50.063807016150385f / fConst107);
		fConst109 = (0.9351401670315425f + fConst108);
		fConst110 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst105) / fConst104)));
		fConst111 = (11.052052171507189f / fConst107);
		fConst112 = (1.450071084655647f + fConst111);
		fConst113 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst105) / fConst104)));
		fConst114 = (0.0017661728399818856f / fConst107);
		fConst115 = (0.00040767818495825777f + fConst114);
		fConst116 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst67) / fConst66)));
		fConst117 = (53.53615295455673f + fConst95);
		fConst118 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst67) / fConst66)));
		fConst119 = (7.621731298870603f + fConst95);
		fConst120 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst67) / fConst66)));
		fConst121 = (9.9999997055e-05f / fConst69);
		fConst122 = (0.000433227200555f + fConst121);
		fConst123 = (0.24529150870616f + ((fConst67 - 0.782413046821645f) / fConst66));
		fConst124 = (2 * (0.24529150870616f - fConst95));
		for (int i=0; i<3; i++) fRec47[i] = 0;
		fConst125 = (2 * (0.000433227200555f - fConst121));
		fConst126 = (0.689621364484675f + ((fConst67 - 0.512478641889141f) / fConst66));
		fConst127 = (2 * (0.689621364484675f - fConst95));
		for (int i=0; i<3; i++) fRec46[i] = 0;
		fConst128 = (2 * (7.621731298870603f - fConst95));
		fConst129 = (1.069358407707312f + ((fConst67 - 0.168404871113589f) / fConst66));
		fConst130 = (2 * (1.069358407707312f - fConst95));
		for (int i=0; i<3; i++) fRec45[i] = 0;
		fConst131 = (2 * (53.53615295455673f - fConst95));
		fConst132 = (4.076781969643807f + ((fConst105 - 3.1897274020965583f) / fConst104));
		fConst133 = (1.0f / fConst107);
		fConst134 = (2 * (4.076781969643807f - fConst133));
		for (int i=0; i<3; i++) fRec44[i] = 0;
		fConst135 = (2 * (0.00040767818495825777f - fConst114));
		fConst136 = (1.450071084655647f + ((fConst105 - 0.7431304601070396f) / fConst104));
		fConst137 = (2 * (1.450071084655647f - fConst133));
		for (int i=0; i<3; i++) fRec43[i] = 0;
		fConst138 = (2 * (1.450071084655647f - fConst111));
		fConst139 = (0.9351401670315425f + ((fConst105 - 0.157482159302087f) / fConst104));
		fConst140 = (2 * (0.9351401670315425f - fConst133));
		for (int i=0; i<3; i++) fRec42[i] = 0;
		fConst141 = (2 * (0.9351401670315425f - fConst108));
		for (int i=0; i<2; i++) fRec41[i] = 0;
		fConst142 = tanf((4947.698393066091f / float(iConst0)));
		fConst143 = (1.0f / fConst142);
		fConst144 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst143) / fConst142)));
		fConst145 = faustpower<2>(fConst142);
		fConst146 = (50.063807016150385f / fConst145);
		fConst147 = (0.9351401670315425f + fConst146);
		fConst148 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst143) / fConst142)));
		fConst149 = (11.052052171507189f / fConst145);
		fConst150 = (1.450071084655647f + fConst149);
		fConst151 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst143) / fConst142)));
		fConst152 = (0.0017661728399818856f / fConst145);
		fConst153 = (0.00040767818495825777f + fConst152);
		fConst154 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst105) / fConst104)));
		fConst155 = (53.53615295455673f + fConst133);
		fConst156 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst105) / fConst104)));
		fConst157 = (7.621731298870603f + fConst133);
		fConst158 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst105) / fConst104)));
		fConst159 = (9.9999997055e-05f / fConst107);
		fConst160 = (0.000433227200555f + fConst159);
		fConst161 = (0.24529150870616f + ((fConst105 - 0.782413046821645f) / fConst104));
		fConst162 = (2 * (0.24529150870616f - fConst133));
		for (int i=0; i<3; i++) fRec54[i] = 0;
		fConst163 = (2 * (0.000433227200555f - fConst159));
		fConst164 = (0.689621364484675f + ((fConst105 - 0.512478641889141f) / fConst104));
		fConst165 = (2 * (0.689621364484675f - fConst133));
		for (int i=0; i<3; i++) fRec53[i] = 0;
		fConst166 = (2 * (7.621731298870603f - fConst133));
		fConst167 = (1.069358407707312f + ((fConst105 - 0.168404871113589f) / fConst104));
		fConst168 = (2 * (1.069358407707312f - fConst133));
		for (int i=0; i<3; i++) fRec52[i] = 0;
		fConst169 = (2 * (53.53615295455673f - fConst133));
		fConst170 = (4.076781969643807f + ((fConst143 - 3.1897274020965583f) / fConst142));
		fConst171 = (1.0f / fConst145);
		fConst172 = (2 * (4.076781969643807f - fConst171));
		for (int i=0; i<3; i++) fRec51[i] = 0;
		fConst173 = (2 * (0.00040767818495825777f - fConst152));
		fConst174 = (1.450071084655647f + ((fConst143 - 0.7431304601070396f) / fConst142));
		fConst175 = (2 * (1.450071084655647f - fConst171));
		for (int i=0; i<3; i++) fRec50[i] = 0;
		fConst176 = (2 * (1.450071084655647f - fConst149));
		fConst177 = (0.9351401670315425f + ((fConst143 - 0.157482159302087f) / fConst142));
		fConst178 = (2 * (0.9351401670315425f - fConst171));
		for (int i=0; i<3; i++) fRec49[i] = 0;
		fConst179 = (2 * (0.9351401670315425f - fConst146));
		for (int i=0; i<2; i++) fRec48[i] = 0;
		fConst180 = tanf((3116.8546769775025f / float(iConst0)));
		fConst181 = (1.0f / fConst180);
		fConst182 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst181) / fConst180)));
		fConst183 = faustpower<2>(fConst180);
		fConst184 = (50.063807016150385f / fConst183);
		fConst185 = (0.9351401670315425f + fConst184);
		fConst186 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst181) / fConst180)));
		fConst187 = (11.052052171507189f / fConst183);
		fConst188 = (1.450071084655647f + fConst187);
		fConst189 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst181) / fConst180)));
		fConst190 = (0.0017661728399818856f / fConst183);
		fConst191 = (0.00040767818495825777f + fConst190);
		fConst192 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst143) / fConst142)));
		fConst193 = (53.53615295455673f + fConst171);
		fConst194 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst143) / fConst142)));
		fConst195 = (7.621731298870603f + fConst171);
		fConst196 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst143) / fConst142)));
		fConst197 = (9.9999997055e-05f / fConst145);
		fConst198 = (0.000433227200555f + fConst197);
		fConst199 = (0.24529150870616f + ((fConst143 - 0.782413046821645f) / fConst142));
		fConst200 = (2 * (0.24529150870616f - fConst171));
		for (int i=0; i<3; i++) fRec61[i] = 0;
		fConst201 = (2 * (0.000433227200555f - fConst197));
		fConst202 = (0.689621364484675f + ((fConst143 - 0.512478641889141f) / fConst142));
		fConst203 = (2 * (0.689621364484675f - fConst171));
		for (int i=0; i<3; i++) fRec60[i] = 0;
		fConst204 = (2 * (7.621731298870603f - fConst171));
		fConst205 = (1.069358407707312f + ((fConst143 - 0.168404871113589f) / fConst142));
		fConst206 = (2 * (1.069358407707312f - fConst171));
		for (int i=0; i<3; i++) fRec59[i] = 0;
		fConst207 = (2 * (53.53615295455673f - fConst171));
		fConst208 = (4.076781969643807f + ((fConst181 - 3.1897274020965583f) / fConst180));
		fConst209 = (1.0f / fConst183);
		fConst210 = (2 * (4.076781969643807f - fConst209));
		for (int i=0; i<3; i++) fRec58[i] = 0;
		fConst211 = (2 * (0.00040767818495825777f - fConst190));
		fConst212 = (1.450071084655647f + ((fConst181 - 0.7431304601070396f) / fConst180));
		fConst213 = (2 * (1.450071084655647f - fConst209));
		for (int i=0; i<3; i++) fRec57[i] = 0;
		fConst214 = (2 * (1.450071084655647f - fConst187));
		fConst215 = (0.9351401670315425f + ((fConst181 - 0.157482159302087f) / fConst180));
		fConst216 = (2 * (0.9351401670315425f - fConst209));
		for (int i=0; i<3; i++) fRec56[i] = 0;
		fConst217 = (2 * (0.9351401670315425f - fConst184));
		for (int i=0; i<2; i++) fRec55[i] = 0;
		fConst218 = tanf((1963.4954084936207f / float(iConst0)));
		fConst219 = (1.0f / fConst218);
		fConst220 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst219) / fConst218)));
		fConst221 = faustpower<2>(fConst218);
		fConst222 = (50.063807016150385f / fConst221);
		fConst223 = (0.9351401670315425f + fConst222);
		fConst224 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst219) / fConst218)));
		fConst225 = (11.052052171507189f / fConst221);
		fConst226 = (1.450071084655647f + fConst225);
		fConst227 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst219) / fConst218)));
		fConst228 = (0.0017661728399818856f / fConst221);
		fConst229 = (0.00040767818495825777f + fConst228);
		fConst230 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst181) / fConst180)));
		fConst231 = (53.53615295455673f + fConst209);
		fConst232 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst181) / fConst180)));
		fConst233 = (7.621731298870603f + fConst209);
		fConst234 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst181) / fConst180)));
		fConst235 = (9.9999997055e-05f / fConst183);
		fConst236 = (0.000433227200555f + fConst235);
		fConst237 = (0.24529150870616f + ((fConst181 - 0.782413046821645f) / fConst180));
		fConst238 = (2 * (0.24529150870616f - fConst209));
		for (int i=0; i<3; i++) fRec68[i] = 0;
		fConst239 = (2 * (0.000433227200555f - fConst235));
		fConst240 = (0.689621364484675f + ((fConst181 - 0.512478641889141f) / fConst180));
		fConst241 = (2 * (0.689621364484675f - fConst209));
		for (int i=0; i<3; i++) fRec67[i] = 0;
		fConst242 = (2 * (7.621731298870603f - fConst209));
		fConst243 = (1.069358407707312f + ((fConst181 - 0.168404871113589f) / fConst180));
		fConst244 = (2 * (1.069358407707312f - fConst209));
		for (int i=0; i<3; i++) fRec66[i] = 0;
		fConst245 = (2 * (53.53615295455673f - fConst209));
		fConst246 = (4.076781969643807f + ((fConst219 - 3.1897274020965583f) / fConst218));
		fConst247 = (1.0f / fConst221);
		fConst248 = (2 * (4.076781969643807f - fConst247));
		for (int i=0; i<3; i++) fRec65[i] = 0;
		fConst249 = (2 * (0.00040767818495825777f - fConst228));
		fConst250 = (1.450071084655647f + ((fConst219 - 0.7431304601070396f) / fConst218));
		fConst251 = (2 * (1.450071084655647f - fConst247));
		for (int i=0; i<3; i++) fRec64[i] = 0;
		fConst252 = (2 * (1.450071084655647f - fConst225));
		fConst253 = (0.9351401670315425f + ((fConst219 - 0.157482159302087f) / fConst218));
		fConst254 = (2 * (0.9351401670315425f - fConst247));
		for (int i=0; i<3; i++) fRec63[i] = 0;
		fConst255 = (2 * (0.9351401670315425f - fConst222));
		for (int i=0; i<2; i++) fRec62[i] = 0;
		fConst256 = tanf((1236.9245982665225f / float(iConst0)));
		fConst257 = (1.0f / fConst256);
		fConst258 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst257) / fConst256)));
		fConst259 = faustpower<2>(fConst256);
		fConst260 = (50.063807016150385f / fConst259);
		fConst261 = (0.9351401670315425f + fConst260);
		fConst262 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst257) / fConst256)));
		fConst263 = (11.052052171507189f / fConst259);
		fConst264 = (1.450071084655647f + fConst263);
		fConst265 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst257) / fConst256)));
		fConst266 = (0.0017661728399818856f / fConst259);
		fConst267 = (0.00040767818495825777f + fConst266);
		fConst268 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst219) / fConst218)));
		fConst269 = (53.53615295455673f + fConst247);
		fConst270 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst219) / fConst218)));
		fConst271 = (7.621731298870603f + fConst247);
		fConst272 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst219) / fConst218)));
		fConst273 = (9.9999997055e-05f / fConst221);
		fConst274 = (0.000433227200555f + fConst273);
		fConst275 = (0.24529150870616f + ((fConst219 - 0.782413046821645f) / fConst218));
		fConst276 = (2 * (0.24529150870616f - fConst247));
		for (int i=0; i<3; i++) fRec75[i] = 0;
		fConst277 = (2 * (0.000433227200555f - fConst273));
		fConst278 = (0.689621364484675f + ((fConst219 - 0.512478641889141f) / fConst218));
		fConst279 = (2 * (0.689621364484675f - fConst247));
		for (int i=0; i<3; i++) fRec74[i] = 0;
		fConst280 = (2 * (7.621731298870603f - fConst247));
		fConst281 = (1.069358407707312f + ((fConst219 - 0.168404871113589f) / fConst218));
		fConst282 = (2 * (1.069358407707312f - fConst247));
		for (int i=0; i<3; i++) fRec73[i] = 0;
		fConst283 = (2 * (53.53615295455673f - fConst247));
		fConst284 = (4.076781969643807f + ((fConst257 - 3.1897274020965583f) / fConst256));
		fConst285 = (1.0f / fConst259);
		fConst286 = (2 * (4.076781969643807f - fConst285));
		for (int i=0; i<3; i++) fRec72[i] = 0;
		fConst287 = (2 * (0.00040767818495825777f - fConst266));
		fConst288 = (1.450071084655647f + ((fConst257 - 0.7431304601070396f) / fConst256));
		fConst289 = (2 * (1.450071084655647f - fConst285));
		for (int i=0; i<3; i++) fRec71[i] = 0;
		fConst290 = (2 * (1.450071084655647f - fConst263));
		fConst291 = (0.9351401670315425f + ((fConst257 - 0.157482159302087f) / fConst256));
		fConst292 = (2 * (0.9351401670315425f - fConst285));
		for (int i=0; i<3; i++) fRec70[i] = 0;
		fConst293 = (2 * (0.9351401670315425f - fConst260));
		for (int i=0; i<2; i++) fRec69[i] = 0;
		fConst294 = tanf((779.213669244376f / float(iConst0)));
		fConst295 = (1.0f / fConst294);
		fConst296 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst295) / fConst294)));
		fConst297 = faustpower<2>(fConst294);
		fConst298 = (50.063807016150385f / fConst297);
		fConst299 = (0.9351401670315425f + fConst298);
		fConst300 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst295) / fConst294)));
		fConst301 = (11.052052171507189f / fConst297);
		fConst302 = (1.450071084655647f + fConst301);
		fConst303 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst295) / fConst294)));
		fConst304 = (0.0017661728399818856f / fConst297);
		fConst305 = (0.00040767818495825777f + fConst304);
		fConst306 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst257) / fConst256)));
		fConst307 = (53.53615295455673f + fConst285);
		fConst308 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst257) / fConst256)));
		fConst309 = (7.621731298870603f + fConst285);
		fConst310 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst257) / fConst256)));
		fConst311 = (9.9999997055e-05f / fConst259);
		fConst312 = (0.000433227200555f + fConst311);
		fConst313 = (0.24529150870616f + ((fConst257 - 0.782413046821645f) / fConst256));
		fConst314 = (2 * (0.24529150870616f - fConst285));
		for (int i=0; i<3; i++) fRec82[i] = 0;
		fConst315 = (2 * (0.000433227200555f - fConst311));
		fConst316 = (0.689621364484675f + ((fConst257 - 0.512478641889141f) / fConst256));
		fConst317 = (2 * (0.689621364484675f - fConst285));
		for (int i=0; i<3; i++) fRec81[i] = 0;
		fConst318 = (2 * (7.621731298870603f - fConst285));
		fConst319 = (1.069358407707312f + ((fConst257 - 0.168404871113589f) / fConst256));
		fConst320 = (2 * (1.069358407707312f - fConst285));
		for (int i=0; i<3; i++) fRec80[i] = 0;
		fConst321 = (2 * (53.53615295455673f - fConst285));
		fConst322 = (4.076781969643807f + ((fConst295 - 3.1897274020965583f) / fConst294));
		fConst323 = (1.0f / fConst297);
		fConst324 = (2 * (4.076781969643807f - fConst323));
		for (int i=0; i<3; i++) fRec79[i] = 0;
		fConst325 = (2 * (0.00040767818495825777f - fConst304));
		fConst326 = (1.450071084655647f + ((fConst295 - 0.7431304601070396f) / fConst294));
		fConst327 = (2 * (1.450071084655647f - fConst323));
		for (int i=0; i<3; i++) fRec78[i] = 0;
		fConst328 = (2 * (1.450071084655647f - fConst301));
		fConst329 = (0.9351401670315425f + ((fConst295 - 0.157482159302087f) / fConst294));
		fConst330 = (2 * (0.9351401670315425f - fConst323));
		for (int i=0; i<3; i++) fRec77[i] = 0;
		fConst331 = (2 * (0.9351401670315425f - fConst298));
		for (int i=0; i<2; i++) fRec76[i] = 0;
		fConst332 = tanf((490.8738521234052f / float(iConst0)));
		fConst333 = (1.0f / fConst332);
		fConst334 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst333) / fConst332)));
		fConst335 = faustpower<2>(fConst332);
		fConst336 = (50.063807016150385f / fConst335);
		fConst337 = (0.9351401670315425f + fConst336);
		fConst338 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst333) / fConst332)));
		fConst339 = (11.052052171507189f / fConst335);
		fConst340 = (1.450071084655647f + fConst339);
		fConst341 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst333) / fConst332)));
		fConst342 = (0.0017661728399818856f / fConst335);
		fConst343 = (0.00040767818495825777f + fConst342);
		fConst344 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst295) / fConst294)));
		fConst345 = (53.53615295455673f + fConst323);
		fConst346 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst295) / fConst294)));
		fConst347 = (7.621731298870603f + fConst323);
		fConst348 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst295) / fConst294)));
		fConst349 = (9.9999997055e-05f / fConst297);
		fConst350 = (0.000433227200555f + fConst349);
		fConst351 = (0.24529150870616f + ((fConst295 - 0.782413046821645f) / fConst294));
		fConst352 = (2 * (0.24529150870616f - fConst323));
		for (int i=0; i<3; i++) fRec89[i] = 0;
		fConst353 = (2 * (0.000433227200555f - fConst349));
		fConst354 = (0.689621364484675f + ((fConst295 - 0.512478641889141f) / fConst294));
		fConst355 = (2 * (0.689621364484675f - fConst323));
		for (int i=0; i<3; i++) fRec88[i] = 0;
		fConst356 = (2 * (7.621731298870603f - fConst323));
		fConst357 = (1.069358407707312f + ((fConst295 - 0.168404871113589f) / fConst294));
		fConst358 = (2 * (1.069358407707312f - fConst323));
		for (int i=0; i<3; i++) fRec87[i] = 0;
		fConst359 = (2 * (53.53615295455673f - fConst323));
		fConst360 = (4.076781969643807f + ((fConst333 - 3.1897274020965583f) / fConst332));
		fConst361 = (1.0f / fConst335);
		fConst362 = (2 * (4.076781969643807f - fConst361));
		for (int i=0; i<3; i++) fRec86[i] = 0;
		fConst363 = (2 * (0.00040767818495825777f - fConst342));
		fConst364 = (1.450071084655647f + ((fConst333 - 0.7431304601070396f) / fConst332));
		fConst365 = (2 * (1.450071084655647f - fConst361));
		for (int i=0; i<3; i++) fRec85[i] = 0;
		fConst366 = (2 * (1.450071084655647f - fConst339));
		fConst367 = (0.9351401670315425f + ((fConst333 - 0.157482159302087f) / fConst332));
		fConst368 = (2 * (0.9351401670315425f - fConst361));
		for (int i=0; i<3; i++) fRec84[i] = 0;
		fConst369 = (2 * (0.9351401670315425f - fConst336));
		for (int i=0; i<2; i++) fRec83[i] = 0;
		fConst370 = tanf((309.2311495666306f / float(iConst0)));
		fConst371 = (1.0f / fConst370);
		fConst372 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst371) / fConst370)));
		fConst373 = faustpower<2>(fConst370);
		fConst374 = (50.063807016150385f / fConst373);
		fConst375 = (0.9351401670315425f + fConst374);
		fConst376 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst371) / fConst370)));
		fConst377 = (11.052052171507189f / fConst373);
		fConst378 = (1.450071084655647f + fConst377);
		fConst379 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst371) / fConst370)));
		fConst380 = (0.0017661728399818856f / fConst373);
		fConst381 = (0.00040767818495825777f + fConst380);
		fConst382 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst333) / fConst332)));
		fConst383 = (53.53615295455673f + fConst361);
		fConst384 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst333) / fConst332)));
		fConst385 = (7.621731298870603f + fConst361);
		fConst386 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst333) / fConst332)));
		fConst387 = (9.9999997055e-05f / fConst335);
		fConst388 = (0.000433227200555f + fConst387);
		fConst389 = (0.24529150870616f + ((fConst333 - 0.782413046821645f) / fConst332));
		fConst390 = (2 * (0.24529150870616f - fConst361));
		for (int i=0; i<3; i++) fRec96[i] = 0;
		fConst391 = (2 * (0.000433227200555f - fConst387));
		fConst392 = (0.689621364484675f + ((fConst333 - 0.512478641889141f) / fConst332));
		fConst393 = (2 * (0.689621364484675f - fConst361));
		for (int i=0; i<3; i++) fRec95[i] = 0;
		fConst394 = (2 * (7.621731298870603f - fConst361));
		fConst395 = (1.069358407707312f + ((fConst333 - 0.168404871113589f) / fConst332));
		fConst396 = (2 * (1.069358407707312f - fConst361));
		for (int i=0; i<3; i++) fRec94[i] = 0;
		fConst397 = (2 * (53.53615295455673f - fConst361));
		fConst398 = (4.076781969643807f + ((fConst371 - 3.1897274020965583f) / fConst370));
		fConst399 = (1.0f / fConst373);
		fConst400 = (2 * (4.076781969643807f - fConst399));
		for (int i=0; i<3; i++) fRec93[i] = 0;
		fConst401 = (2 * (0.00040767818495825777f - fConst380));
		fConst402 = (1.450071084655647f + ((fConst371 - 0.7431304601070396f) / fConst370));
		fConst403 = (2 * (1.450071084655647f - fConst399));
		for (int i=0; i<3; i++) fRec92[i] = 0;
		fConst404 = (2 * (1.450071084655647f - fConst377));
		fConst405 = (0.9351401670315425f + ((fConst371 - 0.157482159302087f) / fConst370));
		fConst406 = (2 * (0.9351401670315425f - fConst399));
		for (int i=0; i<3; i++) fRec91[i] = 0;
		fConst407 = (2 * (0.9351401670315425f - fConst374));
		for (int i=0; i<2; i++) fRec90[i] = 0;
		fConst408 = tanf((194.803417311094f / float(iConst0)));
		fConst409 = (1.0f / fConst408);
		fConst410 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst409) / fConst408)));
		fConst411 = faustpower<2>(fConst408);
		fConst412 = (50.063807016150385f / fConst411);
		fConst413 = (0.9351401670315425f + fConst412);
		fConst414 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst409) / fConst408)));
		fConst415 = (11.052052171507189f / fConst411);
		fConst416 = (1.450071084655647f + fConst415);
		fConst417 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst409) / fConst408)));
		fConst418 = (0.0017661728399818856f / fConst411);
		fConst419 = (0.00040767818495825777f + fConst418);
		fConst420 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst371) / fConst370)));
		fConst421 = (53.53615295455673f + fConst399);
		fConst422 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst371) / fConst370)));
		fConst423 = (7.621731298870603f + fConst399);
		fConst424 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst371) / fConst370)));
		fConst425 = (9.9999997055e-05f / fConst373);
		fConst426 = (0.000433227200555f + fConst425);
		fConst427 = (0.24529150870616f + ((fConst371 - 0.782413046821645f) / fConst370));
		fConst428 = (2 * (0.24529150870616f - fConst399));
		for (int i=0; i<3; i++) fRec103[i] = 0;
		fConst429 = (2 * (0.000433227200555f - fConst425));
		fConst430 = (0.689621364484675f + ((fConst371 - 0.512478641889141f) / fConst370));
		fConst431 = (2 * (0.689621364484675f - fConst399));
		for (int i=0; i<3; i++) fRec102[i] = 0;
		fConst432 = (2 * (7.621731298870603f - fConst399));
		fConst433 = (1.069358407707312f + ((fConst371 - 0.168404871113589f) / fConst370));
		fConst434 = (2 * (1.069358407707312f - fConst399));
		for (int i=0; i<3; i++) fRec101[i] = 0;
		fConst435 = (2 * (53.53615295455673f - fConst399));
		fConst436 = (4.076781969643807f + ((fConst409 - 3.1897274020965583f) / fConst408));
		fConst437 = (1.0f / fConst411);
		fConst438 = (2 * (4.076781969643807f - fConst437));
		for (int i=0; i<3; i++) fRec100[i] = 0;
		fConst439 = (2 * (0.00040767818495825777f - fConst418));
		fConst440 = (1.450071084655647f + ((fConst409 - 0.7431304601070396f) / fConst408));
		fConst441 = (2 * (1.450071084655647f - fConst437));
		for (int i=0; i<3; i++) fRec99[i] = 0;
		fConst442 = (2 * (1.450071084655647f - fConst415));
		fConst443 = (0.9351401670315425f + ((fConst409 - 0.157482159302087f) / fConst408));
		fConst444 = (2 * (0.9351401670315425f - fConst437));
		for (int i=0; i<3; i++) fRec98[i] = 0;
		fConst445 = (2 * (0.9351401670315425f - fConst412));
		for (int i=0; i<2; i++) fRec97[i] = 0;
		fConst446 = tanf((122.7184630308513f / float(iConst0)));
		fConst447 = (1.0f / fConst446);
		fConst448 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst447) / fConst446)));
		fConst449 = faustpower<2>(fConst446);
		fConst450 = (50.063807016150385f / fConst449);
		fConst451 = (0.9351401670315425f + fConst450);
		fConst452 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst447) / fConst446)));
		fConst453 = (11.052052171507189f / fConst449);
		fConst454 = (1.450071084655647f + fConst453);
		fConst455 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst447) / fConst446)));
		fConst456 = (0.0017661728399818856f / fConst449);
		fConst457 = (0.00040767818495825777f + fConst456);
		fConst458 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst409) / fConst408)));
		fConst459 = (53.53615295455673f + fConst437);
		fConst460 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst409) / fConst408)));
		fConst461 = (7.621731298870603f + fConst437);
		fConst462 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst409) / fConst408)));
		fConst463 = (9.9999997055e-05f / fConst411);
		fConst464 = (0.000433227200555f + fConst463);
		fConst465 = (0.24529150870616f + ((fConst409 - 0.782413046821645f) / fConst408));
		fConst466 = (2 * (0.24529150870616f - fConst437));
		for (int i=0; i<3; i++) fRec110[i] = 0;
		fConst467 = (2 * (0.000433227200555f - fConst463));
		fConst468 = (0.689621364484675f + ((fConst409 - 0.512478641889141f) / fConst408));
		fConst469 = (2 * (0.689621364484675f - fConst437));
		for (int i=0; i<3; i++) fRec109[i] = 0;
		fConst470 = (2 * (7.621731298870603f - fConst437));
		fConst471 = (1.069358407707312f + ((fConst409 - 0.168404871113589f) / fConst408));
		fConst472 = (2 * (1.069358407707312f - fConst437));
		for (int i=0; i<3; i++) fRec108[i] = 0;
		fConst473 = (2 * (53.53615295455673f - fConst437));
		fConst474 = (4.076781969643807f + ((fConst447 - 3.1897274020965583f) / fConst446));
		fConst475 = (1.0f / fConst449);
		fConst476 = (2 * (4.076781969643807f - fConst475));
		for (int i=0; i<3; i++) fRec107[i] = 0;
		fConst477 = (2 * (0.00040767818495825777f - fConst456));
		fConst478 = (1.450071084655647f + ((fConst447 - 0.7431304601070396f) / fConst446));
		fConst479 = (2 * (1.450071084655647f - fConst475));
		for (int i=0; i<3; i++) fRec106[i] = 0;
		fConst480 = (2 * (1.450071084655647f - fConst453));
		fConst481 = (0.9351401670315425f + ((fConst447 - 0.157482159302087f) / fConst446));
		fConst482 = (2 * (0.9351401670315425f - fConst475));
		for (int i=0; i<3; i++) fRec105[i] = 0;
		fConst483 = (2 * (0.9351401670315425f - fConst450));
		for (int i=0; i<2; i++) fRec104[i] = 0;
		fConst484 = tanf((77.3077873916577f / float(iConst0)));
		fConst485 = (1.0f / fConst484);
		fConst486 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst485) / fConst484)));
		fConst487 = faustpower<2>(fConst484);
		fConst488 = (50.063807016150385f / fConst487);
		fConst489 = (0.9351401670315425f + fConst488);
		fConst490 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst485) / fConst484)));
		fConst491 = (11.052052171507189f / fConst487);
		fConst492 = (1.450071084655647f + fConst491);
		fConst493 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst485) / fConst484)));
		fConst494 = (0.0017661728399818856f / fConst487);
		fConst495 = (0.00040767818495825777f + fConst494);
		fConst496 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst447) / fConst446)));
		fConst497 = (53.53615295455673f + fConst475);
		fConst498 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst447) / fConst446)));
		fConst499 = (7.621731298870603f + fConst475);
		fConst500 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst447) / fConst446)));
		fConst501 = (9.9999997055e-05f / fConst449);
		fConst502 = (0.000433227200555f + fConst501);
		fConst503 = (0.24529150870616f + ((fConst447 - 0.782413046821645f) / fConst446));
		fConst504 = (2 * (0.24529150870616f - fConst475));
		for (int i=0; i<3; i++) fRec117[i] = 0;
		fConst505 = (2 * (0.000433227200555f - fConst501));
		fConst506 = (0.689621364484675f + ((fConst447 - 0.512478641889141f) / fConst446));
		fConst507 = (2 * (0.689621364484675f - fConst475));
		for (int i=0; i<3; i++) fRec116[i] = 0;
		fConst508 = (2 * (7.621731298870603f - fConst475));
		fConst509 = (1.069358407707312f + ((fConst447 - 0.168404871113589f) / fConst446));
		fConst510 = (2 * (1.069358407707312f - fConst475));
		for (int i=0; i<3; i++) fRec115[i] = 0;
		fConst511 = (2 * (53.53615295455673f - fConst475));
		fConst512 = (4.076781969643807f + ((fConst485 - 3.1897274020965583f) / fConst484));
		fConst513 = (1.0f / fConst487);
		fConst514 = (2 * (4.076781969643807f - fConst513));
		for (int i=0; i<3; i++) fRec114[i] = 0;
		fConst515 = (2 * (0.00040767818495825777f - fConst494));
		fConst516 = (1.450071084655647f + ((fConst485 - 0.7431304601070396f) / fConst484));
		fConst517 = (2 * (1.450071084655647f - fConst513));
		for (int i=0; i<3; i++) fRec113[i] = 0;
		fConst518 = (2 * (1.450071084655647f - fConst491));
		fConst519 = (0.9351401670315425f + ((fConst485 - 0.157482159302087f) / fConst484));
		fConst520 = (2 * (0.9351401670315425f - fConst513));
		for (int i=0; i<3; i++) fRec112[i] = 0;
		fConst521 = (2 * (0.9351401670315425f - fConst488));
		for (int i=0; i<2; i++) fRec111[i] = 0;
		fConst522 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst485) / fConst484)));
		fConst523 = (53.53615295455673f + fConst513);
		fConst524 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst485) / fConst484)));
		fConst525 = (7.621731298870603f + fConst513);
		fConst526 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst485) / fConst484)));
		fConst527 = (9.9999997055e-05f / fConst487);
		fConst528 = (0.000433227200555f + fConst527);
		fConst529 = (0.24529150870616f + ((fConst485 - 0.782413046821645f) / fConst484));
		fConst530 = (2 * (0.24529150870616f - fConst513));
		for (int i=0; i<3; i++) fRec121[i] = 0;
		fConst531 = (2 * (0.000433227200555f - fConst527));
		fConst532 = (0.689621364484675f + ((fConst485 - 0.512478641889141f) / fConst484));
		fConst533 = (2 * (0.689621364484675f - fConst513));
		for (int i=0; i<3; i++) fRec120[i] = 0;
		fConst534 = (2 * (7.621731298870603f - fConst513));
		fConst535 = (1.069358407707312f + ((fConst485 - 0.168404871113589f) / fConst484));
		fConst536 = (2 * (1.069358407707312f - fConst513));
		for (int i=0; i<3; i++) fRec119[i] = 0;
		fConst537 = (2 * (53.53615295455673f - fConst513));
		for (int i=0; i<2; i++) fRec118[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("phaser_flanger");
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
		interface->declare(&fcheckbox3, "0", "");
		interface->declare(&fcheckbox3, "tooltip", "Pink Noise (or 1/f noise) is Constant-Q Noise, meaning that it has the same total power in every octave");
		interface->addCheckButton("Pink Noise Instead (uses only Amplitude control on the left)", &fcheckbox3);
		interface->declare(&fcheckbox2, "1", "");
		interface->addCheckButton("External Input Instead (overrides Sawtooth/Noise selection above)", &fcheckbox2);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "2", "");
		interface->openVerticalBox("");
		interface->declare(0, "tooltip", "Reference: https://ccrma.stanford.edu/~jos/pasp/Flanging.html");
		interface->openVerticalBox("FLANGER");
		interface->declare(0, "0", "");
		interface->openHorizontalBox("");
		interface->declare(&fcheckbox1, "0", "");
		interface->declare(&fcheckbox1, "tooltip", "When this is checked, the flanger has no effect");
		interface->addCheckButton("Bypass", &fcheckbox1);
		interface->declare(&fcheckbox4, "1", "");
		interface->addCheckButton("Invert Flange Sum", &fcheckbox4);
		interface->declare(&fbargraph0, "2", "");
		interface->declare(&fbargraph0, "style", "led");
		interface->declare(&fbargraph0, "tooltip", "Display sum of flange delays");
		interface->addHorizontalBargraph("Flange LFO", &fbargraph0, -1.5f, 1.5f);
		interface->closeBox();
		interface->declare(0, "1", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider8, "1", "");
		interface->declare(&fslider8, "style", "knob");
		interface->declare(&fslider8, "unit", "Hz");
		interface->addHorizontalSlider("Speed", &fslider8, 0.5f, 0.0f, 1e+01f, 0.01f);
		interface->declare(&fslider9, "2", "");
		interface->declare(&fslider9, "style", "knob");
		interface->addHorizontalSlider("Depth", &fslider9, 1.0f, 0.0f, 1.0f, 0.001f);
		interface->declare(&fslider10, "3", "");
		interface->declare(&fslider10, "style", "knob");
		interface->addHorizontalSlider("Feedback", &fslider10, 0.0f, -0.999f, 0.999f, 0.001f);
		interface->closeBox();
		interface->declare(0, "2", "");
		interface->openHorizontalBox("Delay Controls");
		interface->declare(&fslider12, "1", "");
		interface->declare(&fslider12, "style", "knob");
		interface->declare(&fslider12, "unit", "ms");
		interface->addHorizontalSlider("Flange Delay", &fslider12, 1e+01f, 0.0f, 2e+01f, 0.001f);
		interface->declare(&fslider11, "2", "");
		interface->declare(&fslider11, "style", "knob");
		interface->declare(&fslider11, "unit", "ms");
		interface->addHorizontalSlider("Delay Offset", &fslider11, 1.0f, 0.0f, 2e+01f, 0.001f);
		interface->closeBox();
		interface->declare(0, "3", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider7, "unit", "dB");
		interface->addHorizontalSlider("Flanger Output Level", &fslider7, 0.0f, -6e+01f, 1e+01f, 0.1f);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "3", "");
		interface->openVerticalBox("");
		interface->declare(0, "tooltip", "Reference: https://ccrma.stanford.edu/~jos/pasp/Flanging.html");
		interface->openVerticalBox("PHASER2");
		interface->declare(0, "0", "");
		interface->openHorizontalBox("");
		interface->declare(&fcheckbox0, "0", "");
		interface->declare(&fcheckbox0, "tooltip", "When this is checked, the phaser has no effect");
		interface->addCheckButton("Bypass", &fcheckbox0);
		interface->declare(&fcheckbox6, "1", "");
		interface->addCheckButton("Invert Internal Phaser Sum", &fcheckbox6);
		interface->declare(&fcheckbox5, "2", "");
		interface->addCheckButton("Vibrato Mode", &fcheckbox5);
		interface->closeBox();
		interface->declare(0, "1", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider19, "1", "");
		interface->declare(&fslider19, "style", "knob");
		interface->declare(&fslider19, "unit", "Hz");
		interface->addHorizontalSlider("Speed", &fslider19, 0.5f, 0.0f, 1e+01f, 0.001f);
		interface->declare(&fslider14, "2", "");
		interface->declare(&fslider14, "style", "knob");
		interface->addHorizontalSlider("Notch Depth (Intensity)", &fslider14, 1.0f, 0.0f, 1.0f, 0.001f);
		interface->declare(&fslider20, "3", "");
		interface->declare(&fslider20, "style", "knob");
		interface->addHorizontalSlider("Feedback Gain", &fslider20, 0.0f, -0.999f, 0.999f, 0.001f);
		interface->closeBox();
		interface->declare(0, "2", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider15, "1", "");
		interface->declare(&fslider15, "style", "knob");
		interface->declare(&fslider15, "unit", "Hz");
		interface->addHorizontalSlider("Notch width", &fslider15, 1e+03f, 1e+01f, 5e+03f, 1.0f);
		interface->declare(&fslider17, "2", "");
		interface->declare(&fslider17, "style", "knob");
		interface->declare(&fslider17, "unit", "Hz");
		interface->addHorizontalSlider("Min Notch1 Freq", &fslider17, 1e+02f, 2e+01f, 5e+03f, 1.0f);
		interface->declare(&fslider18, "3", "");
		interface->declare(&fslider18, "style", "knob");
		interface->declare(&fslider18, "unit", "Hz");
		interface->addHorizontalSlider("Max Notch1 Freq", &fslider18, 8e+02f, 2e+01f, 1e+04f, 1.0f);
		interface->declare(&fslider16, "4", "");
		interface->declare(&fslider16, "style", "knob");
		interface->addHorizontalSlider("Notch Freq Ratio: NotchFreq(n+1)/NotchFreq(n)", &fslider16, 1.5f, 1.1f, 4.0f, 0.001f);
		interface->closeBox();
		interface->declare(0, "3", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider13, "unit", "dB");
		interface->addHorizontalSlider("Phaser Output Level", &fslider13, 0.0f, -6e+01f, 1e+01f, 0.1f);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "4", "");
		interface->openVerticalBox("");
		interface->declare(0, "0", "");
		interface->declare(0, "tooltip", "See Faust's filter.lib for documentation and references");
		interface->openHorizontalBox("CONSTANT-Q SPECTRUM ANALYZER (6E)");
		interface->declare(&fbargraph15, "0", "");
		interface->declare(&fbargraph15, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph15, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph15, -5e+01f, 1e+01f);
		interface->declare(&fbargraph14, "1", "");
		interface->declare(&fbargraph14, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph14, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph14, -5e+01f, 1e+01f);
		interface->declare(&fbargraph13, "2", "");
		interface->declare(&fbargraph13, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph13, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph13, -5e+01f, 1e+01f);
		interface->declare(&fbargraph12, "3", "");
		interface->declare(&fbargraph12, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph12, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph12, -5e+01f, 1e+01f);
		interface->declare(&fbargraph11, "4", "");
		interface->declare(&fbargraph11, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph11, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph11, -5e+01f, 1e+01f);
		interface->declare(&fbargraph10, "5", "");
		interface->declare(&fbargraph10, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph10, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph10, -5e+01f, 1e+01f);
		interface->declare(&fbargraph9, "6", "");
		interface->declare(&fbargraph9, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph9, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph9, -5e+01f, 1e+01f);
		interface->declare(&fbargraph8, "7", "");
		interface->declare(&fbargraph8, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph8, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph8, -5e+01f, 1e+01f);
		interface->declare(&fbargraph7, "8", "");
		interface->declare(&fbargraph7, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph7, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph7, -5e+01f, 1e+01f);
		interface->declare(&fbargraph6, "9", "");
		interface->declare(&fbargraph6, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph6, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph6, -5e+01f, 1e+01f);
		interface->declare(&fbargraph5, "10", "");
		interface->declare(&fbargraph5, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph5, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph5, -5e+01f, 1e+01f);
		interface->declare(&fbargraph4, "11", "");
		interface->declare(&fbargraph4, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph4, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph4, -5e+01f, 1e+01f);
		interface->declare(&fbargraph3, "12", "");
		interface->declare(&fbargraph3, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph3, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph3, -5e+01f, 1e+01f);
		interface->declare(&fbargraph2, "13", "");
		interface->declare(&fbargraph2, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph2, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph2, -5e+01f, 1e+01f);
		interface->declare(&fbargraph1, "14", "");
		interface->declare(&fbargraph1, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph1, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph1, -5e+01f, 1e+01f);
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
		int 	iSlow4 = int(fcheckbox1);
		float 	fSlow5 = (0.0010000000000000009f * powf(10,(0.05f * fslider2)));
		int 	iSlow6 = int(fcheckbox2);
		int 	iSlow7 = int(fcheckbox3);
		float 	fSlow8 = expf((0 - (fConst1 / fslider3)));
		float 	fSlow9 = (4.4e+02f * (powf(2.0f,(0.08333333333333333f * (fslider4 - 49.0f))) * (1.0f - fSlow8)));
		float 	fSlow10 = (1 + (0.01f * fslider5));
		float 	fSlow11 = (1.0f / fSlow10);
		float 	fSlow12 = (fConst16 * fSlow10);
		float 	fSlow13 = (fConst15 / fSlow10);
		float 	fSlow14 = (1 + (0.01f * fslider6));
		float 	fSlow15 = (1.0f / fSlow14);
		float 	fSlow16 = (fConst16 * fSlow14);
		float 	fSlow17 = (fConst15 / fSlow14);
		float 	fSlow18 = powf(10,(0.05f * fslider7));
		float 	fSlow19 = (fConst17 * fslider8);
		float 	fSlow20 = sinf(fSlow19);
		float 	fSlow21 = cosf(fSlow19);
		float 	fSlow22 = (0 - fSlow20);
		float 	fSlow23 = fslider9;
		float 	fSlow24 = ((int(fcheckbox4))?(0 - fSlow23):fSlow23);
		float 	fSlow25 = fslider10;
		float 	fSlow26 = (0.001f * fslider11);
		float 	fSlow27 = (0.0005f * fslider12);
		float 	fSlow28 = powf(10,(0.05f * fslider13));
		float 	fSlow29 = (0.5f * ((int(fcheckbox5))?2:fslider14));
		float 	fSlow30 = (1 - fSlow29);
		float 	fSlow31 = expf((fConst1 * (0 - (3.141592653589793f * fslider15))));
		float 	fSlow32 = faustpower<2>(fSlow31);
		float 	fSlow33 = (0 - (2 * fSlow31));
		float 	fSlow34 = fslider16;
		float 	fSlow35 = (fConst1 * faustpower<3>(fSlow34));
		float 	fSlow36 = fslider17;
		float 	fSlow37 = (6.283185307179586f * fSlow36);
		float 	fSlow38 = (0.5f * ((6.283185307179586f * max(fSlow36, fslider18)) - fSlow37));
		float 	fSlow39 = (fConst17 * fslider19);
		float 	fSlow40 = sinf(fSlow39);
		float 	fSlow41 = cosf(fSlow39);
		float 	fSlow42 = (0 - fSlow40);
		float 	fSlow43 = (fConst1 * faustpower<2>(fSlow34));
		float 	fSlow44 = (fConst1 * fSlow34);
		float 	fSlow45 = fslider20;
		float 	fSlow46 = (fConst1 * faustpower<4>(fSlow34));
		float 	fSlow47 = ((int(fcheckbox6))?(0 - fSlow29):fSlow29);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			iVec0[0] = 1;
			fRec4[0] = ((0.999f * fRec4[1]) + fSlow5);
			iRec6[0] = (12345 + (1103515245 * iRec6[1]));
			fRec5[0] = (((0.5221894f * fRec5[3]) + ((4.656612875245797e-10f * iRec6[0]) + (2.494956002f * fRec5[1]))) - (2.017265875f * fRec5[2]));
			fVec1[0] = 0.25f;
			fRec7[0] = ((fSlow8 * fRec7[1]) + fSlow9);
			fRec8[0] = fmodf((1.0f + fRec8[1]),(fConst15 / fRec7[0]));
			float fTemp0 = faustpower<2>(((fConst16 * (fRec7[0] * fRec8[0])) - 1.0f));
			fVec2[0] = fTemp0;
			fRec9[0] = fmodf((1.0f + fRec9[1]),(fSlow13 / fRec7[0]));
			float fTemp1 = faustpower<2>(((fSlow12 * (fRec7[0] * fRec9[0])) - 1.0f));
			fVec3[0] = fTemp1;
			fRec10[0] = fmodf((1.0f + fRec10[1]),(fSlow17 / fRec7[0]));
			float fTemp2 = faustpower<2>(((fSlow16 * (fRec7[0] * fRec10[0])) - 1.0f));
			fVec4[0] = fTemp2;
			float fTemp3 = (fRec4[0] * ((iSlow6)?(float)input0[i]:((iSlow7)?(((0.049922035f * fRec5[0]) + (0.050612699f * fRec5[2])) - ((0.095993537f * fRec5[1]) + (0.004408786f * fRec5[3]))):(fConst14 * (((fRec4[0] * fVec1[1]) * (((fVec2[0] - fVec2[1]) + (fSlow11 * (fVec3[0] - fVec3[1]))) + (fSlow15 * (fVec4[0] - fVec4[1])))) / fRec7[0])))));
			fRec11[0] = ((fSlow20 * fRec12[1]) + (fSlow21 * fRec11[1]));
			fRec12[0] = ((1 + ((fSlow21 * fRec12[1]) + (fSlow22 * fRec11[1]))) - iVec0[1]);
			fbargraph0 = (fRec12[0] + fRec11[0]);
			float fTemp4 = ((iSlow4)?0:fTemp3);
			float fTemp5 = (fSlow18 * fTemp4);
			float fTemp6 = ((fSlow25 * fRec13[1]) - fTemp5);
			fVec5[IOTA&2047] = fTemp6;
			float fTemp7 = (iConst0 * (fSlow26 + (fSlow27 * (1 + fRec11[0]))));
			int iTemp8 = int(fTemp7);
			int iTemp9 = (1 + iTemp8);
			fRec13[0] = ((fVec5[(IOTA-int((iTemp8 & 2047)))&2047] * (iTemp9 - fTemp7)) + ((fTemp7 - iTemp8) * fVec5[(IOTA-int((int(iTemp9) & 2047)))&2047]));
			float fTemp10 = ((iSlow4)?fTemp3:(0.5f * (fTemp5 + (fSlow24 * fRec13[0]))));
			float fTemp11 = ((iSlow3)?0:fTemp10);
			fRec16[0] = ((fSlow40 * fRec17[1]) + (fSlow41 * fRec16[1]));
			fRec17[0] = ((1 + ((fSlow41 * fRec17[1]) + (fSlow42 * fRec16[1]))) - iVec0[1]);
			float fTemp12 = (fSlow37 + (fSlow38 * (1 - fRec16[0])));
			float fTemp13 = (cosf((fSlow44 * fTemp12)) * fRec20[1]);
			fRec20[0] = (0 - (((fSlow33 * fTemp13) + (fSlow32 * fRec20[2])) - ((fSlow28 * fTemp11) + (fSlow45 * fRec14[1]))));
			float fTemp14 = (cosf((fSlow43 * fTemp12)) * fRec19[1]);
			fRec19[0] = ((fSlow33 * (fTemp13 - fTemp14)) + (fRec20[2] + (fSlow32 * (fRec20[0] - fRec19[2]))));
			float fTemp15 = (cosf((fSlow35 * fTemp12)) * fRec18[1]);
			fRec18[0] = ((fSlow33 * (fTemp14 - fTemp15)) + (fRec19[2] + (fSlow32 * (fRec19[0] - fRec18[2]))));
			float fTemp16 = (cosf((fSlow46 * fTemp12)) * fRec15[1]);
			fRec15[0] = ((fSlow33 * (fTemp15 - fTemp16)) + (fRec18[2] + (fSlow32 * (fRec18[0] - fRec15[2]))));
			fRec14[0] = ((fSlow32 * fRec15[0]) + ((fSlow33 * fTemp16) + fRec15[2]));
			float fTemp17 = ((iSlow3)?fTemp10:((fSlow28 * (fTemp11 * fSlow30)) + (fRec14[0] * fSlow47)));
			float fTemp18 = (fSlow18 * fTemp4);
			float fTemp19 = ((fSlow25 * fRec21[1]) - fTemp18);
			fVec6[IOTA&2047] = fTemp19;
			float fTemp20 = (iConst0 * (fSlow26 + (fSlow27 * (1 + fRec12[0]))));
			int iTemp21 = int(fTemp20);
			int iTemp22 = (1 + iTemp21);
			fRec21[0] = ((fVec6[(IOTA-int((iTemp21 & 2047)))&2047] * (iTemp22 - fTemp20)) + ((fTemp20 - iTemp21) * fVec6[(IOTA-int((int(iTemp22) & 2047)))&2047]));
			float fTemp23 = ((iSlow4)?fTemp3:(0.5f * (fTemp18 + (fSlow24 * fRec21[0]))));
			float fTemp24 = ((iSlow3)?0:fTemp23);
			float fTemp25 = (fSlow37 + (fSlow38 * (1 - fRec17[0])));
			float fTemp26 = (cosf((fSlow44 * fTemp25)) * fRec26[1]);
			fRec26[0] = (0 - (((fSlow33 * fTemp26) + (fSlow32 * fRec26[2])) - ((fSlow28 * fTemp24) + (fSlow45 * fRec22[1]))));
			float fTemp27 = (cosf((fSlow43 * fTemp25)) * fRec25[1]);
			fRec25[0] = ((fSlow33 * (fTemp26 - fTemp27)) + (fRec26[2] + (fSlow32 * (fRec26[0] - fRec25[2]))));
			float fTemp28 = (cosf((fSlow35 * fTemp25)) * fRec24[1]);
			fRec24[0] = ((fSlow33 * (fTemp27 - fTemp28)) + (fRec25[2] + (fSlow32 * (fRec25[0] - fRec24[2]))));
			float fTemp29 = (cosf((fSlow46 * fTemp25)) * fRec23[1]);
			fRec23[0] = ((fSlow33 * (fTemp28 - fTemp29)) + (fRec24[2] + (fSlow32 * (fRec24[0] - fRec23[2]))));
			fRec22[0] = ((fSlow32 * fRec23[0]) + ((fSlow33 * fTemp29) + fRec23[2]));
			float fTemp30 = ((iSlow3)?fTemp23:((fSlow28 * (fSlow30 * fTemp24)) + (fRec22[0] * fSlow47)));
			float fTemp31 = (fTemp17 + fTemp30);
			fRec3[0] = (fTemp31 - (fConst11 * ((fConst18 * fRec3[2]) + (fConst20 * fRec3[1]))));
			fRec2[0] = ((fConst11 * (((fConst13 * fRec3[0]) + (fConst21 * fRec3[1])) + (fConst13 * fRec3[2]))) - (fConst8 * ((fConst22 * fRec2[2]) + (fConst23 * fRec2[1]))));
			fRec1[0] = ((fConst8 * (((fConst10 * fRec2[0]) + (fConst24 * fRec2[1])) + (fConst10 * fRec2[2]))) - (fConst4 * ((fConst25 * fRec1[2]) + (fConst26 * fRec1[1]))));
			fRec0[0] = ((fSlow1 * fRec0[1]) + (fSlow2 * fabsf((fConst4 * (((fConst7 * fRec1[0]) + (fConst27 * fRec1[1])) + (fConst7 * fRec1[2]))))));
			fbargraph1 = (fSlow0 + (20 * log10f(fRec0[0])));
			fRec33[0] = (fTemp31 - (fConst44 * ((fConst47 * fRec33[2]) + (fConst48 * fRec33[1]))));
			fRec32[0] = ((fConst44 * (((fConst46 * fRec33[0]) + (fConst49 * fRec33[1])) + (fConst46 * fRec33[2]))) - (fConst42 * ((fConst50 * fRec32[2]) + (fConst51 * fRec32[1]))));
			fRec31[0] = ((fConst42 * (((fConst43 * fRec32[0]) + (fConst52 * fRec32[1])) + (fConst43 * fRec32[2]))) - (fConst40 * ((fConst53 * fRec31[2]) + (fConst54 * fRec31[1]))));
			float fTemp32 = (fConst40 * (((fConst41 * fRec31[0]) + (fConst55 * fRec31[1])) + (fConst41 * fRec31[2])));
			fRec30[0] = (fTemp32 - (fConst37 * ((fConst56 * fRec30[2]) + (fConst58 * fRec30[1]))));
			fRec29[0] = ((fConst37 * (((fConst39 * fRec30[0]) + (fConst59 * fRec30[1])) + (fConst39 * fRec30[2]))) - (fConst34 * ((fConst60 * fRec29[2]) + (fConst61 * fRec29[1]))));
			fRec28[0] = ((fConst34 * (((fConst36 * fRec29[0]) + (fConst62 * fRec29[1])) + (fConst36 * fRec29[2]))) - (fConst30 * ((fConst63 * fRec28[2]) + (fConst64 * fRec28[1]))));
			fRec27[0] = ((fSlow1 * fRec27[1]) + (fSlow2 * fabsf((fConst30 * (((fConst33 * fRec28[0]) + (fConst65 * fRec28[1])) + (fConst33 * fRec28[2]))))));
			fbargraph2 = (fSlow0 + (20 * log10f(fRec27[0])));
			fRec40[0] = (fTemp32 - (fConst82 * ((fConst85 * fRec40[2]) + (fConst86 * fRec40[1]))));
			fRec39[0] = ((fConst82 * (((fConst84 * fRec40[0]) + (fConst87 * fRec40[1])) + (fConst84 * fRec40[2]))) - (fConst80 * ((fConst88 * fRec39[2]) + (fConst89 * fRec39[1]))));
			fRec38[0] = ((fConst80 * (((fConst81 * fRec39[0]) + (fConst90 * fRec39[1])) + (fConst81 * fRec39[2]))) - (fConst78 * ((fConst91 * fRec38[2]) + (fConst92 * fRec38[1]))));
			float fTemp33 = (fConst78 * (((fConst79 * fRec38[0]) + (fConst93 * fRec38[1])) + (fConst79 * fRec38[2])));
			fRec37[0] = (fTemp33 - (fConst75 * ((fConst94 * fRec37[2]) + (fConst96 * fRec37[1]))));
			fRec36[0] = ((fConst75 * (((fConst77 * fRec37[0]) + (fConst97 * fRec37[1])) + (fConst77 * fRec37[2]))) - (fConst72 * ((fConst98 * fRec36[2]) + (fConst99 * fRec36[1]))));
			fRec35[0] = ((fConst72 * (((fConst74 * fRec36[0]) + (fConst100 * fRec36[1])) + (fConst74 * fRec36[2]))) - (fConst68 * ((fConst101 * fRec35[2]) + (fConst102 * fRec35[1]))));
			fRec34[0] = ((fSlow1 * fRec34[1]) + (fSlow2 * fabsf((fConst68 * (((fConst71 * fRec35[0]) + (fConst103 * fRec35[1])) + (fConst71 * fRec35[2]))))));
			fbargraph3 = (fSlow0 + (20 * log10f(fRec34[0])));
			fRec47[0] = (fTemp33 - (fConst120 * ((fConst123 * fRec47[2]) + (fConst124 * fRec47[1]))));
			fRec46[0] = ((fConst120 * (((fConst122 * fRec47[0]) + (fConst125 * fRec47[1])) + (fConst122 * fRec47[2]))) - (fConst118 * ((fConst126 * fRec46[2]) + (fConst127 * fRec46[1]))));
			fRec45[0] = ((fConst118 * (((fConst119 * fRec46[0]) + (fConst128 * fRec46[1])) + (fConst119 * fRec46[2]))) - (fConst116 * ((fConst129 * fRec45[2]) + (fConst130 * fRec45[1]))));
			float fTemp34 = (fConst116 * (((fConst117 * fRec45[0]) + (fConst131 * fRec45[1])) + (fConst117 * fRec45[2])));
			fRec44[0] = (fTemp34 - (fConst113 * ((fConst132 * fRec44[2]) + (fConst134 * fRec44[1]))));
			fRec43[0] = ((fConst113 * (((fConst115 * fRec44[0]) + (fConst135 * fRec44[1])) + (fConst115 * fRec44[2]))) - (fConst110 * ((fConst136 * fRec43[2]) + (fConst137 * fRec43[1]))));
			fRec42[0] = ((fConst110 * (((fConst112 * fRec43[0]) + (fConst138 * fRec43[1])) + (fConst112 * fRec43[2]))) - (fConst106 * ((fConst139 * fRec42[2]) + (fConst140 * fRec42[1]))));
			fRec41[0] = ((fSlow1 * fRec41[1]) + (fSlow2 * fabsf((fConst106 * (((fConst109 * fRec42[0]) + (fConst141 * fRec42[1])) + (fConst109 * fRec42[2]))))));
			fbargraph4 = (fSlow0 + (20 * log10f(fRec41[0])));
			fRec54[0] = (fTemp34 - (fConst158 * ((fConst161 * fRec54[2]) + (fConst162 * fRec54[1]))));
			fRec53[0] = ((fConst158 * (((fConst160 * fRec54[0]) + (fConst163 * fRec54[1])) + (fConst160 * fRec54[2]))) - (fConst156 * ((fConst164 * fRec53[2]) + (fConst165 * fRec53[1]))));
			fRec52[0] = ((fConst156 * (((fConst157 * fRec53[0]) + (fConst166 * fRec53[1])) + (fConst157 * fRec53[2]))) - (fConst154 * ((fConst167 * fRec52[2]) + (fConst168 * fRec52[1]))));
			float fTemp35 = (fConst154 * (((fConst155 * fRec52[0]) + (fConst169 * fRec52[1])) + (fConst155 * fRec52[2])));
			fRec51[0] = (fTemp35 - (fConst151 * ((fConst170 * fRec51[2]) + (fConst172 * fRec51[1]))));
			fRec50[0] = ((fConst151 * (((fConst153 * fRec51[0]) + (fConst173 * fRec51[1])) + (fConst153 * fRec51[2]))) - (fConst148 * ((fConst174 * fRec50[2]) + (fConst175 * fRec50[1]))));
			fRec49[0] = ((fConst148 * (((fConst150 * fRec50[0]) + (fConst176 * fRec50[1])) + (fConst150 * fRec50[2]))) - (fConst144 * ((fConst177 * fRec49[2]) + (fConst178 * fRec49[1]))));
			fRec48[0] = ((fSlow1 * fRec48[1]) + (fSlow2 * fabsf((fConst144 * (((fConst147 * fRec49[0]) + (fConst179 * fRec49[1])) + (fConst147 * fRec49[2]))))));
			fbargraph5 = (fSlow0 + (20 * log10f(fRec48[0])));
			fRec61[0] = (fTemp35 - (fConst196 * ((fConst199 * fRec61[2]) + (fConst200 * fRec61[1]))));
			fRec60[0] = ((fConst196 * (((fConst198 * fRec61[0]) + (fConst201 * fRec61[1])) + (fConst198 * fRec61[2]))) - (fConst194 * ((fConst202 * fRec60[2]) + (fConst203 * fRec60[1]))));
			fRec59[0] = ((fConst194 * (((fConst195 * fRec60[0]) + (fConst204 * fRec60[1])) + (fConst195 * fRec60[2]))) - (fConst192 * ((fConst205 * fRec59[2]) + (fConst206 * fRec59[1]))));
			float fTemp36 = (fConst192 * (((fConst193 * fRec59[0]) + (fConst207 * fRec59[1])) + (fConst193 * fRec59[2])));
			fRec58[0] = (fTemp36 - (fConst189 * ((fConst208 * fRec58[2]) + (fConst210 * fRec58[1]))));
			fRec57[0] = ((fConst189 * (((fConst191 * fRec58[0]) + (fConst211 * fRec58[1])) + (fConst191 * fRec58[2]))) - (fConst186 * ((fConst212 * fRec57[2]) + (fConst213 * fRec57[1]))));
			fRec56[0] = ((fConst186 * (((fConst188 * fRec57[0]) + (fConst214 * fRec57[1])) + (fConst188 * fRec57[2]))) - (fConst182 * ((fConst215 * fRec56[2]) + (fConst216 * fRec56[1]))));
			fRec55[0] = ((fSlow1 * fRec55[1]) + (fSlow2 * fabsf((fConst182 * (((fConst185 * fRec56[0]) + (fConst217 * fRec56[1])) + (fConst185 * fRec56[2]))))));
			fbargraph6 = (fSlow0 + (20 * log10f(fRec55[0])));
			fRec68[0] = (fTemp36 - (fConst234 * ((fConst237 * fRec68[2]) + (fConst238 * fRec68[1]))));
			fRec67[0] = ((fConst234 * (((fConst236 * fRec68[0]) + (fConst239 * fRec68[1])) + (fConst236 * fRec68[2]))) - (fConst232 * ((fConst240 * fRec67[2]) + (fConst241 * fRec67[1]))));
			fRec66[0] = ((fConst232 * (((fConst233 * fRec67[0]) + (fConst242 * fRec67[1])) + (fConst233 * fRec67[2]))) - (fConst230 * ((fConst243 * fRec66[2]) + (fConst244 * fRec66[1]))));
			float fTemp37 = (fConst230 * (((fConst231 * fRec66[0]) + (fConst245 * fRec66[1])) + (fConst231 * fRec66[2])));
			fRec65[0] = (fTemp37 - (fConst227 * ((fConst246 * fRec65[2]) + (fConst248 * fRec65[1]))));
			fRec64[0] = ((fConst227 * (((fConst229 * fRec65[0]) + (fConst249 * fRec65[1])) + (fConst229 * fRec65[2]))) - (fConst224 * ((fConst250 * fRec64[2]) + (fConst251 * fRec64[1]))));
			fRec63[0] = ((fConst224 * (((fConst226 * fRec64[0]) + (fConst252 * fRec64[1])) + (fConst226 * fRec64[2]))) - (fConst220 * ((fConst253 * fRec63[2]) + (fConst254 * fRec63[1]))));
			fRec62[0] = ((fSlow1 * fRec62[1]) + (fSlow2 * fabsf((fConst220 * (((fConst223 * fRec63[0]) + (fConst255 * fRec63[1])) + (fConst223 * fRec63[2]))))));
			fbargraph7 = (fSlow0 + (20 * log10f(fRec62[0])));
			fRec75[0] = (fTemp37 - (fConst272 * ((fConst275 * fRec75[2]) + (fConst276 * fRec75[1]))));
			fRec74[0] = ((fConst272 * (((fConst274 * fRec75[0]) + (fConst277 * fRec75[1])) + (fConst274 * fRec75[2]))) - (fConst270 * ((fConst278 * fRec74[2]) + (fConst279 * fRec74[1]))));
			fRec73[0] = ((fConst270 * (((fConst271 * fRec74[0]) + (fConst280 * fRec74[1])) + (fConst271 * fRec74[2]))) - (fConst268 * ((fConst281 * fRec73[2]) + (fConst282 * fRec73[1]))));
			float fTemp38 = (fConst268 * (((fConst269 * fRec73[0]) + (fConst283 * fRec73[1])) + (fConst269 * fRec73[2])));
			fRec72[0] = (fTemp38 - (fConst265 * ((fConst284 * fRec72[2]) + (fConst286 * fRec72[1]))));
			fRec71[0] = ((fConst265 * (((fConst267 * fRec72[0]) + (fConst287 * fRec72[1])) + (fConst267 * fRec72[2]))) - (fConst262 * ((fConst288 * fRec71[2]) + (fConst289 * fRec71[1]))));
			fRec70[0] = ((fConst262 * (((fConst264 * fRec71[0]) + (fConst290 * fRec71[1])) + (fConst264 * fRec71[2]))) - (fConst258 * ((fConst291 * fRec70[2]) + (fConst292 * fRec70[1]))));
			fRec69[0] = ((fSlow1 * fRec69[1]) + (fSlow2 * fabsf((fConst258 * (((fConst261 * fRec70[0]) + (fConst293 * fRec70[1])) + (fConst261 * fRec70[2]))))));
			fbargraph8 = (fSlow0 + (20 * log10f(fRec69[0])));
			fRec82[0] = (fTemp38 - (fConst310 * ((fConst313 * fRec82[2]) + (fConst314 * fRec82[1]))));
			fRec81[0] = ((fConst310 * (((fConst312 * fRec82[0]) + (fConst315 * fRec82[1])) + (fConst312 * fRec82[2]))) - (fConst308 * ((fConst316 * fRec81[2]) + (fConst317 * fRec81[1]))));
			fRec80[0] = ((fConst308 * (((fConst309 * fRec81[0]) + (fConst318 * fRec81[1])) + (fConst309 * fRec81[2]))) - (fConst306 * ((fConst319 * fRec80[2]) + (fConst320 * fRec80[1]))));
			float fTemp39 = (fConst306 * (((fConst307 * fRec80[0]) + (fConst321 * fRec80[1])) + (fConst307 * fRec80[2])));
			fRec79[0] = (fTemp39 - (fConst303 * ((fConst322 * fRec79[2]) + (fConst324 * fRec79[1]))));
			fRec78[0] = ((fConst303 * (((fConst305 * fRec79[0]) + (fConst325 * fRec79[1])) + (fConst305 * fRec79[2]))) - (fConst300 * ((fConst326 * fRec78[2]) + (fConst327 * fRec78[1]))));
			fRec77[0] = ((fConst300 * (((fConst302 * fRec78[0]) + (fConst328 * fRec78[1])) + (fConst302 * fRec78[2]))) - (fConst296 * ((fConst329 * fRec77[2]) + (fConst330 * fRec77[1]))));
			fRec76[0] = ((fSlow1 * fRec76[1]) + (fSlow2 * fabsf((fConst296 * (((fConst299 * fRec77[0]) + (fConst331 * fRec77[1])) + (fConst299 * fRec77[2]))))));
			fbargraph9 = (fSlow0 + (20 * log10f(fRec76[0])));
			fRec89[0] = (fTemp39 - (fConst348 * ((fConst351 * fRec89[2]) + (fConst352 * fRec89[1]))));
			fRec88[0] = ((fConst348 * (((fConst350 * fRec89[0]) + (fConst353 * fRec89[1])) + (fConst350 * fRec89[2]))) - (fConst346 * ((fConst354 * fRec88[2]) + (fConst355 * fRec88[1]))));
			fRec87[0] = ((fConst346 * (((fConst347 * fRec88[0]) + (fConst356 * fRec88[1])) + (fConst347 * fRec88[2]))) - (fConst344 * ((fConst357 * fRec87[2]) + (fConst358 * fRec87[1]))));
			float fTemp40 = (fConst344 * (((fConst345 * fRec87[0]) + (fConst359 * fRec87[1])) + (fConst345 * fRec87[2])));
			fRec86[0] = (fTemp40 - (fConst341 * ((fConst360 * fRec86[2]) + (fConst362 * fRec86[1]))));
			fRec85[0] = ((fConst341 * (((fConst343 * fRec86[0]) + (fConst363 * fRec86[1])) + (fConst343 * fRec86[2]))) - (fConst338 * ((fConst364 * fRec85[2]) + (fConst365 * fRec85[1]))));
			fRec84[0] = ((fConst338 * (((fConst340 * fRec85[0]) + (fConst366 * fRec85[1])) + (fConst340 * fRec85[2]))) - (fConst334 * ((fConst367 * fRec84[2]) + (fConst368 * fRec84[1]))));
			fRec83[0] = ((fSlow1 * fRec83[1]) + (fSlow2 * fabsf((fConst334 * (((fConst337 * fRec84[0]) + (fConst369 * fRec84[1])) + (fConst337 * fRec84[2]))))));
			fbargraph10 = (fSlow0 + (20 * log10f(fRec83[0])));
			fRec96[0] = (fTemp40 - (fConst386 * ((fConst389 * fRec96[2]) + (fConst390 * fRec96[1]))));
			fRec95[0] = ((fConst386 * (((fConst388 * fRec96[0]) + (fConst391 * fRec96[1])) + (fConst388 * fRec96[2]))) - (fConst384 * ((fConst392 * fRec95[2]) + (fConst393 * fRec95[1]))));
			fRec94[0] = ((fConst384 * (((fConst385 * fRec95[0]) + (fConst394 * fRec95[1])) + (fConst385 * fRec95[2]))) - (fConst382 * ((fConst395 * fRec94[2]) + (fConst396 * fRec94[1]))));
			float fTemp41 = (fConst382 * (((fConst383 * fRec94[0]) + (fConst397 * fRec94[1])) + (fConst383 * fRec94[2])));
			fRec93[0] = (fTemp41 - (fConst379 * ((fConst398 * fRec93[2]) + (fConst400 * fRec93[1]))));
			fRec92[0] = ((fConst379 * (((fConst381 * fRec93[0]) + (fConst401 * fRec93[1])) + (fConst381 * fRec93[2]))) - (fConst376 * ((fConst402 * fRec92[2]) + (fConst403 * fRec92[1]))));
			fRec91[0] = ((fConst376 * (((fConst378 * fRec92[0]) + (fConst404 * fRec92[1])) + (fConst378 * fRec92[2]))) - (fConst372 * ((fConst405 * fRec91[2]) + (fConst406 * fRec91[1]))));
			fRec90[0] = ((fSlow1 * fRec90[1]) + (fSlow2 * fabsf((fConst372 * (((fConst375 * fRec91[0]) + (fConst407 * fRec91[1])) + (fConst375 * fRec91[2]))))));
			fbargraph11 = (fSlow0 + (20 * log10f(fRec90[0])));
			fRec103[0] = (fTemp41 - (fConst424 * ((fConst427 * fRec103[2]) + (fConst428 * fRec103[1]))));
			fRec102[0] = ((fConst424 * (((fConst426 * fRec103[0]) + (fConst429 * fRec103[1])) + (fConst426 * fRec103[2]))) - (fConst422 * ((fConst430 * fRec102[2]) + (fConst431 * fRec102[1]))));
			fRec101[0] = ((fConst422 * (((fConst423 * fRec102[0]) + (fConst432 * fRec102[1])) + (fConst423 * fRec102[2]))) - (fConst420 * ((fConst433 * fRec101[2]) + (fConst434 * fRec101[1]))));
			float fTemp42 = (fConst420 * (((fConst421 * fRec101[0]) + (fConst435 * fRec101[1])) + (fConst421 * fRec101[2])));
			fRec100[0] = (fTemp42 - (fConst417 * ((fConst436 * fRec100[2]) + (fConst438 * fRec100[1]))));
			fRec99[0] = ((fConst417 * (((fConst419 * fRec100[0]) + (fConst439 * fRec100[1])) + (fConst419 * fRec100[2]))) - (fConst414 * ((fConst440 * fRec99[2]) + (fConst441 * fRec99[1]))));
			fRec98[0] = ((fConst414 * (((fConst416 * fRec99[0]) + (fConst442 * fRec99[1])) + (fConst416 * fRec99[2]))) - (fConst410 * ((fConst443 * fRec98[2]) + (fConst444 * fRec98[1]))));
			fRec97[0] = ((fSlow1 * fRec97[1]) + (fSlow2 * fabsf((fConst410 * (((fConst413 * fRec98[0]) + (fConst445 * fRec98[1])) + (fConst413 * fRec98[2]))))));
			fbargraph12 = (fSlow0 + (20 * log10f(fRec97[0])));
			fRec110[0] = (fTemp42 - (fConst462 * ((fConst465 * fRec110[2]) + (fConst466 * fRec110[1]))));
			fRec109[0] = ((fConst462 * (((fConst464 * fRec110[0]) + (fConst467 * fRec110[1])) + (fConst464 * fRec110[2]))) - (fConst460 * ((fConst468 * fRec109[2]) + (fConst469 * fRec109[1]))));
			fRec108[0] = ((fConst460 * (((fConst461 * fRec109[0]) + (fConst470 * fRec109[1])) + (fConst461 * fRec109[2]))) - (fConst458 * ((fConst471 * fRec108[2]) + (fConst472 * fRec108[1]))));
			float fTemp43 = (fConst458 * (((fConst459 * fRec108[0]) + (fConst473 * fRec108[1])) + (fConst459 * fRec108[2])));
			fRec107[0] = (fTemp43 - (fConst455 * ((fConst474 * fRec107[2]) + (fConst476 * fRec107[1]))));
			fRec106[0] = ((fConst455 * (((fConst457 * fRec107[0]) + (fConst477 * fRec107[1])) + (fConst457 * fRec107[2]))) - (fConst452 * ((fConst478 * fRec106[2]) + (fConst479 * fRec106[1]))));
			fRec105[0] = ((fConst452 * (((fConst454 * fRec106[0]) + (fConst480 * fRec106[1])) + (fConst454 * fRec106[2]))) - (fConst448 * ((fConst481 * fRec105[2]) + (fConst482 * fRec105[1]))));
			fRec104[0] = ((fSlow1 * fRec104[1]) + (fSlow2 * fabsf((fConst448 * (((fConst451 * fRec105[0]) + (fConst483 * fRec105[1])) + (fConst451 * fRec105[2]))))));
			fbargraph13 = (fSlow0 + (20 * log10f(fRec104[0])));
			fRec117[0] = (fTemp43 - (fConst500 * ((fConst503 * fRec117[2]) + (fConst504 * fRec117[1]))));
			fRec116[0] = ((fConst500 * (((fConst502 * fRec117[0]) + (fConst505 * fRec117[1])) + (fConst502 * fRec117[2]))) - (fConst498 * ((fConst506 * fRec116[2]) + (fConst507 * fRec116[1]))));
			fRec115[0] = ((fConst498 * (((fConst499 * fRec116[0]) + (fConst508 * fRec116[1])) + (fConst499 * fRec116[2]))) - (fConst496 * ((fConst509 * fRec115[2]) + (fConst510 * fRec115[1]))));
			float fTemp44 = (fConst496 * (((fConst497 * fRec115[0]) + (fConst511 * fRec115[1])) + (fConst497 * fRec115[2])));
			fRec114[0] = (fTemp44 - (fConst493 * ((fConst512 * fRec114[2]) + (fConst514 * fRec114[1]))));
			fRec113[0] = ((fConst493 * (((fConst495 * fRec114[0]) + (fConst515 * fRec114[1])) + (fConst495 * fRec114[2]))) - (fConst490 * ((fConst516 * fRec113[2]) + (fConst517 * fRec113[1]))));
			fRec112[0] = ((fConst490 * (((fConst492 * fRec113[0]) + (fConst518 * fRec113[1])) + (fConst492 * fRec113[2]))) - (fConst486 * ((fConst519 * fRec112[2]) + (fConst520 * fRec112[1]))));
			fRec111[0] = ((fSlow1 * fRec111[1]) + (fSlow2 * fabsf((fConst486 * (((fConst489 * fRec112[0]) + (fConst521 * fRec112[1])) + (fConst489 * fRec112[2]))))));
			fbargraph14 = (fSlow0 + (20 * log10f(fRec111[0])));
			fRec121[0] = (fTemp44 - (fConst526 * ((fConst529 * fRec121[2]) + (fConst530 * fRec121[1]))));
			fRec120[0] = ((fConst526 * (((fConst528 * fRec121[0]) + (fConst531 * fRec121[1])) + (fConst528 * fRec121[2]))) - (fConst524 * ((fConst532 * fRec120[2]) + (fConst533 * fRec120[1]))));
			fRec119[0] = ((fConst524 * (((fConst525 * fRec120[0]) + (fConst534 * fRec120[1])) + (fConst525 * fRec120[2]))) - (fConst522 * ((fConst535 * fRec119[2]) + (fConst536 * fRec119[1]))));
			fRec118[0] = ((fSlow1 * fRec118[1]) + (fSlow2 * fabsf((fConst522 * (((fConst523 * fRec119[0]) + (fConst537 * fRec119[1])) + (fConst523 * fRec119[2]))))));
			fbargraph15 = (fSlow0 + (20 * log10f(fRec118[0])));
			output0[i] = (FAUSTFLOAT)fTemp17;
			output1[i] = (FAUSTFLOAT)fTemp30;
			// post processing
			fRec118[1] = fRec118[0];
			fRec119[2] = fRec119[1]; fRec119[1] = fRec119[0];
			fRec120[2] = fRec120[1]; fRec120[1] = fRec120[0];
			fRec121[2] = fRec121[1]; fRec121[1] = fRec121[0];
			fRec111[1] = fRec111[0];
			fRec112[2] = fRec112[1]; fRec112[1] = fRec112[0];
			fRec113[2] = fRec113[1]; fRec113[1] = fRec113[0];
			fRec114[2] = fRec114[1]; fRec114[1] = fRec114[0];
			fRec115[2] = fRec115[1]; fRec115[1] = fRec115[0];
			fRec116[2] = fRec116[1]; fRec116[1] = fRec116[0];
			fRec117[2] = fRec117[1]; fRec117[1] = fRec117[0];
			fRec104[1] = fRec104[0];
			fRec105[2] = fRec105[1]; fRec105[1] = fRec105[0];
			fRec106[2] = fRec106[1]; fRec106[1] = fRec106[0];
			fRec107[2] = fRec107[1]; fRec107[1] = fRec107[0];
			fRec108[2] = fRec108[1]; fRec108[1] = fRec108[0];
			fRec109[2] = fRec109[1]; fRec109[1] = fRec109[0];
			fRec110[2] = fRec110[1]; fRec110[1] = fRec110[0];
			fRec97[1] = fRec97[0];
			fRec98[2] = fRec98[1]; fRec98[1] = fRec98[0];
			fRec99[2] = fRec99[1]; fRec99[1] = fRec99[0];
			fRec100[2] = fRec100[1]; fRec100[1] = fRec100[0];
			fRec101[2] = fRec101[1]; fRec101[1] = fRec101[0];
			fRec102[2] = fRec102[1]; fRec102[1] = fRec102[0];
			fRec103[2] = fRec103[1]; fRec103[1] = fRec103[0];
			fRec90[1] = fRec90[0];
			fRec91[2] = fRec91[1]; fRec91[1] = fRec91[0];
			fRec92[2] = fRec92[1]; fRec92[1] = fRec92[0];
			fRec93[2] = fRec93[1]; fRec93[1] = fRec93[0];
			fRec94[2] = fRec94[1]; fRec94[1] = fRec94[0];
			fRec95[2] = fRec95[1]; fRec95[1] = fRec95[0];
			fRec96[2] = fRec96[1]; fRec96[1] = fRec96[0];
			fRec83[1] = fRec83[0];
			fRec84[2] = fRec84[1]; fRec84[1] = fRec84[0];
			fRec85[2] = fRec85[1]; fRec85[1] = fRec85[0];
			fRec86[2] = fRec86[1]; fRec86[1] = fRec86[0];
			fRec87[2] = fRec87[1]; fRec87[1] = fRec87[0];
			fRec88[2] = fRec88[1]; fRec88[1] = fRec88[0];
			fRec89[2] = fRec89[1]; fRec89[1] = fRec89[0];
			fRec76[1] = fRec76[0];
			fRec77[2] = fRec77[1]; fRec77[1] = fRec77[0];
			fRec78[2] = fRec78[1]; fRec78[1] = fRec78[0];
			fRec79[2] = fRec79[1]; fRec79[1] = fRec79[0];
			fRec80[2] = fRec80[1]; fRec80[1] = fRec80[0];
			fRec81[2] = fRec81[1]; fRec81[1] = fRec81[0];
			fRec82[2] = fRec82[1]; fRec82[1] = fRec82[0];
			fRec69[1] = fRec69[0];
			fRec70[2] = fRec70[1]; fRec70[1] = fRec70[0];
			fRec71[2] = fRec71[1]; fRec71[1] = fRec71[0];
			fRec72[2] = fRec72[1]; fRec72[1] = fRec72[0];
			fRec73[2] = fRec73[1]; fRec73[1] = fRec73[0];
			fRec74[2] = fRec74[1]; fRec74[1] = fRec74[0];
			fRec75[2] = fRec75[1]; fRec75[1] = fRec75[0];
			fRec62[1] = fRec62[0];
			fRec63[2] = fRec63[1]; fRec63[1] = fRec63[0];
			fRec64[2] = fRec64[1]; fRec64[1] = fRec64[0];
			fRec65[2] = fRec65[1]; fRec65[1] = fRec65[0];
			fRec66[2] = fRec66[1]; fRec66[1] = fRec66[0];
			fRec67[2] = fRec67[1]; fRec67[1] = fRec67[0];
			fRec68[2] = fRec68[1]; fRec68[1] = fRec68[0];
			fRec55[1] = fRec55[0];
			fRec56[2] = fRec56[1]; fRec56[1] = fRec56[0];
			fRec57[2] = fRec57[1]; fRec57[1] = fRec57[0];
			fRec58[2] = fRec58[1]; fRec58[1] = fRec58[0];
			fRec59[2] = fRec59[1]; fRec59[1] = fRec59[0];
			fRec60[2] = fRec60[1]; fRec60[1] = fRec60[0];
			fRec61[2] = fRec61[1]; fRec61[1] = fRec61[0];
			fRec48[1] = fRec48[0];
			fRec49[2] = fRec49[1]; fRec49[1] = fRec49[0];
			fRec50[2] = fRec50[1]; fRec50[1] = fRec50[0];
			fRec51[2] = fRec51[1]; fRec51[1] = fRec51[0];
			fRec52[2] = fRec52[1]; fRec52[1] = fRec52[0];
			fRec53[2] = fRec53[1]; fRec53[1] = fRec53[0];
			fRec54[2] = fRec54[1]; fRec54[1] = fRec54[0];
			fRec41[1] = fRec41[0];
			fRec42[2] = fRec42[1]; fRec42[1] = fRec42[0];
			fRec43[2] = fRec43[1]; fRec43[1] = fRec43[0];
			fRec44[2] = fRec44[1]; fRec44[1] = fRec44[0];
			fRec45[2] = fRec45[1]; fRec45[1] = fRec45[0];
			fRec46[2] = fRec46[1]; fRec46[1] = fRec46[0];
			fRec47[2] = fRec47[1]; fRec47[1] = fRec47[0];
			fRec34[1] = fRec34[0];
			fRec35[2] = fRec35[1]; fRec35[1] = fRec35[0];
			fRec36[2] = fRec36[1]; fRec36[1] = fRec36[0];
			fRec37[2] = fRec37[1]; fRec37[1] = fRec37[0];
			fRec38[2] = fRec38[1]; fRec38[1] = fRec38[0];
			fRec39[2] = fRec39[1]; fRec39[1] = fRec39[0];
			fRec40[2] = fRec40[1]; fRec40[1] = fRec40[0];
			fRec27[1] = fRec27[0];
			fRec28[2] = fRec28[1]; fRec28[1] = fRec28[0];
			fRec29[2] = fRec29[1]; fRec29[1] = fRec29[0];
			fRec30[2] = fRec30[1]; fRec30[1] = fRec30[0];
			fRec31[2] = fRec31[1]; fRec31[1] = fRec31[0];
			fRec32[2] = fRec32[1]; fRec32[1] = fRec32[0];
			fRec33[2] = fRec33[1]; fRec33[1] = fRec33[0];
			fRec0[1] = fRec0[0];
			fRec1[2] = fRec1[1]; fRec1[1] = fRec1[0];
			fRec2[2] = fRec2[1]; fRec2[1] = fRec2[0];
			fRec3[2] = fRec3[1]; fRec3[1] = fRec3[0];
			fRec22[1] = fRec22[0];
			fRec23[2] = fRec23[1]; fRec23[1] = fRec23[0];
			fRec24[2] = fRec24[1]; fRec24[1] = fRec24[0];
			fRec25[2] = fRec25[1]; fRec25[1] = fRec25[0];
			fRec26[2] = fRec26[1]; fRec26[1] = fRec26[0];
			fRec21[1] = fRec21[0];
			fRec14[1] = fRec14[0];
			fRec15[2] = fRec15[1]; fRec15[1] = fRec15[0];
			fRec18[2] = fRec18[1]; fRec18[1] = fRec18[0];
			fRec19[2] = fRec19[1]; fRec19[1] = fRec19[0];
			fRec20[2] = fRec20[1]; fRec20[1] = fRec20[0];
			fRec17[1] = fRec17[0];
			fRec16[1] = fRec16[0];
			fRec13[1] = fRec13[0];
			IOTA = IOTA+1;
			fRec12[1] = fRec12[0];
			fRec11[1] = fRec11[0];
			fVec4[1] = fVec4[0];
			fRec10[1] = fRec10[0];
			fVec3[1] = fVec3[0];
			fRec9[1] = fRec9[0];
			fVec2[1] = fVec2[0];
			fRec8[1] = fRec8[0];
			fRec7[1] = fRec7[0];
			fVec1[1] = fVec1[0];
			for (int i=3; i>0; i--) fRec5[i] = fRec5[i-1];
			iRec6[1] = iRec6[0];
			fRec4[1] = fRec4[0];
			iVec0[1] = iVec0[0];
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
