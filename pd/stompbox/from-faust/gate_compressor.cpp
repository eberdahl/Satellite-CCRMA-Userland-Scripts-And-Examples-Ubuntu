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
	FAUSTFLOAT 	fcheckbox1;
	FAUSTFLOAT 	fslider2;
	float 	fRec4[2];
	FAUSTFLOAT 	fcheckbox2;
	FAUSTFLOAT 	fcheckbox3;
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
	FAUSTFLOAT 	fslider7;
	FAUSTFLOAT 	fslider8;
	float 	fConst18;
	float 	fRec18[2];
	float 	fRec17[2];
	FAUSTFLOAT 	fslider9;
	int 	iVec4[2];
	FAUSTFLOAT 	fslider10;
	int 	iRec19[2];
	float 	fRec16[2];
	float 	fRec15[2];
	FAUSTFLOAT 	fbargraph0;
	float 	fRec14[2];
	float 	fRec13[2];
	int 	iVec5[2];
	int 	iRec20[2];
	float 	fRec12[2];
	float 	fRec11[2];
	FAUSTFLOAT 	fslider11;
	FAUSTFLOAT 	fslider12;
	FAUSTFLOAT 	fslider13;
	FAUSTFLOAT 	fslider14;
	float 	fRec23[2];
	float 	fRec22[2];
	FAUSTFLOAT 	fslider15;
	float 	fRec21[2];
	float 	fRec26[2];
	float 	fRec25[2];
	float 	fRec24[2];
	FAUSTFLOAT 	fbargraph1;
	float 	fConst19;
	float 	fConst20;
	float 	fConst21;
	float 	fRec3[3];
	float 	fConst22;
	float 	fConst23;
	float 	fConst24;
	float 	fRec2[3];
	float 	fConst25;
	float 	fConst26;
	float 	fConst27;
	float 	fRec1[3];
	float 	fConst28;
	float 	fRec0[2];
	FAUSTFLOAT 	fbargraph2;
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
	float 	fConst49;
	float 	fRec33[3];
	float 	fConst50;
	float 	fConst51;
	float 	fConst52;
	float 	fRec32[3];
	float 	fConst53;
	float 	fConst54;
	float 	fConst55;
	float 	fRec31[3];
	float 	fConst56;
	float 	fConst57;
	float 	fConst58;
	float 	fConst59;
	float 	fRec30[3];
	float 	fConst60;
	float 	fConst61;
	float 	fConst62;
	float 	fRec29[3];
	float 	fConst63;
	float 	fConst64;
	float 	fConst65;
	float 	fRec28[3];
	float 	fConst66;
	float 	fRec27[2];
	FAUSTFLOAT 	fbargraph3;
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
	float 	fConst87;
	float 	fRec40[3];
	float 	fConst88;
	float 	fConst89;
	float 	fConst90;
	float 	fRec39[3];
	float 	fConst91;
	float 	fConst92;
	float 	fConst93;
	float 	fRec38[3];
	float 	fConst94;
	float 	fConst95;
	float 	fConst96;
	float 	fConst97;
	float 	fRec37[3];
	float 	fConst98;
	float 	fConst99;
	float 	fConst100;
	float 	fRec36[3];
	float 	fConst101;
	float 	fConst102;
	float 	fConst103;
	float 	fRec35[3];
	float 	fConst104;
	float 	fRec34[2];
	FAUSTFLOAT 	fbargraph4;
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
	float 	fConst125;
	float 	fRec47[3];
	float 	fConst126;
	float 	fConst127;
	float 	fConst128;
	float 	fRec46[3];
	float 	fConst129;
	float 	fConst130;
	float 	fConst131;
	float 	fRec45[3];
	float 	fConst132;
	float 	fConst133;
	float 	fConst134;
	float 	fConst135;
	float 	fRec44[3];
	float 	fConst136;
	float 	fConst137;
	float 	fConst138;
	float 	fRec43[3];
	float 	fConst139;
	float 	fConst140;
	float 	fConst141;
	float 	fRec42[3];
	float 	fConst142;
	float 	fRec41[2];
	FAUSTFLOAT 	fbargraph5;
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
	float 	fConst163;
	float 	fRec54[3];
	float 	fConst164;
	float 	fConst165;
	float 	fConst166;
	float 	fRec53[3];
	float 	fConst167;
	float 	fConst168;
	float 	fConst169;
	float 	fRec52[3];
	float 	fConst170;
	float 	fConst171;
	float 	fConst172;
	float 	fConst173;
	float 	fRec51[3];
	float 	fConst174;
	float 	fConst175;
	float 	fConst176;
	float 	fRec50[3];
	float 	fConst177;
	float 	fConst178;
	float 	fConst179;
	float 	fRec49[3];
	float 	fConst180;
	float 	fRec48[2];
	FAUSTFLOAT 	fbargraph6;
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
	float 	fConst201;
	float 	fRec61[3];
	float 	fConst202;
	float 	fConst203;
	float 	fConst204;
	float 	fRec60[3];
	float 	fConst205;
	float 	fConst206;
	float 	fConst207;
	float 	fRec59[3];
	float 	fConst208;
	float 	fConst209;
	float 	fConst210;
	float 	fConst211;
	float 	fRec58[3];
	float 	fConst212;
	float 	fConst213;
	float 	fConst214;
	float 	fRec57[3];
	float 	fConst215;
	float 	fConst216;
	float 	fConst217;
	float 	fRec56[3];
	float 	fConst218;
	float 	fRec55[2];
	FAUSTFLOAT 	fbargraph7;
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
	float 	fConst239;
	float 	fRec68[3];
	float 	fConst240;
	float 	fConst241;
	float 	fConst242;
	float 	fRec67[3];
	float 	fConst243;
	float 	fConst244;
	float 	fConst245;
	float 	fRec66[3];
	float 	fConst246;
	float 	fConst247;
	float 	fConst248;
	float 	fConst249;
	float 	fRec65[3];
	float 	fConst250;
	float 	fConst251;
	float 	fConst252;
	float 	fRec64[3];
	float 	fConst253;
	float 	fConst254;
	float 	fConst255;
	float 	fRec63[3];
	float 	fConst256;
	float 	fRec62[2];
	FAUSTFLOAT 	fbargraph8;
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
	float 	fConst277;
	float 	fRec75[3];
	float 	fConst278;
	float 	fConst279;
	float 	fConst280;
	float 	fRec74[3];
	float 	fConst281;
	float 	fConst282;
	float 	fConst283;
	float 	fRec73[3];
	float 	fConst284;
	float 	fConst285;
	float 	fConst286;
	float 	fConst287;
	float 	fRec72[3];
	float 	fConst288;
	float 	fConst289;
	float 	fConst290;
	float 	fRec71[3];
	float 	fConst291;
	float 	fConst292;
	float 	fConst293;
	float 	fRec70[3];
	float 	fConst294;
	float 	fRec69[2];
	FAUSTFLOAT 	fbargraph9;
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
	float 	fConst315;
	float 	fRec82[3];
	float 	fConst316;
	float 	fConst317;
	float 	fConst318;
	float 	fRec81[3];
	float 	fConst319;
	float 	fConst320;
	float 	fConst321;
	float 	fRec80[3];
	float 	fConst322;
	float 	fConst323;
	float 	fConst324;
	float 	fConst325;
	float 	fRec79[3];
	float 	fConst326;
	float 	fConst327;
	float 	fConst328;
	float 	fRec78[3];
	float 	fConst329;
	float 	fConst330;
	float 	fConst331;
	float 	fRec77[3];
	float 	fConst332;
	float 	fRec76[2];
	FAUSTFLOAT 	fbargraph10;
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
	float 	fConst353;
	float 	fRec89[3];
	float 	fConst354;
	float 	fConst355;
	float 	fConst356;
	float 	fRec88[3];
	float 	fConst357;
	float 	fConst358;
	float 	fConst359;
	float 	fRec87[3];
	float 	fConst360;
	float 	fConst361;
	float 	fConst362;
	float 	fConst363;
	float 	fRec86[3];
	float 	fConst364;
	float 	fConst365;
	float 	fConst366;
	float 	fRec85[3];
	float 	fConst367;
	float 	fConst368;
	float 	fConst369;
	float 	fRec84[3];
	float 	fConst370;
	float 	fRec83[2];
	FAUSTFLOAT 	fbargraph11;
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
	float 	fConst391;
	float 	fRec96[3];
	float 	fConst392;
	float 	fConst393;
	float 	fConst394;
	float 	fRec95[3];
	float 	fConst395;
	float 	fConst396;
	float 	fConst397;
	float 	fRec94[3];
	float 	fConst398;
	float 	fConst399;
	float 	fConst400;
	float 	fConst401;
	float 	fRec93[3];
	float 	fConst402;
	float 	fConst403;
	float 	fConst404;
	float 	fRec92[3];
	float 	fConst405;
	float 	fConst406;
	float 	fConst407;
	float 	fRec91[3];
	float 	fConst408;
	float 	fRec90[2];
	FAUSTFLOAT 	fbargraph12;
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
	float 	fConst429;
	float 	fRec103[3];
	float 	fConst430;
	float 	fConst431;
	float 	fConst432;
	float 	fRec102[3];
	float 	fConst433;
	float 	fConst434;
	float 	fConst435;
	float 	fRec101[3];
	float 	fConst436;
	float 	fConst437;
	float 	fConst438;
	float 	fConst439;
	float 	fRec100[3];
	float 	fConst440;
	float 	fConst441;
	float 	fConst442;
	float 	fRec99[3];
	float 	fConst443;
	float 	fConst444;
	float 	fConst445;
	float 	fRec98[3];
	float 	fConst446;
	float 	fRec97[2];
	FAUSTFLOAT 	fbargraph13;
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
	float 	fConst467;
	float 	fRec110[3];
	float 	fConst468;
	float 	fConst469;
	float 	fConst470;
	float 	fRec109[3];
	float 	fConst471;
	float 	fConst472;
	float 	fConst473;
	float 	fRec108[3];
	float 	fConst474;
	float 	fConst475;
	float 	fConst476;
	float 	fConst477;
	float 	fRec107[3];
	float 	fConst478;
	float 	fConst479;
	float 	fConst480;
	float 	fRec106[3];
	float 	fConst481;
	float 	fConst482;
	float 	fConst483;
	float 	fRec105[3];
	float 	fConst484;
	float 	fRec104[2];
	FAUSTFLOAT 	fbargraph14;
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
	float 	fConst505;
	float 	fRec117[3];
	float 	fConst506;
	float 	fConst507;
	float 	fConst508;
	float 	fRec116[3];
	float 	fConst509;
	float 	fConst510;
	float 	fConst511;
	float 	fRec115[3];
	float 	fConst512;
	float 	fConst513;
	float 	fConst514;
	float 	fConst515;
	float 	fRec114[3];
	float 	fConst516;
	float 	fConst517;
	float 	fConst518;
	float 	fRec113[3];
	float 	fConst519;
	float 	fConst520;
	float 	fConst521;
	float 	fRec112[3];
	float 	fConst522;
	float 	fRec111[2];
	FAUSTFLOAT 	fbargraph15;
	float 	fConst523;
	float 	fConst524;
	float 	fConst525;
	float 	fConst526;
	float 	fConst527;
	float 	fConst528;
	float 	fConst529;
	float 	fConst530;
	float 	fConst531;
	float 	fRec121[3];
	float 	fConst532;
	float 	fConst533;
	float 	fConst534;
	float 	fRec120[3];
	float 	fConst535;
	float 	fConst536;
	float 	fConst537;
	float 	fRec119[3];
	float 	fConst538;
	float 	fRec118[2];
	FAUSTFLOAT 	fbargraph16;
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
		fConst17 = (1.0f / fConst15);
		fslider7 = 1e+01f;
		fslider8 = 1e+02f;
		fConst18 = (2.0f / float(iConst0));
		for (int i=0; i<2; i++) fRec18[i] = 0;
		for (int i=0; i<2; i++) fRec17[i] = 0;
		fslider9 = -3e+01f;
		for (int i=0; i<2; i++) iVec4[i] = 0;
		fslider10 = 2e+02f;
		for (int i=0; i<2; i++) iRec19[i] = 0;
		for (int i=0; i<2; i++) fRec16[i] = 0;
		for (int i=0; i<2; i++) fRec15[i] = 0;
		for (int i=0; i<2; i++) fRec14[i] = 0;
		for (int i=0; i<2; i++) fRec13[i] = 0;
		for (int i=0; i<2; i++) iVec5[i] = 0;
		for (int i=0; i<2; i++) iRec20[i] = 0;
		for (int i=0; i<2; i++) fRec12[i] = 0;
		for (int i=0; i<2; i++) fRec11[i] = 0;
		fslider11 = 4e+01f;
		fslider12 = 5e+01f;
		fslider13 = 5.0f;
		fslider14 = 5e+02f;
		for (int i=0; i<2; i++) fRec23[i] = 0;
		for (int i=0; i<2; i++) fRec22[i] = 0;
		fslider15 = -3e+01f;
		for (int i=0; i<2; i++) fRec21[i] = 0;
		for (int i=0; i<2; i++) fRec26[i] = 0;
		for (int i=0; i<2; i++) fRec25[i] = 0;
		for (int i=0; i<2; i++) fRec24[i] = 0;
		fConst19 = (4.076781969643807f + ((fConst3 - 3.1897274020965583f) / fConst2));
		fConst20 = (1.0f / fConst5);
		fConst21 = (2 * (4.076781969643807f - fConst20));
		for (int i=0; i<3; i++) fRec3[i] = 0;
		fConst22 = (2 * (0.00040767818495825777f - fConst12));
		fConst23 = (1.450071084655647f + ((fConst3 - 0.7431304601070396f) / fConst2));
		fConst24 = (2 * (1.450071084655647f - fConst20));
		for (int i=0; i<3; i++) fRec2[i] = 0;
		fConst25 = (2 * (1.450071084655647f - fConst9));
		fConst26 = (0.9351401670315425f + ((fConst3 - 0.157482159302087f) / fConst2));
		fConst27 = (2 * (0.9351401670315425f - fConst20));
		for (int i=0; i<3; i++) fRec1[i] = 0;
		fConst28 = (2 * (0.9351401670315425f - fConst6));
		for (int i=0; i<2; i++) fRec0[i] = 0;
		fConst29 = tanf((19790.793572264363f / float(iConst0)));
		fConst30 = (1.0f / fConst29);
		fConst31 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst30) / fConst29)));
		fConst32 = faustpower<2>(fConst29);
		fConst33 = (50.063807016150385f / fConst32);
		fConst34 = (0.9351401670315425f + fConst33);
		fConst35 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst30) / fConst29)));
		fConst36 = (11.052052171507189f / fConst32);
		fConst37 = (1.450071084655647f + fConst36);
		fConst38 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst30) / fConst29)));
		fConst39 = (0.0017661728399818856f / fConst32);
		fConst40 = (0.00040767818495825777f + fConst39);
		fConst41 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst3) / fConst2)));
		fConst42 = (53.53615295455673f + fConst20);
		fConst43 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst3) / fConst2)));
		fConst44 = (7.621731298870603f + fConst20);
		fConst45 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst3) / fConst2)));
		fConst46 = (9.9999997055e-05f / fConst5);
		fConst47 = (0.000433227200555f + fConst46);
		fConst48 = (0.24529150870616f + ((fConst3 - 0.782413046821645f) / fConst2));
		fConst49 = (2 * (0.24529150870616f - fConst20));
		for (int i=0; i<3; i++) fRec33[i] = 0;
		fConst50 = (2 * (0.000433227200555f - fConst46));
		fConst51 = (0.689621364484675f + ((fConst3 - 0.512478641889141f) / fConst2));
		fConst52 = (2 * (0.689621364484675f - fConst20));
		for (int i=0; i<3; i++) fRec32[i] = 0;
		fConst53 = (2 * (7.621731298870603f - fConst20));
		fConst54 = (1.069358407707312f + ((fConst3 - 0.168404871113589f) / fConst2));
		fConst55 = (2 * (1.069358407707312f - fConst20));
		for (int i=0; i<3; i++) fRec31[i] = 0;
		fConst56 = (2 * (53.53615295455673f - fConst20));
		fConst57 = (4.076781969643807f + ((fConst30 - 3.1897274020965583f) / fConst29));
		fConst58 = (1.0f / fConst32);
		fConst59 = (2 * (4.076781969643807f - fConst58));
		for (int i=0; i<3; i++) fRec30[i] = 0;
		fConst60 = (2 * (0.00040767818495825777f - fConst39));
		fConst61 = (1.450071084655647f + ((fConst30 - 0.7431304601070396f) / fConst29));
		fConst62 = (2 * (1.450071084655647f - fConst58));
		for (int i=0; i<3; i++) fRec29[i] = 0;
		fConst63 = (2 * (1.450071084655647f - fConst36));
		fConst64 = (0.9351401670315425f + ((fConst30 - 0.157482159302087f) / fConst29));
		fConst65 = (2 * (0.9351401670315425f - fConst58));
		for (int i=0; i<3; i++) fRec28[i] = 0;
		fConst66 = (2 * (0.9351401670315425f - fConst33));
		for (int i=0; i<2; i++) fRec27[i] = 0;
		fConst67 = tanf((12467.418707910012f / float(iConst0)));
		fConst68 = (1.0f / fConst67);
		fConst69 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst68) / fConst67)));
		fConst70 = faustpower<2>(fConst67);
		fConst71 = (50.063807016150385f / fConst70);
		fConst72 = (0.9351401670315425f + fConst71);
		fConst73 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst68) / fConst67)));
		fConst74 = (11.052052171507189f / fConst70);
		fConst75 = (1.450071084655647f + fConst74);
		fConst76 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst68) / fConst67)));
		fConst77 = (0.0017661728399818856f / fConst70);
		fConst78 = (0.00040767818495825777f + fConst77);
		fConst79 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst30) / fConst29)));
		fConst80 = (53.53615295455673f + fConst58);
		fConst81 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst30) / fConst29)));
		fConst82 = (7.621731298870603f + fConst58);
		fConst83 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst30) / fConst29)));
		fConst84 = (9.9999997055e-05f / fConst32);
		fConst85 = (0.000433227200555f + fConst84);
		fConst86 = (0.24529150870616f + ((fConst30 - 0.782413046821645f) / fConst29));
		fConst87 = (2 * (0.24529150870616f - fConst58));
		for (int i=0; i<3; i++) fRec40[i] = 0;
		fConst88 = (2 * (0.000433227200555f - fConst84));
		fConst89 = (0.689621364484675f + ((fConst30 - 0.512478641889141f) / fConst29));
		fConst90 = (2 * (0.689621364484675f - fConst58));
		for (int i=0; i<3; i++) fRec39[i] = 0;
		fConst91 = (2 * (7.621731298870603f - fConst58));
		fConst92 = (1.069358407707312f + ((fConst30 - 0.168404871113589f) / fConst29));
		fConst93 = (2 * (1.069358407707312f - fConst58));
		for (int i=0; i<3; i++) fRec38[i] = 0;
		fConst94 = (2 * (53.53615295455673f - fConst58));
		fConst95 = (4.076781969643807f + ((fConst68 - 3.1897274020965583f) / fConst67));
		fConst96 = (1.0f / fConst70);
		fConst97 = (2 * (4.076781969643807f - fConst96));
		for (int i=0; i<3; i++) fRec37[i] = 0;
		fConst98 = (2 * (0.00040767818495825777f - fConst77));
		fConst99 = (1.450071084655647f + ((fConst68 - 0.7431304601070396f) / fConst67));
		fConst100 = (2 * (1.450071084655647f - fConst96));
		for (int i=0; i<3; i++) fRec36[i] = 0;
		fConst101 = (2 * (1.450071084655647f - fConst74));
		fConst102 = (0.9351401670315425f + ((fConst68 - 0.157482159302087f) / fConst67));
		fConst103 = (2 * (0.9351401670315425f - fConst96));
		for (int i=0; i<3; i++) fRec35[i] = 0;
		fConst104 = (2 * (0.9351401670315425f - fConst71));
		for (int i=0; i<2; i++) fRec34[i] = 0;
		fConst105 = tanf((7853.981633974483f / float(iConst0)));
		fConst106 = (1.0f / fConst105);
		fConst107 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst106) / fConst105)));
		fConst108 = faustpower<2>(fConst105);
		fConst109 = (50.063807016150385f / fConst108);
		fConst110 = (0.9351401670315425f + fConst109);
		fConst111 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst106) / fConst105)));
		fConst112 = (11.052052171507189f / fConst108);
		fConst113 = (1.450071084655647f + fConst112);
		fConst114 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst106) / fConst105)));
		fConst115 = (0.0017661728399818856f / fConst108);
		fConst116 = (0.00040767818495825777f + fConst115);
		fConst117 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst68) / fConst67)));
		fConst118 = (53.53615295455673f + fConst96);
		fConst119 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst68) / fConst67)));
		fConst120 = (7.621731298870603f + fConst96);
		fConst121 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst68) / fConst67)));
		fConst122 = (9.9999997055e-05f / fConst70);
		fConst123 = (0.000433227200555f + fConst122);
		fConst124 = (0.24529150870616f + ((fConst68 - 0.782413046821645f) / fConst67));
		fConst125 = (2 * (0.24529150870616f - fConst96));
		for (int i=0; i<3; i++) fRec47[i] = 0;
		fConst126 = (2 * (0.000433227200555f - fConst122));
		fConst127 = (0.689621364484675f + ((fConst68 - 0.512478641889141f) / fConst67));
		fConst128 = (2 * (0.689621364484675f - fConst96));
		for (int i=0; i<3; i++) fRec46[i] = 0;
		fConst129 = (2 * (7.621731298870603f - fConst96));
		fConst130 = (1.069358407707312f + ((fConst68 - 0.168404871113589f) / fConst67));
		fConst131 = (2 * (1.069358407707312f - fConst96));
		for (int i=0; i<3; i++) fRec45[i] = 0;
		fConst132 = (2 * (53.53615295455673f - fConst96));
		fConst133 = (4.076781969643807f + ((fConst106 - 3.1897274020965583f) / fConst105));
		fConst134 = (1.0f / fConst108);
		fConst135 = (2 * (4.076781969643807f - fConst134));
		for (int i=0; i<3; i++) fRec44[i] = 0;
		fConst136 = (2 * (0.00040767818495825777f - fConst115));
		fConst137 = (1.450071084655647f + ((fConst106 - 0.7431304601070396f) / fConst105));
		fConst138 = (2 * (1.450071084655647f - fConst134));
		for (int i=0; i<3; i++) fRec43[i] = 0;
		fConst139 = (2 * (1.450071084655647f - fConst112));
		fConst140 = (0.9351401670315425f + ((fConst106 - 0.157482159302087f) / fConst105));
		fConst141 = (2 * (0.9351401670315425f - fConst134));
		for (int i=0; i<3; i++) fRec42[i] = 0;
		fConst142 = (2 * (0.9351401670315425f - fConst109));
		for (int i=0; i<2; i++) fRec41[i] = 0;
		fConst143 = tanf((4947.698393066091f / float(iConst0)));
		fConst144 = (1.0f / fConst143);
		fConst145 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst144) / fConst143)));
		fConst146 = faustpower<2>(fConst143);
		fConst147 = (50.063807016150385f / fConst146);
		fConst148 = (0.9351401670315425f + fConst147);
		fConst149 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst144) / fConst143)));
		fConst150 = (11.052052171507189f / fConst146);
		fConst151 = (1.450071084655647f + fConst150);
		fConst152 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst144) / fConst143)));
		fConst153 = (0.0017661728399818856f / fConst146);
		fConst154 = (0.00040767818495825777f + fConst153);
		fConst155 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst106) / fConst105)));
		fConst156 = (53.53615295455673f + fConst134);
		fConst157 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst106) / fConst105)));
		fConst158 = (7.621731298870603f + fConst134);
		fConst159 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst106) / fConst105)));
		fConst160 = (9.9999997055e-05f / fConst108);
		fConst161 = (0.000433227200555f + fConst160);
		fConst162 = (0.24529150870616f + ((fConst106 - 0.782413046821645f) / fConst105));
		fConst163 = (2 * (0.24529150870616f - fConst134));
		for (int i=0; i<3; i++) fRec54[i] = 0;
		fConst164 = (2 * (0.000433227200555f - fConst160));
		fConst165 = (0.689621364484675f + ((fConst106 - 0.512478641889141f) / fConst105));
		fConst166 = (2 * (0.689621364484675f - fConst134));
		for (int i=0; i<3; i++) fRec53[i] = 0;
		fConst167 = (2 * (7.621731298870603f - fConst134));
		fConst168 = (1.069358407707312f + ((fConst106 - 0.168404871113589f) / fConst105));
		fConst169 = (2 * (1.069358407707312f - fConst134));
		for (int i=0; i<3; i++) fRec52[i] = 0;
		fConst170 = (2 * (53.53615295455673f - fConst134));
		fConst171 = (4.076781969643807f + ((fConst144 - 3.1897274020965583f) / fConst143));
		fConst172 = (1.0f / fConst146);
		fConst173 = (2 * (4.076781969643807f - fConst172));
		for (int i=0; i<3; i++) fRec51[i] = 0;
		fConst174 = (2 * (0.00040767818495825777f - fConst153));
		fConst175 = (1.450071084655647f + ((fConst144 - 0.7431304601070396f) / fConst143));
		fConst176 = (2 * (1.450071084655647f - fConst172));
		for (int i=0; i<3; i++) fRec50[i] = 0;
		fConst177 = (2 * (1.450071084655647f - fConst150));
		fConst178 = (0.9351401670315425f + ((fConst144 - 0.157482159302087f) / fConst143));
		fConst179 = (2 * (0.9351401670315425f - fConst172));
		for (int i=0; i<3; i++) fRec49[i] = 0;
		fConst180 = (2 * (0.9351401670315425f - fConst147));
		for (int i=0; i<2; i++) fRec48[i] = 0;
		fConst181 = tanf((3116.8546769775025f / float(iConst0)));
		fConst182 = (1.0f / fConst181);
		fConst183 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst182) / fConst181)));
		fConst184 = faustpower<2>(fConst181);
		fConst185 = (50.063807016150385f / fConst184);
		fConst186 = (0.9351401670315425f + fConst185);
		fConst187 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst182) / fConst181)));
		fConst188 = (11.052052171507189f / fConst184);
		fConst189 = (1.450071084655647f + fConst188);
		fConst190 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst182) / fConst181)));
		fConst191 = (0.0017661728399818856f / fConst184);
		fConst192 = (0.00040767818495825777f + fConst191);
		fConst193 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst144) / fConst143)));
		fConst194 = (53.53615295455673f + fConst172);
		fConst195 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst144) / fConst143)));
		fConst196 = (7.621731298870603f + fConst172);
		fConst197 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst144) / fConst143)));
		fConst198 = (9.9999997055e-05f / fConst146);
		fConst199 = (0.000433227200555f + fConst198);
		fConst200 = (0.24529150870616f + ((fConst144 - 0.782413046821645f) / fConst143));
		fConst201 = (2 * (0.24529150870616f - fConst172));
		for (int i=0; i<3; i++) fRec61[i] = 0;
		fConst202 = (2 * (0.000433227200555f - fConst198));
		fConst203 = (0.689621364484675f + ((fConst144 - 0.512478641889141f) / fConst143));
		fConst204 = (2 * (0.689621364484675f - fConst172));
		for (int i=0; i<3; i++) fRec60[i] = 0;
		fConst205 = (2 * (7.621731298870603f - fConst172));
		fConst206 = (1.069358407707312f + ((fConst144 - 0.168404871113589f) / fConst143));
		fConst207 = (2 * (1.069358407707312f - fConst172));
		for (int i=0; i<3; i++) fRec59[i] = 0;
		fConst208 = (2 * (53.53615295455673f - fConst172));
		fConst209 = (4.076781969643807f + ((fConst182 - 3.1897274020965583f) / fConst181));
		fConst210 = (1.0f / fConst184);
		fConst211 = (2 * (4.076781969643807f - fConst210));
		for (int i=0; i<3; i++) fRec58[i] = 0;
		fConst212 = (2 * (0.00040767818495825777f - fConst191));
		fConst213 = (1.450071084655647f + ((fConst182 - 0.7431304601070396f) / fConst181));
		fConst214 = (2 * (1.450071084655647f - fConst210));
		for (int i=0; i<3; i++) fRec57[i] = 0;
		fConst215 = (2 * (1.450071084655647f - fConst188));
		fConst216 = (0.9351401670315425f + ((fConst182 - 0.157482159302087f) / fConst181));
		fConst217 = (2 * (0.9351401670315425f - fConst210));
		for (int i=0; i<3; i++) fRec56[i] = 0;
		fConst218 = (2 * (0.9351401670315425f - fConst185));
		for (int i=0; i<2; i++) fRec55[i] = 0;
		fConst219 = tanf((1963.4954084936207f / float(iConst0)));
		fConst220 = (1.0f / fConst219);
		fConst221 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst220) / fConst219)));
		fConst222 = faustpower<2>(fConst219);
		fConst223 = (50.063807016150385f / fConst222);
		fConst224 = (0.9351401670315425f + fConst223);
		fConst225 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst220) / fConst219)));
		fConst226 = (11.052052171507189f / fConst222);
		fConst227 = (1.450071084655647f + fConst226);
		fConst228 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst220) / fConst219)));
		fConst229 = (0.0017661728399818856f / fConst222);
		fConst230 = (0.00040767818495825777f + fConst229);
		fConst231 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst182) / fConst181)));
		fConst232 = (53.53615295455673f + fConst210);
		fConst233 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst182) / fConst181)));
		fConst234 = (7.621731298870603f + fConst210);
		fConst235 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst182) / fConst181)));
		fConst236 = (9.9999997055e-05f / fConst184);
		fConst237 = (0.000433227200555f + fConst236);
		fConst238 = (0.24529150870616f + ((fConst182 - 0.782413046821645f) / fConst181));
		fConst239 = (2 * (0.24529150870616f - fConst210));
		for (int i=0; i<3; i++) fRec68[i] = 0;
		fConst240 = (2 * (0.000433227200555f - fConst236));
		fConst241 = (0.689621364484675f + ((fConst182 - 0.512478641889141f) / fConst181));
		fConst242 = (2 * (0.689621364484675f - fConst210));
		for (int i=0; i<3; i++) fRec67[i] = 0;
		fConst243 = (2 * (7.621731298870603f - fConst210));
		fConst244 = (1.069358407707312f + ((fConst182 - 0.168404871113589f) / fConst181));
		fConst245 = (2 * (1.069358407707312f - fConst210));
		for (int i=0; i<3; i++) fRec66[i] = 0;
		fConst246 = (2 * (53.53615295455673f - fConst210));
		fConst247 = (4.076781969643807f + ((fConst220 - 3.1897274020965583f) / fConst219));
		fConst248 = (1.0f / fConst222);
		fConst249 = (2 * (4.076781969643807f - fConst248));
		for (int i=0; i<3; i++) fRec65[i] = 0;
		fConst250 = (2 * (0.00040767818495825777f - fConst229));
		fConst251 = (1.450071084655647f + ((fConst220 - 0.7431304601070396f) / fConst219));
		fConst252 = (2 * (1.450071084655647f - fConst248));
		for (int i=0; i<3; i++) fRec64[i] = 0;
		fConst253 = (2 * (1.450071084655647f - fConst226));
		fConst254 = (0.9351401670315425f + ((fConst220 - 0.157482159302087f) / fConst219));
		fConst255 = (2 * (0.9351401670315425f - fConst248));
		for (int i=0; i<3; i++) fRec63[i] = 0;
		fConst256 = (2 * (0.9351401670315425f - fConst223));
		for (int i=0; i<2; i++) fRec62[i] = 0;
		fConst257 = tanf((1236.9245982665225f / float(iConst0)));
		fConst258 = (1.0f / fConst257);
		fConst259 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst258) / fConst257)));
		fConst260 = faustpower<2>(fConst257);
		fConst261 = (50.063807016150385f / fConst260);
		fConst262 = (0.9351401670315425f + fConst261);
		fConst263 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst258) / fConst257)));
		fConst264 = (11.052052171507189f / fConst260);
		fConst265 = (1.450071084655647f + fConst264);
		fConst266 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst258) / fConst257)));
		fConst267 = (0.0017661728399818856f / fConst260);
		fConst268 = (0.00040767818495825777f + fConst267);
		fConst269 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst220) / fConst219)));
		fConst270 = (53.53615295455673f + fConst248);
		fConst271 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst220) / fConst219)));
		fConst272 = (7.621731298870603f + fConst248);
		fConst273 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst220) / fConst219)));
		fConst274 = (9.9999997055e-05f / fConst222);
		fConst275 = (0.000433227200555f + fConst274);
		fConst276 = (0.24529150870616f + ((fConst220 - 0.782413046821645f) / fConst219));
		fConst277 = (2 * (0.24529150870616f - fConst248));
		for (int i=0; i<3; i++) fRec75[i] = 0;
		fConst278 = (2 * (0.000433227200555f - fConst274));
		fConst279 = (0.689621364484675f + ((fConst220 - 0.512478641889141f) / fConst219));
		fConst280 = (2 * (0.689621364484675f - fConst248));
		for (int i=0; i<3; i++) fRec74[i] = 0;
		fConst281 = (2 * (7.621731298870603f - fConst248));
		fConst282 = (1.069358407707312f + ((fConst220 - 0.168404871113589f) / fConst219));
		fConst283 = (2 * (1.069358407707312f - fConst248));
		for (int i=0; i<3; i++) fRec73[i] = 0;
		fConst284 = (2 * (53.53615295455673f - fConst248));
		fConst285 = (4.076781969643807f + ((fConst258 - 3.1897274020965583f) / fConst257));
		fConst286 = (1.0f / fConst260);
		fConst287 = (2 * (4.076781969643807f - fConst286));
		for (int i=0; i<3; i++) fRec72[i] = 0;
		fConst288 = (2 * (0.00040767818495825777f - fConst267));
		fConst289 = (1.450071084655647f + ((fConst258 - 0.7431304601070396f) / fConst257));
		fConst290 = (2 * (1.450071084655647f - fConst286));
		for (int i=0; i<3; i++) fRec71[i] = 0;
		fConst291 = (2 * (1.450071084655647f - fConst264));
		fConst292 = (0.9351401670315425f + ((fConst258 - 0.157482159302087f) / fConst257));
		fConst293 = (2 * (0.9351401670315425f - fConst286));
		for (int i=0; i<3; i++) fRec70[i] = 0;
		fConst294 = (2 * (0.9351401670315425f - fConst261));
		for (int i=0; i<2; i++) fRec69[i] = 0;
		fConst295 = tanf((779.213669244376f / float(iConst0)));
		fConst296 = (1.0f / fConst295);
		fConst297 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst296) / fConst295)));
		fConst298 = faustpower<2>(fConst295);
		fConst299 = (50.063807016150385f / fConst298);
		fConst300 = (0.9351401670315425f + fConst299);
		fConst301 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst296) / fConst295)));
		fConst302 = (11.052052171507189f / fConst298);
		fConst303 = (1.450071084655647f + fConst302);
		fConst304 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst296) / fConst295)));
		fConst305 = (0.0017661728399818856f / fConst298);
		fConst306 = (0.00040767818495825777f + fConst305);
		fConst307 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst258) / fConst257)));
		fConst308 = (53.53615295455673f + fConst286);
		fConst309 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst258) / fConst257)));
		fConst310 = (7.621731298870603f + fConst286);
		fConst311 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst258) / fConst257)));
		fConst312 = (9.9999997055e-05f / fConst260);
		fConst313 = (0.000433227200555f + fConst312);
		fConst314 = (0.24529150870616f + ((fConst258 - 0.782413046821645f) / fConst257));
		fConst315 = (2 * (0.24529150870616f - fConst286));
		for (int i=0; i<3; i++) fRec82[i] = 0;
		fConst316 = (2 * (0.000433227200555f - fConst312));
		fConst317 = (0.689621364484675f + ((fConst258 - 0.512478641889141f) / fConst257));
		fConst318 = (2 * (0.689621364484675f - fConst286));
		for (int i=0; i<3; i++) fRec81[i] = 0;
		fConst319 = (2 * (7.621731298870603f - fConst286));
		fConst320 = (1.069358407707312f + ((fConst258 - 0.168404871113589f) / fConst257));
		fConst321 = (2 * (1.069358407707312f - fConst286));
		for (int i=0; i<3; i++) fRec80[i] = 0;
		fConst322 = (2 * (53.53615295455673f - fConst286));
		fConst323 = (4.076781969643807f + ((fConst296 - 3.1897274020965583f) / fConst295));
		fConst324 = (1.0f / fConst298);
		fConst325 = (2 * (4.076781969643807f - fConst324));
		for (int i=0; i<3; i++) fRec79[i] = 0;
		fConst326 = (2 * (0.00040767818495825777f - fConst305));
		fConst327 = (1.450071084655647f + ((fConst296 - 0.7431304601070396f) / fConst295));
		fConst328 = (2 * (1.450071084655647f - fConst324));
		for (int i=0; i<3; i++) fRec78[i] = 0;
		fConst329 = (2 * (1.450071084655647f - fConst302));
		fConst330 = (0.9351401670315425f + ((fConst296 - 0.157482159302087f) / fConst295));
		fConst331 = (2 * (0.9351401670315425f - fConst324));
		for (int i=0; i<3; i++) fRec77[i] = 0;
		fConst332 = (2 * (0.9351401670315425f - fConst299));
		for (int i=0; i<2; i++) fRec76[i] = 0;
		fConst333 = tanf((490.8738521234052f / float(iConst0)));
		fConst334 = (1.0f / fConst333);
		fConst335 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst334) / fConst333)));
		fConst336 = faustpower<2>(fConst333);
		fConst337 = (50.063807016150385f / fConst336);
		fConst338 = (0.9351401670315425f + fConst337);
		fConst339 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst334) / fConst333)));
		fConst340 = (11.052052171507189f / fConst336);
		fConst341 = (1.450071084655647f + fConst340);
		fConst342 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst334) / fConst333)));
		fConst343 = (0.0017661728399818856f / fConst336);
		fConst344 = (0.00040767818495825777f + fConst343);
		fConst345 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst296) / fConst295)));
		fConst346 = (53.53615295455673f + fConst324);
		fConst347 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst296) / fConst295)));
		fConst348 = (7.621731298870603f + fConst324);
		fConst349 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst296) / fConst295)));
		fConst350 = (9.9999997055e-05f / fConst298);
		fConst351 = (0.000433227200555f + fConst350);
		fConst352 = (0.24529150870616f + ((fConst296 - 0.782413046821645f) / fConst295));
		fConst353 = (2 * (0.24529150870616f - fConst324));
		for (int i=0; i<3; i++) fRec89[i] = 0;
		fConst354 = (2 * (0.000433227200555f - fConst350));
		fConst355 = (0.689621364484675f + ((fConst296 - 0.512478641889141f) / fConst295));
		fConst356 = (2 * (0.689621364484675f - fConst324));
		for (int i=0; i<3; i++) fRec88[i] = 0;
		fConst357 = (2 * (7.621731298870603f - fConst324));
		fConst358 = (1.069358407707312f + ((fConst296 - 0.168404871113589f) / fConst295));
		fConst359 = (2 * (1.069358407707312f - fConst324));
		for (int i=0; i<3; i++) fRec87[i] = 0;
		fConst360 = (2 * (53.53615295455673f - fConst324));
		fConst361 = (4.076781969643807f + ((fConst334 - 3.1897274020965583f) / fConst333));
		fConst362 = (1.0f / fConst336);
		fConst363 = (2 * (4.076781969643807f - fConst362));
		for (int i=0; i<3; i++) fRec86[i] = 0;
		fConst364 = (2 * (0.00040767818495825777f - fConst343));
		fConst365 = (1.450071084655647f + ((fConst334 - 0.7431304601070396f) / fConst333));
		fConst366 = (2 * (1.450071084655647f - fConst362));
		for (int i=0; i<3; i++) fRec85[i] = 0;
		fConst367 = (2 * (1.450071084655647f - fConst340));
		fConst368 = (0.9351401670315425f + ((fConst334 - 0.157482159302087f) / fConst333));
		fConst369 = (2 * (0.9351401670315425f - fConst362));
		for (int i=0; i<3; i++) fRec84[i] = 0;
		fConst370 = (2 * (0.9351401670315425f - fConst337));
		for (int i=0; i<2; i++) fRec83[i] = 0;
		fConst371 = tanf((309.2311495666306f / float(iConst0)));
		fConst372 = (1.0f / fConst371);
		fConst373 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst372) / fConst371)));
		fConst374 = faustpower<2>(fConst371);
		fConst375 = (50.063807016150385f / fConst374);
		fConst376 = (0.9351401670315425f + fConst375);
		fConst377 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst372) / fConst371)));
		fConst378 = (11.052052171507189f / fConst374);
		fConst379 = (1.450071084655647f + fConst378);
		fConst380 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst372) / fConst371)));
		fConst381 = (0.0017661728399818856f / fConst374);
		fConst382 = (0.00040767818495825777f + fConst381);
		fConst383 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst334) / fConst333)));
		fConst384 = (53.53615295455673f + fConst362);
		fConst385 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst334) / fConst333)));
		fConst386 = (7.621731298870603f + fConst362);
		fConst387 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst334) / fConst333)));
		fConst388 = (9.9999997055e-05f / fConst336);
		fConst389 = (0.000433227200555f + fConst388);
		fConst390 = (0.24529150870616f + ((fConst334 - 0.782413046821645f) / fConst333));
		fConst391 = (2 * (0.24529150870616f - fConst362));
		for (int i=0; i<3; i++) fRec96[i] = 0;
		fConst392 = (2 * (0.000433227200555f - fConst388));
		fConst393 = (0.689621364484675f + ((fConst334 - 0.512478641889141f) / fConst333));
		fConst394 = (2 * (0.689621364484675f - fConst362));
		for (int i=0; i<3; i++) fRec95[i] = 0;
		fConst395 = (2 * (7.621731298870603f - fConst362));
		fConst396 = (1.069358407707312f + ((fConst334 - 0.168404871113589f) / fConst333));
		fConst397 = (2 * (1.069358407707312f - fConst362));
		for (int i=0; i<3; i++) fRec94[i] = 0;
		fConst398 = (2 * (53.53615295455673f - fConst362));
		fConst399 = (4.076781969643807f + ((fConst372 - 3.1897274020965583f) / fConst371));
		fConst400 = (1.0f / fConst374);
		fConst401 = (2 * (4.076781969643807f - fConst400));
		for (int i=0; i<3; i++) fRec93[i] = 0;
		fConst402 = (2 * (0.00040767818495825777f - fConst381));
		fConst403 = (1.450071084655647f + ((fConst372 - 0.7431304601070396f) / fConst371));
		fConst404 = (2 * (1.450071084655647f - fConst400));
		for (int i=0; i<3; i++) fRec92[i] = 0;
		fConst405 = (2 * (1.450071084655647f - fConst378));
		fConst406 = (0.9351401670315425f + ((fConst372 - 0.157482159302087f) / fConst371));
		fConst407 = (2 * (0.9351401670315425f - fConst400));
		for (int i=0; i<3; i++) fRec91[i] = 0;
		fConst408 = (2 * (0.9351401670315425f - fConst375));
		for (int i=0; i<2; i++) fRec90[i] = 0;
		fConst409 = tanf((194.803417311094f / float(iConst0)));
		fConst410 = (1.0f / fConst409);
		fConst411 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst410) / fConst409)));
		fConst412 = faustpower<2>(fConst409);
		fConst413 = (50.063807016150385f / fConst412);
		fConst414 = (0.9351401670315425f + fConst413);
		fConst415 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst410) / fConst409)));
		fConst416 = (11.052052171507189f / fConst412);
		fConst417 = (1.450071084655647f + fConst416);
		fConst418 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst410) / fConst409)));
		fConst419 = (0.0017661728399818856f / fConst412);
		fConst420 = (0.00040767818495825777f + fConst419);
		fConst421 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst372) / fConst371)));
		fConst422 = (53.53615295455673f + fConst400);
		fConst423 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst372) / fConst371)));
		fConst424 = (7.621731298870603f + fConst400);
		fConst425 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst372) / fConst371)));
		fConst426 = (9.9999997055e-05f / fConst374);
		fConst427 = (0.000433227200555f + fConst426);
		fConst428 = (0.24529150870616f + ((fConst372 - 0.782413046821645f) / fConst371));
		fConst429 = (2 * (0.24529150870616f - fConst400));
		for (int i=0; i<3; i++) fRec103[i] = 0;
		fConst430 = (2 * (0.000433227200555f - fConst426));
		fConst431 = (0.689621364484675f + ((fConst372 - 0.512478641889141f) / fConst371));
		fConst432 = (2 * (0.689621364484675f - fConst400));
		for (int i=0; i<3; i++) fRec102[i] = 0;
		fConst433 = (2 * (7.621731298870603f - fConst400));
		fConst434 = (1.069358407707312f + ((fConst372 - 0.168404871113589f) / fConst371));
		fConst435 = (2 * (1.069358407707312f - fConst400));
		for (int i=0; i<3; i++) fRec101[i] = 0;
		fConst436 = (2 * (53.53615295455673f - fConst400));
		fConst437 = (4.076781969643807f + ((fConst410 - 3.1897274020965583f) / fConst409));
		fConst438 = (1.0f / fConst412);
		fConst439 = (2 * (4.076781969643807f - fConst438));
		for (int i=0; i<3; i++) fRec100[i] = 0;
		fConst440 = (2 * (0.00040767818495825777f - fConst419));
		fConst441 = (1.450071084655647f + ((fConst410 - 0.7431304601070396f) / fConst409));
		fConst442 = (2 * (1.450071084655647f - fConst438));
		for (int i=0; i<3; i++) fRec99[i] = 0;
		fConst443 = (2 * (1.450071084655647f - fConst416));
		fConst444 = (0.9351401670315425f + ((fConst410 - 0.157482159302087f) / fConst409));
		fConst445 = (2 * (0.9351401670315425f - fConst438));
		for (int i=0; i<3; i++) fRec98[i] = 0;
		fConst446 = (2 * (0.9351401670315425f - fConst413));
		for (int i=0; i<2; i++) fRec97[i] = 0;
		fConst447 = tanf((122.7184630308513f / float(iConst0)));
		fConst448 = (1.0f / fConst447);
		fConst449 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst448) / fConst447)));
		fConst450 = faustpower<2>(fConst447);
		fConst451 = (50.063807016150385f / fConst450);
		fConst452 = (0.9351401670315425f + fConst451);
		fConst453 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst448) / fConst447)));
		fConst454 = (11.052052171507189f / fConst450);
		fConst455 = (1.450071084655647f + fConst454);
		fConst456 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst448) / fConst447)));
		fConst457 = (0.0017661728399818856f / fConst450);
		fConst458 = (0.00040767818495825777f + fConst457);
		fConst459 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst410) / fConst409)));
		fConst460 = (53.53615295455673f + fConst438);
		fConst461 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst410) / fConst409)));
		fConst462 = (7.621731298870603f + fConst438);
		fConst463 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst410) / fConst409)));
		fConst464 = (9.9999997055e-05f / fConst412);
		fConst465 = (0.000433227200555f + fConst464);
		fConst466 = (0.24529150870616f + ((fConst410 - 0.782413046821645f) / fConst409));
		fConst467 = (2 * (0.24529150870616f - fConst438));
		for (int i=0; i<3; i++) fRec110[i] = 0;
		fConst468 = (2 * (0.000433227200555f - fConst464));
		fConst469 = (0.689621364484675f + ((fConst410 - 0.512478641889141f) / fConst409));
		fConst470 = (2 * (0.689621364484675f - fConst438));
		for (int i=0; i<3; i++) fRec109[i] = 0;
		fConst471 = (2 * (7.621731298870603f - fConst438));
		fConst472 = (1.069358407707312f + ((fConst410 - 0.168404871113589f) / fConst409));
		fConst473 = (2 * (1.069358407707312f - fConst438));
		for (int i=0; i<3; i++) fRec108[i] = 0;
		fConst474 = (2 * (53.53615295455673f - fConst438));
		fConst475 = (4.076781969643807f + ((fConst448 - 3.1897274020965583f) / fConst447));
		fConst476 = (1.0f / fConst450);
		fConst477 = (2 * (4.076781969643807f - fConst476));
		for (int i=0; i<3; i++) fRec107[i] = 0;
		fConst478 = (2 * (0.00040767818495825777f - fConst457));
		fConst479 = (1.450071084655647f + ((fConst448 - 0.7431304601070396f) / fConst447));
		fConst480 = (2 * (1.450071084655647f - fConst476));
		for (int i=0; i<3; i++) fRec106[i] = 0;
		fConst481 = (2 * (1.450071084655647f - fConst454));
		fConst482 = (0.9351401670315425f + ((fConst448 - 0.157482159302087f) / fConst447));
		fConst483 = (2 * (0.9351401670315425f - fConst476));
		for (int i=0; i<3; i++) fRec105[i] = 0;
		fConst484 = (2 * (0.9351401670315425f - fConst451));
		for (int i=0; i<2; i++) fRec104[i] = 0;
		fConst485 = tanf((77.3077873916577f / float(iConst0)));
		fConst486 = (1.0f / fConst485);
		fConst487 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst486) / fConst485)));
		fConst488 = faustpower<2>(fConst485);
		fConst489 = (50.063807016150385f / fConst488);
		fConst490 = (0.9351401670315425f + fConst489);
		fConst491 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst486) / fConst485)));
		fConst492 = (11.052052171507189f / fConst488);
		fConst493 = (1.450071084655647f + fConst492);
		fConst494 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst486) / fConst485)));
		fConst495 = (0.0017661728399818856f / fConst488);
		fConst496 = (0.00040767818495825777f + fConst495);
		fConst497 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst448) / fConst447)));
		fConst498 = (53.53615295455673f + fConst476);
		fConst499 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst448) / fConst447)));
		fConst500 = (7.621731298870603f + fConst476);
		fConst501 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst448) / fConst447)));
		fConst502 = (9.9999997055e-05f / fConst450);
		fConst503 = (0.000433227200555f + fConst502);
		fConst504 = (0.24529150870616f + ((fConst448 - 0.782413046821645f) / fConst447));
		fConst505 = (2 * (0.24529150870616f - fConst476));
		for (int i=0; i<3; i++) fRec117[i] = 0;
		fConst506 = (2 * (0.000433227200555f - fConst502));
		fConst507 = (0.689621364484675f + ((fConst448 - 0.512478641889141f) / fConst447));
		fConst508 = (2 * (0.689621364484675f - fConst476));
		for (int i=0; i<3; i++) fRec116[i] = 0;
		fConst509 = (2 * (7.621731298870603f - fConst476));
		fConst510 = (1.069358407707312f + ((fConst448 - 0.168404871113589f) / fConst447));
		fConst511 = (2 * (1.069358407707312f - fConst476));
		for (int i=0; i<3; i++) fRec115[i] = 0;
		fConst512 = (2 * (53.53615295455673f - fConst476));
		fConst513 = (4.076781969643807f + ((fConst486 - 3.1897274020965583f) / fConst485));
		fConst514 = (1.0f / fConst488);
		fConst515 = (2 * (4.076781969643807f - fConst514));
		for (int i=0; i<3; i++) fRec114[i] = 0;
		fConst516 = (2 * (0.00040767818495825777f - fConst495));
		fConst517 = (1.450071084655647f + ((fConst486 - 0.7431304601070396f) / fConst485));
		fConst518 = (2 * (1.450071084655647f - fConst514));
		for (int i=0; i<3; i++) fRec113[i] = 0;
		fConst519 = (2 * (1.450071084655647f - fConst492));
		fConst520 = (0.9351401670315425f + ((fConst486 - 0.157482159302087f) / fConst485));
		fConst521 = (2 * (0.9351401670315425f - fConst514));
		for (int i=0; i<3; i++) fRec112[i] = 0;
		fConst522 = (2 * (0.9351401670315425f - fConst489));
		for (int i=0; i<2; i++) fRec111[i] = 0;
		fConst523 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst486) / fConst485)));
		fConst524 = (53.53615295455673f + fConst514);
		fConst525 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst486) / fConst485)));
		fConst526 = (7.621731298870603f + fConst514);
		fConst527 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst486) / fConst485)));
		fConst528 = (9.9999997055e-05f / fConst488);
		fConst529 = (0.000433227200555f + fConst528);
		fConst530 = (0.24529150870616f + ((fConst486 - 0.782413046821645f) / fConst485));
		fConst531 = (2 * (0.24529150870616f - fConst514));
		for (int i=0; i<3; i++) fRec121[i] = 0;
		fConst532 = (2 * (0.000433227200555f - fConst528));
		fConst533 = (0.689621364484675f + ((fConst486 - 0.512478641889141f) / fConst485));
		fConst534 = (2 * (0.689621364484675f - fConst514));
		for (int i=0; i<3; i++) fRec120[i] = 0;
		fConst535 = (2 * (7.621731298870603f - fConst514));
		fConst536 = (1.069358407707312f + ((fConst486 - 0.168404871113589f) / fConst485));
		fConst537 = (2 * (1.069358407707312f - fConst514));
		for (int i=0; i<3; i++) fRec119[i] = 0;
		fConst538 = (2 * (53.53615295455673f - fConst514));
		for (int i=0; i<2; i++) fRec118[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("gate_compressor");
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
		interface->declare(0, "tooltip", "Reference: http://en.wikipedia.org/wiki/Noise_gate");
		interface->openVerticalBox("GATE");
		interface->declare(0, "0", "");
		interface->openHorizontalBox("");
		interface->declare(&fcheckbox1, "0", "");
		interface->declare(&fcheckbox1, "tooltip", "When this is checked, the gate has no effect");
		interface->addCheckButton("Bypass", &fcheckbox1);
		interface->declare(&fbargraph0, "1", "");
		interface->declare(&fbargraph0, "tooltip", "Current gain of the gate in dB");
		interface->declare(&fbargraph0, "unit", "dB");
		interface->addHorizontalBargraph("Gate Gain", &fbargraph0, -5e+01f, 1e+01f);
		interface->closeBox();
		interface->declare(0, "1", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider9, "1", "");
		interface->declare(&fslider9, "style", "knob");
		interface->declare(&fslider9, "tooltip", "When the signal level falls below the Threshold (expressed in dB), the signal is muted");
		interface->declare(&fslider9, "unit", "dB");
		interface->addHorizontalSlider("Threshold", &fslider9, -3e+01f, -1.2e+02f, 0.0f, 0.1f);
		interface->declare(&fslider7, "2", "");
		interface->declare(&fslider7, "style", "knob");
		interface->declare(&fslider7, "tooltip", "Time constant in MICROseconds (1/e smoothing time) for the gate gain to go (exponentially) from 0 (muted) to 1 (unmuted)");
		interface->declare(&fslider7, "unit", "us");
		interface->addHorizontalSlider("Attack", &fslider7, 1e+01f, 1e+01f, 1e+04f, 1.0f);
		interface->declare(&fslider10, "3", "");
		interface->declare(&fslider10, "style", "knob");
		interface->declare(&fslider10, "tooltip", "Time in ms to keep the gate open (no muting) after the signal level falls below the Threshold");
		interface->declare(&fslider10, "unit", "ms");
		interface->addHorizontalSlider("Hold", &fslider10, 2e+02f, 0.0f, 1e+03f, 1.0f);
		interface->declare(&fslider8, "4", "");
		interface->declare(&fslider8, "style", "knob");
		interface->declare(&fslider8, "tooltip", "Time constant in ms (1/e smoothing time) for the gain to go (exponentially) from 1 (unmuted) to 0 (muted)");
		interface->declare(&fslider8, "unit", "ms");
		interface->addHorizontalSlider("Release", &fslider8, 1e+02f, 0.0f, 1e+03f, 1.0f);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "3", "");
		interface->openVerticalBox("");
		interface->declare(0, "tooltip", "Reference: http://en.wikipedia.org/wiki/Dynamic_range_compression");
		interface->openVerticalBox("COMPRESSOR");
		interface->declare(0, "0", "");
		interface->openHorizontalBox("");
		interface->declare(&fcheckbox0, "0", "");
		interface->declare(&fcheckbox0, "tooltip", "When this is checked, the compressor has no effect");
		interface->addCheckButton("Bypass", &fcheckbox0);
		interface->declare(&fbargraph1, "1", "");
		interface->declare(&fbargraph1, "tooltip", "Current gain of the compressor in dB");
		interface->declare(&fbargraph1, "unit", "dB");
		interface->addHorizontalBargraph("Compressor Gain", &fbargraph1, -5e+01f, 1e+01f);
		interface->closeBox();
		interface->declare(0, "1", "");
		interface->openHorizontalBox("");
		interface->declare(0, "3", "");
		interface->openHorizontalBox("Compression Control");
		interface->declare(&fslider13, "0", "");
		interface->declare(&fslider13, "style", "knob");
		interface->declare(&fslider13, "tooltip", "A compression Ratio of N means that for each N dB increase in input signal level above Threshold, the output level goes up 1 dB");
		interface->addHorizontalSlider("Ratio", &fslider13, 5.0f, 1.0f, 2e+01f, 0.1f);
		interface->declare(&fslider15, "1", "");
		interface->declare(&fslider15, "style", "knob");
		interface->declare(&fslider15, "tooltip", "When the signal level exceeds the Threshold (in dB), its level is compressed according to the Ratio");
		interface->declare(&fslider15, "unit", "dB");
		interface->addHorizontalSlider("Threshold", &fslider15, -3e+01f, -1e+02f, 1e+01f, 0.1f);
		interface->closeBox();
		interface->declare(0, "4", "");
		interface->openHorizontalBox("Compression Response");
		interface->declare(&fslider12, "1", "");
		interface->declare(&fslider12, "style", "knob");
		interface->declare(&fslider12, "tooltip", "Time constant in ms (1/e smoothing time) for the compression gain to approach (exponentially) a new lower target level (the compression `kicking in')");
		interface->declare(&fslider12, "unit", "ms");
		interface->addHorizontalSlider("Attack", &fslider12, 5e+01f, 0.0f, 5e+02f, 0.1f);
		interface->declare(&fslider14, "2", "");
		interface->declare(&fslider14, "style", "knob");
		interface->declare(&fslider14, "tooltip", "Time constant in ms (1/e smoothing time) for the compression gain to approach (exponentially) a new higher target level (the compression 'releasing')");
		interface->declare(&fslider14, "unit", "ms");
		interface->addHorizontalSlider("Release", &fslider14, 5e+02f, 0.0f, 1e+03f, 0.1f);
		interface->closeBox();
		interface->closeBox();
		interface->declare(&fslider11, "5", "");
		interface->declare(&fslider11, "tooltip", "The compressed-signal output level is increased by this amount (in dB) to make up for the level lost due to compression");
		interface->declare(&fslider11, "unit", "dB");
		interface->addHorizontalSlider("Makeup Gain", &fslider11, 4e+01f, -96.0f, 96.0f, 0.1f);
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "4", "");
		interface->openVerticalBox("");
		interface->declare(0, "0", "");
		interface->declare(0, "tooltip", "See Faust's filter.lib for documentation and references");
		interface->openHorizontalBox("CONSTANT-Q SPECTRUM ANALYZER (6E)");
		interface->declare(&fbargraph16, "0", "");
		interface->declare(&fbargraph16, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph16, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph16, -5e+01f, 1e+01f);
		interface->declare(&fbargraph15, "1", "");
		interface->declare(&fbargraph15, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph15, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph15, -5e+01f, 1e+01f);
		interface->declare(&fbargraph14, "2", "");
		interface->declare(&fbargraph14, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph14, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph14, -5e+01f, 1e+01f);
		interface->declare(&fbargraph13, "3", "");
		interface->declare(&fbargraph13, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph13, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph13, -5e+01f, 1e+01f);
		interface->declare(&fbargraph12, "4", "");
		interface->declare(&fbargraph12, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph12, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph12, -5e+01f, 1e+01f);
		interface->declare(&fbargraph11, "5", "");
		interface->declare(&fbargraph11, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph11, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph11, -5e+01f, 1e+01f);
		interface->declare(&fbargraph10, "6", "");
		interface->declare(&fbargraph10, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph10, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph10, -5e+01f, 1e+01f);
		interface->declare(&fbargraph9, "7", "");
		interface->declare(&fbargraph9, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph9, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph9, -5e+01f, 1e+01f);
		interface->declare(&fbargraph8, "8", "");
		interface->declare(&fbargraph8, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph8, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph8, -5e+01f, 1e+01f);
		interface->declare(&fbargraph7, "9", "");
		interface->declare(&fbargraph7, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph7, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph7, -5e+01f, 1e+01f);
		interface->declare(&fbargraph6, "10", "");
		interface->declare(&fbargraph6, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph6, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph6, -5e+01f, 1e+01f);
		interface->declare(&fbargraph5, "11", "");
		interface->declare(&fbargraph5, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph5, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph5, -5e+01f, 1e+01f);
		interface->declare(&fbargraph4, "12", "");
		interface->declare(&fbargraph4, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph4, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph4, -5e+01f, 1e+01f);
		interface->declare(&fbargraph3, "13", "");
		interface->declare(&fbargraph3, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph3, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph3, -5e+01f, 1e+01f);
		interface->declare(&fbargraph2, "14", "");
		interface->declare(&fbargraph2, "tooltip", "Spectral Band Level in dB");
		interface->declare(&fbargraph2, "unit", "dB");
		interface->addVerticalBargraph("", &fbargraph2, -5e+01f, 1e+01f);
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
		float 	fSlow18 = max(fConst17, (1e-06f * fslider7));
		float 	fSlow19 = expf((0 - (fConst1 / fSlow18)));
		float 	fSlow20 = (1.0f - fSlow19);
		float 	fSlow21 = max(fConst17, (0.001f * fslider8));
		float 	fSlow22 = expf((0 - (fConst1 / fSlow21)));
		float 	fSlow23 = expf((0 - (fConst18 / fSlow18)));
		float 	fSlow24 = (1.0f - fSlow23);
		float 	fSlow25 = expf((0 - (fConst18 / fSlow21)));
		float 	fSlow26 = (1.0f - fSlow25);
		float 	fSlow27 = powf(10,(0.05f * fslider9));
		int 	iSlow28 = int((iConst0 * max(fConst17, (0.001f * fslider10))));
		float 	fSlow29 = (1.0f - fSlow22);
		float 	fSlow30 = powf(10,(0.05f * fslider11));
		float 	fSlow31 = max(fConst1, (0.001f * fslider12));
		float 	fSlow32 = expf((0 - (fConst18 / fSlow31)));
		float 	fSlow33 = (((1.0f / float(fslider13)) - 1.0f) * (1.0f - fSlow32));
		float 	fSlow34 = expf((0 - (fConst1 / fSlow31)));
		float 	fSlow35 = (1.0f - fSlow34);
		float 	fSlow36 = expf((0 - (fConst1 / max(fConst1, (0.001f * fslider14)))));
		float 	fSlow37 = (1.0f - fSlow36);
		float 	fSlow38 = fslider15;
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			fRec4[0] = ((0.999f * fRec4[1]) + fSlow5);
			iRec6[0] = (12345 + (1103515245 * iRec6[1]));
			fRec5[0] = (((0.5221894f * fRec5[3]) + ((4.656612875245797e-10f * iRec6[0]) + (2.494956002f * fRec5[1]))) - (2.017265875f * fRec5[2]));
			fVec0[0] = 0.25f;
			fRec7[0] = ((fSlow8 * fRec7[1]) + fSlow9);
			fRec8[0] = fmodf((1.0f + fRec8[1]),(fConst15 / fRec7[0]));
			float fTemp0 = faustpower<2>(((fConst16 * (fRec7[0] * fRec8[0])) - 1.0f));
			fVec1[0] = fTemp0;
			fRec9[0] = fmodf((1.0f + fRec9[1]),(fSlow13 / fRec7[0]));
			float fTemp1 = faustpower<2>(((fSlow12 * (fRec7[0] * fRec9[0])) - 1.0f));
			fVec2[0] = fTemp1;
			fRec10[0] = fmodf((1.0f + fRec10[1]),(fSlow17 / fRec7[0]));
			float fTemp2 = faustpower<2>(((fSlow16 * (fRec7[0] * fRec10[0])) - 1.0f));
			fVec3[0] = fTemp2;
			float fTemp3 = (fRec4[0] * ((iSlow6)?(float)input0[i]:((iSlow7)?(((0.049922035f * fRec5[0]) + (0.050612699f * fRec5[2])) - ((0.095993537f * fRec5[1]) + (0.004408786f * fRec5[3]))):(fConst14 * (((fRec4[0] * fVec0[1]) * (((fVec1[0] - fVec1[1]) + (fSlow11 * (fVec2[0] - fVec2[1]))) + (fSlow15 * (fVec3[0] - fVec3[1])))) / fRec7[0])))));
			float fTemp4 = ((iSlow4)?0:fTemp3);
			float fTemp5 = fabsf(fTemp4);
			float fTemp6 = fabsf((2 * fTemp5));
			fRec18[0] = ((fSlow25 * max(fTemp6, fRec18[1])) + (fSlow26 * fTemp6));
			fRec17[0] = ((fSlow23 * fRec17[1]) + (fSlow24 * fRec18[0]));
			int iTemp7 = (fRec17[0] > fSlow27);
			iVec4[0] = iTemp7;
			iRec19[0] = max((int)((iVec4[0] > iVec4[1]) & iSlow28), (int)(iRec19[1] - 1));
			float fTemp8 = fabsf(max(float(iVec4[0]), (float)(iRec19[0] > 0)));
			fRec16[0] = ((fSlow22 * max(fTemp8, fRec16[1])) + (fSlow29 * fTemp8));
			fRec15[0] = ((fSlow19 * fRec15[1]) + (fSlow20 * fRec16[0]));
			fbargraph0 = (20 * log10f(fRec15[0]));
			float fTemp9 = fTemp4;
			float fTemp10 = fabsf((fTemp5 + fabsf(fTemp9)));
			fRec14[0] = ((fSlow25 * max(fTemp10, fRec14[1])) + (fSlow26 * fTemp10));
			fRec13[0] = ((fSlow23 * fRec13[1]) + (fSlow24 * fRec14[0]));
			int iTemp11 = (fRec13[0] > fSlow27);
			iVec5[0] = iTemp11;
			iRec20[0] = max((int)((iVec5[0] > iVec5[1]) & iSlow28), (int)(iRec20[1] - 1));
			float fTemp12 = fabsf(max(float(iVec5[0]), (float)(iRec20[0] > 0)));
			fRec12[0] = ((fSlow22 * max(fTemp12, fRec12[1])) + (fSlow29 * fTemp12));
			fRec11[0] = ((fSlow19 * fRec11[1]) + (fSlow20 * fRec12[0]));
			float fTemp13 = ((iSlow4)?fTemp3:(fRec11[0] * fTemp9));
			float fTemp14 = ((iSlow3)?0:fTemp13);
			float fTemp15 = ((iSlow4)?fTemp3:(fRec11[0] * fTemp4));
			float fTemp16 = ((iSlow3)?0:fTemp15);
			float fTemp17 = fabsf((fabsf(fTemp14) + fabsf(fTemp16)));
			fRec23[0] = ((fSlow36 * max(fTemp17, fRec23[1])) + (fSlow37 * fTemp17));
			fRec22[0] = ((fSlow34 * fRec22[1]) + (fSlow35 * fRec23[0]));
			fRec21[0] = ((fSlow32 * fRec21[1]) + (fSlow33 * max(((20 * log10f(fRec22[0])) - fSlow38), 0.0f)));
			float fTemp18 = powf(10,(0.05f * fRec21[0]));
			float fTemp19 = (fTemp14 * fTemp18);
			float fTemp20 = (fTemp16 * fTemp18);
			float fTemp21 = fabsf((fabsf(fTemp19) + fabsf(fTemp20)));
			fRec26[0] = ((fSlow36 * max(fTemp21, fRec26[1])) + (fSlow37 * fTemp21));
			fRec25[0] = ((fSlow34 * fRec25[1]) + (fSlow35 * fRec26[0]));
			fRec24[0] = ((fSlow32 * fRec24[1]) + (fSlow33 * max(((20 * log10f(fRec25[0])) - fSlow38), 0.0f)));
			fbargraph1 = (20 * log10f(powf(10,(0.05f * fRec24[0]))));
			float fTemp22 = (((iSlow3)?fTemp13:(fSlow30 * fTemp19)) + ((iSlow3)?fTemp15:(fSlow30 * fTemp20)));
			fRec3[0] = (fTemp22 - (fConst11 * ((fConst19 * fRec3[2]) + (fConst21 * fRec3[1]))));
			fRec2[0] = ((fConst11 * (((fConst13 * fRec3[0]) + (fConst22 * fRec3[1])) + (fConst13 * fRec3[2]))) - (fConst8 * ((fConst23 * fRec2[2]) + (fConst24 * fRec2[1]))));
			fRec1[0] = ((fConst8 * (((fConst10 * fRec2[0]) + (fConst25 * fRec2[1])) + (fConst10 * fRec2[2]))) - (fConst4 * ((fConst26 * fRec1[2]) + (fConst27 * fRec1[1]))));
			fRec0[0] = ((fSlow1 * fRec0[1]) + (fSlow2 * fabsf((fConst4 * (((fConst7 * fRec1[0]) + (fConst28 * fRec1[1])) + (fConst7 * fRec1[2]))))));
			fbargraph2 = (fSlow0 + (20 * log10f(fRec0[0])));
			fRec33[0] = (fTemp22 - (fConst45 * ((fConst48 * fRec33[2]) + (fConst49 * fRec33[1]))));
			fRec32[0] = ((fConst45 * (((fConst47 * fRec33[0]) + (fConst50 * fRec33[1])) + (fConst47 * fRec33[2]))) - (fConst43 * ((fConst51 * fRec32[2]) + (fConst52 * fRec32[1]))));
			fRec31[0] = ((fConst43 * (((fConst44 * fRec32[0]) + (fConst53 * fRec32[1])) + (fConst44 * fRec32[2]))) - (fConst41 * ((fConst54 * fRec31[2]) + (fConst55 * fRec31[1]))));
			float fTemp23 = (fConst41 * (((fConst42 * fRec31[0]) + (fConst56 * fRec31[1])) + (fConst42 * fRec31[2])));
			fRec30[0] = (fTemp23 - (fConst38 * ((fConst57 * fRec30[2]) + (fConst59 * fRec30[1]))));
			fRec29[0] = ((fConst38 * (((fConst40 * fRec30[0]) + (fConst60 * fRec30[1])) + (fConst40 * fRec30[2]))) - (fConst35 * ((fConst61 * fRec29[2]) + (fConst62 * fRec29[1]))));
			fRec28[0] = ((fConst35 * (((fConst37 * fRec29[0]) + (fConst63 * fRec29[1])) + (fConst37 * fRec29[2]))) - (fConst31 * ((fConst64 * fRec28[2]) + (fConst65 * fRec28[1]))));
			fRec27[0] = ((fSlow1 * fRec27[1]) + (fSlow2 * fabsf((fConst31 * (((fConst34 * fRec28[0]) + (fConst66 * fRec28[1])) + (fConst34 * fRec28[2]))))));
			fbargraph3 = (fSlow0 + (20 * log10f(fRec27[0])));
			fRec40[0] = (fTemp23 - (fConst83 * ((fConst86 * fRec40[2]) + (fConst87 * fRec40[1]))));
			fRec39[0] = ((fConst83 * (((fConst85 * fRec40[0]) + (fConst88 * fRec40[1])) + (fConst85 * fRec40[2]))) - (fConst81 * ((fConst89 * fRec39[2]) + (fConst90 * fRec39[1]))));
			fRec38[0] = ((fConst81 * (((fConst82 * fRec39[0]) + (fConst91 * fRec39[1])) + (fConst82 * fRec39[2]))) - (fConst79 * ((fConst92 * fRec38[2]) + (fConst93 * fRec38[1]))));
			float fTemp24 = (fConst79 * (((fConst80 * fRec38[0]) + (fConst94 * fRec38[1])) + (fConst80 * fRec38[2])));
			fRec37[0] = (fTemp24 - (fConst76 * ((fConst95 * fRec37[2]) + (fConst97 * fRec37[1]))));
			fRec36[0] = ((fConst76 * (((fConst78 * fRec37[0]) + (fConst98 * fRec37[1])) + (fConst78 * fRec37[2]))) - (fConst73 * ((fConst99 * fRec36[2]) + (fConst100 * fRec36[1]))));
			fRec35[0] = ((fConst73 * (((fConst75 * fRec36[0]) + (fConst101 * fRec36[1])) + (fConst75 * fRec36[2]))) - (fConst69 * ((fConst102 * fRec35[2]) + (fConst103 * fRec35[1]))));
			fRec34[0] = ((fSlow1 * fRec34[1]) + (fSlow2 * fabsf((fConst69 * (((fConst72 * fRec35[0]) + (fConst104 * fRec35[1])) + (fConst72 * fRec35[2]))))));
			fbargraph4 = (fSlow0 + (20 * log10f(fRec34[0])));
			fRec47[0] = (fTemp24 - (fConst121 * ((fConst124 * fRec47[2]) + (fConst125 * fRec47[1]))));
			fRec46[0] = ((fConst121 * (((fConst123 * fRec47[0]) + (fConst126 * fRec47[1])) + (fConst123 * fRec47[2]))) - (fConst119 * ((fConst127 * fRec46[2]) + (fConst128 * fRec46[1]))));
			fRec45[0] = ((fConst119 * (((fConst120 * fRec46[0]) + (fConst129 * fRec46[1])) + (fConst120 * fRec46[2]))) - (fConst117 * ((fConst130 * fRec45[2]) + (fConst131 * fRec45[1]))));
			float fTemp25 = (fConst117 * (((fConst118 * fRec45[0]) + (fConst132 * fRec45[1])) + (fConst118 * fRec45[2])));
			fRec44[0] = (fTemp25 - (fConst114 * ((fConst133 * fRec44[2]) + (fConst135 * fRec44[1]))));
			fRec43[0] = ((fConst114 * (((fConst116 * fRec44[0]) + (fConst136 * fRec44[1])) + (fConst116 * fRec44[2]))) - (fConst111 * ((fConst137 * fRec43[2]) + (fConst138 * fRec43[1]))));
			fRec42[0] = ((fConst111 * (((fConst113 * fRec43[0]) + (fConst139 * fRec43[1])) + (fConst113 * fRec43[2]))) - (fConst107 * ((fConst140 * fRec42[2]) + (fConst141 * fRec42[1]))));
			fRec41[0] = ((fSlow1 * fRec41[1]) + (fSlow2 * fabsf((fConst107 * (((fConst110 * fRec42[0]) + (fConst142 * fRec42[1])) + (fConst110 * fRec42[2]))))));
			fbargraph5 = (fSlow0 + (20 * log10f(fRec41[0])));
			fRec54[0] = (fTemp25 - (fConst159 * ((fConst162 * fRec54[2]) + (fConst163 * fRec54[1]))));
			fRec53[0] = ((fConst159 * (((fConst161 * fRec54[0]) + (fConst164 * fRec54[1])) + (fConst161 * fRec54[2]))) - (fConst157 * ((fConst165 * fRec53[2]) + (fConst166 * fRec53[1]))));
			fRec52[0] = ((fConst157 * (((fConst158 * fRec53[0]) + (fConst167 * fRec53[1])) + (fConst158 * fRec53[2]))) - (fConst155 * ((fConst168 * fRec52[2]) + (fConst169 * fRec52[1]))));
			float fTemp26 = (fConst155 * (((fConst156 * fRec52[0]) + (fConst170 * fRec52[1])) + (fConst156 * fRec52[2])));
			fRec51[0] = (fTemp26 - (fConst152 * ((fConst171 * fRec51[2]) + (fConst173 * fRec51[1]))));
			fRec50[0] = ((fConst152 * (((fConst154 * fRec51[0]) + (fConst174 * fRec51[1])) + (fConst154 * fRec51[2]))) - (fConst149 * ((fConst175 * fRec50[2]) + (fConst176 * fRec50[1]))));
			fRec49[0] = ((fConst149 * (((fConst151 * fRec50[0]) + (fConst177 * fRec50[1])) + (fConst151 * fRec50[2]))) - (fConst145 * ((fConst178 * fRec49[2]) + (fConst179 * fRec49[1]))));
			fRec48[0] = ((fSlow1 * fRec48[1]) + (fSlow2 * fabsf((fConst145 * (((fConst148 * fRec49[0]) + (fConst180 * fRec49[1])) + (fConst148 * fRec49[2]))))));
			fbargraph6 = (fSlow0 + (20 * log10f(fRec48[0])));
			fRec61[0] = (fTemp26 - (fConst197 * ((fConst200 * fRec61[2]) + (fConst201 * fRec61[1]))));
			fRec60[0] = ((fConst197 * (((fConst199 * fRec61[0]) + (fConst202 * fRec61[1])) + (fConst199 * fRec61[2]))) - (fConst195 * ((fConst203 * fRec60[2]) + (fConst204 * fRec60[1]))));
			fRec59[0] = ((fConst195 * (((fConst196 * fRec60[0]) + (fConst205 * fRec60[1])) + (fConst196 * fRec60[2]))) - (fConst193 * ((fConst206 * fRec59[2]) + (fConst207 * fRec59[1]))));
			float fTemp27 = (fConst193 * (((fConst194 * fRec59[0]) + (fConst208 * fRec59[1])) + (fConst194 * fRec59[2])));
			fRec58[0] = (fTemp27 - (fConst190 * ((fConst209 * fRec58[2]) + (fConst211 * fRec58[1]))));
			fRec57[0] = ((fConst190 * (((fConst192 * fRec58[0]) + (fConst212 * fRec58[1])) + (fConst192 * fRec58[2]))) - (fConst187 * ((fConst213 * fRec57[2]) + (fConst214 * fRec57[1]))));
			fRec56[0] = ((fConst187 * (((fConst189 * fRec57[0]) + (fConst215 * fRec57[1])) + (fConst189 * fRec57[2]))) - (fConst183 * ((fConst216 * fRec56[2]) + (fConst217 * fRec56[1]))));
			fRec55[0] = ((fSlow1 * fRec55[1]) + (fSlow2 * fabsf((fConst183 * (((fConst186 * fRec56[0]) + (fConst218 * fRec56[1])) + (fConst186 * fRec56[2]))))));
			fbargraph7 = (fSlow0 + (20 * log10f(fRec55[0])));
			fRec68[0] = (fTemp27 - (fConst235 * ((fConst238 * fRec68[2]) + (fConst239 * fRec68[1]))));
			fRec67[0] = ((fConst235 * (((fConst237 * fRec68[0]) + (fConst240 * fRec68[1])) + (fConst237 * fRec68[2]))) - (fConst233 * ((fConst241 * fRec67[2]) + (fConst242 * fRec67[1]))));
			fRec66[0] = ((fConst233 * (((fConst234 * fRec67[0]) + (fConst243 * fRec67[1])) + (fConst234 * fRec67[2]))) - (fConst231 * ((fConst244 * fRec66[2]) + (fConst245 * fRec66[1]))));
			float fTemp28 = (fConst231 * (((fConst232 * fRec66[0]) + (fConst246 * fRec66[1])) + (fConst232 * fRec66[2])));
			fRec65[0] = (fTemp28 - (fConst228 * ((fConst247 * fRec65[2]) + (fConst249 * fRec65[1]))));
			fRec64[0] = ((fConst228 * (((fConst230 * fRec65[0]) + (fConst250 * fRec65[1])) + (fConst230 * fRec65[2]))) - (fConst225 * ((fConst251 * fRec64[2]) + (fConst252 * fRec64[1]))));
			fRec63[0] = ((fConst225 * (((fConst227 * fRec64[0]) + (fConst253 * fRec64[1])) + (fConst227 * fRec64[2]))) - (fConst221 * ((fConst254 * fRec63[2]) + (fConst255 * fRec63[1]))));
			fRec62[0] = ((fSlow1 * fRec62[1]) + (fSlow2 * fabsf((fConst221 * (((fConst224 * fRec63[0]) + (fConst256 * fRec63[1])) + (fConst224 * fRec63[2]))))));
			fbargraph8 = (fSlow0 + (20 * log10f(fRec62[0])));
			fRec75[0] = (fTemp28 - (fConst273 * ((fConst276 * fRec75[2]) + (fConst277 * fRec75[1]))));
			fRec74[0] = ((fConst273 * (((fConst275 * fRec75[0]) + (fConst278 * fRec75[1])) + (fConst275 * fRec75[2]))) - (fConst271 * ((fConst279 * fRec74[2]) + (fConst280 * fRec74[1]))));
			fRec73[0] = ((fConst271 * (((fConst272 * fRec74[0]) + (fConst281 * fRec74[1])) + (fConst272 * fRec74[2]))) - (fConst269 * ((fConst282 * fRec73[2]) + (fConst283 * fRec73[1]))));
			float fTemp29 = (fConst269 * (((fConst270 * fRec73[0]) + (fConst284 * fRec73[1])) + (fConst270 * fRec73[2])));
			fRec72[0] = (fTemp29 - (fConst266 * ((fConst285 * fRec72[2]) + (fConst287 * fRec72[1]))));
			fRec71[0] = ((fConst266 * (((fConst268 * fRec72[0]) + (fConst288 * fRec72[1])) + (fConst268 * fRec72[2]))) - (fConst263 * ((fConst289 * fRec71[2]) + (fConst290 * fRec71[1]))));
			fRec70[0] = ((fConst263 * (((fConst265 * fRec71[0]) + (fConst291 * fRec71[1])) + (fConst265 * fRec71[2]))) - (fConst259 * ((fConst292 * fRec70[2]) + (fConst293 * fRec70[1]))));
			fRec69[0] = ((fSlow1 * fRec69[1]) + (fSlow2 * fabsf((fConst259 * (((fConst262 * fRec70[0]) + (fConst294 * fRec70[1])) + (fConst262 * fRec70[2]))))));
			fbargraph9 = (fSlow0 + (20 * log10f(fRec69[0])));
			fRec82[0] = (fTemp29 - (fConst311 * ((fConst314 * fRec82[2]) + (fConst315 * fRec82[1]))));
			fRec81[0] = ((fConst311 * (((fConst313 * fRec82[0]) + (fConst316 * fRec82[1])) + (fConst313 * fRec82[2]))) - (fConst309 * ((fConst317 * fRec81[2]) + (fConst318 * fRec81[1]))));
			fRec80[0] = ((fConst309 * (((fConst310 * fRec81[0]) + (fConst319 * fRec81[1])) + (fConst310 * fRec81[2]))) - (fConst307 * ((fConst320 * fRec80[2]) + (fConst321 * fRec80[1]))));
			float fTemp30 = (fConst307 * (((fConst308 * fRec80[0]) + (fConst322 * fRec80[1])) + (fConst308 * fRec80[2])));
			fRec79[0] = (fTemp30 - (fConst304 * ((fConst323 * fRec79[2]) + (fConst325 * fRec79[1]))));
			fRec78[0] = ((fConst304 * (((fConst306 * fRec79[0]) + (fConst326 * fRec79[1])) + (fConst306 * fRec79[2]))) - (fConst301 * ((fConst327 * fRec78[2]) + (fConst328 * fRec78[1]))));
			fRec77[0] = ((fConst301 * (((fConst303 * fRec78[0]) + (fConst329 * fRec78[1])) + (fConst303 * fRec78[2]))) - (fConst297 * ((fConst330 * fRec77[2]) + (fConst331 * fRec77[1]))));
			fRec76[0] = ((fSlow1 * fRec76[1]) + (fSlow2 * fabsf((fConst297 * (((fConst300 * fRec77[0]) + (fConst332 * fRec77[1])) + (fConst300 * fRec77[2]))))));
			fbargraph10 = (fSlow0 + (20 * log10f(fRec76[0])));
			fRec89[0] = (fTemp30 - (fConst349 * ((fConst352 * fRec89[2]) + (fConst353 * fRec89[1]))));
			fRec88[0] = ((fConst349 * (((fConst351 * fRec89[0]) + (fConst354 * fRec89[1])) + (fConst351 * fRec89[2]))) - (fConst347 * ((fConst355 * fRec88[2]) + (fConst356 * fRec88[1]))));
			fRec87[0] = ((fConst347 * (((fConst348 * fRec88[0]) + (fConst357 * fRec88[1])) + (fConst348 * fRec88[2]))) - (fConst345 * ((fConst358 * fRec87[2]) + (fConst359 * fRec87[1]))));
			float fTemp31 = (fConst345 * (((fConst346 * fRec87[0]) + (fConst360 * fRec87[1])) + (fConst346 * fRec87[2])));
			fRec86[0] = (fTemp31 - (fConst342 * ((fConst361 * fRec86[2]) + (fConst363 * fRec86[1]))));
			fRec85[0] = ((fConst342 * (((fConst344 * fRec86[0]) + (fConst364 * fRec86[1])) + (fConst344 * fRec86[2]))) - (fConst339 * ((fConst365 * fRec85[2]) + (fConst366 * fRec85[1]))));
			fRec84[0] = ((fConst339 * (((fConst341 * fRec85[0]) + (fConst367 * fRec85[1])) + (fConst341 * fRec85[2]))) - (fConst335 * ((fConst368 * fRec84[2]) + (fConst369 * fRec84[1]))));
			fRec83[0] = ((fSlow1 * fRec83[1]) + (fSlow2 * fabsf((fConst335 * (((fConst338 * fRec84[0]) + (fConst370 * fRec84[1])) + (fConst338 * fRec84[2]))))));
			fbargraph11 = (fSlow0 + (20 * log10f(fRec83[0])));
			fRec96[0] = (fTemp31 - (fConst387 * ((fConst390 * fRec96[2]) + (fConst391 * fRec96[1]))));
			fRec95[0] = ((fConst387 * (((fConst389 * fRec96[0]) + (fConst392 * fRec96[1])) + (fConst389 * fRec96[2]))) - (fConst385 * ((fConst393 * fRec95[2]) + (fConst394 * fRec95[1]))));
			fRec94[0] = ((fConst385 * (((fConst386 * fRec95[0]) + (fConst395 * fRec95[1])) + (fConst386 * fRec95[2]))) - (fConst383 * ((fConst396 * fRec94[2]) + (fConst397 * fRec94[1]))));
			float fTemp32 = (fConst383 * (((fConst384 * fRec94[0]) + (fConst398 * fRec94[1])) + (fConst384 * fRec94[2])));
			fRec93[0] = (fTemp32 - (fConst380 * ((fConst399 * fRec93[2]) + (fConst401 * fRec93[1]))));
			fRec92[0] = ((fConst380 * (((fConst382 * fRec93[0]) + (fConst402 * fRec93[1])) + (fConst382 * fRec93[2]))) - (fConst377 * ((fConst403 * fRec92[2]) + (fConst404 * fRec92[1]))));
			fRec91[0] = ((fConst377 * (((fConst379 * fRec92[0]) + (fConst405 * fRec92[1])) + (fConst379 * fRec92[2]))) - (fConst373 * ((fConst406 * fRec91[2]) + (fConst407 * fRec91[1]))));
			fRec90[0] = ((fSlow1 * fRec90[1]) + (fSlow2 * fabsf((fConst373 * (((fConst376 * fRec91[0]) + (fConst408 * fRec91[1])) + (fConst376 * fRec91[2]))))));
			fbargraph12 = (fSlow0 + (20 * log10f(fRec90[0])));
			fRec103[0] = (fTemp32 - (fConst425 * ((fConst428 * fRec103[2]) + (fConst429 * fRec103[1]))));
			fRec102[0] = ((fConst425 * (((fConst427 * fRec103[0]) + (fConst430 * fRec103[1])) + (fConst427 * fRec103[2]))) - (fConst423 * ((fConst431 * fRec102[2]) + (fConst432 * fRec102[1]))));
			fRec101[0] = ((fConst423 * (((fConst424 * fRec102[0]) + (fConst433 * fRec102[1])) + (fConst424 * fRec102[2]))) - (fConst421 * ((fConst434 * fRec101[2]) + (fConst435 * fRec101[1]))));
			float fTemp33 = (fConst421 * (((fConst422 * fRec101[0]) + (fConst436 * fRec101[1])) + (fConst422 * fRec101[2])));
			fRec100[0] = (fTemp33 - (fConst418 * ((fConst437 * fRec100[2]) + (fConst439 * fRec100[1]))));
			fRec99[0] = ((fConst418 * (((fConst420 * fRec100[0]) + (fConst440 * fRec100[1])) + (fConst420 * fRec100[2]))) - (fConst415 * ((fConst441 * fRec99[2]) + (fConst442 * fRec99[1]))));
			fRec98[0] = ((fConst415 * (((fConst417 * fRec99[0]) + (fConst443 * fRec99[1])) + (fConst417 * fRec99[2]))) - (fConst411 * ((fConst444 * fRec98[2]) + (fConst445 * fRec98[1]))));
			fRec97[0] = ((fSlow1 * fRec97[1]) + (fSlow2 * fabsf((fConst411 * (((fConst414 * fRec98[0]) + (fConst446 * fRec98[1])) + (fConst414 * fRec98[2]))))));
			fbargraph13 = (fSlow0 + (20 * log10f(fRec97[0])));
			fRec110[0] = (fTemp33 - (fConst463 * ((fConst466 * fRec110[2]) + (fConst467 * fRec110[1]))));
			fRec109[0] = ((fConst463 * (((fConst465 * fRec110[0]) + (fConst468 * fRec110[1])) + (fConst465 * fRec110[2]))) - (fConst461 * ((fConst469 * fRec109[2]) + (fConst470 * fRec109[1]))));
			fRec108[0] = ((fConst461 * (((fConst462 * fRec109[0]) + (fConst471 * fRec109[1])) + (fConst462 * fRec109[2]))) - (fConst459 * ((fConst472 * fRec108[2]) + (fConst473 * fRec108[1]))));
			float fTemp34 = (fConst459 * (((fConst460 * fRec108[0]) + (fConst474 * fRec108[1])) + (fConst460 * fRec108[2])));
			fRec107[0] = (fTemp34 - (fConst456 * ((fConst475 * fRec107[2]) + (fConst477 * fRec107[1]))));
			fRec106[0] = ((fConst456 * (((fConst458 * fRec107[0]) + (fConst478 * fRec107[1])) + (fConst458 * fRec107[2]))) - (fConst453 * ((fConst479 * fRec106[2]) + (fConst480 * fRec106[1]))));
			fRec105[0] = ((fConst453 * (((fConst455 * fRec106[0]) + (fConst481 * fRec106[1])) + (fConst455 * fRec106[2]))) - (fConst449 * ((fConst482 * fRec105[2]) + (fConst483 * fRec105[1]))));
			fRec104[0] = ((fSlow1 * fRec104[1]) + (fSlow2 * fabsf((fConst449 * (((fConst452 * fRec105[0]) + (fConst484 * fRec105[1])) + (fConst452 * fRec105[2]))))));
			fbargraph14 = (fSlow0 + (20 * log10f(fRec104[0])));
			fRec117[0] = (fTemp34 - (fConst501 * ((fConst504 * fRec117[2]) + (fConst505 * fRec117[1]))));
			fRec116[0] = ((fConst501 * (((fConst503 * fRec117[0]) + (fConst506 * fRec117[1])) + (fConst503 * fRec117[2]))) - (fConst499 * ((fConst507 * fRec116[2]) + (fConst508 * fRec116[1]))));
			fRec115[0] = ((fConst499 * (((fConst500 * fRec116[0]) + (fConst509 * fRec116[1])) + (fConst500 * fRec116[2]))) - (fConst497 * ((fConst510 * fRec115[2]) + (fConst511 * fRec115[1]))));
			float fTemp35 = (fConst497 * (((fConst498 * fRec115[0]) + (fConst512 * fRec115[1])) + (fConst498 * fRec115[2])));
			fRec114[0] = (fTemp35 - (fConst494 * ((fConst513 * fRec114[2]) + (fConst515 * fRec114[1]))));
			fRec113[0] = ((fConst494 * (((fConst496 * fRec114[0]) + (fConst516 * fRec114[1])) + (fConst496 * fRec114[2]))) - (fConst491 * ((fConst517 * fRec113[2]) + (fConst518 * fRec113[1]))));
			fRec112[0] = ((fConst491 * (((fConst493 * fRec113[0]) + (fConst519 * fRec113[1])) + (fConst493 * fRec113[2]))) - (fConst487 * ((fConst520 * fRec112[2]) + (fConst521 * fRec112[1]))));
			fRec111[0] = ((fSlow1 * fRec111[1]) + (fSlow2 * fabsf((fConst487 * (((fConst490 * fRec112[0]) + (fConst522 * fRec112[1])) + (fConst490 * fRec112[2]))))));
			fbargraph15 = (fSlow0 + (20 * log10f(fRec111[0])));
			fRec121[0] = (fTemp35 - (fConst527 * ((fConst530 * fRec121[2]) + (fConst531 * fRec121[1]))));
			fRec120[0] = ((fConst527 * (((fConst529 * fRec121[0]) + (fConst532 * fRec121[1])) + (fConst529 * fRec121[2]))) - (fConst525 * ((fConst533 * fRec120[2]) + (fConst534 * fRec120[1]))));
			fRec119[0] = ((fConst525 * (((fConst526 * fRec120[0]) + (fConst535 * fRec120[1])) + (fConst526 * fRec120[2]))) - (fConst523 * ((fConst536 * fRec119[2]) + (fConst537 * fRec119[1]))));
			fRec118[0] = ((fSlow1 * fRec118[1]) + (fSlow2 * fabsf((fConst523 * (((fConst524 * fRec119[0]) + (fConst538 * fRec119[1])) + (fConst524 * fRec119[2]))))));
			fbargraph16 = (fSlow0 + (20 * log10f(fRec118[0])));
			float fTemp36 = fTemp22;
			output0[i] = (FAUSTFLOAT)fTemp36;
			output1[i] = (FAUSTFLOAT)fTemp36;
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
			fRec24[1] = fRec24[0];
			fRec25[1] = fRec25[0];
			fRec26[1] = fRec26[0];
			fRec21[1] = fRec21[0];
			fRec22[1] = fRec22[0];
			fRec23[1] = fRec23[0];
			fRec11[1] = fRec11[0];
			fRec12[1] = fRec12[0];
			iRec20[1] = iRec20[0];
			iVec5[1] = iVec5[0];
			fRec13[1] = fRec13[0];
			fRec14[1] = fRec14[0];
			fRec15[1] = fRec15[0];
			fRec16[1] = fRec16[0];
			iRec19[1] = iRec19[0];
			iVec4[1] = iVec4[0];
			fRec17[1] = fRec17[0];
			fRec18[1] = fRec18[0];
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
