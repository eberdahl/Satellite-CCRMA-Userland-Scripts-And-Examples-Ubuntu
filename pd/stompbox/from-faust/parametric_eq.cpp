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
	float 	fConst14;
	FAUSTFLOAT 	fslider2;
	float 	fConst15;
	FAUSTFLOAT 	fslider3;
	float 	fRec6[2];
	FAUSTFLOAT 	fslider4;
	FAUSTFLOAT 	fslider5;
	float 	fRec10[2];
	FAUSTFLOAT 	fcheckbox0;
	FAUSTFLOAT 	fcheckbox1;
	int 	iRec12[2];
	float 	fRec11[4];
	float 	fConst16;
	float 	fVec0[2];
	float 	fConst17;
	float 	fConst18;
	FAUSTFLOAT 	fslider6;
	FAUSTFLOAT 	fslider7;
	float 	fRec13[2];
	float 	fRec14[2];
	float 	fVec1[2];
	FAUSTFLOAT 	fslider8;
	float 	fRec15[2];
	float 	fVec2[2];
	FAUSTFLOAT 	fslider9;
	float 	fRec16[2];
	float 	fVec3[2];
	float 	fVec4[2];
	float 	fRec9[2];
	float 	fRec8[3];
	FAUSTFLOAT 	fslider10;
	float 	fRec18[2];
	float 	fRec17[3];
	FAUSTFLOAT 	fslider11;
	float 	fConst19;
	float 	fConst20;
	FAUSTFLOAT 	fslider12;
	float 	fConst21;
	float 	fRec7[3];
	float 	fVec5[2];
	float 	fRec5[2];
	float 	fRec4[3];
	FAUSTFLOAT 	fslider13;
	float 	fRec20[2];
	float 	fRec19[3];
	float 	fConst22;
	float 	fConst23;
	float 	fConst24;
	float 	fRec3[3];
	float 	fConst25;
	float 	fConst26;
	float 	fConst27;
	float 	fRec2[3];
	float 	fConst28;
	float 	fConst29;
	float 	fConst30;
	float 	fRec1[3];
	float 	fConst31;
	float 	fRec0[2];
	FAUSTFLOAT 	fbargraph0;
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
	float 	fConst49;
	float 	fConst50;
	float 	fConst51;
	float 	fConst52;
	float 	fRec27[3];
	float 	fConst53;
	float 	fConst54;
	float 	fConst55;
	float 	fRec26[3];
	float 	fConst56;
	float 	fConst57;
	float 	fConst58;
	float 	fRec25[3];
	float 	fConst59;
	float 	fConst60;
	float 	fConst61;
	float 	fConst62;
	float 	fRec24[3];
	float 	fConst63;
	float 	fConst64;
	float 	fConst65;
	float 	fRec23[3];
	float 	fConst66;
	float 	fConst67;
	float 	fConst68;
	float 	fRec22[3];
	float 	fConst69;
	float 	fRec21[2];
	FAUSTFLOAT 	fbargraph1;
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
	float 	fConst87;
	float 	fConst88;
	float 	fConst89;
	float 	fConst90;
	float 	fRec34[3];
	float 	fConst91;
	float 	fConst92;
	float 	fConst93;
	float 	fRec33[3];
	float 	fConst94;
	float 	fConst95;
	float 	fConst96;
	float 	fRec32[3];
	float 	fConst97;
	float 	fConst98;
	float 	fConst99;
	float 	fConst100;
	float 	fRec31[3];
	float 	fConst101;
	float 	fConst102;
	float 	fConst103;
	float 	fRec30[3];
	float 	fConst104;
	float 	fConst105;
	float 	fConst106;
	float 	fRec29[3];
	float 	fConst107;
	float 	fRec28[2];
	FAUSTFLOAT 	fbargraph2;
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
	float 	fConst125;
	float 	fConst126;
	float 	fConst127;
	float 	fConst128;
	float 	fRec41[3];
	float 	fConst129;
	float 	fConst130;
	float 	fConst131;
	float 	fRec40[3];
	float 	fConst132;
	float 	fConst133;
	float 	fConst134;
	float 	fRec39[3];
	float 	fConst135;
	float 	fConst136;
	float 	fConst137;
	float 	fConst138;
	float 	fRec38[3];
	float 	fConst139;
	float 	fConst140;
	float 	fConst141;
	float 	fRec37[3];
	float 	fConst142;
	float 	fConst143;
	float 	fConst144;
	float 	fRec36[3];
	float 	fConst145;
	float 	fRec35[2];
	FAUSTFLOAT 	fbargraph3;
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
	float 	fConst163;
	float 	fConst164;
	float 	fConst165;
	float 	fConst166;
	float 	fRec48[3];
	float 	fConst167;
	float 	fConst168;
	float 	fConst169;
	float 	fRec47[3];
	float 	fConst170;
	float 	fConst171;
	float 	fConst172;
	float 	fRec46[3];
	float 	fConst173;
	float 	fConst174;
	float 	fConst175;
	float 	fConst176;
	float 	fRec45[3];
	float 	fConst177;
	float 	fConst178;
	float 	fConst179;
	float 	fRec44[3];
	float 	fConst180;
	float 	fConst181;
	float 	fConst182;
	float 	fRec43[3];
	float 	fConst183;
	float 	fRec42[2];
	FAUSTFLOAT 	fbargraph4;
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
	float 	fConst201;
	float 	fConst202;
	float 	fConst203;
	float 	fConst204;
	float 	fRec55[3];
	float 	fConst205;
	float 	fConst206;
	float 	fConst207;
	float 	fRec54[3];
	float 	fConst208;
	float 	fConst209;
	float 	fConst210;
	float 	fRec53[3];
	float 	fConst211;
	float 	fConst212;
	float 	fConst213;
	float 	fConst214;
	float 	fRec52[3];
	float 	fConst215;
	float 	fConst216;
	float 	fConst217;
	float 	fRec51[3];
	float 	fConst218;
	float 	fConst219;
	float 	fConst220;
	float 	fRec50[3];
	float 	fConst221;
	float 	fRec49[2];
	FAUSTFLOAT 	fbargraph5;
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
	float 	fConst239;
	float 	fConst240;
	float 	fConst241;
	float 	fConst242;
	float 	fRec62[3];
	float 	fConst243;
	float 	fConst244;
	float 	fConst245;
	float 	fRec61[3];
	float 	fConst246;
	float 	fConst247;
	float 	fConst248;
	float 	fRec60[3];
	float 	fConst249;
	float 	fConst250;
	float 	fConst251;
	float 	fConst252;
	float 	fRec59[3];
	float 	fConst253;
	float 	fConst254;
	float 	fConst255;
	float 	fRec58[3];
	float 	fConst256;
	float 	fConst257;
	float 	fConst258;
	float 	fRec57[3];
	float 	fConst259;
	float 	fRec56[2];
	FAUSTFLOAT 	fbargraph6;
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
	float 	fConst277;
	float 	fConst278;
	float 	fConst279;
	float 	fConst280;
	float 	fRec69[3];
	float 	fConst281;
	float 	fConst282;
	float 	fConst283;
	float 	fRec68[3];
	float 	fConst284;
	float 	fConst285;
	float 	fConst286;
	float 	fRec67[3];
	float 	fConst287;
	float 	fConst288;
	float 	fConst289;
	float 	fConst290;
	float 	fRec66[3];
	float 	fConst291;
	float 	fConst292;
	float 	fConst293;
	float 	fRec65[3];
	float 	fConst294;
	float 	fConst295;
	float 	fConst296;
	float 	fRec64[3];
	float 	fConst297;
	float 	fRec63[2];
	FAUSTFLOAT 	fbargraph7;
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
	float 	fConst315;
	float 	fConst316;
	float 	fConst317;
	float 	fConst318;
	float 	fRec76[3];
	float 	fConst319;
	float 	fConst320;
	float 	fConst321;
	float 	fRec75[3];
	float 	fConst322;
	float 	fConst323;
	float 	fConst324;
	float 	fRec74[3];
	float 	fConst325;
	float 	fConst326;
	float 	fConst327;
	float 	fConst328;
	float 	fRec73[3];
	float 	fConst329;
	float 	fConst330;
	float 	fConst331;
	float 	fRec72[3];
	float 	fConst332;
	float 	fConst333;
	float 	fConst334;
	float 	fRec71[3];
	float 	fConst335;
	float 	fRec70[2];
	FAUSTFLOAT 	fbargraph8;
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
	float 	fConst353;
	float 	fConst354;
	float 	fConst355;
	float 	fConst356;
	float 	fRec83[3];
	float 	fConst357;
	float 	fConst358;
	float 	fConst359;
	float 	fRec82[3];
	float 	fConst360;
	float 	fConst361;
	float 	fConst362;
	float 	fRec81[3];
	float 	fConst363;
	float 	fConst364;
	float 	fConst365;
	float 	fConst366;
	float 	fRec80[3];
	float 	fConst367;
	float 	fConst368;
	float 	fConst369;
	float 	fRec79[3];
	float 	fConst370;
	float 	fConst371;
	float 	fConst372;
	float 	fRec78[3];
	float 	fConst373;
	float 	fRec77[2];
	FAUSTFLOAT 	fbargraph9;
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
	float 	fConst391;
	float 	fConst392;
	float 	fConst393;
	float 	fConst394;
	float 	fRec90[3];
	float 	fConst395;
	float 	fConst396;
	float 	fConst397;
	float 	fRec89[3];
	float 	fConst398;
	float 	fConst399;
	float 	fConst400;
	float 	fRec88[3];
	float 	fConst401;
	float 	fConst402;
	float 	fConst403;
	float 	fConst404;
	float 	fRec87[3];
	float 	fConst405;
	float 	fConst406;
	float 	fConst407;
	float 	fRec86[3];
	float 	fConst408;
	float 	fConst409;
	float 	fConst410;
	float 	fRec85[3];
	float 	fConst411;
	float 	fRec84[2];
	FAUSTFLOAT 	fbargraph10;
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
	float 	fConst429;
	float 	fConst430;
	float 	fConst431;
	float 	fConst432;
	float 	fRec97[3];
	float 	fConst433;
	float 	fConst434;
	float 	fConst435;
	float 	fRec96[3];
	float 	fConst436;
	float 	fConst437;
	float 	fConst438;
	float 	fRec95[3];
	float 	fConst439;
	float 	fConst440;
	float 	fConst441;
	float 	fConst442;
	float 	fRec94[3];
	float 	fConst443;
	float 	fConst444;
	float 	fConst445;
	float 	fRec93[3];
	float 	fConst446;
	float 	fConst447;
	float 	fConst448;
	float 	fRec92[3];
	float 	fConst449;
	float 	fRec91[2];
	FAUSTFLOAT 	fbargraph11;
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
	float 	fConst467;
	float 	fConst468;
	float 	fConst469;
	float 	fConst470;
	float 	fRec104[3];
	float 	fConst471;
	float 	fConst472;
	float 	fConst473;
	float 	fRec103[3];
	float 	fConst474;
	float 	fConst475;
	float 	fConst476;
	float 	fRec102[3];
	float 	fConst477;
	float 	fConst478;
	float 	fConst479;
	float 	fConst480;
	float 	fRec101[3];
	float 	fConst481;
	float 	fConst482;
	float 	fConst483;
	float 	fRec100[3];
	float 	fConst484;
	float 	fConst485;
	float 	fConst486;
	float 	fRec99[3];
	float 	fConst487;
	float 	fRec98[2];
	FAUSTFLOAT 	fbargraph12;
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
	float 	fConst505;
	float 	fConst506;
	float 	fConst507;
	float 	fConst508;
	float 	fRec111[3];
	float 	fConst509;
	float 	fConst510;
	float 	fConst511;
	float 	fRec110[3];
	float 	fConst512;
	float 	fConst513;
	float 	fConst514;
	float 	fRec109[3];
	float 	fConst515;
	float 	fConst516;
	float 	fConst517;
	float 	fConst518;
	float 	fRec108[3];
	float 	fConst519;
	float 	fConst520;
	float 	fConst521;
	float 	fRec107[3];
	float 	fConst522;
	float 	fConst523;
	float 	fConst524;
	float 	fRec106[3];
	float 	fConst525;
	float 	fRec105[2];
	FAUSTFLOAT 	fbargraph13;
	float 	fConst526;
	float 	fConst527;
	float 	fConst528;
	float 	fConst529;
	float 	fConst530;
	float 	fConst531;
	float 	fConst532;
	float 	fConst533;
	float 	fConst534;
	float 	fConst535;
	float 	fConst536;
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
	float 	fRec118[3];
	float 	fConst547;
	float 	fConst548;
	float 	fConst549;
	float 	fRec117[3];
	float 	fConst550;
	float 	fConst551;
	float 	fConst552;
	float 	fRec116[3];
	float 	fConst553;
	float 	fConst554;
	float 	fConst555;
	float 	fConst556;
	float 	fRec115[3];
	float 	fConst557;
	float 	fConst558;
	float 	fConst559;
	float 	fRec114[3];
	float 	fConst560;
	float 	fConst561;
	float 	fConst562;
	float 	fRec113[3];
	float 	fConst563;
	float 	fRec112[2];
	FAUSTFLOAT 	fbargraph14;
	float 	fConst564;
	float 	fConst565;
	float 	fConst566;
	float 	fConst567;
	float 	fConst568;
	float 	fConst569;
	float 	fConst570;
	float 	fConst571;
	float 	fConst572;
	float 	fConst573;
	float 	fConst574;
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
	float 	fRec125[3];
	float 	fConst585;
	float 	fConst586;
	float 	fConst587;
	float 	fRec124[3];
	float 	fConst588;
	float 	fConst589;
	float 	fConst590;
	float 	fRec123[3];
	float 	fConst591;
	float 	fConst592;
	float 	fConst593;
	float 	fConst594;
	float 	fRec122[3];
	float 	fConst595;
	float 	fConst596;
	float 	fConst597;
	float 	fRec121[3];
	float 	fConst598;
	float 	fConst599;
	float 	fConst600;
	float 	fRec120[3];
	float 	fConst601;
	float 	fRec119[2];
	FAUSTFLOAT 	fbargraph15;
	float 	fConst602;
	float 	fConst603;
	float 	fConst604;
	float 	fConst605;
	float 	fConst606;
	float 	fConst607;
	float 	fConst608;
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
	float 	fRec132[3];
	float 	fConst623;
	float 	fConst624;
	float 	fConst625;
	float 	fRec131[3];
	float 	fConst626;
	float 	fConst627;
	float 	fConst628;
	float 	fRec130[3];
	float 	fConst629;
	float 	fConst630;
	float 	fConst631;
	float 	fConst632;
	float 	fRec129[3];
	float 	fConst633;
	float 	fConst634;
	float 	fConst635;
	float 	fRec128[3];
	float 	fConst636;
	float 	fConst637;
	float 	fConst638;
	float 	fRec127[3];
	float 	fConst639;
	float 	fRec126[2];
	FAUSTFLOAT 	fbargraph16;
	float 	fConst640;
	float 	fConst641;
	float 	fConst642;
	float 	fConst643;
	float 	fConst644;
	float 	fConst645;
	float 	fConst646;
	float 	fConst647;
	float 	fConst648;
	float 	fConst649;
	float 	fConst650;
	float 	fConst651;
	float 	fConst652;
	float 	fConst653;
	float 	fConst654;
	float 	fConst655;
	float 	fConst656;
	float 	fConst657;
	float 	fConst658;
	float 	fConst659;
	float 	fConst660;
	float 	fRec139[3];
	float 	fConst661;
	float 	fConst662;
	float 	fConst663;
	float 	fRec138[3];
	float 	fConst664;
	float 	fConst665;
	float 	fConst666;
	float 	fRec137[3];
	float 	fConst667;
	float 	fConst668;
	float 	fConst669;
	float 	fConst670;
	float 	fRec136[3];
	float 	fConst671;
	float 	fConst672;
	float 	fConst673;
	float 	fRec135[3];
	float 	fConst674;
	float 	fConst675;
	float 	fConst676;
	float 	fRec134[3];
	float 	fConst677;
	float 	fRec133[2];
	FAUSTFLOAT 	fbargraph17;
	float 	fConst678;
	float 	fConst679;
	float 	fConst680;
	float 	fConst681;
	float 	fConst682;
	float 	fConst683;
	float 	fConst684;
	float 	fConst685;
	float 	fConst686;
	float 	fConst687;
	float 	fConst688;
	float 	fConst689;
	float 	fConst690;
	float 	fConst691;
	float 	fConst692;
	float 	fConst693;
	float 	fConst694;
	float 	fConst695;
	float 	fConst696;
	float 	fConst697;
	float 	fConst698;
	float 	fRec146[3];
	float 	fConst699;
	float 	fConst700;
	float 	fConst701;
	float 	fRec145[3];
	float 	fConst702;
	float 	fConst703;
	float 	fConst704;
	float 	fRec144[3];
	float 	fConst705;
	float 	fConst706;
	float 	fConst707;
	float 	fConst708;
	float 	fRec143[3];
	float 	fConst709;
	float 	fConst710;
	float 	fConst711;
	float 	fRec142[3];
	float 	fConst712;
	float 	fConst713;
	float 	fConst714;
	float 	fRec141[3];
	float 	fConst715;
	float 	fRec140[2];
	FAUSTFLOAT 	fbargraph18;
	float 	fConst716;
	float 	fConst717;
	float 	fConst718;
	float 	fConst719;
	float 	fConst720;
	float 	fConst721;
	float 	fConst722;
	float 	fConst723;
	float 	fConst724;
	float 	fRec150[3];
	float 	fConst725;
	float 	fConst726;
	float 	fConst727;
	float 	fRec149[3];
	float 	fConst728;
	float 	fConst729;
	float 	fConst730;
	float 	fRec148[3];
	float 	fConst731;
	float 	fRec147[2];
	FAUSTFLOAT 	fbargraph19;
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
		fConst11 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst3) / fConst2)));
		fConst12 = (0.0017661728399818856f / fConst5);
		fConst13 = (0.00040767818495825777f + fConst12);
		fConst14 = (3.141592653589793f / float(iConst0));
		fslider2 = 8e+03f;
		fConst15 = (1382.300767579509f / float(iConst0));
		fslider3 = 49.0f;
		for (int i=0; i<2; i++) fRec6[i] = 0;
		fslider4 = 2e+02f;
		fslider5 = -2e+01f;
		for (int i=0; i<2; i++) fRec10[i] = 0;
		fcheckbox0 = 0.0;
		fcheckbox1 = 0.0;
		for (int i=0; i<2; i++) iRec12[i] = 0;
		for (int i=0; i<4; i++) fRec11[i] = 0;
		fConst16 = (0.3333333333333333f * iConst0);
		for (int i=0; i<2; i++) fVec0[i] = 0;
		fConst17 = float(iConst0);
		fConst18 = (2.0f / fConst17);
		fslider6 = 0.1f;
		fslider7 = 49.0f;
		for (int i=0; i<2; i++) fRec13[i] = 0;
		for (int i=0; i<2; i++) fRec14[i] = 0;
		for (int i=0; i<2; i++) fVec1[i] = 0;
		fslider8 = -0.1f;
		for (int i=0; i<2; i++) fRec15[i] = 0;
		for (int i=0; i<2; i++) fVec2[i] = 0;
		fslider9 = 0.1f;
		for (int i=0; i<2; i++) fRec16[i] = 0;
		for (int i=0; i<2; i++) fVec3[i] = 0;
		for (int i=0; i<2; i++) fVec4[i] = 0;
		for (int i=0; i<2; i++) fRec9[i] = 0;
		for (int i=0; i<3; i++) fRec8[i] = 0;
		fslider10 = 0.0f;
		for (int i=0; i<2; i++) fRec18[i] = 0;
		for (int i=0; i<3; i++) fRec17[i] = 0;
		fslider11 = 0.0f;
		fConst19 = float(fConst1);
		fConst20 = (1382.300767579509f * fConst19);
		fslider12 = 4e+01f;
		fConst21 = (2764.601535159018f * fConst19);
		for (int i=0; i<3; i++) fRec7[i] = 0;
		for (int i=0; i<2; i++) fVec5[i] = 0;
		for (int i=0; i<2; i++) fRec5[i] = 0;
		for (int i=0; i<3; i++) fRec4[i] = 0;
		fslider13 = 0.0f;
		for (int i=0; i<2; i++) fRec20[i] = 0;
		for (int i=0; i<3; i++) fRec19[i] = 0;
		fConst22 = (4.076781969643807f + ((fConst3 - 3.1897274020965583f) / fConst2));
		fConst23 = (1.0f / fConst5);
		fConst24 = (2 * (4.076781969643807f - fConst23));
		for (int i=0; i<3; i++) fRec3[i] = 0;
		fConst25 = (2 * (0.00040767818495825777f - fConst12));
		fConst26 = (1.450071084655647f + ((fConst3 - 0.7431304601070396f) / fConst2));
		fConst27 = (2 * (1.450071084655647f - fConst23));
		for (int i=0; i<3; i++) fRec2[i] = 0;
		fConst28 = (2 * (1.450071084655647f - fConst9));
		fConst29 = (0.9351401670315425f + ((fConst3 - 0.157482159302087f) / fConst2));
		fConst30 = (2 * (0.9351401670315425f - fConst23));
		for (int i=0; i<3; i++) fRec1[i] = 0;
		fConst31 = (2 * (0.9351401670315425f - fConst6));
		for (int i=0; i<2; i++) fRec0[i] = 0;
		fConst32 = tanf((22214.414690791833f / float(iConst0)));
		fConst33 = (1.0f / fConst32);
		fConst34 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst33) / fConst32)));
		fConst35 = faustpower<2>(fConst32);
		fConst36 = (50.063807016150385f / fConst35);
		fConst37 = (0.9351401670315425f + fConst36);
		fConst38 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst33) / fConst32)));
		fConst39 = (11.052052171507189f / fConst35);
		fConst40 = (1.450071084655647f + fConst39);
		fConst41 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst33) / fConst32)));
		fConst42 = (0.0017661728399818856f / fConst35);
		fConst43 = (0.00040767818495825777f + fConst42);
		fConst44 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst3) / fConst2)));
		fConst45 = (53.53615295455673f + fConst23);
		fConst46 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst3) / fConst2)));
		fConst47 = (7.621731298870603f + fConst23);
		fConst48 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst3) / fConst2)));
		fConst49 = (9.9999997055e-05f / fConst5);
		fConst50 = (0.000433227200555f + fConst49);
		fConst51 = (0.24529150870616f + ((fConst3 - 0.782413046821645f) / fConst2));
		fConst52 = (2 * (0.24529150870616f - fConst23));
		for (int i=0; i<3; i++) fRec27[i] = 0;
		fConst53 = (2 * (0.000433227200555f - fConst49));
		fConst54 = (0.689621364484675f + ((fConst3 - 0.512478641889141f) / fConst2));
		fConst55 = (2 * (0.689621364484675f - fConst23));
		for (int i=0; i<3; i++) fRec26[i] = 0;
		fConst56 = (2 * (7.621731298870603f - fConst23));
		fConst57 = (1.069358407707312f + ((fConst3 - 0.168404871113589f) / fConst2));
		fConst58 = (2 * (1.069358407707312f - fConst23));
		for (int i=0; i<3; i++) fRec25[i] = 0;
		fConst59 = (2 * (53.53615295455673f - fConst23));
		fConst60 = (4.076781969643807f + ((fConst33 - 3.1897274020965583f) / fConst32));
		fConst61 = (1.0f / fConst35);
		fConst62 = (2 * (4.076781969643807f - fConst61));
		for (int i=0; i<3; i++) fRec24[i] = 0;
		fConst63 = (2 * (0.00040767818495825777f - fConst42));
		fConst64 = (1.450071084655647f + ((fConst33 - 0.7431304601070396f) / fConst32));
		fConst65 = (2 * (1.450071084655647f - fConst61));
		for (int i=0; i<3; i++) fRec23[i] = 0;
		fConst66 = (2 * (1.450071084655647f - fConst39));
		fConst67 = (0.9351401670315425f + ((fConst33 - 0.157482159302087f) / fConst32));
		fConst68 = (2 * (0.9351401670315425f - fConst61));
		for (int i=0; i<3; i++) fRec22[i] = 0;
		fConst69 = (2 * (0.9351401670315425f - fConst36));
		for (int i=0; i<2; i++) fRec21[i] = 0;
		fConst70 = tanf((15707.963267948966f / float(iConst0)));
		fConst71 = (1.0f / fConst70);
		fConst72 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst71) / fConst70)));
		fConst73 = faustpower<2>(fConst70);
		fConst74 = (50.063807016150385f / fConst73);
		fConst75 = (0.9351401670315425f + fConst74);
		fConst76 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst71) / fConst70)));
		fConst77 = (11.052052171507189f / fConst73);
		fConst78 = (1.450071084655647f + fConst77);
		fConst79 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst71) / fConst70)));
		fConst80 = (0.0017661728399818856f / fConst73);
		fConst81 = (0.00040767818495825777f + fConst80);
		fConst82 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst33) / fConst32)));
		fConst83 = (53.53615295455673f + fConst61);
		fConst84 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst33) / fConst32)));
		fConst85 = (7.621731298870603f + fConst61);
		fConst86 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst33) / fConst32)));
		fConst87 = (9.9999997055e-05f / fConst35);
		fConst88 = (0.000433227200555f + fConst87);
		fConst89 = (0.24529150870616f + ((fConst33 - 0.782413046821645f) / fConst32));
		fConst90 = (2 * (0.24529150870616f - fConst61));
		for (int i=0; i<3; i++) fRec34[i] = 0;
		fConst91 = (2 * (0.000433227200555f - fConst87));
		fConst92 = (0.689621364484675f + ((fConst33 - 0.512478641889141f) / fConst32));
		fConst93 = (2 * (0.689621364484675f - fConst61));
		for (int i=0; i<3; i++) fRec33[i] = 0;
		fConst94 = (2 * (7.621731298870603f - fConst61));
		fConst95 = (1.069358407707312f + ((fConst33 - 0.168404871113589f) / fConst32));
		fConst96 = (2 * (1.069358407707312f - fConst61));
		for (int i=0; i<3; i++) fRec32[i] = 0;
		fConst97 = (2 * (53.53615295455673f - fConst61));
		fConst98 = (4.076781969643807f + ((fConst71 - 3.1897274020965583f) / fConst70));
		fConst99 = (1.0f / fConst73);
		fConst100 = (2 * (4.076781969643807f - fConst99));
		for (int i=0; i<3; i++) fRec31[i] = 0;
		fConst101 = (2 * (0.00040767818495825777f - fConst80));
		fConst102 = (1.450071084655647f + ((fConst71 - 0.7431304601070396f) / fConst70));
		fConst103 = (2 * (1.450071084655647f - fConst99));
		for (int i=0; i<3; i++) fRec30[i] = 0;
		fConst104 = (2 * (1.450071084655647f - fConst77));
		fConst105 = (0.9351401670315425f + ((fConst71 - 0.157482159302087f) / fConst70));
		fConst106 = (2 * (0.9351401670315425f - fConst99));
		for (int i=0; i<3; i++) fRec29[i] = 0;
		fConst107 = (2 * (0.9351401670315425f - fConst74));
		for (int i=0; i<2; i++) fRec28[i] = 0;
		fConst108 = tanf((11107.207345395916f / float(iConst0)));
		fConst109 = (1.0f / fConst108);
		fConst110 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst109) / fConst108)));
		fConst111 = faustpower<2>(fConst108);
		fConst112 = (50.063807016150385f / fConst111);
		fConst113 = (0.9351401670315425f + fConst112);
		fConst114 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst109) / fConst108)));
		fConst115 = (11.052052171507189f / fConst111);
		fConst116 = (1.450071084655647f + fConst115);
		fConst117 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst109) / fConst108)));
		fConst118 = (0.0017661728399818856f / fConst111);
		fConst119 = (0.00040767818495825777f + fConst118);
		fConst120 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst71) / fConst70)));
		fConst121 = (53.53615295455673f + fConst99);
		fConst122 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst71) / fConst70)));
		fConst123 = (7.621731298870603f + fConst99);
		fConst124 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst71) / fConst70)));
		fConst125 = (9.9999997055e-05f / fConst73);
		fConst126 = (0.000433227200555f + fConst125);
		fConst127 = (0.24529150870616f + ((fConst71 - 0.782413046821645f) / fConst70));
		fConst128 = (2 * (0.24529150870616f - fConst99));
		for (int i=0; i<3; i++) fRec41[i] = 0;
		fConst129 = (2 * (0.000433227200555f - fConst125));
		fConst130 = (0.689621364484675f + ((fConst71 - 0.512478641889141f) / fConst70));
		fConst131 = (2 * (0.689621364484675f - fConst99));
		for (int i=0; i<3; i++) fRec40[i] = 0;
		fConst132 = (2 * (7.621731298870603f - fConst99));
		fConst133 = (1.069358407707312f + ((fConst71 - 0.168404871113589f) / fConst70));
		fConst134 = (2 * (1.069358407707312f - fConst99));
		for (int i=0; i<3; i++) fRec39[i] = 0;
		fConst135 = (2 * (53.53615295455673f - fConst99));
		fConst136 = (4.076781969643807f + ((fConst109 - 3.1897274020965583f) / fConst108));
		fConst137 = (1.0f / fConst111);
		fConst138 = (2 * (4.076781969643807f - fConst137));
		for (int i=0; i<3; i++) fRec38[i] = 0;
		fConst139 = (2 * (0.00040767818495825777f - fConst118));
		fConst140 = (1.450071084655647f + ((fConst109 - 0.7431304601070396f) / fConst108));
		fConst141 = (2 * (1.450071084655647f - fConst137));
		for (int i=0; i<3; i++) fRec37[i] = 0;
		fConst142 = (2 * (1.450071084655647f - fConst115));
		fConst143 = (0.9351401670315425f + ((fConst109 - 0.157482159302087f) / fConst108));
		fConst144 = (2 * (0.9351401670315425f - fConst137));
		for (int i=0; i<3; i++) fRec36[i] = 0;
		fConst145 = (2 * (0.9351401670315425f - fConst112));
		for (int i=0; i<2; i++) fRec35[i] = 0;
		fConst146 = tanf((7853.981633974483f / float(iConst0)));
		fConst147 = (1.0f / fConst146);
		fConst148 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst147) / fConst146)));
		fConst149 = faustpower<2>(fConst146);
		fConst150 = (50.063807016150385f / fConst149);
		fConst151 = (0.9351401670315425f + fConst150);
		fConst152 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst147) / fConst146)));
		fConst153 = (11.052052171507189f / fConst149);
		fConst154 = (1.450071084655647f + fConst153);
		fConst155 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst147) / fConst146)));
		fConst156 = (0.0017661728399818856f / fConst149);
		fConst157 = (0.00040767818495825777f + fConst156);
		fConst158 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst109) / fConst108)));
		fConst159 = (53.53615295455673f + fConst137);
		fConst160 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst109) / fConst108)));
		fConst161 = (7.621731298870603f + fConst137);
		fConst162 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst109) / fConst108)));
		fConst163 = (9.9999997055e-05f / fConst111);
		fConst164 = (0.000433227200555f + fConst163);
		fConst165 = (0.24529150870616f + ((fConst109 - 0.782413046821645f) / fConst108));
		fConst166 = (2 * (0.24529150870616f - fConst137));
		for (int i=0; i<3; i++) fRec48[i] = 0;
		fConst167 = (2 * (0.000433227200555f - fConst163));
		fConst168 = (0.689621364484675f + ((fConst109 - 0.512478641889141f) / fConst108));
		fConst169 = (2 * (0.689621364484675f - fConst137));
		for (int i=0; i<3; i++) fRec47[i] = 0;
		fConst170 = (2 * (7.621731298870603f - fConst137));
		fConst171 = (1.069358407707312f + ((fConst109 - 0.168404871113589f) / fConst108));
		fConst172 = (2 * (1.069358407707312f - fConst137));
		for (int i=0; i<3; i++) fRec46[i] = 0;
		fConst173 = (2 * (53.53615295455673f - fConst137));
		fConst174 = (4.076781969643807f + ((fConst147 - 3.1897274020965583f) / fConst146));
		fConst175 = (1.0f / fConst149);
		fConst176 = (2 * (4.076781969643807f - fConst175));
		for (int i=0; i<3; i++) fRec45[i] = 0;
		fConst177 = (2 * (0.00040767818495825777f - fConst156));
		fConst178 = (1.450071084655647f + ((fConst147 - 0.7431304601070396f) / fConst146));
		fConst179 = (2 * (1.450071084655647f - fConst175));
		for (int i=0; i<3; i++) fRec44[i] = 0;
		fConst180 = (2 * (1.450071084655647f - fConst153));
		fConst181 = (0.9351401670315425f + ((fConst147 - 0.157482159302087f) / fConst146));
		fConst182 = (2 * (0.9351401670315425f - fConst175));
		for (int i=0; i<3; i++) fRec43[i] = 0;
		fConst183 = (2 * (0.9351401670315425f - fConst150));
		for (int i=0; i<2; i++) fRec42[i] = 0;
		fConst184 = tanf((5553.603672697958f / float(iConst0)));
		fConst185 = (1.0f / fConst184);
		fConst186 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst185) / fConst184)));
		fConst187 = faustpower<2>(fConst184);
		fConst188 = (50.063807016150385f / fConst187);
		fConst189 = (0.9351401670315425f + fConst188);
		fConst190 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst185) / fConst184)));
		fConst191 = (11.052052171507189f / fConst187);
		fConst192 = (1.450071084655647f + fConst191);
		fConst193 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst185) / fConst184)));
		fConst194 = (0.0017661728399818856f / fConst187);
		fConst195 = (0.00040767818495825777f + fConst194);
		fConst196 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst147) / fConst146)));
		fConst197 = (53.53615295455673f + fConst175);
		fConst198 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst147) / fConst146)));
		fConst199 = (7.621731298870603f + fConst175);
		fConst200 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst147) / fConst146)));
		fConst201 = (9.9999997055e-05f / fConst149);
		fConst202 = (0.000433227200555f + fConst201);
		fConst203 = (0.24529150870616f + ((fConst147 - 0.782413046821645f) / fConst146));
		fConst204 = (2 * (0.24529150870616f - fConst175));
		for (int i=0; i<3; i++) fRec55[i] = 0;
		fConst205 = (2 * (0.000433227200555f - fConst201));
		fConst206 = (0.689621364484675f + ((fConst147 - 0.512478641889141f) / fConst146));
		fConst207 = (2 * (0.689621364484675f - fConst175));
		for (int i=0; i<3; i++) fRec54[i] = 0;
		fConst208 = (2 * (7.621731298870603f - fConst175));
		fConst209 = (1.069358407707312f + ((fConst147 - 0.168404871113589f) / fConst146));
		fConst210 = (2 * (1.069358407707312f - fConst175));
		for (int i=0; i<3; i++) fRec53[i] = 0;
		fConst211 = (2 * (53.53615295455673f - fConst175));
		fConst212 = (4.076781969643807f + ((fConst185 - 3.1897274020965583f) / fConst184));
		fConst213 = (1.0f / fConst187);
		fConst214 = (2 * (4.076781969643807f - fConst213));
		for (int i=0; i<3; i++) fRec52[i] = 0;
		fConst215 = (2 * (0.00040767818495825777f - fConst194));
		fConst216 = (1.450071084655647f + ((fConst185 - 0.7431304601070396f) / fConst184));
		fConst217 = (2 * (1.450071084655647f - fConst213));
		for (int i=0; i<3; i++) fRec51[i] = 0;
		fConst218 = (2 * (1.450071084655647f - fConst191));
		fConst219 = (0.9351401670315425f + ((fConst185 - 0.157482159302087f) / fConst184));
		fConst220 = (2 * (0.9351401670315425f - fConst213));
		for (int i=0; i<3; i++) fRec50[i] = 0;
		fConst221 = (2 * (0.9351401670315425f - fConst188));
		for (int i=0; i<2; i++) fRec49[i] = 0;
		fConst222 = tanf((3926.9908169872415f / float(iConst0)));
		fConst223 = (1.0f / fConst222);
		fConst224 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst223) / fConst222)));
		fConst225 = faustpower<2>(fConst222);
		fConst226 = (50.063807016150385f / fConst225);
		fConst227 = (0.9351401670315425f + fConst226);
		fConst228 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst223) / fConst222)));
		fConst229 = (11.052052171507189f / fConst225);
		fConst230 = (1.450071084655647f + fConst229);
		fConst231 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst223) / fConst222)));
		fConst232 = (0.0017661728399818856f / fConst225);
		fConst233 = (0.00040767818495825777f + fConst232);
		fConst234 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst185) / fConst184)));
		fConst235 = (53.53615295455673f + fConst213);
		fConst236 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst185) / fConst184)));
		fConst237 = (7.621731298870603f + fConst213);
		fConst238 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst185) / fConst184)));
		fConst239 = (9.9999997055e-05f / fConst187);
		fConst240 = (0.000433227200555f + fConst239);
		fConst241 = (0.24529150870616f + ((fConst185 - 0.782413046821645f) / fConst184));
		fConst242 = (2 * (0.24529150870616f - fConst213));
		for (int i=0; i<3; i++) fRec62[i] = 0;
		fConst243 = (2 * (0.000433227200555f - fConst239));
		fConst244 = (0.689621364484675f + ((fConst185 - 0.512478641889141f) / fConst184));
		fConst245 = (2 * (0.689621364484675f - fConst213));
		for (int i=0; i<3; i++) fRec61[i] = 0;
		fConst246 = (2 * (7.621731298870603f - fConst213));
		fConst247 = (1.069358407707312f + ((fConst185 - 0.168404871113589f) / fConst184));
		fConst248 = (2 * (1.069358407707312f - fConst213));
		for (int i=0; i<3; i++) fRec60[i] = 0;
		fConst249 = (2 * (53.53615295455673f - fConst213));
		fConst250 = (4.076781969643807f + ((fConst223 - 3.1897274020965583f) / fConst222));
		fConst251 = (1.0f / fConst225);
		fConst252 = (2 * (4.076781969643807f - fConst251));
		for (int i=0; i<3; i++) fRec59[i] = 0;
		fConst253 = (2 * (0.00040767818495825777f - fConst232));
		fConst254 = (1.450071084655647f + ((fConst223 - 0.7431304601070396f) / fConst222));
		fConst255 = (2 * (1.450071084655647f - fConst251));
		for (int i=0; i<3; i++) fRec58[i] = 0;
		fConst256 = (2 * (1.450071084655647f - fConst229));
		fConst257 = (0.9351401670315425f + ((fConst223 - 0.157482159302087f) / fConst222));
		fConst258 = (2 * (0.9351401670315425f - fConst251));
		for (int i=0; i<3; i++) fRec57[i] = 0;
		fConst259 = (2 * (0.9351401670315425f - fConst226));
		for (int i=0; i<2; i++) fRec56[i] = 0;
		fConst260 = tanf((2776.801836348979f / float(iConst0)));
		fConst261 = (1.0f / fConst260);
		fConst262 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst261) / fConst260)));
		fConst263 = faustpower<2>(fConst260);
		fConst264 = (50.063807016150385f / fConst263);
		fConst265 = (0.9351401670315425f + fConst264);
		fConst266 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst261) / fConst260)));
		fConst267 = (11.052052171507189f / fConst263);
		fConst268 = (1.450071084655647f + fConst267);
		fConst269 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst261) / fConst260)));
		fConst270 = (0.0017661728399818856f / fConst263);
		fConst271 = (0.00040767818495825777f + fConst270);
		fConst272 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst223) / fConst222)));
		fConst273 = (53.53615295455673f + fConst251);
		fConst274 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst223) / fConst222)));
		fConst275 = (7.621731298870603f + fConst251);
		fConst276 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst223) / fConst222)));
		fConst277 = (9.9999997055e-05f / fConst225);
		fConst278 = (0.000433227200555f + fConst277);
		fConst279 = (0.24529150870616f + ((fConst223 - 0.782413046821645f) / fConst222));
		fConst280 = (2 * (0.24529150870616f - fConst251));
		for (int i=0; i<3; i++) fRec69[i] = 0;
		fConst281 = (2 * (0.000433227200555f - fConst277));
		fConst282 = (0.689621364484675f + ((fConst223 - 0.512478641889141f) / fConst222));
		fConst283 = (2 * (0.689621364484675f - fConst251));
		for (int i=0; i<3; i++) fRec68[i] = 0;
		fConst284 = (2 * (7.621731298870603f - fConst251));
		fConst285 = (1.069358407707312f + ((fConst223 - 0.168404871113589f) / fConst222));
		fConst286 = (2 * (1.069358407707312f - fConst251));
		for (int i=0; i<3; i++) fRec67[i] = 0;
		fConst287 = (2 * (53.53615295455673f - fConst251));
		fConst288 = (4.076781969643807f + ((fConst261 - 3.1897274020965583f) / fConst260));
		fConst289 = (1.0f / fConst263);
		fConst290 = (2 * (4.076781969643807f - fConst289));
		for (int i=0; i<3; i++) fRec66[i] = 0;
		fConst291 = (2 * (0.00040767818495825777f - fConst270));
		fConst292 = (1.450071084655647f + ((fConst261 - 0.7431304601070396f) / fConst260));
		fConst293 = (2 * (1.450071084655647f - fConst289));
		for (int i=0; i<3; i++) fRec65[i] = 0;
		fConst294 = (2 * (1.450071084655647f - fConst267));
		fConst295 = (0.9351401670315425f + ((fConst261 - 0.157482159302087f) / fConst260));
		fConst296 = (2 * (0.9351401670315425f - fConst289));
		for (int i=0; i<3; i++) fRec64[i] = 0;
		fConst297 = (2 * (0.9351401670315425f - fConst264));
		for (int i=0; i<2; i++) fRec63[i] = 0;
		fConst298 = tanf((1963.4954084936207f / float(iConst0)));
		fConst299 = (1.0f / fConst298);
		fConst300 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst299) / fConst298)));
		fConst301 = faustpower<2>(fConst298);
		fConst302 = (50.063807016150385f / fConst301);
		fConst303 = (0.9351401670315425f + fConst302);
		fConst304 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst299) / fConst298)));
		fConst305 = (11.052052171507189f / fConst301);
		fConst306 = (1.450071084655647f + fConst305);
		fConst307 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst299) / fConst298)));
		fConst308 = (0.0017661728399818856f / fConst301);
		fConst309 = (0.00040767818495825777f + fConst308);
		fConst310 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst261) / fConst260)));
		fConst311 = (53.53615295455673f + fConst289);
		fConst312 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst261) / fConst260)));
		fConst313 = (7.621731298870603f + fConst289);
		fConst314 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst261) / fConst260)));
		fConst315 = (9.9999997055e-05f / fConst263);
		fConst316 = (0.000433227200555f + fConst315);
		fConst317 = (0.24529150870616f + ((fConst261 - 0.782413046821645f) / fConst260));
		fConst318 = (2 * (0.24529150870616f - fConst289));
		for (int i=0; i<3; i++) fRec76[i] = 0;
		fConst319 = (2 * (0.000433227200555f - fConst315));
		fConst320 = (0.689621364484675f + ((fConst261 - 0.512478641889141f) / fConst260));
		fConst321 = (2 * (0.689621364484675f - fConst289));
		for (int i=0; i<3; i++) fRec75[i] = 0;
		fConst322 = (2 * (7.621731298870603f - fConst289));
		fConst323 = (1.069358407707312f + ((fConst261 - 0.168404871113589f) / fConst260));
		fConst324 = (2 * (1.069358407707312f - fConst289));
		for (int i=0; i<3; i++) fRec74[i] = 0;
		fConst325 = (2 * (53.53615295455673f - fConst289));
		fConst326 = (4.076781969643807f + ((fConst299 - 3.1897274020965583f) / fConst298));
		fConst327 = (1.0f / fConst301);
		fConst328 = (2 * (4.076781969643807f - fConst327));
		for (int i=0; i<3; i++) fRec73[i] = 0;
		fConst329 = (2 * (0.00040767818495825777f - fConst308));
		fConst330 = (1.450071084655647f + ((fConst299 - 0.7431304601070396f) / fConst298));
		fConst331 = (2 * (1.450071084655647f - fConst327));
		for (int i=0; i<3; i++) fRec72[i] = 0;
		fConst332 = (2 * (1.450071084655647f - fConst305));
		fConst333 = (0.9351401670315425f + ((fConst299 - 0.157482159302087f) / fConst298));
		fConst334 = (2 * (0.9351401670315425f - fConst327));
		for (int i=0; i<3; i++) fRec71[i] = 0;
		fConst335 = (2 * (0.9351401670315425f - fConst302));
		for (int i=0; i<2; i++) fRec70[i] = 0;
		fConst336 = tanf((1388.4009181744896f / float(iConst0)));
		fConst337 = (1.0f / fConst336);
		fConst338 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst337) / fConst336)));
		fConst339 = faustpower<2>(fConst336);
		fConst340 = (50.063807016150385f / fConst339);
		fConst341 = (0.9351401670315425f + fConst340);
		fConst342 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst337) / fConst336)));
		fConst343 = (11.052052171507189f / fConst339);
		fConst344 = (1.450071084655647f + fConst343);
		fConst345 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst337) / fConst336)));
		fConst346 = (0.0017661728399818856f / fConst339);
		fConst347 = (0.00040767818495825777f + fConst346);
		fConst348 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst299) / fConst298)));
		fConst349 = (53.53615295455673f + fConst327);
		fConst350 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst299) / fConst298)));
		fConst351 = (7.621731298870603f + fConst327);
		fConst352 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst299) / fConst298)));
		fConst353 = (9.9999997055e-05f / fConst301);
		fConst354 = (0.000433227200555f + fConst353);
		fConst355 = (0.24529150870616f + ((fConst299 - 0.782413046821645f) / fConst298));
		fConst356 = (2 * (0.24529150870616f - fConst327));
		for (int i=0; i<3; i++) fRec83[i] = 0;
		fConst357 = (2 * (0.000433227200555f - fConst353));
		fConst358 = (0.689621364484675f + ((fConst299 - 0.512478641889141f) / fConst298));
		fConst359 = (2 * (0.689621364484675f - fConst327));
		for (int i=0; i<3; i++) fRec82[i] = 0;
		fConst360 = (2 * (7.621731298870603f - fConst327));
		fConst361 = (1.069358407707312f + ((fConst299 - 0.168404871113589f) / fConst298));
		fConst362 = (2 * (1.069358407707312f - fConst327));
		for (int i=0; i<3; i++) fRec81[i] = 0;
		fConst363 = (2 * (53.53615295455673f - fConst327));
		fConst364 = (4.076781969643807f + ((fConst337 - 3.1897274020965583f) / fConst336));
		fConst365 = (1.0f / fConst339);
		fConst366 = (2 * (4.076781969643807f - fConst365));
		for (int i=0; i<3; i++) fRec80[i] = 0;
		fConst367 = (2 * (0.00040767818495825777f - fConst346));
		fConst368 = (1.450071084655647f + ((fConst337 - 0.7431304601070396f) / fConst336));
		fConst369 = (2 * (1.450071084655647f - fConst365));
		for (int i=0; i<3; i++) fRec79[i] = 0;
		fConst370 = (2 * (1.450071084655647f - fConst343));
		fConst371 = (0.9351401670315425f + ((fConst337 - 0.157482159302087f) / fConst336));
		fConst372 = (2 * (0.9351401670315425f - fConst365));
		for (int i=0; i<3; i++) fRec78[i] = 0;
		fConst373 = (2 * (0.9351401670315425f - fConst340));
		for (int i=0; i<2; i++) fRec77[i] = 0;
		fConst374 = tanf((981.7477042468104f / float(iConst0)));
		fConst375 = (1.0f / fConst374);
		fConst376 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst375) / fConst374)));
		fConst377 = faustpower<2>(fConst374);
		fConst378 = (50.063807016150385f / fConst377);
		fConst379 = (0.9351401670315425f + fConst378);
		fConst380 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst375) / fConst374)));
		fConst381 = (11.052052171507189f / fConst377);
		fConst382 = (1.450071084655647f + fConst381);
		fConst383 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst375) / fConst374)));
		fConst384 = (0.0017661728399818856f / fConst377);
		fConst385 = (0.00040767818495825777f + fConst384);
		fConst386 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst337) / fConst336)));
		fConst387 = (53.53615295455673f + fConst365);
		fConst388 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst337) / fConst336)));
		fConst389 = (7.621731298870603f + fConst365);
		fConst390 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst337) / fConst336)));
		fConst391 = (9.9999997055e-05f / fConst339);
		fConst392 = (0.000433227200555f + fConst391);
		fConst393 = (0.24529150870616f + ((fConst337 - 0.782413046821645f) / fConst336));
		fConst394 = (2 * (0.24529150870616f - fConst365));
		for (int i=0; i<3; i++) fRec90[i] = 0;
		fConst395 = (2 * (0.000433227200555f - fConst391));
		fConst396 = (0.689621364484675f + ((fConst337 - 0.512478641889141f) / fConst336));
		fConst397 = (2 * (0.689621364484675f - fConst365));
		for (int i=0; i<3; i++) fRec89[i] = 0;
		fConst398 = (2 * (7.621731298870603f - fConst365));
		fConst399 = (1.069358407707312f + ((fConst337 - 0.168404871113589f) / fConst336));
		fConst400 = (2 * (1.069358407707312f - fConst365));
		for (int i=0; i<3; i++) fRec88[i] = 0;
		fConst401 = (2 * (53.53615295455673f - fConst365));
		fConst402 = (4.076781969643807f + ((fConst375 - 3.1897274020965583f) / fConst374));
		fConst403 = (1.0f / fConst377);
		fConst404 = (2 * (4.076781969643807f - fConst403));
		for (int i=0; i<3; i++) fRec87[i] = 0;
		fConst405 = (2 * (0.00040767818495825777f - fConst384));
		fConst406 = (1.450071084655647f + ((fConst375 - 0.7431304601070396f) / fConst374));
		fConst407 = (2 * (1.450071084655647f - fConst403));
		for (int i=0; i<3; i++) fRec86[i] = 0;
		fConst408 = (2 * (1.450071084655647f - fConst381));
		fConst409 = (0.9351401670315425f + ((fConst375 - 0.157482159302087f) / fConst374));
		fConst410 = (2 * (0.9351401670315425f - fConst403));
		for (int i=0; i<3; i++) fRec85[i] = 0;
		fConst411 = (2 * (0.9351401670315425f - fConst378));
		for (int i=0; i<2; i++) fRec84[i] = 0;
		fConst412 = tanf((694.2004590872448f / float(iConst0)));
		fConst413 = (1.0f / fConst412);
		fConst414 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst413) / fConst412)));
		fConst415 = faustpower<2>(fConst412);
		fConst416 = (50.063807016150385f / fConst415);
		fConst417 = (0.9351401670315425f + fConst416);
		fConst418 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst413) / fConst412)));
		fConst419 = (11.052052171507189f / fConst415);
		fConst420 = (1.450071084655647f + fConst419);
		fConst421 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst413) / fConst412)));
		fConst422 = (0.0017661728399818856f / fConst415);
		fConst423 = (0.00040767818495825777f + fConst422);
		fConst424 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst375) / fConst374)));
		fConst425 = (53.53615295455673f + fConst403);
		fConst426 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst375) / fConst374)));
		fConst427 = (7.621731298870603f + fConst403);
		fConst428 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst375) / fConst374)));
		fConst429 = (9.9999997055e-05f / fConst377);
		fConst430 = (0.000433227200555f + fConst429);
		fConst431 = (0.24529150870616f + ((fConst375 - 0.782413046821645f) / fConst374));
		fConst432 = (2 * (0.24529150870616f - fConst403));
		for (int i=0; i<3; i++) fRec97[i] = 0;
		fConst433 = (2 * (0.000433227200555f - fConst429));
		fConst434 = (0.689621364484675f + ((fConst375 - 0.512478641889141f) / fConst374));
		fConst435 = (2 * (0.689621364484675f - fConst403));
		for (int i=0; i<3; i++) fRec96[i] = 0;
		fConst436 = (2 * (7.621731298870603f - fConst403));
		fConst437 = (1.069358407707312f + ((fConst375 - 0.168404871113589f) / fConst374));
		fConst438 = (2 * (1.069358407707312f - fConst403));
		for (int i=0; i<3; i++) fRec95[i] = 0;
		fConst439 = (2 * (53.53615295455673f - fConst403));
		fConst440 = (4.076781969643807f + ((fConst413 - 3.1897274020965583f) / fConst412));
		fConst441 = (1.0f / fConst415);
		fConst442 = (2 * (4.076781969643807f - fConst441));
		for (int i=0; i<3; i++) fRec94[i] = 0;
		fConst443 = (2 * (0.00040767818495825777f - fConst422));
		fConst444 = (1.450071084655647f + ((fConst413 - 0.7431304601070396f) / fConst412));
		fConst445 = (2 * (1.450071084655647f - fConst441));
		for (int i=0; i<3; i++) fRec93[i] = 0;
		fConst446 = (2 * (1.450071084655647f - fConst419));
		fConst447 = (0.9351401670315425f + ((fConst413 - 0.157482159302087f) / fConst412));
		fConst448 = (2 * (0.9351401670315425f - fConst441));
		for (int i=0; i<3; i++) fRec92[i] = 0;
		fConst449 = (2 * (0.9351401670315425f - fConst416));
		for (int i=0; i<2; i++) fRec91[i] = 0;
		fConst450 = tanf((490.8738521234052f / float(iConst0)));
		fConst451 = (1.0f / fConst450);
		fConst452 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst451) / fConst450)));
		fConst453 = faustpower<2>(fConst450);
		fConst454 = (50.063807016150385f / fConst453);
		fConst455 = (0.9351401670315425f + fConst454);
		fConst456 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst451) / fConst450)));
		fConst457 = (11.052052171507189f / fConst453);
		fConst458 = (1.450071084655647f + fConst457);
		fConst459 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst451) / fConst450)));
		fConst460 = (0.0017661728399818856f / fConst453);
		fConst461 = (0.00040767818495825777f + fConst460);
		fConst462 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst413) / fConst412)));
		fConst463 = (53.53615295455673f + fConst441);
		fConst464 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst413) / fConst412)));
		fConst465 = (7.621731298870603f + fConst441);
		fConst466 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst413) / fConst412)));
		fConst467 = (9.9999997055e-05f / fConst415);
		fConst468 = (0.000433227200555f + fConst467);
		fConst469 = (0.24529150870616f + ((fConst413 - 0.782413046821645f) / fConst412));
		fConst470 = (2 * (0.24529150870616f - fConst441));
		for (int i=0; i<3; i++) fRec104[i] = 0;
		fConst471 = (2 * (0.000433227200555f - fConst467));
		fConst472 = (0.689621364484675f + ((fConst413 - 0.512478641889141f) / fConst412));
		fConst473 = (2 * (0.689621364484675f - fConst441));
		for (int i=0; i<3; i++) fRec103[i] = 0;
		fConst474 = (2 * (7.621731298870603f - fConst441));
		fConst475 = (1.069358407707312f + ((fConst413 - 0.168404871113589f) / fConst412));
		fConst476 = (2 * (1.069358407707312f - fConst441));
		for (int i=0; i<3; i++) fRec102[i] = 0;
		fConst477 = (2 * (53.53615295455673f - fConst441));
		fConst478 = (4.076781969643807f + ((fConst451 - 3.1897274020965583f) / fConst450));
		fConst479 = (1.0f / fConst453);
		fConst480 = (2 * (4.076781969643807f - fConst479));
		for (int i=0; i<3; i++) fRec101[i] = 0;
		fConst481 = (2 * (0.00040767818495825777f - fConst460));
		fConst482 = (1.450071084655647f + ((fConst451 - 0.7431304601070396f) / fConst450));
		fConst483 = (2 * (1.450071084655647f - fConst479));
		for (int i=0; i<3; i++) fRec100[i] = 0;
		fConst484 = (2 * (1.450071084655647f - fConst457));
		fConst485 = (0.9351401670315425f + ((fConst451 - 0.157482159302087f) / fConst450));
		fConst486 = (2 * (0.9351401670315425f - fConst479));
		for (int i=0; i<3; i++) fRec99[i] = 0;
		fConst487 = (2 * (0.9351401670315425f - fConst454));
		for (int i=0; i<2; i++) fRec98[i] = 0;
		fConst488 = tanf((347.1002295436224f / float(iConst0)));
		fConst489 = (1.0f / fConst488);
		fConst490 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst489) / fConst488)));
		fConst491 = faustpower<2>(fConst488);
		fConst492 = (50.063807016150385f / fConst491);
		fConst493 = (0.9351401670315425f + fConst492);
		fConst494 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst489) / fConst488)));
		fConst495 = (11.052052171507189f / fConst491);
		fConst496 = (1.450071084655647f + fConst495);
		fConst497 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst489) / fConst488)));
		fConst498 = (0.0017661728399818856f / fConst491);
		fConst499 = (0.00040767818495825777f + fConst498);
		fConst500 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst451) / fConst450)));
		fConst501 = (53.53615295455673f + fConst479);
		fConst502 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst451) / fConst450)));
		fConst503 = (7.621731298870603f + fConst479);
		fConst504 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst451) / fConst450)));
		fConst505 = (9.9999997055e-05f / fConst453);
		fConst506 = (0.000433227200555f + fConst505);
		fConst507 = (0.24529150870616f + ((fConst451 - 0.782413046821645f) / fConst450));
		fConst508 = (2 * (0.24529150870616f - fConst479));
		for (int i=0; i<3; i++) fRec111[i] = 0;
		fConst509 = (2 * (0.000433227200555f - fConst505));
		fConst510 = (0.689621364484675f + ((fConst451 - 0.512478641889141f) / fConst450));
		fConst511 = (2 * (0.689621364484675f - fConst479));
		for (int i=0; i<3; i++) fRec110[i] = 0;
		fConst512 = (2 * (7.621731298870603f - fConst479));
		fConst513 = (1.069358407707312f + ((fConst451 - 0.168404871113589f) / fConst450));
		fConst514 = (2 * (1.069358407707312f - fConst479));
		for (int i=0; i<3; i++) fRec109[i] = 0;
		fConst515 = (2 * (53.53615295455673f - fConst479));
		fConst516 = (4.076781969643807f + ((fConst489 - 3.1897274020965583f) / fConst488));
		fConst517 = (1.0f / fConst491);
		fConst518 = (2 * (4.076781969643807f - fConst517));
		for (int i=0; i<3; i++) fRec108[i] = 0;
		fConst519 = (2 * (0.00040767818495825777f - fConst498));
		fConst520 = (1.450071084655647f + ((fConst489 - 0.7431304601070396f) / fConst488));
		fConst521 = (2 * (1.450071084655647f - fConst517));
		for (int i=0; i<3; i++) fRec107[i] = 0;
		fConst522 = (2 * (1.450071084655647f - fConst495));
		fConst523 = (0.9351401670315425f + ((fConst489 - 0.157482159302087f) / fConst488));
		fConst524 = (2 * (0.9351401670315425f - fConst517));
		for (int i=0; i<3; i++) fRec106[i] = 0;
		fConst525 = (2 * (0.9351401670315425f - fConst492));
		for (int i=0; i<2; i++) fRec105[i] = 0;
		fConst526 = tanf((245.4369260617026f / float(iConst0)));
		fConst527 = (1.0f / fConst526);
		fConst528 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst527) / fConst526)));
		fConst529 = faustpower<2>(fConst526);
		fConst530 = (50.063807016150385f / fConst529);
		fConst531 = (0.9351401670315425f + fConst530);
		fConst532 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst527) / fConst526)));
		fConst533 = (11.052052171507189f / fConst529);
		fConst534 = (1.450071084655647f + fConst533);
		fConst535 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst527) / fConst526)));
		fConst536 = (0.0017661728399818856f / fConst529);
		fConst537 = (0.00040767818495825777f + fConst536);
		fConst538 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst489) / fConst488)));
		fConst539 = (53.53615295455673f + fConst517);
		fConst540 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst489) / fConst488)));
		fConst541 = (7.621731298870603f + fConst517);
		fConst542 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst489) / fConst488)));
		fConst543 = (9.9999997055e-05f / fConst491);
		fConst544 = (0.000433227200555f + fConst543);
		fConst545 = (0.24529150870616f + ((fConst489 - 0.782413046821645f) / fConst488));
		fConst546 = (2 * (0.24529150870616f - fConst517));
		for (int i=0; i<3; i++) fRec118[i] = 0;
		fConst547 = (2 * (0.000433227200555f - fConst543));
		fConst548 = (0.689621364484675f + ((fConst489 - 0.512478641889141f) / fConst488));
		fConst549 = (2 * (0.689621364484675f - fConst517));
		for (int i=0; i<3; i++) fRec117[i] = 0;
		fConst550 = (2 * (7.621731298870603f - fConst517));
		fConst551 = (1.069358407707312f + ((fConst489 - 0.168404871113589f) / fConst488));
		fConst552 = (2 * (1.069358407707312f - fConst517));
		for (int i=0; i<3; i++) fRec116[i] = 0;
		fConst553 = (2 * (53.53615295455673f - fConst517));
		fConst554 = (4.076781969643807f + ((fConst527 - 3.1897274020965583f) / fConst526));
		fConst555 = (1.0f / fConst529);
		fConst556 = (2 * (4.076781969643807f - fConst555));
		for (int i=0; i<3; i++) fRec115[i] = 0;
		fConst557 = (2 * (0.00040767818495825777f - fConst536));
		fConst558 = (1.450071084655647f + ((fConst527 - 0.7431304601070396f) / fConst526));
		fConst559 = (2 * (1.450071084655647f - fConst555));
		for (int i=0; i<3; i++) fRec114[i] = 0;
		fConst560 = (2 * (1.450071084655647f - fConst533));
		fConst561 = (0.9351401670315425f + ((fConst527 - 0.157482159302087f) / fConst526));
		fConst562 = (2 * (0.9351401670315425f - fConst555));
		for (int i=0; i<3; i++) fRec113[i] = 0;
		fConst563 = (2 * (0.9351401670315425f - fConst530));
		for (int i=0; i<2; i++) fRec112[i] = 0;
		fConst564 = tanf((173.5501147718112f / float(iConst0)));
		fConst565 = (1.0f / fConst564);
		fConst566 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst565) / fConst564)));
		fConst567 = faustpower<2>(fConst564);
		fConst568 = (50.063807016150385f / fConst567);
		fConst569 = (0.9351401670315425f + fConst568);
		fConst570 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst565) / fConst564)));
		fConst571 = (11.052052171507189f / fConst567);
		fConst572 = (1.450071084655647f + fConst571);
		fConst573 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst565) / fConst564)));
		fConst574 = (0.0017661728399818856f / fConst567);
		fConst575 = (0.00040767818495825777f + fConst574);
		fConst576 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst527) / fConst526)));
		fConst577 = (53.53615295455673f + fConst555);
		fConst578 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst527) / fConst526)));
		fConst579 = (7.621731298870603f + fConst555);
		fConst580 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst527) / fConst526)));
		fConst581 = (9.9999997055e-05f / fConst529);
		fConst582 = (0.000433227200555f + fConst581);
		fConst583 = (0.24529150870616f + ((fConst527 - 0.782413046821645f) / fConst526));
		fConst584 = (2 * (0.24529150870616f - fConst555));
		for (int i=0; i<3; i++) fRec125[i] = 0;
		fConst585 = (2 * (0.000433227200555f - fConst581));
		fConst586 = (0.689621364484675f + ((fConst527 - 0.512478641889141f) / fConst526));
		fConst587 = (2 * (0.689621364484675f - fConst555));
		for (int i=0; i<3; i++) fRec124[i] = 0;
		fConst588 = (2 * (7.621731298870603f - fConst555));
		fConst589 = (1.069358407707312f + ((fConst527 - 0.168404871113589f) / fConst526));
		fConst590 = (2 * (1.069358407707312f - fConst555));
		for (int i=0; i<3; i++) fRec123[i] = 0;
		fConst591 = (2 * (53.53615295455673f - fConst555));
		fConst592 = (4.076781969643807f + ((fConst565 - 3.1897274020965583f) / fConst564));
		fConst593 = (1.0f / fConst567);
		fConst594 = (2 * (4.076781969643807f - fConst593));
		for (int i=0; i<3; i++) fRec122[i] = 0;
		fConst595 = (2 * (0.00040767818495825777f - fConst574));
		fConst596 = (1.450071084655647f + ((fConst565 - 0.7431304601070396f) / fConst564));
		fConst597 = (2 * (1.450071084655647f - fConst593));
		for (int i=0; i<3; i++) fRec121[i] = 0;
		fConst598 = (2 * (1.450071084655647f - fConst571));
		fConst599 = (0.9351401670315425f + ((fConst565 - 0.157482159302087f) / fConst564));
		fConst600 = (2 * (0.9351401670315425f - fConst593));
		for (int i=0; i<3; i++) fRec120[i] = 0;
		fConst601 = (2 * (0.9351401670315425f - fConst568));
		for (int i=0; i<2; i++) fRec119[i] = 0;
		fConst602 = tanf((122.7184630308513f / float(iConst0)));
		fConst603 = (1.0f / fConst602);
		fConst604 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst603) / fConst602)));
		fConst605 = faustpower<2>(fConst602);
		fConst606 = (50.063807016150385f / fConst605);
		fConst607 = (0.9351401670315425f + fConst606);
		fConst608 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst603) / fConst602)));
		fConst609 = (11.052052171507189f / fConst605);
		fConst610 = (1.450071084655647f + fConst609);
		fConst611 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst603) / fConst602)));
		fConst612 = (0.0017661728399818856f / fConst605);
		fConst613 = (0.00040767818495825777f + fConst612);
		fConst614 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst565) / fConst564)));
		fConst615 = (53.53615295455673f + fConst593);
		fConst616 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst565) / fConst564)));
		fConst617 = (7.621731298870603f + fConst593);
		fConst618 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst565) / fConst564)));
		fConst619 = (9.9999997055e-05f / fConst567);
		fConst620 = (0.000433227200555f + fConst619);
		fConst621 = (0.24529150870616f + ((fConst565 - 0.782413046821645f) / fConst564));
		fConst622 = (2 * (0.24529150870616f - fConst593));
		for (int i=0; i<3; i++) fRec132[i] = 0;
		fConst623 = (2 * (0.000433227200555f - fConst619));
		fConst624 = (0.689621364484675f + ((fConst565 - 0.512478641889141f) / fConst564));
		fConst625 = (2 * (0.689621364484675f - fConst593));
		for (int i=0; i<3; i++) fRec131[i] = 0;
		fConst626 = (2 * (7.621731298870603f - fConst593));
		fConst627 = (1.069358407707312f + ((fConst565 - 0.168404871113589f) / fConst564));
		fConst628 = (2 * (1.069358407707312f - fConst593));
		for (int i=0; i<3; i++) fRec130[i] = 0;
		fConst629 = (2 * (53.53615295455673f - fConst593));
		fConst630 = (4.076781969643807f + ((fConst603 - 3.1897274020965583f) / fConst602));
		fConst631 = (1.0f / fConst605);
		fConst632 = (2 * (4.076781969643807f - fConst631));
		for (int i=0; i<3; i++) fRec129[i] = 0;
		fConst633 = (2 * (0.00040767818495825777f - fConst612));
		fConst634 = (1.450071084655647f + ((fConst603 - 0.7431304601070396f) / fConst602));
		fConst635 = (2 * (1.450071084655647f - fConst631));
		for (int i=0; i<3; i++) fRec128[i] = 0;
		fConst636 = (2 * (1.450071084655647f - fConst609));
		fConst637 = (0.9351401670315425f + ((fConst603 - 0.157482159302087f) / fConst602));
		fConst638 = (2 * (0.9351401670315425f - fConst631));
		for (int i=0; i<3; i++) fRec127[i] = 0;
		fConst639 = (2 * (0.9351401670315425f - fConst606));
		for (int i=0; i<2; i++) fRec126[i] = 0;
		fConst640 = tanf((86.7750573859056f / float(iConst0)));
		fConst641 = (1.0f / fConst640);
		fConst642 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst641) / fConst640)));
		fConst643 = faustpower<2>(fConst640);
		fConst644 = (50.063807016150385f / fConst643);
		fConst645 = (0.9351401670315425f + fConst644);
		fConst646 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst641) / fConst640)));
		fConst647 = (11.052052171507189f / fConst643);
		fConst648 = (1.450071084655647f + fConst647);
		fConst649 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst641) / fConst640)));
		fConst650 = (0.0017661728399818856f / fConst643);
		fConst651 = (0.00040767818495825777f + fConst650);
		fConst652 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst603) / fConst602)));
		fConst653 = (53.53615295455673f + fConst631);
		fConst654 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst603) / fConst602)));
		fConst655 = (7.621731298870603f + fConst631);
		fConst656 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst603) / fConst602)));
		fConst657 = (9.9999997055e-05f / fConst605);
		fConst658 = (0.000433227200555f + fConst657);
		fConst659 = (0.24529150870616f + ((fConst603 - 0.782413046821645f) / fConst602));
		fConst660 = (2 * (0.24529150870616f - fConst631));
		for (int i=0; i<3; i++) fRec139[i] = 0;
		fConst661 = (2 * (0.000433227200555f - fConst657));
		fConst662 = (0.689621364484675f + ((fConst603 - 0.512478641889141f) / fConst602));
		fConst663 = (2 * (0.689621364484675f - fConst631));
		for (int i=0; i<3; i++) fRec138[i] = 0;
		fConst664 = (2 * (7.621731298870603f - fConst631));
		fConst665 = (1.069358407707312f + ((fConst603 - 0.168404871113589f) / fConst602));
		fConst666 = (2 * (1.069358407707312f - fConst631));
		for (int i=0; i<3; i++) fRec137[i] = 0;
		fConst667 = (2 * (53.53615295455673f - fConst631));
		fConst668 = (4.076781969643807f + ((fConst641 - 3.1897274020965583f) / fConst640));
		fConst669 = (1.0f / fConst643);
		fConst670 = (2 * (4.076781969643807f - fConst669));
		for (int i=0; i<3; i++) fRec136[i] = 0;
		fConst671 = (2 * (0.00040767818495825777f - fConst650));
		fConst672 = (1.450071084655647f + ((fConst641 - 0.7431304601070396f) / fConst640));
		fConst673 = (2 * (1.450071084655647f - fConst669));
		for (int i=0; i<3; i++) fRec135[i] = 0;
		fConst674 = (2 * (1.450071084655647f - fConst647));
		fConst675 = (0.9351401670315425f + ((fConst641 - 0.157482159302087f) / fConst640));
		fConst676 = (2 * (0.9351401670315425f - fConst669));
		for (int i=0; i<3; i++) fRec134[i] = 0;
		fConst677 = (2 * (0.9351401670315425f - fConst644));
		for (int i=0; i<2; i++) fRec133[i] = 0;
		fConst678 = tanf((61.35923151542565f / float(iConst0)));
		fConst679 = (1.0f / fConst678);
		fConst680 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst679) / fConst678)));
		fConst681 = faustpower<2>(fConst678);
		fConst682 = (50.063807016150385f / fConst681);
		fConst683 = (0.9351401670315425f + fConst682);
		fConst684 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst679) / fConst678)));
		fConst685 = (11.052052171507189f / fConst681);
		fConst686 = (1.450071084655647f + fConst685);
		fConst687 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst679) / fConst678)));
		fConst688 = (0.0017661728399818856f / fConst681);
		fConst689 = (0.00040767818495825777f + fConst688);
		fConst690 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst641) / fConst640)));
		fConst691 = (53.53615295455673f + fConst669);
		fConst692 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst641) / fConst640)));
		fConst693 = (7.621731298870603f + fConst669);
		fConst694 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst641) / fConst640)));
		fConst695 = (9.9999997055e-05f / fConst643);
		fConst696 = (0.000433227200555f + fConst695);
		fConst697 = (0.24529150870616f + ((fConst641 - 0.782413046821645f) / fConst640));
		fConst698 = (2 * (0.24529150870616f - fConst669));
		for (int i=0; i<3; i++) fRec146[i] = 0;
		fConst699 = (2 * (0.000433227200555f - fConst695));
		fConst700 = (0.689621364484675f + ((fConst641 - 0.512478641889141f) / fConst640));
		fConst701 = (2 * (0.689621364484675f - fConst669));
		for (int i=0; i<3; i++) fRec145[i] = 0;
		fConst702 = (2 * (7.621731298870603f - fConst669));
		fConst703 = (1.069358407707312f + ((fConst641 - 0.168404871113589f) / fConst640));
		fConst704 = (2 * (1.069358407707312f - fConst669));
		for (int i=0; i<3; i++) fRec144[i] = 0;
		fConst705 = (2 * (53.53615295455673f - fConst669));
		fConst706 = (4.076781969643807f + ((fConst679 - 3.1897274020965583f) / fConst678));
		fConst707 = (1.0f / fConst681);
		fConst708 = (2 * (4.076781969643807f - fConst707));
		for (int i=0; i<3; i++) fRec143[i] = 0;
		fConst709 = (2 * (0.00040767818495825777f - fConst688));
		fConst710 = (1.450071084655647f + ((fConst679 - 0.7431304601070396f) / fConst678));
		fConst711 = (2 * (1.450071084655647f - fConst707));
		for (int i=0; i<3; i++) fRec142[i] = 0;
		fConst712 = (2 * (1.450071084655647f - fConst685));
		fConst713 = (0.9351401670315425f + ((fConst679 - 0.157482159302087f) / fConst678));
		fConst714 = (2 * (0.9351401670315425f - fConst707));
		for (int i=0; i<3; i++) fRec141[i] = 0;
		fConst715 = (2 * (0.9351401670315425f - fConst682));
		for (int i=0; i<2; i++) fRec140[i] = 0;
		fConst716 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst679) / fConst678)));
		fConst717 = (53.53615295455673f + fConst707);
		fConst718 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst679) / fConst678)));
		fConst719 = (7.621731298870603f + fConst707);
		fConst720 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst679) / fConst678)));
		fConst721 = (9.9999997055e-05f / fConst681);
		fConst722 = (0.000433227200555f + fConst721);
		fConst723 = (0.24529150870616f + ((fConst679 - 0.782413046821645f) / fConst678));
		fConst724 = (2 * (0.24529150870616f - fConst707));
		for (int i=0; i<3; i++) fRec150[i] = 0;
		fConst725 = (2 * (0.000433227200555f - fConst721));
		fConst726 = (0.689621364484675f + ((fConst679 - 0.512478641889141f) / fConst678));
		fConst727 = (2 * (0.689621364484675f - fConst707));
		for (int i=0; i<3; i++) fRec149[i] = 0;
		fConst728 = (2 * (7.621731298870603f - fConst707));
		fConst729 = (1.069358407707312f + ((fConst679 - 0.168404871113589f) / fConst678));
		fConst730 = (2 * (1.069358407707312f - fConst707));
		for (int i=0; i<3; i++) fRec148[i] = 0;
		fConst731 = (2 * (53.53615295455673f - fConst707));
		for (int i=0; i<2; i++) fRec147[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("parametric_eq");
		interface->declare(0, "1", "");
		interface->openVerticalBox("");
		interface->declare(0, "0", "");
		interface->declare(0, "tooltip", "See Faust's oscillator.lib for documentation and references");
		interface->openVerticalBox("SAWTOOTH OSCILLATOR");
		interface->declare(0, "1", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider5, "1", "");
		interface->declare(&fslider5, "style", "knob");
		interface->declare(&fslider5, "tooltip", "Sawtooth waveform amplitude");
		interface->declare(&fslider5, "unit", "dB");
		interface->addVerticalSlider("Amplitude", &fslider5, -2e+01f, -1.2e+02f, 1e+01f, 0.1f);
		interface->declare(&fslider7, "2", "");
		interface->declare(&fslider7, "style", "knob");
		interface->declare(&fslider7, "tooltip", "Sawtooth frequency as a Piano Key (PK) number (A440 = key 49)");
		interface->declare(&fslider7, "unit", "PK");
		interface->addVerticalSlider("Frequency", &fslider7, 49.0f, 1.0f, 88.0f, 0.01f);
		interface->declare(&fslider8, "3", "");
		interface->declare(&fslider8, "style", "knob");
		interface->declare(&fslider8, "tooltip", "Percentange frequency-shift up or down for second oscillator");
		interface->declare(&fslider8, "unit", "%%");
		interface->addVerticalSlider("Detuning 1", &fslider8, -0.1f, -1e+01f, 1e+01f, 0.01f);
		interface->declare(&fslider9, "4", "");
		interface->declare(&fslider9, "style", "knob");
		interface->declare(&fslider9, "tooltip", "Percentange frequency-shift up or down for third detuned oscillator");
		interface->declare(&fslider9, "unit", "%%");
		interface->addVerticalSlider("Detuning 2", &fslider9, 0.1f, -1e+01f, 1e+01f, 0.01f);
		interface->declare(&fslider6, "5", "");
		interface->declare(&fslider6, "style", "knob");
		interface->declare(&fslider6, "tooltip", "Portamento (frequency-glide) time-constant in seconds");
		interface->declare(&fslider6, "unit", "sec");
		interface->addVerticalSlider("Portamento", &fslider6, 0.1f, 0.01f, 1.0f, 0.001f);
		interface->declare(0, "6", "");
		interface->openVerticalBox("Alternate Signals");
		interface->declare(&fcheckbox1, "0", "");
		interface->declare(&fcheckbox1, "tooltip", "Pink Noise (or 1/f noise) is Constant-Q Noise, meaning that it has the same total power in every octave");
		interface->addCheckButton("Pink Noise Instead (uses only Amplitude control on the left)", &fcheckbox1);
		interface->declare(&fcheckbox0, "1", "");
		interface->addCheckButton("External Input Instead (overrides Sawtooth/Noise selection above)", &fcheckbox0);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "2", "");
		interface->openVerticalBox("");
		interface->declare(0, "0", "");
		interface->declare(0, "tooltip", "See Faust's filter.lib for info and pointers");
		interface->openHorizontalBox("PARAMETRIC EQ SECTIONS");
		interface->declare(0, "1", "");
		interface->openVerticalBox("Low Shelf");
		interface->declare(&fslider10, "0", "");
		interface->declare(&fslider10, "style", "knob");
		interface->declare(&fslider10, "tooltip", "Amount of low-frequency boost or cut in decibels");
		interface->declare(&fslider10, "unit", "dB");
		interface->addHorizontalSlider("Low Boost|Cut", &fslider10, 0.0f, -4e+01f, 4e+01f, 0.1f);
		interface->declare(&fslider4, "1", "");
		interface->declare(&fslider4, "style", "knob");
		interface->declare(&fslider4, "tooltip", "Transition-frequency from boost (cut) to unity gain");
		interface->declare(&fslider4, "unit", "Hz");
		interface->addHorizontalSlider("Transition Frequency", &fslider4, 2e+02f, 1.0f, 5e+03f, 1.0f);
		interface->closeBox();
		interface->declare(0, "2", "");
		interface->declare(0, "tooltip", "Parametric Equalizer sections from filter.lib");
		interface->openVerticalBox("Peaking Equalizer");
		interface->declare(&fslider11, "0", "");
		interface->declare(&fslider11, "style", "knob");
		interface->declare(&fslider11, "tooltip", "Amount of local boost or cut in decibels");
		interface->declare(&fslider11, "unit", "dB");
		interface->addHorizontalSlider("Peak Boost|Cut", &fslider11, 0.0f, -4e+01f, 4e+01f, 0.1f);
		interface->declare(&fslider3, "1", "");
		interface->declare(&fslider3, "style", "knob");
		interface->declare(&fslider3, "tooltip", "Peak Frequency in Piano Key (PK) units (A-440= 49 PK)");
		interface->declare(&fslider3, "unit", "PK");
		interface->addHorizontalSlider("Peak Frequency", &fslider3, 49.0f, 1.0f, 1e+02f, 1.0f);
		interface->declare(&fslider12, "2", "");
		interface->declare(&fslider12, "style", "knob");
		interface->declare(&fslider12, "tooltip", "Quality factor (Q) of the peak = center-frequency/bandwidth");
		interface->addHorizontalSlider("Peak Q", &fslider12, 4e+01f, 1.0f, 5e+01f, 0.1f);
		interface->closeBox();
		interface->declare(0, "3", "");
		interface->declare(0, "tooltip", "A high shelf provides a boost or cut                            above some frequency");
		interface->openVerticalBox("High Shelf");
		interface->declare(&fslider13, "0", "");
		interface->declare(&fslider13, "style", "knob");
		interface->declare(&fslider13, "tooltip", "Amount of high-frequency boost or cut in decibels");
		interface->declare(&fslider13, "unit", "dB");
		interface->addHorizontalSlider("High Boost|Cut", &fslider13, 0.0f, -4e+01f, 4e+01f, 0.1f);
		interface->declare(&fslider2, "1", "");
		interface->declare(&fslider2, "style", "knob");
		interface->declare(&fslider2, "tooltip", "Transition-frequency from boost (cut) to unity gain");
		interface->declare(&fslider2, "unit", "Hz");
		interface->addHorizontalSlider("Transition Frequency", &fslider2, 8e+03f, 2e+01f, 1e+04f, 1.0f);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "3", "");
		interface->openVerticalBox("");
		interface->declare(0, "0", "");
		interface->declare(0, "tooltip", "See Faust's filter.lib for documentation and references");
		interface->openHorizontalBox("CONSTANT-Q SPECTRUM ANALYZER (6E)");
		interface->declare(&fbargraph19, "0", "");
		interface->declare(&fbargraph19, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph19, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph19, -5e+01f, 1e+01f);
		interface->declare(&fbargraph18, "1", "");
		interface->declare(&fbargraph18, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph18, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph18, -5e+01f, 1e+01f);
		interface->declare(&fbargraph17, "2", "");
		interface->declare(&fbargraph17, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph17, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph17, -5e+01f, 1e+01f);
		interface->declare(&fbargraph16, "3", "");
		interface->declare(&fbargraph16, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph16, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph16, -5e+01f, 1e+01f);
		interface->declare(&fbargraph15, "4", "");
		interface->declare(&fbargraph15, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph15, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph15, -5e+01f, 1e+01f);
		interface->declare(&fbargraph14, "5", "");
		interface->declare(&fbargraph14, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph14, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph14, -5e+01f, 1e+01f);
		interface->declare(&fbargraph13, "6", "");
		interface->declare(&fbargraph13, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph13, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph13, -5e+01f, 1e+01f);
		interface->declare(&fbargraph12, "7", "");
		interface->declare(&fbargraph12, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph12, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph12, -5e+01f, 1e+01f);
		interface->declare(&fbargraph11, "8", "");
		interface->declare(&fbargraph11, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph11, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph11, -5e+01f, 1e+01f);
		interface->declare(&fbargraph10, "9", "");
		interface->declare(&fbargraph10, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph10, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph10, -5e+01f, 1e+01f);
		interface->declare(&fbargraph9, "10", "");
		interface->declare(&fbargraph9, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph9, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph9, -5e+01f, 1e+01f);
		interface->declare(&fbargraph8, "11", "");
		interface->declare(&fbargraph8, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph8, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph8, -5e+01f, 1e+01f);
		interface->declare(&fbargraph7, "12", "");
		interface->declare(&fbargraph7, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph7, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph7, -5e+01f, 1e+01f);
		interface->declare(&fbargraph6, "13", "");
		interface->declare(&fbargraph6, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph6, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph6, -5e+01f, 1e+01f);
		interface->declare(&fbargraph5, "14", "");
		interface->declare(&fbargraph5, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph5, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph5, -5e+01f, 1e+01f);
		interface->declare(&fbargraph4, "15", "");
		interface->declare(&fbargraph4, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph4, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph4, -5e+01f, 1e+01f);
		interface->declare(&fbargraph3, "16", "");
		interface->declare(&fbargraph3, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph3, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph3, -5e+01f, 1e+01f);
		interface->declare(&fbargraph2, "17", "");
		interface->declare(&fbargraph2, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph2, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph2, -5e+01f, 1e+01f);
		interface->declare(&fbargraph1, "18", "");
		interface->declare(&fbargraph1, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph1, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph1, -5e+01f, 1e+01f);
		interface->declare(&fbargraph0, "19", "");
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
		float 	fSlow3 = tanf((fConst14 * fslider2));
		float 	fSlow4 = (1.0f / fSlow3);
		float 	fSlow5 = (1.0f / (1 + ((1.0000000000000004f + fSlow4) / fSlow3)));
		float 	fSlow6 = (1 + fSlow4);
		float 	fSlow7 = (0 - ((1 - fSlow4) / fSlow6));
		float 	fSlow8 = (1.0f / fSlow6);
		float 	fSlow9 = (0.0010000000000000009f * fslider3);
		float 	fSlow10 = tanf((fConst14 * fslider4));
		float 	fSlow11 = (1.0f / fSlow10);
		float 	fSlow12 = (1.0f / (1 + ((1.0000000000000004f + fSlow11) / fSlow10)));
		float 	fSlow13 = (1.0f / faustpower<2>(fSlow10));
		float 	fSlow14 = (1 + fSlow11);
		float 	fSlow15 = (0 - ((1 - fSlow11) / fSlow14));
		float 	fSlow16 = (1.0f / fSlow14);
		float 	fSlow17 = (0.0010000000000000009f * powf(10,(0.05f * fslider5)));
		int 	iSlow18 = int(fcheckbox0);
		int 	iSlow19 = int(fcheckbox1);
		float 	fSlow20 = expf((0 - (fConst1 / fslider6)));
		float 	fSlow21 = (4.4e+02f * (powf(2.0f,(0.08333333333333333f * (fslider7 - 49.0f))) * (1.0f - fSlow20)));
		float 	fSlow22 = (1 + (0.01f * fslider8));
		float 	fSlow23 = (1.0f / fSlow22);
		float 	fSlow24 = (fConst18 * fSlow22);
		float 	fSlow25 = (fConst17 / fSlow22);
		float 	fSlow26 = (1 + (0.01f * fslider9));
		float 	fSlow27 = (1.0f / fSlow26);
		float 	fSlow28 = (fConst18 * fSlow26);
		float 	fSlow29 = (fConst17 / fSlow26);
		float 	fSlow30 = (0 - fSlow11);
		float 	fSlow31 = (1 + ((fSlow11 - 1.0000000000000004f) / fSlow10));
		float 	fSlow32 = (2 * (1 - fSlow13));
		float 	fSlow33 = (2 * (0 - fSlow13));
		float 	fSlow34 = powf(10,(0.05f * fslider10));
		float 	fSlow35 = fslider11;
		int 	iSlow36 = int((fSlow35 > 0));
		float 	fSlow37 = fslider12;
		float 	fSlow38 = (fConst20 / fSlow37);
		float 	fSlow39 = (fConst20 * (powf(10,(0.05f * fabsf(fSlow35))) / fSlow37));
		float 	fSlow40 = (1 + ((fSlow4 - 1.0000000000000004f) / fSlow3));
		float 	fSlow41 = (1.0f / faustpower<2>(fSlow3));
		float 	fSlow42 = (2 * (1 - fSlow41));
		float 	fSlow43 = powf(10,(0.05f * fslider13));
		float 	fSlow44 = (0 - fSlow4);
		float 	fSlow45 = (2 * (0 - fSlow41));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			fRec6[0] = ((0.999f * fRec6[1]) + fSlow9);
			float fTemp0 = powf(2.0f,(0.08333333333333333f * (fRec6[0] - 49.0f)));
			float fTemp1 = tanf((fConst15 * fTemp0));
			fRec10[0] = ((0.999f * fRec10[1]) + fSlow17);
			iRec12[0] = (12345 + (1103515245 * iRec12[1]));
			fRec11[0] = (((0.5221894f * fRec11[3]) + ((4.656612875245797e-10f * iRec12[0]) + (2.494956002f * fRec11[1]))) - (2.017265875f * fRec11[2]));
			fVec0[0] = 0.25f;
			fRec13[0] = ((fSlow20 * fRec13[1]) + fSlow21);
			fRec14[0] = fmodf((1.0f + fRec14[1]),(fConst17 / fRec13[0]));
			float fTemp2 = faustpower<2>(((fConst18 * (fRec13[0] * fRec14[0])) - 1.0f));
			fVec1[0] = fTemp2;
			fRec15[0] = fmodf((1.0f + fRec15[1]),(fSlow25 / fRec13[0]));
			float fTemp3 = faustpower<2>(((fSlow24 * (fRec13[0] * fRec15[0])) - 1.0f));
			fVec2[0] = fTemp3;
			fRec16[0] = fmodf((1.0f + fRec16[1]),(fSlow29 / fRec13[0]));
			float fTemp4 = faustpower<2>(((fSlow28 * (fRec13[0] * fRec16[0])) - 1.0f));
			fVec3[0] = fTemp4;
			float fTemp5 = (fRec10[0] * ((iSlow18)?(float)input0[i]:((iSlow19)?(((0.049922035f * fRec11[0]) + (0.050612699f * fRec11[2])) - ((0.095993537f * fRec11[1]) + (0.004408786f * fRec11[3]))):(fConst16 * (((fRec10[0] * fVec0[1]) * (((fVec1[0] - fVec1[1]) + (fSlow23 * (fVec2[0] - fVec2[1]))) + (fSlow27 * (fVec3[0] - fVec3[1])))) / fRec13[0])))));
			fVec4[0] = fTemp5;
			fRec9[0] = ((fSlow15 * fRec9[1]) + (fSlow16 * ((fSlow11 * fVec4[0]) + (fSlow30 * fVec4[1]))));
			fRec8[0] = (fRec9[0] - (fSlow12 * ((fSlow31 * fRec8[2]) + (fSlow32 * fRec8[1]))));
			fRec18[0] = ((fSlow15 * fRec18[1]) + (fSlow16 * (fVec4[0] + fVec4[1])));
			fRec17[0] = (fRec18[0] - (fSlow12 * ((fSlow31 * fRec17[2]) + (fSlow32 * fRec17[1]))));
			float fTemp6 = (1.0f / fTemp1);
			float fTemp7 = (fTemp0 / sinf((fConst21 * fTemp0)));
			float fTemp8 = (fSlow38 * fTemp7);
			float fTemp9 = (fSlow39 * fTemp7);
			float fTemp10 = ((iSlow36)?fTemp8:fTemp9);
			float fTemp11 = (2 * ((1 - (1.0f / faustpower<2>(fTemp1))) * fRec7[1]));
			float fTemp12 = (1 + ((fTemp6 + fTemp10) / fTemp1));
			fRec7[0] = ((fSlow12 * ((((fSlow13 * fRec8[0]) + (fSlow33 * fRec8[1])) + (fSlow13 * fRec8[2])) + (fSlow34 * (fRec17[2] + (fRec17[0] + (2 * fRec17[1])))))) - ((((1 + ((fTemp6 - fTemp10) / fTemp1)) * fRec7[2]) + fTemp11) / fTemp12));
			float fTemp13 = ((iSlow36)?fTemp9:fTemp8);
			float fTemp14 = (((fTemp11 + (fRec7[0] * (1 + ((fTemp13 + fTemp6) / fTemp1)))) + (fRec7[2] * (1 + ((fTemp6 - fTemp13) / fTemp1)))) / fTemp12);
			fVec5[0] = fTemp14;
			fRec5[0] = ((fSlow7 * fRec5[1]) + (fSlow8 * (fVec5[0] + fVec5[1])));
			fRec4[0] = (fRec5[0] - (fSlow5 * ((fSlow40 * fRec4[2]) + (fSlow42 * fRec4[1]))));
			fRec20[0] = ((fSlow7 * fRec20[1]) + (fSlow8 * ((fSlow4 * fVec5[0]) + (fSlow44 * fVec5[1]))));
			fRec19[0] = (fRec20[0] - (fSlow5 * ((fSlow40 * fRec19[2]) + (fSlow42 * fRec19[1]))));
			float fTemp15 = (fSlow5 * ((fRec4[2] + (fRec4[0] + (2 * fRec4[1]))) + (fSlow43 * (((fSlow41 * fRec19[0]) + (fSlow45 * fRec19[1])) + (fSlow41 * fRec19[2])))));
			fRec3[0] = (fTemp15 - (fConst11 * ((fConst22 * fRec3[2]) + (fConst24 * fRec3[1]))));
			fRec2[0] = ((fConst11 * (((fConst13 * fRec3[0]) + (fConst25 * fRec3[1])) + (fConst13 * fRec3[2]))) - (fConst8 * ((fConst26 * fRec2[2]) + (fConst27 * fRec2[1]))));
			fRec1[0] = ((fConst8 * (((fConst10 * fRec2[0]) + (fConst28 * fRec2[1])) + (fConst10 * fRec2[2]))) - (fConst4 * ((fConst29 * fRec1[2]) + (fConst30 * fRec1[1]))));
			fRec0[0] = ((fSlow1 * fRec0[1]) + (fSlow2 * fabsf((fConst4 * (((fConst7 * fRec1[0]) + (fConst31 * fRec1[1])) + (fConst7 * fRec1[2]))))));
			fbargraph0 = (fSlow0 + (20 * log10f(fRec0[0])));
			fRec27[0] = (fTemp15 - (fConst48 * ((fConst51 * fRec27[2]) + (fConst52 * fRec27[1]))));
			fRec26[0] = ((fConst48 * (((fConst50 * fRec27[0]) + (fConst53 * fRec27[1])) + (fConst50 * fRec27[2]))) - (fConst46 * ((fConst54 * fRec26[2]) + (fConst55 * fRec26[1]))));
			fRec25[0] = ((fConst46 * (((fConst47 * fRec26[0]) + (fConst56 * fRec26[1])) + (fConst47 * fRec26[2]))) - (fConst44 * ((fConst57 * fRec25[2]) + (fConst58 * fRec25[1]))));
			float fTemp16 = (fConst44 * (((fConst45 * fRec25[0]) + (fConst59 * fRec25[1])) + (fConst45 * fRec25[2])));
			fRec24[0] = (fTemp16 - (fConst41 * ((fConst60 * fRec24[2]) + (fConst62 * fRec24[1]))));
			fRec23[0] = ((fConst41 * (((fConst43 * fRec24[0]) + (fConst63 * fRec24[1])) + (fConst43 * fRec24[2]))) - (fConst38 * ((fConst64 * fRec23[2]) + (fConst65 * fRec23[1]))));
			fRec22[0] = ((fConst38 * (((fConst40 * fRec23[0]) + (fConst66 * fRec23[1])) + (fConst40 * fRec23[2]))) - (fConst34 * ((fConst67 * fRec22[2]) + (fConst68 * fRec22[1]))));
			fRec21[0] = ((fSlow1 * fRec21[1]) + (fSlow2 * fabsf((fConst34 * (((fConst37 * fRec22[0]) + (fConst69 * fRec22[1])) + (fConst37 * fRec22[2]))))));
			fbargraph1 = (fSlow0 + (20 * log10f(fRec21[0])));
			fRec34[0] = (fTemp16 - (fConst86 * ((fConst89 * fRec34[2]) + (fConst90 * fRec34[1]))));
			fRec33[0] = ((fConst86 * (((fConst88 * fRec34[0]) + (fConst91 * fRec34[1])) + (fConst88 * fRec34[2]))) - (fConst84 * ((fConst92 * fRec33[2]) + (fConst93 * fRec33[1]))));
			fRec32[0] = ((fConst84 * (((fConst85 * fRec33[0]) + (fConst94 * fRec33[1])) + (fConst85 * fRec33[2]))) - (fConst82 * ((fConst95 * fRec32[2]) + (fConst96 * fRec32[1]))));
			float fTemp17 = (fConst82 * (((fConst83 * fRec32[0]) + (fConst97 * fRec32[1])) + (fConst83 * fRec32[2])));
			fRec31[0] = (fTemp17 - (fConst79 * ((fConst98 * fRec31[2]) + (fConst100 * fRec31[1]))));
			fRec30[0] = ((fConst79 * (((fConst81 * fRec31[0]) + (fConst101 * fRec31[1])) + (fConst81 * fRec31[2]))) - (fConst76 * ((fConst102 * fRec30[2]) + (fConst103 * fRec30[1]))));
			fRec29[0] = ((fConst76 * (((fConst78 * fRec30[0]) + (fConst104 * fRec30[1])) + (fConst78 * fRec30[2]))) - (fConst72 * ((fConst105 * fRec29[2]) + (fConst106 * fRec29[1]))));
			fRec28[0] = ((fSlow1 * fRec28[1]) + (fSlow2 * fabsf((fConst72 * (((fConst75 * fRec29[0]) + (fConst107 * fRec29[1])) + (fConst75 * fRec29[2]))))));
			fbargraph2 = (fSlow0 + (20 * log10f(fRec28[0])));
			fRec41[0] = (fTemp17 - (fConst124 * ((fConst127 * fRec41[2]) + (fConst128 * fRec41[1]))));
			fRec40[0] = ((fConst124 * (((fConst126 * fRec41[0]) + (fConst129 * fRec41[1])) + (fConst126 * fRec41[2]))) - (fConst122 * ((fConst130 * fRec40[2]) + (fConst131 * fRec40[1]))));
			fRec39[0] = ((fConst122 * (((fConst123 * fRec40[0]) + (fConst132 * fRec40[1])) + (fConst123 * fRec40[2]))) - (fConst120 * ((fConst133 * fRec39[2]) + (fConst134 * fRec39[1]))));
			float fTemp18 = (fConst120 * (((fConst121 * fRec39[0]) + (fConst135 * fRec39[1])) + (fConst121 * fRec39[2])));
			fRec38[0] = (fTemp18 - (fConst117 * ((fConst136 * fRec38[2]) + (fConst138 * fRec38[1]))));
			fRec37[0] = ((fConst117 * (((fConst119 * fRec38[0]) + (fConst139 * fRec38[1])) + (fConst119 * fRec38[2]))) - (fConst114 * ((fConst140 * fRec37[2]) + (fConst141 * fRec37[1]))));
			fRec36[0] = ((fConst114 * (((fConst116 * fRec37[0]) + (fConst142 * fRec37[1])) + (fConst116 * fRec37[2]))) - (fConst110 * ((fConst143 * fRec36[2]) + (fConst144 * fRec36[1]))));
			fRec35[0] = ((fSlow1 * fRec35[1]) + (fSlow2 * fabsf((fConst110 * (((fConst113 * fRec36[0]) + (fConst145 * fRec36[1])) + (fConst113 * fRec36[2]))))));
			fbargraph3 = (fSlow0 + (20 * log10f(fRec35[0])));
			fRec48[0] = (fTemp18 - (fConst162 * ((fConst165 * fRec48[2]) + (fConst166 * fRec48[1]))));
			fRec47[0] = ((fConst162 * (((fConst164 * fRec48[0]) + (fConst167 * fRec48[1])) + (fConst164 * fRec48[2]))) - (fConst160 * ((fConst168 * fRec47[2]) + (fConst169 * fRec47[1]))));
			fRec46[0] = ((fConst160 * (((fConst161 * fRec47[0]) + (fConst170 * fRec47[1])) + (fConst161 * fRec47[2]))) - (fConst158 * ((fConst171 * fRec46[2]) + (fConst172 * fRec46[1]))));
			float fTemp19 = (fConst158 * (((fConst159 * fRec46[0]) + (fConst173 * fRec46[1])) + (fConst159 * fRec46[2])));
			fRec45[0] = (fTemp19 - (fConst155 * ((fConst174 * fRec45[2]) + (fConst176 * fRec45[1]))));
			fRec44[0] = ((fConst155 * (((fConst157 * fRec45[0]) + (fConst177 * fRec45[1])) + (fConst157 * fRec45[2]))) - (fConst152 * ((fConst178 * fRec44[2]) + (fConst179 * fRec44[1]))));
			fRec43[0] = ((fConst152 * (((fConst154 * fRec44[0]) + (fConst180 * fRec44[1])) + (fConst154 * fRec44[2]))) - (fConst148 * ((fConst181 * fRec43[2]) + (fConst182 * fRec43[1]))));
			fRec42[0] = ((fSlow1 * fRec42[1]) + (fSlow2 * fabsf((fConst148 * (((fConst151 * fRec43[0]) + (fConst183 * fRec43[1])) + (fConst151 * fRec43[2]))))));
			fbargraph4 = (fSlow0 + (20 * log10f(fRec42[0])));
			fRec55[0] = (fTemp19 - (fConst200 * ((fConst203 * fRec55[2]) + (fConst204 * fRec55[1]))));
			fRec54[0] = ((fConst200 * (((fConst202 * fRec55[0]) + (fConst205 * fRec55[1])) + (fConst202 * fRec55[2]))) - (fConst198 * ((fConst206 * fRec54[2]) + (fConst207 * fRec54[1]))));
			fRec53[0] = ((fConst198 * (((fConst199 * fRec54[0]) + (fConst208 * fRec54[1])) + (fConst199 * fRec54[2]))) - (fConst196 * ((fConst209 * fRec53[2]) + (fConst210 * fRec53[1]))));
			float fTemp20 = (fConst196 * (((fConst197 * fRec53[0]) + (fConst211 * fRec53[1])) + (fConst197 * fRec53[2])));
			fRec52[0] = (fTemp20 - (fConst193 * ((fConst212 * fRec52[2]) + (fConst214 * fRec52[1]))));
			fRec51[0] = ((fConst193 * (((fConst195 * fRec52[0]) + (fConst215 * fRec52[1])) + (fConst195 * fRec52[2]))) - (fConst190 * ((fConst216 * fRec51[2]) + (fConst217 * fRec51[1]))));
			fRec50[0] = ((fConst190 * (((fConst192 * fRec51[0]) + (fConst218 * fRec51[1])) + (fConst192 * fRec51[2]))) - (fConst186 * ((fConst219 * fRec50[2]) + (fConst220 * fRec50[1]))));
			fRec49[0] = ((fSlow1 * fRec49[1]) + (fSlow2 * fabsf((fConst186 * (((fConst189 * fRec50[0]) + (fConst221 * fRec50[1])) + (fConst189 * fRec50[2]))))));
			fbargraph5 = (fSlow0 + (20 * log10f(fRec49[0])));
			fRec62[0] = (fTemp20 - (fConst238 * ((fConst241 * fRec62[2]) + (fConst242 * fRec62[1]))));
			fRec61[0] = ((fConst238 * (((fConst240 * fRec62[0]) + (fConst243 * fRec62[1])) + (fConst240 * fRec62[2]))) - (fConst236 * ((fConst244 * fRec61[2]) + (fConst245 * fRec61[1]))));
			fRec60[0] = ((fConst236 * (((fConst237 * fRec61[0]) + (fConst246 * fRec61[1])) + (fConst237 * fRec61[2]))) - (fConst234 * ((fConst247 * fRec60[2]) + (fConst248 * fRec60[1]))));
			float fTemp21 = (fConst234 * (((fConst235 * fRec60[0]) + (fConst249 * fRec60[1])) + (fConst235 * fRec60[2])));
			fRec59[0] = (fTemp21 - (fConst231 * ((fConst250 * fRec59[2]) + (fConst252 * fRec59[1]))));
			fRec58[0] = ((fConst231 * (((fConst233 * fRec59[0]) + (fConst253 * fRec59[1])) + (fConst233 * fRec59[2]))) - (fConst228 * ((fConst254 * fRec58[2]) + (fConst255 * fRec58[1]))));
			fRec57[0] = ((fConst228 * (((fConst230 * fRec58[0]) + (fConst256 * fRec58[1])) + (fConst230 * fRec58[2]))) - (fConst224 * ((fConst257 * fRec57[2]) + (fConst258 * fRec57[1]))));
			fRec56[0] = ((fSlow1 * fRec56[1]) + (fSlow2 * fabsf((fConst224 * (((fConst227 * fRec57[0]) + (fConst259 * fRec57[1])) + (fConst227 * fRec57[2]))))));
			fbargraph6 = (fSlow0 + (20 * log10f(fRec56[0])));
			fRec69[0] = (fTemp21 - (fConst276 * ((fConst279 * fRec69[2]) + (fConst280 * fRec69[1]))));
			fRec68[0] = ((fConst276 * (((fConst278 * fRec69[0]) + (fConst281 * fRec69[1])) + (fConst278 * fRec69[2]))) - (fConst274 * ((fConst282 * fRec68[2]) + (fConst283 * fRec68[1]))));
			fRec67[0] = ((fConst274 * (((fConst275 * fRec68[0]) + (fConst284 * fRec68[1])) + (fConst275 * fRec68[2]))) - (fConst272 * ((fConst285 * fRec67[2]) + (fConst286 * fRec67[1]))));
			float fTemp22 = (fConst272 * (((fConst273 * fRec67[0]) + (fConst287 * fRec67[1])) + (fConst273 * fRec67[2])));
			fRec66[0] = (fTemp22 - (fConst269 * ((fConst288 * fRec66[2]) + (fConst290 * fRec66[1]))));
			fRec65[0] = ((fConst269 * (((fConst271 * fRec66[0]) + (fConst291 * fRec66[1])) + (fConst271 * fRec66[2]))) - (fConst266 * ((fConst292 * fRec65[2]) + (fConst293 * fRec65[1]))));
			fRec64[0] = ((fConst266 * (((fConst268 * fRec65[0]) + (fConst294 * fRec65[1])) + (fConst268 * fRec65[2]))) - (fConst262 * ((fConst295 * fRec64[2]) + (fConst296 * fRec64[1]))));
			fRec63[0] = ((fSlow1 * fRec63[1]) + (fSlow2 * fabsf((fConst262 * (((fConst265 * fRec64[0]) + (fConst297 * fRec64[1])) + (fConst265 * fRec64[2]))))));
			fbargraph7 = (fSlow0 + (20 * log10f(fRec63[0])));
			fRec76[0] = (fTemp22 - (fConst314 * ((fConst317 * fRec76[2]) + (fConst318 * fRec76[1]))));
			fRec75[0] = ((fConst314 * (((fConst316 * fRec76[0]) + (fConst319 * fRec76[1])) + (fConst316 * fRec76[2]))) - (fConst312 * ((fConst320 * fRec75[2]) + (fConst321 * fRec75[1]))));
			fRec74[0] = ((fConst312 * (((fConst313 * fRec75[0]) + (fConst322 * fRec75[1])) + (fConst313 * fRec75[2]))) - (fConst310 * ((fConst323 * fRec74[2]) + (fConst324 * fRec74[1]))));
			float fTemp23 = (fConst310 * (((fConst311 * fRec74[0]) + (fConst325 * fRec74[1])) + (fConst311 * fRec74[2])));
			fRec73[0] = (fTemp23 - (fConst307 * ((fConst326 * fRec73[2]) + (fConst328 * fRec73[1]))));
			fRec72[0] = ((fConst307 * (((fConst309 * fRec73[0]) + (fConst329 * fRec73[1])) + (fConst309 * fRec73[2]))) - (fConst304 * ((fConst330 * fRec72[2]) + (fConst331 * fRec72[1]))));
			fRec71[0] = ((fConst304 * (((fConst306 * fRec72[0]) + (fConst332 * fRec72[1])) + (fConst306 * fRec72[2]))) - (fConst300 * ((fConst333 * fRec71[2]) + (fConst334 * fRec71[1]))));
			fRec70[0] = ((fSlow1 * fRec70[1]) + (fSlow2 * fabsf((fConst300 * (((fConst303 * fRec71[0]) + (fConst335 * fRec71[1])) + (fConst303 * fRec71[2]))))));
			fbargraph8 = (fSlow0 + (20 * log10f(fRec70[0])));
			fRec83[0] = (fTemp23 - (fConst352 * ((fConst355 * fRec83[2]) + (fConst356 * fRec83[1]))));
			fRec82[0] = ((fConst352 * (((fConst354 * fRec83[0]) + (fConst357 * fRec83[1])) + (fConst354 * fRec83[2]))) - (fConst350 * ((fConst358 * fRec82[2]) + (fConst359 * fRec82[1]))));
			fRec81[0] = ((fConst350 * (((fConst351 * fRec82[0]) + (fConst360 * fRec82[1])) + (fConst351 * fRec82[2]))) - (fConst348 * ((fConst361 * fRec81[2]) + (fConst362 * fRec81[1]))));
			float fTemp24 = (fConst348 * (((fConst349 * fRec81[0]) + (fConst363 * fRec81[1])) + (fConst349 * fRec81[2])));
			fRec80[0] = (fTemp24 - (fConst345 * ((fConst364 * fRec80[2]) + (fConst366 * fRec80[1]))));
			fRec79[0] = ((fConst345 * (((fConst347 * fRec80[0]) + (fConst367 * fRec80[1])) + (fConst347 * fRec80[2]))) - (fConst342 * ((fConst368 * fRec79[2]) + (fConst369 * fRec79[1]))));
			fRec78[0] = ((fConst342 * (((fConst344 * fRec79[0]) + (fConst370 * fRec79[1])) + (fConst344 * fRec79[2]))) - (fConst338 * ((fConst371 * fRec78[2]) + (fConst372 * fRec78[1]))));
			fRec77[0] = ((fSlow1 * fRec77[1]) + (fSlow2 * fabsf((fConst338 * (((fConst341 * fRec78[0]) + (fConst373 * fRec78[1])) + (fConst341 * fRec78[2]))))));
			fbargraph9 = (fSlow0 + (20 * log10f(fRec77[0])));
			fRec90[0] = (fTemp24 - (fConst390 * ((fConst393 * fRec90[2]) + (fConst394 * fRec90[1]))));
			fRec89[0] = ((fConst390 * (((fConst392 * fRec90[0]) + (fConst395 * fRec90[1])) + (fConst392 * fRec90[2]))) - (fConst388 * ((fConst396 * fRec89[2]) + (fConst397 * fRec89[1]))));
			fRec88[0] = ((fConst388 * (((fConst389 * fRec89[0]) + (fConst398 * fRec89[1])) + (fConst389 * fRec89[2]))) - (fConst386 * ((fConst399 * fRec88[2]) + (fConst400 * fRec88[1]))));
			float fTemp25 = (fConst386 * (((fConst387 * fRec88[0]) + (fConst401 * fRec88[1])) + (fConst387 * fRec88[2])));
			fRec87[0] = (fTemp25 - (fConst383 * ((fConst402 * fRec87[2]) + (fConst404 * fRec87[1]))));
			fRec86[0] = ((fConst383 * (((fConst385 * fRec87[0]) + (fConst405 * fRec87[1])) + (fConst385 * fRec87[2]))) - (fConst380 * ((fConst406 * fRec86[2]) + (fConst407 * fRec86[1]))));
			fRec85[0] = ((fConst380 * (((fConst382 * fRec86[0]) + (fConst408 * fRec86[1])) + (fConst382 * fRec86[2]))) - (fConst376 * ((fConst409 * fRec85[2]) + (fConst410 * fRec85[1]))));
			fRec84[0] = ((fSlow1 * fRec84[1]) + (fSlow2 * fabsf((fConst376 * (((fConst379 * fRec85[0]) + (fConst411 * fRec85[1])) + (fConst379 * fRec85[2]))))));
			fbargraph10 = (fSlow0 + (20 * log10f(fRec84[0])));
			fRec97[0] = (fTemp25 - (fConst428 * ((fConst431 * fRec97[2]) + (fConst432 * fRec97[1]))));
			fRec96[0] = ((fConst428 * (((fConst430 * fRec97[0]) + (fConst433 * fRec97[1])) + (fConst430 * fRec97[2]))) - (fConst426 * ((fConst434 * fRec96[2]) + (fConst435 * fRec96[1]))));
			fRec95[0] = ((fConst426 * (((fConst427 * fRec96[0]) + (fConst436 * fRec96[1])) + (fConst427 * fRec96[2]))) - (fConst424 * ((fConst437 * fRec95[2]) + (fConst438 * fRec95[1]))));
			float fTemp26 = (fConst424 * (((fConst425 * fRec95[0]) + (fConst439 * fRec95[1])) + (fConst425 * fRec95[2])));
			fRec94[0] = (fTemp26 - (fConst421 * ((fConst440 * fRec94[2]) + (fConst442 * fRec94[1]))));
			fRec93[0] = ((fConst421 * (((fConst423 * fRec94[0]) + (fConst443 * fRec94[1])) + (fConst423 * fRec94[2]))) - (fConst418 * ((fConst444 * fRec93[2]) + (fConst445 * fRec93[1]))));
			fRec92[0] = ((fConst418 * (((fConst420 * fRec93[0]) + (fConst446 * fRec93[1])) + (fConst420 * fRec93[2]))) - (fConst414 * ((fConst447 * fRec92[2]) + (fConst448 * fRec92[1]))));
			fRec91[0] = ((fSlow1 * fRec91[1]) + (fSlow2 * fabsf((fConst414 * (((fConst417 * fRec92[0]) + (fConst449 * fRec92[1])) + (fConst417 * fRec92[2]))))));
			fbargraph11 = (fSlow0 + (20 * log10f(fRec91[0])));
			fRec104[0] = (fTemp26 - (fConst466 * ((fConst469 * fRec104[2]) + (fConst470 * fRec104[1]))));
			fRec103[0] = ((fConst466 * (((fConst468 * fRec104[0]) + (fConst471 * fRec104[1])) + (fConst468 * fRec104[2]))) - (fConst464 * ((fConst472 * fRec103[2]) + (fConst473 * fRec103[1]))));
			fRec102[0] = ((fConst464 * (((fConst465 * fRec103[0]) + (fConst474 * fRec103[1])) + (fConst465 * fRec103[2]))) - (fConst462 * ((fConst475 * fRec102[2]) + (fConst476 * fRec102[1]))));
			float fTemp27 = (fConst462 * (((fConst463 * fRec102[0]) + (fConst477 * fRec102[1])) + (fConst463 * fRec102[2])));
			fRec101[0] = (fTemp27 - (fConst459 * ((fConst478 * fRec101[2]) + (fConst480 * fRec101[1]))));
			fRec100[0] = ((fConst459 * (((fConst461 * fRec101[0]) + (fConst481 * fRec101[1])) + (fConst461 * fRec101[2]))) - (fConst456 * ((fConst482 * fRec100[2]) + (fConst483 * fRec100[1]))));
			fRec99[0] = ((fConst456 * (((fConst458 * fRec100[0]) + (fConst484 * fRec100[1])) + (fConst458 * fRec100[2]))) - (fConst452 * ((fConst485 * fRec99[2]) + (fConst486 * fRec99[1]))));
			fRec98[0] = ((fSlow1 * fRec98[1]) + (fSlow2 * fabsf((fConst452 * (((fConst455 * fRec99[0]) + (fConst487 * fRec99[1])) + (fConst455 * fRec99[2]))))));
			fbargraph12 = (fSlow0 + (20 * log10f(fRec98[0])));
			fRec111[0] = (fTemp27 - (fConst504 * ((fConst507 * fRec111[2]) + (fConst508 * fRec111[1]))));
			fRec110[0] = ((fConst504 * (((fConst506 * fRec111[0]) + (fConst509 * fRec111[1])) + (fConst506 * fRec111[2]))) - (fConst502 * ((fConst510 * fRec110[2]) + (fConst511 * fRec110[1]))));
			fRec109[0] = ((fConst502 * (((fConst503 * fRec110[0]) + (fConst512 * fRec110[1])) + (fConst503 * fRec110[2]))) - (fConst500 * ((fConst513 * fRec109[2]) + (fConst514 * fRec109[1]))));
			float fTemp28 = (fConst500 * (((fConst501 * fRec109[0]) + (fConst515 * fRec109[1])) + (fConst501 * fRec109[2])));
			fRec108[0] = (fTemp28 - (fConst497 * ((fConst516 * fRec108[2]) + (fConst518 * fRec108[1]))));
			fRec107[0] = ((fConst497 * (((fConst499 * fRec108[0]) + (fConst519 * fRec108[1])) + (fConst499 * fRec108[2]))) - (fConst494 * ((fConst520 * fRec107[2]) + (fConst521 * fRec107[1]))));
			fRec106[0] = ((fConst494 * (((fConst496 * fRec107[0]) + (fConst522 * fRec107[1])) + (fConst496 * fRec107[2]))) - (fConst490 * ((fConst523 * fRec106[2]) + (fConst524 * fRec106[1]))));
			fRec105[0] = ((fSlow1 * fRec105[1]) + (fSlow2 * fabsf((fConst490 * (((fConst493 * fRec106[0]) + (fConst525 * fRec106[1])) + (fConst493 * fRec106[2]))))));
			fbargraph13 = (fSlow0 + (20 * log10f(fRec105[0])));
			fRec118[0] = (fTemp28 - (fConst542 * ((fConst545 * fRec118[2]) + (fConst546 * fRec118[1]))));
			fRec117[0] = ((fConst542 * (((fConst544 * fRec118[0]) + (fConst547 * fRec118[1])) + (fConst544 * fRec118[2]))) - (fConst540 * ((fConst548 * fRec117[2]) + (fConst549 * fRec117[1]))));
			fRec116[0] = ((fConst540 * (((fConst541 * fRec117[0]) + (fConst550 * fRec117[1])) + (fConst541 * fRec117[2]))) - (fConst538 * ((fConst551 * fRec116[2]) + (fConst552 * fRec116[1]))));
			float fTemp29 = (fConst538 * (((fConst539 * fRec116[0]) + (fConst553 * fRec116[1])) + (fConst539 * fRec116[2])));
			fRec115[0] = (fTemp29 - (fConst535 * ((fConst554 * fRec115[2]) + (fConst556 * fRec115[1]))));
			fRec114[0] = ((fConst535 * (((fConst537 * fRec115[0]) + (fConst557 * fRec115[1])) + (fConst537 * fRec115[2]))) - (fConst532 * ((fConst558 * fRec114[2]) + (fConst559 * fRec114[1]))));
			fRec113[0] = ((fConst532 * (((fConst534 * fRec114[0]) + (fConst560 * fRec114[1])) + (fConst534 * fRec114[2]))) - (fConst528 * ((fConst561 * fRec113[2]) + (fConst562 * fRec113[1]))));
			fRec112[0] = ((fSlow1 * fRec112[1]) + (fSlow2 * fabsf((fConst528 * (((fConst531 * fRec113[0]) + (fConst563 * fRec113[1])) + (fConst531 * fRec113[2]))))));
			fbargraph14 = (fSlow0 + (20 * log10f(fRec112[0])));
			fRec125[0] = (fTemp29 - (fConst580 * ((fConst583 * fRec125[2]) + (fConst584 * fRec125[1]))));
			fRec124[0] = ((fConst580 * (((fConst582 * fRec125[0]) + (fConst585 * fRec125[1])) + (fConst582 * fRec125[2]))) - (fConst578 * ((fConst586 * fRec124[2]) + (fConst587 * fRec124[1]))));
			fRec123[0] = ((fConst578 * (((fConst579 * fRec124[0]) + (fConst588 * fRec124[1])) + (fConst579 * fRec124[2]))) - (fConst576 * ((fConst589 * fRec123[2]) + (fConst590 * fRec123[1]))));
			float fTemp30 = (fConst576 * (((fConst577 * fRec123[0]) + (fConst591 * fRec123[1])) + (fConst577 * fRec123[2])));
			fRec122[0] = (fTemp30 - (fConst573 * ((fConst592 * fRec122[2]) + (fConst594 * fRec122[1]))));
			fRec121[0] = ((fConst573 * (((fConst575 * fRec122[0]) + (fConst595 * fRec122[1])) + (fConst575 * fRec122[2]))) - (fConst570 * ((fConst596 * fRec121[2]) + (fConst597 * fRec121[1]))));
			fRec120[0] = ((fConst570 * (((fConst572 * fRec121[0]) + (fConst598 * fRec121[1])) + (fConst572 * fRec121[2]))) - (fConst566 * ((fConst599 * fRec120[2]) + (fConst600 * fRec120[1]))));
			fRec119[0] = ((fSlow1 * fRec119[1]) + (fSlow2 * fabsf((fConst566 * (((fConst569 * fRec120[0]) + (fConst601 * fRec120[1])) + (fConst569 * fRec120[2]))))));
			fbargraph15 = (fSlow0 + (20 * log10f(fRec119[0])));
			fRec132[0] = (fTemp30 - (fConst618 * ((fConst621 * fRec132[2]) + (fConst622 * fRec132[1]))));
			fRec131[0] = ((fConst618 * (((fConst620 * fRec132[0]) + (fConst623 * fRec132[1])) + (fConst620 * fRec132[2]))) - (fConst616 * ((fConst624 * fRec131[2]) + (fConst625 * fRec131[1]))));
			fRec130[0] = ((fConst616 * (((fConst617 * fRec131[0]) + (fConst626 * fRec131[1])) + (fConst617 * fRec131[2]))) - (fConst614 * ((fConst627 * fRec130[2]) + (fConst628 * fRec130[1]))));
			float fTemp31 = (fConst614 * (((fConst615 * fRec130[0]) + (fConst629 * fRec130[1])) + (fConst615 * fRec130[2])));
			fRec129[0] = (fTemp31 - (fConst611 * ((fConst630 * fRec129[2]) + (fConst632 * fRec129[1]))));
			fRec128[0] = ((fConst611 * (((fConst613 * fRec129[0]) + (fConst633 * fRec129[1])) + (fConst613 * fRec129[2]))) - (fConst608 * ((fConst634 * fRec128[2]) + (fConst635 * fRec128[1]))));
			fRec127[0] = ((fConst608 * (((fConst610 * fRec128[0]) + (fConst636 * fRec128[1])) + (fConst610 * fRec128[2]))) - (fConst604 * ((fConst637 * fRec127[2]) + (fConst638 * fRec127[1]))));
			fRec126[0] = ((fSlow1 * fRec126[1]) + (fSlow2 * fabsf((fConst604 * (((fConst607 * fRec127[0]) + (fConst639 * fRec127[1])) + (fConst607 * fRec127[2]))))));
			fbargraph16 = (fSlow0 + (20 * log10f(fRec126[0])));
			fRec139[0] = (fTemp31 - (fConst656 * ((fConst659 * fRec139[2]) + (fConst660 * fRec139[1]))));
			fRec138[0] = ((fConst656 * (((fConst658 * fRec139[0]) + (fConst661 * fRec139[1])) + (fConst658 * fRec139[2]))) - (fConst654 * ((fConst662 * fRec138[2]) + (fConst663 * fRec138[1]))));
			fRec137[0] = ((fConst654 * (((fConst655 * fRec138[0]) + (fConst664 * fRec138[1])) + (fConst655 * fRec138[2]))) - (fConst652 * ((fConst665 * fRec137[2]) + (fConst666 * fRec137[1]))));
			float fTemp32 = (fConst652 * (((fConst653 * fRec137[0]) + (fConst667 * fRec137[1])) + (fConst653 * fRec137[2])));
			fRec136[0] = (fTemp32 - (fConst649 * ((fConst668 * fRec136[2]) + (fConst670 * fRec136[1]))));
			fRec135[0] = ((fConst649 * (((fConst651 * fRec136[0]) + (fConst671 * fRec136[1])) + (fConst651 * fRec136[2]))) - (fConst646 * ((fConst672 * fRec135[2]) + (fConst673 * fRec135[1]))));
			fRec134[0] = ((fConst646 * (((fConst648 * fRec135[0]) + (fConst674 * fRec135[1])) + (fConst648 * fRec135[2]))) - (fConst642 * ((fConst675 * fRec134[2]) + (fConst676 * fRec134[1]))));
			fRec133[0] = ((fSlow1 * fRec133[1]) + (fSlow2 * fabsf((fConst642 * (((fConst645 * fRec134[0]) + (fConst677 * fRec134[1])) + (fConst645 * fRec134[2]))))));
			fbargraph17 = (fSlow0 + (20 * log10f(fRec133[0])));
			fRec146[0] = (fTemp32 - (fConst694 * ((fConst697 * fRec146[2]) + (fConst698 * fRec146[1]))));
			fRec145[0] = ((fConst694 * (((fConst696 * fRec146[0]) + (fConst699 * fRec146[1])) + (fConst696 * fRec146[2]))) - (fConst692 * ((fConst700 * fRec145[2]) + (fConst701 * fRec145[1]))));
			fRec144[0] = ((fConst692 * (((fConst693 * fRec145[0]) + (fConst702 * fRec145[1])) + (fConst693 * fRec145[2]))) - (fConst690 * ((fConst703 * fRec144[2]) + (fConst704 * fRec144[1]))));
			float fTemp33 = (fConst690 * (((fConst691 * fRec144[0]) + (fConst705 * fRec144[1])) + (fConst691 * fRec144[2])));
			fRec143[0] = (fTemp33 - (fConst687 * ((fConst706 * fRec143[2]) + (fConst708 * fRec143[1]))));
			fRec142[0] = ((fConst687 * (((fConst689 * fRec143[0]) + (fConst709 * fRec143[1])) + (fConst689 * fRec143[2]))) - (fConst684 * ((fConst710 * fRec142[2]) + (fConst711 * fRec142[1]))));
			fRec141[0] = ((fConst684 * (((fConst686 * fRec142[0]) + (fConst712 * fRec142[1])) + (fConst686 * fRec142[2]))) - (fConst680 * ((fConst713 * fRec141[2]) + (fConst714 * fRec141[1]))));
			fRec140[0] = ((fSlow1 * fRec140[1]) + (fSlow2 * fabsf((fConst680 * (((fConst683 * fRec141[0]) + (fConst715 * fRec141[1])) + (fConst683 * fRec141[2]))))));
			fbargraph18 = (fSlow0 + (20 * log10f(fRec140[0])));
			fRec150[0] = (fTemp33 - (fConst720 * ((fConst723 * fRec150[2]) + (fConst724 * fRec150[1]))));
			fRec149[0] = ((fConst720 * (((fConst722 * fRec150[0]) + (fConst725 * fRec150[1])) + (fConst722 * fRec150[2]))) - (fConst718 * ((fConst726 * fRec149[2]) + (fConst727 * fRec149[1]))));
			fRec148[0] = ((fConst718 * (((fConst719 * fRec149[0]) + (fConst728 * fRec149[1])) + (fConst719 * fRec149[2]))) - (fConst716 * ((fConst729 * fRec148[2]) + (fConst730 * fRec148[1]))));
			fRec147[0] = ((fSlow1 * fRec147[1]) + (fSlow2 * fabsf((fConst716 * (((fConst717 * fRec148[0]) + (fConst731 * fRec148[1])) + (fConst717 * fRec148[2]))))));
			fbargraph19 = (fSlow0 + (20 * log10f(fRec147[0])));
			float fTemp34 = fTemp15;
			output0[i] = (FAUSTFLOAT)fTemp34;
			output1[i] = (FAUSTFLOAT)fTemp34;
			// post processing
			fRec147[1] = fRec147[0];
			fRec148[2] = fRec148[1]; fRec148[1] = fRec148[0];
			fRec149[2] = fRec149[1]; fRec149[1] = fRec149[0];
			fRec150[2] = fRec150[1]; fRec150[1] = fRec150[0];
			fRec140[1] = fRec140[0];
			fRec141[2] = fRec141[1]; fRec141[1] = fRec141[0];
			fRec142[2] = fRec142[1]; fRec142[1] = fRec142[0];
			fRec143[2] = fRec143[1]; fRec143[1] = fRec143[0];
			fRec144[2] = fRec144[1]; fRec144[1] = fRec144[0];
			fRec145[2] = fRec145[1]; fRec145[1] = fRec145[0];
			fRec146[2] = fRec146[1]; fRec146[1] = fRec146[0];
			fRec133[1] = fRec133[0];
			fRec134[2] = fRec134[1]; fRec134[1] = fRec134[0];
			fRec135[2] = fRec135[1]; fRec135[1] = fRec135[0];
			fRec136[2] = fRec136[1]; fRec136[1] = fRec136[0];
			fRec137[2] = fRec137[1]; fRec137[1] = fRec137[0];
			fRec138[2] = fRec138[1]; fRec138[1] = fRec138[0];
			fRec139[2] = fRec139[1]; fRec139[1] = fRec139[0];
			fRec126[1] = fRec126[0];
			fRec127[2] = fRec127[1]; fRec127[1] = fRec127[0];
			fRec128[2] = fRec128[1]; fRec128[1] = fRec128[0];
			fRec129[2] = fRec129[1]; fRec129[1] = fRec129[0];
			fRec130[2] = fRec130[1]; fRec130[1] = fRec130[0];
			fRec131[2] = fRec131[1]; fRec131[1] = fRec131[0];
			fRec132[2] = fRec132[1]; fRec132[1] = fRec132[0];
			fRec119[1] = fRec119[0];
			fRec120[2] = fRec120[1]; fRec120[1] = fRec120[0];
			fRec121[2] = fRec121[1]; fRec121[1] = fRec121[0];
			fRec122[2] = fRec122[1]; fRec122[1] = fRec122[0];
			fRec123[2] = fRec123[1]; fRec123[1] = fRec123[0];
			fRec124[2] = fRec124[1]; fRec124[1] = fRec124[0];
			fRec125[2] = fRec125[1]; fRec125[1] = fRec125[0];
			fRec112[1] = fRec112[0];
			fRec113[2] = fRec113[1]; fRec113[1] = fRec113[0];
			fRec114[2] = fRec114[1]; fRec114[1] = fRec114[0];
			fRec115[2] = fRec115[1]; fRec115[1] = fRec115[0];
			fRec116[2] = fRec116[1]; fRec116[1] = fRec116[0];
			fRec117[2] = fRec117[1]; fRec117[1] = fRec117[0];
			fRec118[2] = fRec118[1]; fRec118[1] = fRec118[0];
			fRec105[1] = fRec105[0];
			fRec106[2] = fRec106[1]; fRec106[1] = fRec106[0];
			fRec107[2] = fRec107[1]; fRec107[1] = fRec107[0];
			fRec108[2] = fRec108[1]; fRec108[1] = fRec108[0];
			fRec109[2] = fRec109[1]; fRec109[1] = fRec109[0];
			fRec110[2] = fRec110[1]; fRec110[1] = fRec110[0];
			fRec111[2] = fRec111[1]; fRec111[1] = fRec111[0];
			fRec98[1] = fRec98[0];
			fRec99[2] = fRec99[1]; fRec99[1] = fRec99[0];
			fRec100[2] = fRec100[1]; fRec100[1] = fRec100[0];
			fRec101[2] = fRec101[1]; fRec101[1] = fRec101[0];
			fRec102[2] = fRec102[1]; fRec102[1] = fRec102[0];
			fRec103[2] = fRec103[1]; fRec103[1] = fRec103[0];
			fRec104[2] = fRec104[1]; fRec104[1] = fRec104[0];
			fRec91[1] = fRec91[0];
			fRec92[2] = fRec92[1]; fRec92[1] = fRec92[0];
			fRec93[2] = fRec93[1]; fRec93[1] = fRec93[0];
			fRec94[2] = fRec94[1]; fRec94[1] = fRec94[0];
			fRec95[2] = fRec95[1]; fRec95[1] = fRec95[0];
			fRec96[2] = fRec96[1]; fRec96[1] = fRec96[0];
			fRec97[2] = fRec97[1]; fRec97[1] = fRec97[0];
			fRec84[1] = fRec84[0];
			fRec85[2] = fRec85[1]; fRec85[1] = fRec85[0];
			fRec86[2] = fRec86[1]; fRec86[1] = fRec86[0];
			fRec87[2] = fRec87[1]; fRec87[1] = fRec87[0];
			fRec88[2] = fRec88[1]; fRec88[1] = fRec88[0];
			fRec89[2] = fRec89[1]; fRec89[1] = fRec89[0];
			fRec90[2] = fRec90[1]; fRec90[1] = fRec90[0];
			fRec77[1] = fRec77[0];
			fRec78[2] = fRec78[1]; fRec78[1] = fRec78[0];
			fRec79[2] = fRec79[1]; fRec79[1] = fRec79[0];
			fRec80[2] = fRec80[1]; fRec80[1] = fRec80[0];
			fRec81[2] = fRec81[1]; fRec81[1] = fRec81[0];
			fRec82[2] = fRec82[1]; fRec82[1] = fRec82[0];
			fRec83[2] = fRec83[1]; fRec83[1] = fRec83[0];
			fRec70[1] = fRec70[0];
			fRec71[2] = fRec71[1]; fRec71[1] = fRec71[0];
			fRec72[2] = fRec72[1]; fRec72[1] = fRec72[0];
			fRec73[2] = fRec73[1]; fRec73[1] = fRec73[0];
			fRec74[2] = fRec74[1]; fRec74[1] = fRec74[0];
			fRec75[2] = fRec75[1]; fRec75[1] = fRec75[0];
			fRec76[2] = fRec76[1]; fRec76[1] = fRec76[0];
			fRec63[1] = fRec63[0];
			fRec64[2] = fRec64[1]; fRec64[1] = fRec64[0];
			fRec65[2] = fRec65[1]; fRec65[1] = fRec65[0];
			fRec66[2] = fRec66[1]; fRec66[1] = fRec66[0];
			fRec67[2] = fRec67[1]; fRec67[1] = fRec67[0];
			fRec68[2] = fRec68[1]; fRec68[1] = fRec68[0];
			fRec69[2] = fRec69[1]; fRec69[1] = fRec69[0];
			fRec56[1] = fRec56[0];
			fRec57[2] = fRec57[1]; fRec57[1] = fRec57[0];
			fRec58[2] = fRec58[1]; fRec58[1] = fRec58[0];
			fRec59[2] = fRec59[1]; fRec59[1] = fRec59[0];
			fRec60[2] = fRec60[1]; fRec60[1] = fRec60[0];
			fRec61[2] = fRec61[1]; fRec61[1] = fRec61[0];
			fRec62[2] = fRec62[1]; fRec62[1] = fRec62[0];
			fRec49[1] = fRec49[0];
			fRec50[2] = fRec50[1]; fRec50[1] = fRec50[0];
			fRec51[2] = fRec51[1]; fRec51[1] = fRec51[0];
			fRec52[2] = fRec52[1]; fRec52[1] = fRec52[0];
			fRec53[2] = fRec53[1]; fRec53[1] = fRec53[0];
			fRec54[2] = fRec54[1]; fRec54[1] = fRec54[0];
			fRec55[2] = fRec55[1]; fRec55[1] = fRec55[0];
			fRec42[1] = fRec42[0];
			fRec43[2] = fRec43[1]; fRec43[1] = fRec43[0];
			fRec44[2] = fRec44[1]; fRec44[1] = fRec44[0];
			fRec45[2] = fRec45[1]; fRec45[1] = fRec45[0];
			fRec46[2] = fRec46[1]; fRec46[1] = fRec46[0];
			fRec47[2] = fRec47[1]; fRec47[1] = fRec47[0];
			fRec48[2] = fRec48[1]; fRec48[1] = fRec48[0];
			fRec35[1] = fRec35[0];
			fRec36[2] = fRec36[1]; fRec36[1] = fRec36[0];
			fRec37[2] = fRec37[1]; fRec37[1] = fRec37[0];
			fRec38[2] = fRec38[1]; fRec38[1] = fRec38[0];
			fRec39[2] = fRec39[1]; fRec39[1] = fRec39[0];
			fRec40[2] = fRec40[1]; fRec40[1] = fRec40[0];
			fRec41[2] = fRec41[1]; fRec41[1] = fRec41[0];
			fRec28[1] = fRec28[0];
			fRec29[2] = fRec29[1]; fRec29[1] = fRec29[0];
			fRec30[2] = fRec30[1]; fRec30[1] = fRec30[0];
			fRec31[2] = fRec31[1]; fRec31[1] = fRec31[0];
			fRec32[2] = fRec32[1]; fRec32[1] = fRec32[0];
			fRec33[2] = fRec33[1]; fRec33[1] = fRec33[0];
			fRec34[2] = fRec34[1]; fRec34[1] = fRec34[0];
			fRec21[1] = fRec21[0];
			fRec22[2] = fRec22[1]; fRec22[1] = fRec22[0];
			fRec23[2] = fRec23[1]; fRec23[1] = fRec23[0];
			fRec24[2] = fRec24[1]; fRec24[1] = fRec24[0];
			fRec25[2] = fRec25[1]; fRec25[1] = fRec25[0];
			fRec26[2] = fRec26[1]; fRec26[1] = fRec26[0];
			fRec27[2] = fRec27[1]; fRec27[1] = fRec27[0];
			fRec0[1] = fRec0[0];
			fRec1[2] = fRec1[1]; fRec1[1] = fRec1[0];
			fRec2[2] = fRec2[1]; fRec2[1] = fRec2[0];
			fRec3[2] = fRec3[1]; fRec3[1] = fRec3[0];
			fRec19[2] = fRec19[1]; fRec19[1] = fRec19[0];
			fRec20[1] = fRec20[0];
			fRec4[2] = fRec4[1]; fRec4[1] = fRec4[0];
			fRec5[1] = fRec5[0];
			fVec5[1] = fVec5[0];
			fRec7[2] = fRec7[1]; fRec7[1] = fRec7[0];
			fRec17[2] = fRec17[1]; fRec17[1] = fRec17[0];
			fRec18[1] = fRec18[0];
			fRec8[2] = fRec8[1]; fRec8[1] = fRec8[0];
			fRec9[1] = fRec9[0];
			fVec4[1] = fVec4[0];
			fVec3[1] = fVec3[0];
			fRec16[1] = fRec16[0];
			fVec2[1] = fVec2[0];
			fRec15[1] = fRec15[0];
			fVec1[1] = fVec1[0];
			fRec14[1] = fRec14[0];
			fRec13[1] = fRec13[0];
			fVec0[1] = fVec0[0];
			for (int i=3; i>0; i--) fRec11[i] = fRec11[i-1];
			iRec12[1] = iRec12[0];
			fRec10[1] = fRec10[0];
			fRec6[1] = fRec6[0];
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
