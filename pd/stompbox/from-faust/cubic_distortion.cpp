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
	FAUSTFLOAT 	fslider2;
	float 	fRec4[2];
	float 	fConst14;
	FAUSTFLOAT 	fslider3;
	FAUSTFLOAT 	fslider4;
	float 	fRec7[2];
	float 	fRec5[2];
	float 	fRec6[2];
	FAUSTFLOAT 	fslider5;
	float 	fRec9[2];
	FAUSTFLOAT 	fslider6;
	float 	fRec10[2];
	float 	fVec1[2];
	float 	fRec8[2];
	float 	fConst15;
	float 	fConst16;
	float 	fConst17;
	float 	fRec3[3];
	float 	fConst18;
	float 	fConst19;
	float 	fConst20;
	float 	fRec2[3];
	float 	fConst21;
	float 	fConst22;
	float 	fConst23;
	float 	fRec1[3];
	float 	fConst24;
	float 	fRec0[2];
	FAUSTFLOAT 	fbargraph0;
	float 	fConst25;
	float 	fConst26;
	float 	fConst27;
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
	float 	fRec17[3];
	float 	fConst46;
	float 	fConst47;
	float 	fConst48;
	float 	fRec16[3];
	float 	fConst49;
	float 	fConst50;
	float 	fConst51;
	float 	fRec15[3];
	float 	fConst52;
	float 	fConst53;
	float 	fConst54;
	float 	fConst55;
	float 	fRec14[3];
	float 	fConst56;
	float 	fConst57;
	float 	fConst58;
	float 	fRec13[3];
	float 	fConst59;
	float 	fConst60;
	float 	fConst61;
	float 	fRec12[3];
	float 	fConst62;
	float 	fRec11[2];
	FAUSTFLOAT 	fbargraph1;
	float 	fConst63;
	float 	fConst64;
	float 	fConst65;
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
	float 	fRec24[3];
	float 	fConst84;
	float 	fConst85;
	float 	fConst86;
	float 	fRec23[3];
	float 	fConst87;
	float 	fConst88;
	float 	fConst89;
	float 	fRec22[3];
	float 	fConst90;
	float 	fConst91;
	float 	fConst92;
	float 	fConst93;
	float 	fRec21[3];
	float 	fConst94;
	float 	fConst95;
	float 	fConst96;
	float 	fRec20[3];
	float 	fConst97;
	float 	fConst98;
	float 	fConst99;
	float 	fRec19[3];
	float 	fConst100;
	float 	fRec18[2];
	FAUSTFLOAT 	fbargraph2;
	float 	fConst101;
	float 	fConst102;
	float 	fConst103;
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
	float 	fRec31[3];
	float 	fConst122;
	float 	fConst123;
	float 	fConst124;
	float 	fRec30[3];
	float 	fConst125;
	float 	fConst126;
	float 	fConst127;
	float 	fRec29[3];
	float 	fConst128;
	float 	fConst129;
	float 	fConst130;
	float 	fConst131;
	float 	fRec28[3];
	float 	fConst132;
	float 	fConst133;
	float 	fConst134;
	float 	fRec27[3];
	float 	fConst135;
	float 	fConst136;
	float 	fConst137;
	float 	fRec26[3];
	float 	fConst138;
	float 	fRec25[2];
	FAUSTFLOAT 	fbargraph3;
	float 	fConst139;
	float 	fConst140;
	float 	fConst141;
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
	float 	fRec38[3];
	float 	fConst160;
	float 	fConst161;
	float 	fConst162;
	float 	fRec37[3];
	float 	fConst163;
	float 	fConst164;
	float 	fConst165;
	float 	fRec36[3];
	float 	fConst166;
	float 	fConst167;
	float 	fConst168;
	float 	fConst169;
	float 	fRec35[3];
	float 	fConst170;
	float 	fConst171;
	float 	fConst172;
	float 	fRec34[3];
	float 	fConst173;
	float 	fConst174;
	float 	fConst175;
	float 	fRec33[3];
	float 	fConst176;
	float 	fRec32[2];
	FAUSTFLOAT 	fbargraph4;
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
	float 	fConst190;
	float 	fConst191;
	float 	fConst192;
	float 	fConst193;
	float 	fConst194;
	float 	fConst195;
	float 	fConst196;
	float 	fConst197;
	float 	fRec45[3];
	float 	fConst198;
	float 	fConst199;
	float 	fConst200;
	float 	fRec44[3];
	float 	fConst201;
	float 	fConst202;
	float 	fConst203;
	float 	fRec43[3];
	float 	fConst204;
	float 	fConst205;
	float 	fConst206;
	float 	fConst207;
	float 	fRec42[3];
	float 	fConst208;
	float 	fConst209;
	float 	fConst210;
	float 	fRec41[3];
	float 	fConst211;
	float 	fConst212;
	float 	fConst213;
	float 	fRec40[3];
	float 	fConst214;
	float 	fRec39[2];
	FAUSTFLOAT 	fbargraph5;
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
	float 	fConst228;
	float 	fConst229;
	float 	fConst230;
	float 	fConst231;
	float 	fConst232;
	float 	fConst233;
	float 	fConst234;
	float 	fConst235;
	float 	fRec52[3];
	float 	fConst236;
	float 	fConst237;
	float 	fConst238;
	float 	fRec51[3];
	float 	fConst239;
	float 	fConst240;
	float 	fConst241;
	float 	fRec50[3];
	float 	fConst242;
	float 	fConst243;
	float 	fConst244;
	float 	fConst245;
	float 	fRec49[3];
	float 	fConst246;
	float 	fConst247;
	float 	fConst248;
	float 	fRec48[3];
	float 	fConst249;
	float 	fConst250;
	float 	fConst251;
	float 	fRec47[3];
	float 	fConst252;
	float 	fRec46[2];
	FAUSTFLOAT 	fbargraph6;
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
	float 	fRec59[3];
	float 	fConst274;
	float 	fConst275;
	float 	fConst276;
	float 	fRec58[3];
	float 	fConst277;
	float 	fConst278;
	float 	fConst279;
	float 	fRec57[3];
	float 	fConst280;
	float 	fConst281;
	float 	fConst282;
	float 	fConst283;
	float 	fRec56[3];
	float 	fConst284;
	float 	fConst285;
	float 	fConst286;
	float 	fRec55[3];
	float 	fConst287;
	float 	fConst288;
	float 	fConst289;
	float 	fRec54[3];
	float 	fConst290;
	float 	fRec53[2];
	FAUSTFLOAT 	fbargraph7;
	float 	fConst291;
	float 	fConst292;
	float 	fConst293;
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
	float 	fRec66[3];
	float 	fConst312;
	float 	fConst313;
	float 	fConst314;
	float 	fRec65[3];
	float 	fConst315;
	float 	fConst316;
	float 	fConst317;
	float 	fRec64[3];
	float 	fConst318;
	float 	fConst319;
	float 	fConst320;
	float 	fConst321;
	float 	fRec63[3];
	float 	fConst322;
	float 	fConst323;
	float 	fConst324;
	float 	fRec62[3];
	float 	fConst325;
	float 	fConst326;
	float 	fConst327;
	float 	fRec61[3];
	float 	fConst328;
	float 	fRec60[2];
	FAUSTFLOAT 	fbargraph8;
	float 	fConst329;
	float 	fConst330;
	float 	fConst331;
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
	float 	fRec73[3];
	float 	fConst350;
	float 	fConst351;
	float 	fConst352;
	float 	fRec72[3];
	float 	fConst353;
	float 	fConst354;
	float 	fConst355;
	float 	fRec71[3];
	float 	fConst356;
	float 	fConst357;
	float 	fConst358;
	float 	fConst359;
	float 	fRec70[3];
	float 	fConst360;
	float 	fConst361;
	float 	fConst362;
	float 	fRec69[3];
	float 	fConst363;
	float 	fConst364;
	float 	fConst365;
	float 	fRec68[3];
	float 	fConst366;
	float 	fRec67[2];
	FAUSTFLOAT 	fbargraph9;
	float 	fConst367;
	float 	fConst368;
	float 	fConst369;
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
	float 	fRec80[3];
	float 	fConst388;
	float 	fConst389;
	float 	fConst390;
	float 	fRec79[3];
	float 	fConst391;
	float 	fConst392;
	float 	fConst393;
	float 	fRec78[3];
	float 	fConst394;
	float 	fConst395;
	float 	fConst396;
	float 	fConst397;
	float 	fRec77[3];
	float 	fConst398;
	float 	fConst399;
	float 	fConst400;
	float 	fRec76[3];
	float 	fConst401;
	float 	fConst402;
	float 	fConst403;
	float 	fRec75[3];
	float 	fConst404;
	float 	fRec74[2];
	FAUSTFLOAT 	fbargraph10;
	float 	fConst405;
	float 	fConst406;
	float 	fConst407;
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
	float 	fRec87[3];
	float 	fConst426;
	float 	fConst427;
	float 	fConst428;
	float 	fRec86[3];
	float 	fConst429;
	float 	fConst430;
	float 	fConst431;
	float 	fRec85[3];
	float 	fConst432;
	float 	fConst433;
	float 	fConst434;
	float 	fConst435;
	float 	fRec84[3];
	float 	fConst436;
	float 	fConst437;
	float 	fConst438;
	float 	fRec83[3];
	float 	fConst439;
	float 	fConst440;
	float 	fConst441;
	float 	fRec82[3];
	float 	fConst442;
	float 	fRec81[2];
	FAUSTFLOAT 	fbargraph11;
	float 	fConst443;
	float 	fConst444;
	float 	fConst445;
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
	float 	fRec94[3];
	float 	fConst464;
	float 	fConst465;
	float 	fConst466;
	float 	fRec93[3];
	float 	fConst467;
	float 	fConst468;
	float 	fConst469;
	float 	fRec92[3];
	float 	fConst470;
	float 	fConst471;
	float 	fConst472;
	float 	fConst473;
	float 	fRec91[3];
	float 	fConst474;
	float 	fConst475;
	float 	fConst476;
	float 	fRec90[3];
	float 	fConst477;
	float 	fConst478;
	float 	fConst479;
	float 	fRec89[3];
	float 	fConst480;
	float 	fRec88[2];
	FAUSTFLOAT 	fbargraph12;
	float 	fConst481;
	float 	fConst482;
	float 	fConst483;
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
	float 	fRec101[3];
	float 	fConst502;
	float 	fConst503;
	float 	fConst504;
	float 	fRec100[3];
	float 	fConst505;
	float 	fConst506;
	float 	fConst507;
	float 	fRec99[3];
	float 	fConst508;
	float 	fConst509;
	float 	fConst510;
	float 	fConst511;
	float 	fRec98[3];
	float 	fConst512;
	float 	fConst513;
	float 	fConst514;
	float 	fRec97[3];
	float 	fConst515;
	float 	fConst516;
	float 	fConst517;
	float 	fRec96[3];
	float 	fConst518;
	float 	fRec95[2];
	FAUSTFLOAT 	fbargraph13;
	float 	fConst519;
	float 	fConst520;
	float 	fConst521;
	float 	fConst522;
	float 	fConst523;
	float 	fConst524;
	float 	fConst525;
	float 	fConst526;
	float 	fConst527;
	float 	fRec105[3];
	float 	fConst528;
	float 	fConst529;
	float 	fConst530;
	float 	fRec104[3];
	float 	fConst531;
	float 	fConst532;
	float 	fConst533;
	float 	fRec103[3];
	float 	fConst534;
	float 	fRec102[2];
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

	virtual int getNumInputs() 	{ return 0; }
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
		fslider2 = -2e+01f;
		for (int i=0; i<2; i++) fRec4[i] = 0;
		fConst14 = (6.283185307179586f / float(iConst0));
		fslider3 = 0.1f;
		fslider4 = 49.0f;
		for (int i=0; i<2; i++) fRec7[i] = 0;
		for (int i=0; i<2; i++) fRec5[i] = 0;
		for (int i=0; i<2; i++) fRec6[i] = 0;
		fslider5 = 0.0f;
		for (int i=0; i<2; i++) fRec9[i] = 0;
		fslider6 = 0.0f;
		for (int i=0; i<2; i++) fRec10[i] = 0;
		for (int i=0; i<2; i++) fVec1[i] = 0;
		for (int i=0; i<2; i++) fRec8[i] = 0;
		fConst15 = (4.076781969643807f + ((fConst3 - 3.1897274020965583f) / fConst2));
		fConst16 = (1.0f / fConst5);
		fConst17 = (2 * (4.076781969643807f - fConst16));
		for (int i=0; i<3; i++) fRec3[i] = 0;
		fConst18 = (2 * (0.00040767818495825777f - fConst12));
		fConst19 = (1.450071084655647f + ((fConst3 - 0.7431304601070396f) / fConst2));
		fConst20 = (2 * (1.450071084655647f - fConst16));
		for (int i=0; i<3; i++) fRec2[i] = 0;
		fConst21 = (2 * (1.450071084655647f - fConst9));
		fConst22 = (0.9351401670315425f + ((fConst3 - 0.157482159302087f) / fConst2));
		fConst23 = (2 * (0.9351401670315425f - fConst16));
		for (int i=0; i<3; i++) fRec1[i] = 0;
		fConst24 = (2 * (0.9351401670315425f - fConst6));
		for (int i=0; i<2; i++) fRec0[i] = 0;
		fConst25 = tanf((19790.793572264363f / float(iConst0)));
		fConst26 = (1.0f / fConst25);
		fConst27 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst26) / fConst25)));
		fConst28 = faustpower<2>(fConst25);
		fConst29 = (50.063807016150385f / fConst28);
		fConst30 = (0.9351401670315425f + fConst29);
		fConst31 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst26) / fConst25)));
		fConst32 = (11.052052171507189f / fConst28);
		fConst33 = (1.450071084655647f + fConst32);
		fConst34 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst26) / fConst25)));
		fConst35 = (0.0017661728399818856f / fConst28);
		fConst36 = (0.00040767818495825777f + fConst35);
		fConst37 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst3) / fConst2)));
		fConst38 = (53.53615295455673f + fConst16);
		fConst39 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst3) / fConst2)));
		fConst40 = (7.621731298870603f + fConst16);
		fConst41 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst3) / fConst2)));
		fConst42 = (9.9999997055e-05f / fConst5);
		fConst43 = (0.000433227200555f + fConst42);
		fConst44 = (0.24529150870616f + ((fConst3 - 0.782413046821645f) / fConst2));
		fConst45 = (2 * (0.24529150870616f - fConst16));
		for (int i=0; i<3; i++) fRec17[i] = 0;
		fConst46 = (2 * (0.000433227200555f - fConst42));
		fConst47 = (0.689621364484675f + ((fConst3 - 0.512478641889141f) / fConst2));
		fConst48 = (2 * (0.689621364484675f - fConst16));
		for (int i=0; i<3; i++) fRec16[i] = 0;
		fConst49 = (2 * (7.621731298870603f - fConst16));
		fConst50 = (1.069358407707312f + ((fConst3 - 0.168404871113589f) / fConst2));
		fConst51 = (2 * (1.069358407707312f - fConst16));
		for (int i=0; i<3; i++) fRec15[i] = 0;
		fConst52 = (2 * (53.53615295455673f - fConst16));
		fConst53 = (4.076781969643807f + ((fConst26 - 3.1897274020965583f) / fConst25));
		fConst54 = (1.0f / fConst28);
		fConst55 = (2 * (4.076781969643807f - fConst54));
		for (int i=0; i<3; i++) fRec14[i] = 0;
		fConst56 = (2 * (0.00040767818495825777f - fConst35));
		fConst57 = (1.450071084655647f + ((fConst26 - 0.7431304601070396f) / fConst25));
		fConst58 = (2 * (1.450071084655647f - fConst54));
		for (int i=0; i<3; i++) fRec13[i] = 0;
		fConst59 = (2 * (1.450071084655647f - fConst32));
		fConst60 = (0.9351401670315425f + ((fConst26 - 0.157482159302087f) / fConst25));
		fConst61 = (2 * (0.9351401670315425f - fConst54));
		for (int i=0; i<3; i++) fRec12[i] = 0;
		fConst62 = (2 * (0.9351401670315425f - fConst29));
		for (int i=0; i<2; i++) fRec11[i] = 0;
		fConst63 = tanf((12467.418707910012f / float(iConst0)));
		fConst64 = (1.0f / fConst63);
		fConst65 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst64) / fConst63)));
		fConst66 = faustpower<2>(fConst63);
		fConst67 = (50.063807016150385f / fConst66);
		fConst68 = (0.9351401670315425f + fConst67);
		fConst69 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst64) / fConst63)));
		fConst70 = (11.052052171507189f / fConst66);
		fConst71 = (1.450071084655647f + fConst70);
		fConst72 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst64) / fConst63)));
		fConst73 = (0.0017661728399818856f / fConst66);
		fConst74 = (0.00040767818495825777f + fConst73);
		fConst75 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst26) / fConst25)));
		fConst76 = (53.53615295455673f + fConst54);
		fConst77 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst26) / fConst25)));
		fConst78 = (7.621731298870603f + fConst54);
		fConst79 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst26) / fConst25)));
		fConst80 = (9.9999997055e-05f / fConst28);
		fConst81 = (0.000433227200555f + fConst80);
		fConst82 = (0.24529150870616f + ((fConst26 - 0.782413046821645f) / fConst25));
		fConst83 = (2 * (0.24529150870616f - fConst54));
		for (int i=0; i<3; i++) fRec24[i] = 0;
		fConst84 = (2 * (0.000433227200555f - fConst80));
		fConst85 = (0.689621364484675f + ((fConst26 - 0.512478641889141f) / fConst25));
		fConst86 = (2 * (0.689621364484675f - fConst54));
		for (int i=0; i<3; i++) fRec23[i] = 0;
		fConst87 = (2 * (7.621731298870603f - fConst54));
		fConst88 = (1.069358407707312f + ((fConst26 - 0.168404871113589f) / fConst25));
		fConst89 = (2 * (1.069358407707312f - fConst54));
		for (int i=0; i<3; i++) fRec22[i] = 0;
		fConst90 = (2 * (53.53615295455673f - fConst54));
		fConst91 = (4.076781969643807f + ((fConst64 - 3.1897274020965583f) / fConst63));
		fConst92 = (1.0f / fConst66);
		fConst93 = (2 * (4.076781969643807f - fConst92));
		for (int i=0; i<3; i++) fRec21[i] = 0;
		fConst94 = (2 * (0.00040767818495825777f - fConst73));
		fConst95 = (1.450071084655647f + ((fConst64 - 0.7431304601070396f) / fConst63));
		fConst96 = (2 * (1.450071084655647f - fConst92));
		for (int i=0; i<3; i++) fRec20[i] = 0;
		fConst97 = (2 * (1.450071084655647f - fConst70));
		fConst98 = (0.9351401670315425f + ((fConst64 - 0.157482159302087f) / fConst63));
		fConst99 = (2 * (0.9351401670315425f - fConst92));
		for (int i=0; i<3; i++) fRec19[i] = 0;
		fConst100 = (2 * (0.9351401670315425f - fConst67));
		for (int i=0; i<2; i++) fRec18[i] = 0;
		fConst101 = tanf((7853.981633974483f / float(iConst0)));
		fConst102 = (1.0f / fConst101);
		fConst103 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst102) / fConst101)));
		fConst104 = faustpower<2>(fConst101);
		fConst105 = (50.063807016150385f / fConst104);
		fConst106 = (0.9351401670315425f + fConst105);
		fConst107 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst102) / fConst101)));
		fConst108 = (11.052052171507189f / fConst104);
		fConst109 = (1.450071084655647f + fConst108);
		fConst110 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst102) / fConst101)));
		fConst111 = (0.0017661728399818856f / fConst104);
		fConst112 = (0.00040767818495825777f + fConst111);
		fConst113 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst64) / fConst63)));
		fConst114 = (53.53615295455673f + fConst92);
		fConst115 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst64) / fConst63)));
		fConst116 = (7.621731298870603f + fConst92);
		fConst117 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst64) / fConst63)));
		fConst118 = (9.9999997055e-05f / fConst66);
		fConst119 = (0.000433227200555f + fConst118);
		fConst120 = (0.24529150870616f + ((fConst64 - 0.782413046821645f) / fConst63));
		fConst121 = (2 * (0.24529150870616f - fConst92));
		for (int i=0; i<3; i++) fRec31[i] = 0;
		fConst122 = (2 * (0.000433227200555f - fConst118));
		fConst123 = (0.689621364484675f + ((fConst64 - 0.512478641889141f) / fConst63));
		fConst124 = (2 * (0.689621364484675f - fConst92));
		for (int i=0; i<3; i++) fRec30[i] = 0;
		fConst125 = (2 * (7.621731298870603f - fConst92));
		fConst126 = (1.069358407707312f + ((fConst64 - 0.168404871113589f) / fConst63));
		fConst127 = (2 * (1.069358407707312f - fConst92));
		for (int i=0; i<3; i++) fRec29[i] = 0;
		fConst128 = (2 * (53.53615295455673f - fConst92));
		fConst129 = (4.076781969643807f + ((fConst102 - 3.1897274020965583f) / fConst101));
		fConst130 = (1.0f / fConst104);
		fConst131 = (2 * (4.076781969643807f - fConst130));
		for (int i=0; i<3; i++) fRec28[i] = 0;
		fConst132 = (2 * (0.00040767818495825777f - fConst111));
		fConst133 = (1.450071084655647f + ((fConst102 - 0.7431304601070396f) / fConst101));
		fConst134 = (2 * (1.450071084655647f - fConst130));
		for (int i=0; i<3; i++) fRec27[i] = 0;
		fConst135 = (2 * (1.450071084655647f - fConst108));
		fConst136 = (0.9351401670315425f + ((fConst102 - 0.157482159302087f) / fConst101));
		fConst137 = (2 * (0.9351401670315425f - fConst130));
		for (int i=0; i<3; i++) fRec26[i] = 0;
		fConst138 = (2 * (0.9351401670315425f - fConst105));
		for (int i=0; i<2; i++) fRec25[i] = 0;
		fConst139 = tanf((4947.698393066091f / float(iConst0)));
		fConst140 = (1.0f / fConst139);
		fConst141 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst140) / fConst139)));
		fConst142 = faustpower<2>(fConst139);
		fConst143 = (50.063807016150385f / fConst142);
		fConst144 = (0.9351401670315425f + fConst143);
		fConst145 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst140) / fConst139)));
		fConst146 = (11.052052171507189f / fConst142);
		fConst147 = (1.450071084655647f + fConst146);
		fConst148 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst140) / fConst139)));
		fConst149 = (0.0017661728399818856f / fConst142);
		fConst150 = (0.00040767818495825777f + fConst149);
		fConst151 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst102) / fConst101)));
		fConst152 = (53.53615295455673f + fConst130);
		fConst153 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst102) / fConst101)));
		fConst154 = (7.621731298870603f + fConst130);
		fConst155 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst102) / fConst101)));
		fConst156 = (9.9999997055e-05f / fConst104);
		fConst157 = (0.000433227200555f + fConst156);
		fConst158 = (0.24529150870616f + ((fConst102 - 0.782413046821645f) / fConst101));
		fConst159 = (2 * (0.24529150870616f - fConst130));
		for (int i=0; i<3; i++) fRec38[i] = 0;
		fConst160 = (2 * (0.000433227200555f - fConst156));
		fConst161 = (0.689621364484675f + ((fConst102 - 0.512478641889141f) / fConst101));
		fConst162 = (2 * (0.689621364484675f - fConst130));
		for (int i=0; i<3; i++) fRec37[i] = 0;
		fConst163 = (2 * (7.621731298870603f - fConst130));
		fConst164 = (1.069358407707312f + ((fConst102 - 0.168404871113589f) / fConst101));
		fConst165 = (2 * (1.069358407707312f - fConst130));
		for (int i=0; i<3; i++) fRec36[i] = 0;
		fConst166 = (2 * (53.53615295455673f - fConst130));
		fConst167 = (4.076781969643807f + ((fConst140 - 3.1897274020965583f) / fConst139));
		fConst168 = (1.0f / fConst142);
		fConst169 = (2 * (4.076781969643807f - fConst168));
		for (int i=0; i<3; i++) fRec35[i] = 0;
		fConst170 = (2 * (0.00040767818495825777f - fConst149));
		fConst171 = (1.450071084655647f + ((fConst140 - 0.7431304601070396f) / fConst139));
		fConst172 = (2 * (1.450071084655647f - fConst168));
		for (int i=0; i<3; i++) fRec34[i] = 0;
		fConst173 = (2 * (1.450071084655647f - fConst146));
		fConst174 = (0.9351401670315425f + ((fConst140 - 0.157482159302087f) / fConst139));
		fConst175 = (2 * (0.9351401670315425f - fConst168));
		for (int i=0; i<3; i++) fRec33[i] = 0;
		fConst176 = (2 * (0.9351401670315425f - fConst143));
		for (int i=0; i<2; i++) fRec32[i] = 0;
		fConst177 = tanf((3116.8546769775025f / float(iConst0)));
		fConst178 = (1.0f / fConst177);
		fConst179 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst178) / fConst177)));
		fConst180 = faustpower<2>(fConst177);
		fConst181 = (50.063807016150385f / fConst180);
		fConst182 = (0.9351401670315425f + fConst181);
		fConst183 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst178) / fConst177)));
		fConst184 = (11.052052171507189f / fConst180);
		fConst185 = (1.450071084655647f + fConst184);
		fConst186 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst178) / fConst177)));
		fConst187 = (0.0017661728399818856f / fConst180);
		fConst188 = (0.00040767818495825777f + fConst187);
		fConst189 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst140) / fConst139)));
		fConst190 = (53.53615295455673f + fConst168);
		fConst191 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst140) / fConst139)));
		fConst192 = (7.621731298870603f + fConst168);
		fConst193 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst140) / fConst139)));
		fConst194 = (9.9999997055e-05f / fConst142);
		fConst195 = (0.000433227200555f + fConst194);
		fConst196 = (0.24529150870616f + ((fConst140 - 0.782413046821645f) / fConst139));
		fConst197 = (2 * (0.24529150870616f - fConst168));
		for (int i=0; i<3; i++) fRec45[i] = 0;
		fConst198 = (2 * (0.000433227200555f - fConst194));
		fConst199 = (0.689621364484675f + ((fConst140 - 0.512478641889141f) / fConst139));
		fConst200 = (2 * (0.689621364484675f - fConst168));
		for (int i=0; i<3; i++) fRec44[i] = 0;
		fConst201 = (2 * (7.621731298870603f - fConst168));
		fConst202 = (1.069358407707312f + ((fConst140 - 0.168404871113589f) / fConst139));
		fConst203 = (2 * (1.069358407707312f - fConst168));
		for (int i=0; i<3; i++) fRec43[i] = 0;
		fConst204 = (2 * (53.53615295455673f - fConst168));
		fConst205 = (4.076781969643807f + ((fConst178 - 3.1897274020965583f) / fConst177));
		fConst206 = (1.0f / fConst180);
		fConst207 = (2 * (4.076781969643807f - fConst206));
		for (int i=0; i<3; i++) fRec42[i] = 0;
		fConst208 = (2 * (0.00040767818495825777f - fConst187));
		fConst209 = (1.450071084655647f + ((fConst178 - 0.7431304601070396f) / fConst177));
		fConst210 = (2 * (1.450071084655647f - fConst206));
		for (int i=0; i<3; i++) fRec41[i] = 0;
		fConst211 = (2 * (1.450071084655647f - fConst184));
		fConst212 = (0.9351401670315425f + ((fConst178 - 0.157482159302087f) / fConst177));
		fConst213 = (2 * (0.9351401670315425f - fConst206));
		for (int i=0; i<3; i++) fRec40[i] = 0;
		fConst214 = (2 * (0.9351401670315425f - fConst181));
		for (int i=0; i<2; i++) fRec39[i] = 0;
		fConst215 = tanf((1963.4954084936207f / float(iConst0)));
		fConst216 = (1.0f / fConst215);
		fConst217 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst216) / fConst215)));
		fConst218 = faustpower<2>(fConst215);
		fConst219 = (50.063807016150385f / fConst218);
		fConst220 = (0.9351401670315425f + fConst219);
		fConst221 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst216) / fConst215)));
		fConst222 = (11.052052171507189f / fConst218);
		fConst223 = (1.450071084655647f + fConst222);
		fConst224 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst216) / fConst215)));
		fConst225 = (0.0017661728399818856f / fConst218);
		fConst226 = (0.00040767818495825777f + fConst225);
		fConst227 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst178) / fConst177)));
		fConst228 = (53.53615295455673f + fConst206);
		fConst229 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst178) / fConst177)));
		fConst230 = (7.621731298870603f + fConst206);
		fConst231 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst178) / fConst177)));
		fConst232 = (9.9999997055e-05f / fConst180);
		fConst233 = (0.000433227200555f + fConst232);
		fConst234 = (0.24529150870616f + ((fConst178 - 0.782413046821645f) / fConst177));
		fConst235 = (2 * (0.24529150870616f - fConst206));
		for (int i=0; i<3; i++) fRec52[i] = 0;
		fConst236 = (2 * (0.000433227200555f - fConst232));
		fConst237 = (0.689621364484675f + ((fConst178 - 0.512478641889141f) / fConst177));
		fConst238 = (2 * (0.689621364484675f - fConst206));
		for (int i=0; i<3; i++) fRec51[i] = 0;
		fConst239 = (2 * (7.621731298870603f - fConst206));
		fConst240 = (1.069358407707312f + ((fConst178 - 0.168404871113589f) / fConst177));
		fConst241 = (2 * (1.069358407707312f - fConst206));
		for (int i=0; i<3; i++) fRec50[i] = 0;
		fConst242 = (2 * (53.53615295455673f - fConst206));
		fConst243 = (4.076781969643807f + ((fConst216 - 3.1897274020965583f) / fConst215));
		fConst244 = (1.0f / fConst218);
		fConst245 = (2 * (4.076781969643807f - fConst244));
		for (int i=0; i<3; i++) fRec49[i] = 0;
		fConst246 = (2 * (0.00040767818495825777f - fConst225));
		fConst247 = (1.450071084655647f + ((fConst216 - 0.7431304601070396f) / fConst215));
		fConst248 = (2 * (1.450071084655647f - fConst244));
		for (int i=0; i<3; i++) fRec48[i] = 0;
		fConst249 = (2 * (1.450071084655647f - fConst222));
		fConst250 = (0.9351401670315425f + ((fConst216 - 0.157482159302087f) / fConst215));
		fConst251 = (2 * (0.9351401670315425f - fConst244));
		for (int i=0; i<3; i++) fRec47[i] = 0;
		fConst252 = (2 * (0.9351401670315425f - fConst219));
		for (int i=0; i<2; i++) fRec46[i] = 0;
		fConst253 = tanf((1236.9245982665225f / float(iConst0)));
		fConst254 = (1.0f / fConst253);
		fConst255 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst254) / fConst253)));
		fConst256 = faustpower<2>(fConst253);
		fConst257 = (50.063807016150385f / fConst256);
		fConst258 = (0.9351401670315425f + fConst257);
		fConst259 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst254) / fConst253)));
		fConst260 = (11.052052171507189f / fConst256);
		fConst261 = (1.450071084655647f + fConst260);
		fConst262 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst254) / fConst253)));
		fConst263 = (0.0017661728399818856f / fConst256);
		fConst264 = (0.00040767818495825777f + fConst263);
		fConst265 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst216) / fConst215)));
		fConst266 = (53.53615295455673f + fConst244);
		fConst267 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst216) / fConst215)));
		fConst268 = (7.621731298870603f + fConst244);
		fConst269 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst216) / fConst215)));
		fConst270 = (9.9999997055e-05f / fConst218);
		fConst271 = (0.000433227200555f + fConst270);
		fConst272 = (0.24529150870616f + ((fConst216 - 0.782413046821645f) / fConst215));
		fConst273 = (2 * (0.24529150870616f - fConst244));
		for (int i=0; i<3; i++) fRec59[i] = 0;
		fConst274 = (2 * (0.000433227200555f - fConst270));
		fConst275 = (0.689621364484675f + ((fConst216 - 0.512478641889141f) / fConst215));
		fConst276 = (2 * (0.689621364484675f - fConst244));
		for (int i=0; i<3; i++) fRec58[i] = 0;
		fConst277 = (2 * (7.621731298870603f - fConst244));
		fConst278 = (1.069358407707312f + ((fConst216 - 0.168404871113589f) / fConst215));
		fConst279 = (2 * (1.069358407707312f - fConst244));
		for (int i=0; i<3; i++) fRec57[i] = 0;
		fConst280 = (2 * (53.53615295455673f - fConst244));
		fConst281 = (4.076781969643807f + ((fConst254 - 3.1897274020965583f) / fConst253));
		fConst282 = (1.0f / fConst256);
		fConst283 = (2 * (4.076781969643807f - fConst282));
		for (int i=0; i<3; i++) fRec56[i] = 0;
		fConst284 = (2 * (0.00040767818495825777f - fConst263));
		fConst285 = (1.450071084655647f + ((fConst254 - 0.7431304601070396f) / fConst253));
		fConst286 = (2 * (1.450071084655647f - fConst282));
		for (int i=0; i<3; i++) fRec55[i] = 0;
		fConst287 = (2 * (1.450071084655647f - fConst260));
		fConst288 = (0.9351401670315425f + ((fConst254 - 0.157482159302087f) / fConst253));
		fConst289 = (2 * (0.9351401670315425f - fConst282));
		for (int i=0; i<3; i++) fRec54[i] = 0;
		fConst290 = (2 * (0.9351401670315425f - fConst257));
		for (int i=0; i<2; i++) fRec53[i] = 0;
		fConst291 = tanf((779.213669244376f / float(iConst0)));
		fConst292 = (1.0f / fConst291);
		fConst293 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst292) / fConst291)));
		fConst294 = faustpower<2>(fConst291);
		fConst295 = (50.063807016150385f / fConst294);
		fConst296 = (0.9351401670315425f + fConst295);
		fConst297 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst292) / fConst291)));
		fConst298 = (11.052052171507189f / fConst294);
		fConst299 = (1.450071084655647f + fConst298);
		fConst300 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst292) / fConst291)));
		fConst301 = (0.0017661728399818856f / fConst294);
		fConst302 = (0.00040767818495825777f + fConst301);
		fConst303 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst254) / fConst253)));
		fConst304 = (53.53615295455673f + fConst282);
		fConst305 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst254) / fConst253)));
		fConst306 = (7.621731298870603f + fConst282);
		fConst307 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst254) / fConst253)));
		fConst308 = (9.9999997055e-05f / fConst256);
		fConst309 = (0.000433227200555f + fConst308);
		fConst310 = (0.24529150870616f + ((fConst254 - 0.782413046821645f) / fConst253));
		fConst311 = (2 * (0.24529150870616f - fConst282));
		for (int i=0; i<3; i++) fRec66[i] = 0;
		fConst312 = (2 * (0.000433227200555f - fConst308));
		fConst313 = (0.689621364484675f + ((fConst254 - 0.512478641889141f) / fConst253));
		fConst314 = (2 * (0.689621364484675f - fConst282));
		for (int i=0; i<3; i++) fRec65[i] = 0;
		fConst315 = (2 * (7.621731298870603f - fConst282));
		fConst316 = (1.069358407707312f + ((fConst254 - 0.168404871113589f) / fConst253));
		fConst317 = (2 * (1.069358407707312f - fConst282));
		for (int i=0; i<3; i++) fRec64[i] = 0;
		fConst318 = (2 * (53.53615295455673f - fConst282));
		fConst319 = (4.076781969643807f + ((fConst292 - 3.1897274020965583f) / fConst291));
		fConst320 = (1.0f / fConst294);
		fConst321 = (2 * (4.076781969643807f - fConst320));
		for (int i=0; i<3; i++) fRec63[i] = 0;
		fConst322 = (2 * (0.00040767818495825777f - fConst301));
		fConst323 = (1.450071084655647f + ((fConst292 - 0.7431304601070396f) / fConst291));
		fConst324 = (2 * (1.450071084655647f - fConst320));
		for (int i=0; i<3; i++) fRec62[i] = 0;
		fConst325 = (2 * (1.450071084655647f - fConst298));
		fConst326 = (0.9351401670315425f + ((fConst292 - 0.157482159302087f) / fConst291));
		fConst327 = (2 * (0.9351401670315425f - fConst320));
		for (int i=0; i<3; i++) fRec61[i] = 0;
		fConst328 = (2 * (0.9351401670315425f - fConst295));
		for (int i=0; i<2; i++) fRec60[i] = 0;
		fConst329 = tanf((490.8738521234052f / float(iConst0)));
		fConst330 = (1.0f / fConst329);
		fConst331 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst330) / fConst329)));
		fConst332 = faustpower<2>(fConst329);
		fConst333 = (50.063807016150385f / fConst332);
		fConst334 = (0.9351401670315425f + fConst333);
		fConst335 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst330) / fConst329)));
		fConst336 = (11.052052171507189f / fConst332);
		fConst337 = (1.450071084655647f + fConst336);
		fConst338 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst330) / fConst329)));
		fConst339 = (0.0017661728399818856f / fConst332);
		fConst340 = (0.00040767818495825777f + fConst339);
		fConst341 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst292) / fConst291)));
		fConst342 = (53.53615295455673f + fConst320);
		fConst343 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst292) / fConst291)));
		fConst344 = (7.621731298870603f + fConst320);
		fConst345 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst292) / fConst291)));
		fConst346 = (9.9999997055e-05f / fConst294);
		fConst347 = (0.000433227200555f + fConst346);
		fConst348 = (0.24529150870616f + ((fConst292 - 0.782413046821645f) / fConst291));
		fConst349 = (2 * (0.24529150870616f - fConst320));
		for (int i=0; i<3; i++) fRec73[i] = 0;
		fConst350 = (2 * (0.000433227200555f - fConst346));
		fConst351 = (0.689621364484675f + ((fConst292 - 0.512478641889141f) / fConst291));
		fConst352 = (2 * (0.689621364484675f - fConst320));
		for (int i=0; i<3; i++) fRec72[i] = 0;
		fConst353 = (2 * (7.621731298870603f - fConst320));
		fConst354 = (1.069358407707312f + ((fConst292 - 0.168404871113589f) / fConst291));
		fConst355 = (2 * (1.069358407707312f - fConst320));
		for (int i=0; i<3; i++) fRec71[i] = 0;
		fConst356 = (2 * (53.53615295455673f - fConst320));
		fConst357 = (4.076781969643807f + ((fConst330 - 3.1897274020965583f) / fConst329));
		fConst358 = (1.0f / fConst332);
		fConst359 = (2 * (4.076781969643807f - fConst358));
		for (int i=0; i<3; i++) fRec70[i] = 0;
		fConst360 = (2 * (0.00040767818495825777f - fConst339));
		fConst361 = (1.450071084655647f + ((fConst330 - 0.7431304601070396f) / fConst329));
		fConst362 = (2 * (1.450071084655647f - fConst358));
		for (int i=0; i<3; i++) fRec69[i] = 0;
		fConst363 = (2 * (1.450071084655647f - fConst336));
		fConst364 = (0.9351401670315425f + ((fConst330 - 0.157482159302087f) / fConst329));
		fConst365 = (2 * (0.9351401670315425f - fConst358));
		for (int i=0; i<3; i++) fRec68[i] = 0;
		fConst366 = (2 * (0.9351401670315425f - fConst333));
		for (int i=0; i<2; i++) fRec67[i] = 0;
		fConst367 = tanf((309.2311495666306f / float(iConst0)));
		fConst368 = (1.0f / fConst367);
		fConst369 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst368) / fConst367)));
		fConst370 = faustpower<2>(fConst367);
		fConst371 = (50.063807016150385f / fConst370);
		fConst372 = (0.9351401670315425f + fConst371);
		fConst373 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst368) / fConst367)));
		fConst374 = (11.052052171507189f / fConst370);
		fConst375 = (1.450071084655647f + fConst374);
		fConst376 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst368) / fConst367)));
		fConst377 = (0.0017661728399818856f / fConst370);
		fConst378 = (0.00040767818495825777f + fConst377);
		fConst379 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst330) / fConst329)));
		fConst380 = (53.53615295455673f + fConst358);
		fConst381 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst330) / fConst329)));
		fConst382 = (7.621731298870603f + fConst358);
		fConst383 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst330) / fConst329)));
		fConst384 = (9.9999997055e-05f / fConst332);
		fConst385 = (0.000433227200555f + fConst384);
		fConst386 = (0.24529150870616f + ((fConst330 - 0.782413046821645f) / fConst329));
		fConst387 = (2 * (0.24529150870616f - fConst358));
		for (int i=0; i<3; i++) fRec80[i] = 0;
		fConst388 = (2 * (0.000433227200555f - fConst384));
		fConst389 = (0.689621364484675f + ((fConst330 - 0.512478641889141f) / fConst329));
		fConst390 = (2 * (0.689621364484675f - fConst358));
		for (int i=0; i<3; i++) fRec79[i] = 0;
		fConst391 = (2 * (7.621731298870603f - fConst358));
		fConst392 = (1.069358407707312f + ((fConst330 - 0.168404871113589f) / fConst329));
		fConst393 = (2 * (1.069358407707312f - fConst358));
		for (int i=0; i<3; i++) fRec78[i] = 0;
		fConst394 = (2 * (53.53615295455673f - fConst358));
		fConst395 = (4.076781969643807f + ((fConst368 - 3.1897274020965583f) / fConst367));
		fConst396 = (1.0f / fConst370);
		fConst397 = (2 * (4.076781969643807f - fConst396));
		for (int i=0; i<3; i++) fRec77[i] = 0;
		fConst398 = (2 * (0.00040767818495825777f - fConst377));
		fConst399 = (1.450071084655647f + ((fConst368 - 0.7431304601070396f) / fConst367));
		fConst400 = (2 * (1.450071084655647f - fConst396));
		for (int i=0; i<3; i++) fRec76[i] = 0;
		fConst401 = (2 * (1.450071084655647f - fConst374));
		fConst402 = (0.9351401670315425f + ((fConst368 - 0.157482159302087f) / fConst367));
		fConst403 = (2 * (0.9351401670315425f - fConst396));
		for (int i=0; i<3; i++) fRec75[i] = 0;
		fConst404 = (2 * (0.9351401670315425f - fConst371));
		for (int i=0; i<2; i++) fRec74[i] = 0;
		fConst405 = tanf((194.803417311094f / float(iConst0)));
		fConst406 = (1.0f / fConst405);
		fConst407 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst406) / fConst405)));
		fConst408 = faustpower<2>(fConst405);
		fConst409 = (50.063807016150385f / fConst408);
		fConst410 = (0.9351401670315425f + fConst409);
		fConst411 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst406) / fConst405)));
		fConst412 = (11.052052171507189f / fConst408);
		fConst413 = (1.450071084655647f + fConst412);
		fConst414 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst406) / fConst405)));
		fConst415 = (0.0017661728399818856f / fConst408);
		fConst416 = (0.00040767818495825777f + fConst415);
		fConst417 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst368) / fConst367)));
		fConst418 = (53.53615295455673f + fConst396);
		fConst419 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst368) / fConst367)));
		fConst420 = (7.621731298870603f + fConst396);
		fConst421 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst368) / fConst367)));
		fConst422 = (9.9999997055e-05f / fConst370);
		fConst423 = (0.000433227200555f + fConst422);
		fConst424 = (0.24529150870616f + ((fConst368 - 0.782413046821645f) / fConst367));
		fConst425 = (2 * (0.24529150870616f - fConst396));
		for (int i=0; i<3; i++) fRec87[i] = 0;
		fConst426 = (2 * (0.000433227200555f - fConst422));
		fConst427 = (0.689621364484675f + ((fConst368 - 0.512478641889141f) / fConst367));
		fConst428 = (2 * (0.689621364484675f - fConst396));
		for (int i=0; i<3; i++) fRec86[i] = 0;
		fConst429 = (2 * (7.621731298870603f - fConst396));
		fConst430 = (1.069358407707312f + ((fConst368 - 0.168404871113589f) / fConst367));
		fConst431 = (2 * (1.069358407707312f - fConst396));
		for (int i=0; i<3; i++) fRec85[i] = 0;
		fConst432 = (2 * (53.53615295455673f - fConst396));
		fConst433 = (4.076781969643807f + ((fConst406 - 3.1897274020965583f) / fConst405));
		fConst434 = (1.0f / fConst408);
		fConst435 = (2 * (4.076781969643807f - fConst434));
		for (int i=0; i<3; i++) fRec84[i] = 0;
		fConst436 = (2 * (0.00040767818495825777f - fConst415));
		fConst437 = (1.450071084655647f + ((fConst406 - 0.7431304601070396f) / fConst405));
		fConst438 = (2 * (1.450071084655647f - fConst434));
		for (int i=0; i<3; i++) fRec83[i] = 0;
		fConst439 = (2 * (1.450071084655647f - fConst412));
		fConst440 = (0.9351401670315425f + ((fConst406 - 0.157482159302087f) / fConst405));
		fConst441 = (2 * (0.9351401670315425f - fConst434));
		for (int i=0; i<3; i++) fRec82[i] = 0;
		fConst442 = (2 * (0.9351401670315425f - fConst409));
		for (int i=0; i<2; i++) fRec81[i] = 0;
		fConst443 = tanf((122.7184630308513f / float(iConst0)));
		fConst444 = (1.0f / fConst443);
		fConst445 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst444) / fConst443)));
		fConst446 = faustpower<2>(fConst443);
		fConst447 = (50.063807016150385f / fConst446);
		fConst448 = (0.9351401670315425f + fConst447);
		fConst449 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst444) / fConst443)));
		fConst450 = (11.052052171507189f / fConst446);
		fConst451 = (1.450071084655647f + fConst450);
		fConst452 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst444) / fConst443)));
		fConst453 = (0.0017661728399818856f / fConst446);
		fConst454 = (0.00040767818495825777f + fConst453);
		fConst455 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst406) / fConst405)));
		fConst456 = (53.53615295455673f + fConst434);
		fConst457 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst406) / fConst405)));
		fConst458 = (7.621731298870603f + fConst434);
		fConst459 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst406) / fConst405)));
		fConst460 = (9.9999997055e-05f / fConst408);
		fConst461 = (0.000433227200555f + fConst460);
		fConst462 = (0.24529150870616f + ((fConst406 - 0.782413046821645f) / fConst405));
		fConst463 = (2 * (0.24529150870616f - fConst434));
		for (int i=0; i<3; i++) fRec94[i] = 0;
		fConst464 = (2 * (0.000433227200555f - fConst460));
		fConst465 = (0.689621364484675f + ((fConst406 - 0.512478641889141f) / fConst405));
		fConst466 = (2 * (0.689621364484675f - fConst434));
		for (int i=0; i<3; i++) fRec93[i] = 0;
		fConst467 = (2 * (7.621731298870603f - fConst434));
		fConst468 = (1.069358407707312f + ((fConst406 - 0.168404871113589f) / fConst405));
		fConst469 = (2 * (1.069358407707312f - fConst434));
		for (int i=0; i<3; i++) fRec92[i] = 0;
		fConst470 = (2 * (53.53615295455673f - fConst434));
		fConst471 = (4.076781969643807f + ((fConst444 - 3.1897274020965583f) / fConst443));
		fConst472 = (1.0f / fConst446);
		fConst473 = (2 * (4.076781969643807f - fConst472));
		for (int i=0; i<3; i++) fRec91[i] = 0;
		fConst474 = (2 * (0.00040767818495825777f - fConst453));
		fConst475 = (1.450071084655647f + ((fConst444 - 0.7431304601070396f) / fConst443));
		fConst476 = (2 * (1.450071084655647f - fConst472));
		for (int i=0; i<3; i++) fRec90[i] = 0;
		fConst477 = (2 * (1.450071084655647f - fConst450));
		fConst478 = (0.9351401670315425f + ((fConst444 - 0.157482159302087f) / fConst443));
		fConst479 = (2 * (0.9351401670315425f - fConst472));
		for (int i=0; i<3; i++) fRec89[i] = 0;
		fConst480 = (2 * (0.9351401670315425f - fConst447));
		for (int i=0; i<2; i++) fRec88[i] = 0;
		fConst481 = tanf((77.3077873916577f / float(iConst0)));
		fConst482 = (1.0f / fConst481);
		fConst483 = (1.0f / (0.9351401670315425f + ((0.157482159302087f + fConst482) / fConst481)));
		fConst484 = faustpower<2>(fConst481);
		fConst485 = (50.063807016150385f / fConst484);
		fConst486 = (0.9351401670315425f + fConst485);
		fConst487 = (1.0f / (1.450071084655647f + ((0.7431304601070396f + fConst482) / fConst481)));
		fConst488 = (11.052052171507189f / fConst484);
		fConst489 = (1.450071084655647f + fConst488);
		fConst490 = (1.0f / (4.076781969643807f + ((3.1897274020965583f + fConst482) / fConst481)));
		fConst491 = (0.0017661728399818856f / fConst484);
		fConst492 = (0.00040767818495825777f + fConst491);
		fConst493 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst444) / fConst443)));
		fConst494 = (53.53615295455673f + fConst472);
		fConst495 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst444) / fConst443)));
		fConst496 = (7.621731298870603f + fConst472);
		fConst497 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst444) / fConst443)));
		fConst498 = (9.9999997055e-05f / fConst446);
		fConst499 = (0.000433227200555f + fConst498);
		fConst500 = (0.24529150870616f + ((fConst444 - 0.782413046821645f) / fConst443));
		fConst501 = (2 * (0.24529150870616f - fConst472));
		for (int i=0; i<3; i++) fRec101[i] = 0;
		fConst502 = (2 * (0.000433227200555f - fConst498));
		fConst503 = (0.689621364484675f + ((fConst444 - 0.512478641889141f) / fConst443));
		fConst504 = (2 * (0.689621364484675f - fConst472));
		for (int i=0; i<3; i++) fRec100[i] = 0;
		fConst505 = (2 * (7.621731298870603f - fConst472));
		fConst506 = (1.069358407707312f + ((fConst444 - 0.168404871113589f) / fConst443));
		fConst507 = (2 * (1.069358407707312f - fConst472));
		for (int i=0; i<3; i++) fRec99[i] = 0;
		fConst508 = (2 * (53.53615295455673f - fConst472));
		fConst509 = (4.076781969643807f + ((fConst482 - 3.1897274020965583f) / fConst481));
		fConst510 = (1.0f / fConst484);
		fConst511 = (2 * (4.076781969643807f - fConst510));
		for (int i=0; i<3; i++) fRec98[i] = 0;
		fConst512 = (2 * (0.00040767818495825777f - fConst491));
		fConst513 = (1.450071084655647f + ((fConst482 - 0.7431304601070396f) / fConst481));
		fConst514 = (2 * (1.450071084655647f - fConst510));
		for (int i=0; i<3; i++) fRec97[i] = 0;
		fConst515 = (2 * (1.450071084655647f - fConst488));
		fConst516 = (0.9351401670315425f + ((fConst482 - 0.157482159302087f) / fConst481));
		fConst517 = (2 * (0.9351401670315425f - fConst510));
		for (int i=0; i<3; i++) fRec96[i] = 0;
		fConst518 = (2 * (0.9351401670315425f - fConst485));
		for (int i=0; i<2; i++) fRec95[i] = 0;
		fConst519 = (1.0f / (1.069358407707312f + ((0.168404871113589f + fConst482) / fConst481)));
		fConst520 = (53.53615295455673f + fConst510);
		fConst521 = (1.0f / (0.689621364484675f + ((0.512478641889141f + fConst482) / fConst481)));
		fConst522 = (7.621731298870603f + fConst510);
		fConst523 = (1.0f / (0.24529150870616f + ((0.782413046821645f + fConst482) / fConst481)));
		fConst524 = (9.9999997055e-05f / fConst484);
		fConst525 = (0.000433227200555f + fConst524);
		fConst526 = (0.24529150870616f + ((fConst482 - 0.782413046821645f) / fConst481));
		fConst527 = (2 * (0.24529150870616f - fConst510));
		for (int i=0; i<3; i++) fRec105[i] = 0;
		fConst528 = (2 * (0.000433227200555f - fConst524));
		fConst529 = (0.689621364484675f + ((fConst482 - 0.512478641889141f) / fConst481));
		fConst530 = (2 * (0.689621364484675f - fConst510));
		for (int i=0; i<3; i++) fRec104[i] = 0;
		fConst531 = (2 * (7.621731298870603f - fConst510));
		fConst532 = (1.069358407707312f + ((fConst482 - 0.168404871113589f) / fConst481));
		fConst533 = (2 * (1.069358407707312f - fConst510));
		for (int i=0; i<3; i++) fRec103[i] = 0;
		fConst534 = (2 * (53.53615295455673f - fConst510));
		for (int i=0; i<2; i++) fRec102[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("cubic_distortion");
		interface->declare(0, "1", "");
		interface->openVerticalBox("");
		interface->declare(0, "0", "");
		interface->declare(0, "tooltip", "Sine oscillator based on 2D vector rotation");
		interface->openVerticalBox("SINE WAVE OSCILLATOR oscrs");
		interface->declare(0, "1", "");
		interface->openHorizontalBox("");
		interface->declare(&fslider2, "1", "");
		interface->declare(&fslider2, "tooltip", "Sawtooth waveform amplitude");
		interface->declare(&fslider2, "unit", "dB");
		interface->addHorizontalSlider("Amplitude", &fslider2, -2e+01f, -1.2e+02f, 1e+01f, 0.1f);
		interface->declare(&fslider4, "2", "");
		interface->declare(&fslider4, "tooltip", "Sine wave frequency as a Piano Key (PK) number (A440 = 49 PK)");
		interface->declare(&fslider4, "unit", "PK");
		interface->addHorizontalSlider("Frequency", &fslider4, 49.0f, 1.0f, 88.0f, 0.01f);
		interface->declare(&fslider3, "3", "");
		interface->declare(&fslider3, "tooltip", "Portamento (frequency-glide) time-constant in seconds");
		interface->declare(&fslider3, "unit", "sec");
		interface->addHorizontalSlider("Portamento", &fslider3, 0.1f, 0.0f, 1.0f, 0.001f);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
		interface->declare(0, "2", "");
		interface->openVerticalBox("");
		interface->declare(0, "tooltip", "Reference:          https://ccrma.stanford.edu/~jos/pasp/Cubic_Soft_Clipper.html");
		interface->openVerticalBox("CUBIC NONLINEARITY cubicnl");
		interface->declare(0, "1", "");
		interface->openHorizontalBox("");
		interface->declare(&fcheckbox0, "0", "");
		interface->declare(&fcheckbox0, "tooltip", "When this is checked, the nonlinearity has no effect");
		interface->addCheckButton("Bypass", &fcheckbox0);
		interface->declare(&fslider5, "1", "");
		interface->declare(&fslider5, "tooltip", "Amount of distortion");
		interface->addHorizontalSlider("Drive", &fslider5, 0.0f, 0.0f, 1.0f, 0.01f);
		interface->declare(&fslider6, "2", "");
		interface->declare(&fslider6, "tooltip", "Brings in even harmonics");
		interface->addHorizontalSlider("Offset", &fslider6, 0.0f, 0.0f, 1.0f, 0.01f);
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
		float 	fSlow5 = expf((0 - (fConst1 / fslider3)));
		float 	fSlow6 = (4.4e+02f * (powf(2.0f,(0.08333333333333333f * (fslider4 - 49.0f))) * (1.0f - fSlow5)));
		float 	fSlow7 = (0.0010000000000000009f * fslider5);
		float 	fSlow8 = (0.0010000000000000009f * fslider6);
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			iVec0[0] = 1;
			fRec4[0] = ((0.999f * fRec4[1]) + fSlow4);
			fRec7[0] = ((fSlow5 * fRec7[1]) + fSlow6);
			float fTemp0 = (fConst14 * fRec7[0]);
			float fTemp1 = sinf(fTemp0);
			float fTemp2 = cosf(fTemp0);
			fRec5[0] = ((fRec6[1] * fTemp1) + (fRec5[1] * fTemp2));
			fRec6[0] = ((1 + ((fRec6[1] * fTemp2) + (fRec5[1] * (0 - fTemp1)))) - iVec0[1]);
			float fTemp3 = (fRec4[0] * fRec5[0]);
			fRec9[0] = ((0.999f * fRec9[1]) + fSlow7);
			fRec10[0] = ((0.999f * fRec10[1]) + fSlow8);
			float fTemp4 = max((float)-1, min((float)1, ((((iSlow3)?0:fTemp3) * powf(1e+01f,(2 * fRec9[0]))) + fRec10[0])));
			float fTemp5 = (fTemp4 * (1 - (0.3333333333333333f * faustpower<2>(fTemp4))));
			fVec1[0] = fTemp5;
			fRec8[0] = (((0.995f * fRec8[1]) + fVec1[0]) - fVec1[1]);
			float fTemp6 = ((iSlow3)?fTemp3:fRec8[0]);
			fRec3[0] = (fTemp6 - (fConst11 * ((fConst15 * fRec3[2]) + (fConst17 * fRec3[1]))));
			fRec2[0] = ((fConst11 * (((fConst13 * fRec3[0]) + (fConst18 * fRec3[1])) + (fConst13 * fRec3[2]))) - (fConst8 * ((fConst19 * fRec2[2]) + (fConst20 * fRec2[1]))));
			fRec1[0] = ((fConst8 * (((fConst10 * fRec2[0]) + (fConst21 * fRec2[1])) + (fConst10 * fRec2[2]))) - (fConst4 * ((fConst22 * fRec1[2]) + (fConst23 * fRec1[1]))));
			fRec0[0] = ((fSlow1 * fRec0[1]) + (fSlow2 * fabsf((fConst4 * (((fConst7 * fRec1[0]) + (fConst24 * fRec1[1])) + (fConst7 * fRec1[2]))))));
			fbargraph0 = (fSlow0 + (20 * log10f(fRec0[0])));
			fRec17[0] = (fTemp6 - (fConst41 * ((fConst44 * fRec17[2]) + (fConst45 * fRec17[1]))));
			fRec16[0] = ((fConst41 * (((fConst43 * fRec17[0]) + (fConst46 * fRec17[1])) + (fConst43 * fRec17[2]))) - (fConst39 * ((fConst47 * fRec16[2]) + (fConst48 * fRec16[1]))));
			fRec15[0] = ((fConst39 * (((fConst40 * fRec16[0]) + (fConst49 * fRec16[1])) + (fConst40 * fRec16[2]))) - (fConst37 * ((fConst50 * fRec15[2]) + (fConst51 * fRec15[1]))));
			float fTemp7 = (fConst37 * (((fConst38 * fRec15[0]) + (fConst52 * fRec15[1])) + (fConst38 * fRec15[2])));
			fRec14[0] = (fTemp7 - (fConst34 * ((fConst53 * fRec14[2]) + (fConst55 * fRec14[1]))));
			fRec13[0] = ((fConst34 * (((fConst36 * fRec14[0]) + (fConst56 * fRec14[1])) + (fConst36 * fRec14[2]))) - (fConst31 * ((fConst57 * fRec13[2]) + (fConst58 * fRec13[1]))));
			fRec12[0] = ((fConst31 * (((fConst33 * fRec13[0]) + (fConst59 * fRec13[1])) + (fConst33 * fRec13[2]))) - (fConst27 * ((fConst60 * fRec12[2]) + (fConst61 * fRec12[1]))));
			fRec11[0] = ((fSlow1 * fRec11[1]) + (fSlow2 * fabsf((fConst27 * (((fConst30 * fRec12[0]) + (fConst62 * fRec12[1])) + (fConst30 * fRec12[2]))))));
			fbargraph1 = (fSlow0 + (20 * log10f(fRec11[0])));
			fRec24[0] = (fTemp7 - (fConst79 * ((fConst82 * fRec24[2]) + (fConst83 * fRec24[1]))));
			fRec23[0] = ((fConst79 * (((fConst81 * fRec24[0]) + (fConst84 * fRec24[1])) + (fConst81 * fRec24[2]))) - (fConst77 * ((fConst85 * fRec23[2]) + (fConst86 * fRec23[1]))));
			fRec22[0] = ((fConst77 * (((fConst78 * fRec23[0]) + (fConst87 * fRec23[1])) + (fConst78 * fRec23[2]))) - (fConst75 * ((fConst88 * fRec22[2]) + (fConst89 * fRec22[1]))));
			float fTemp8 = (fConst75 * (((fConst76 * fRec22[0]) + (fConst90 * fRec22[1])) + (fConst76 * fRec22[2])));
			fRec21[0] = (fTemp8 - (fConst72 * ((fConst91 * fRec21[2]) + (fConst93 * fRec21[1]))));
			fRec20[0] = ((fConst72 * (((fConst74 * fRec21[0]) + (fConst94 * fRec21[1])) + (fConst74 * fRec21[2]))) - (fConst69 * ((fConst95 * fRec20[2]) + (fConst96 * fRec20[1]))));
			fRec19[0] = ((fConst69 * (((fConst71 * fRec20[0]) + (fConst97 * fRec20[1])) + (fConst71 * fRec20[2]))) - (fConst65 * ((fConst98 * fRec19[2]) + (fConst99 * fRec19[1]))));
			fRec18[0] = ((fSlow1 * fRec18[1]) + (fSlow2 * fabsf((fConst65 * (((fConst68 * fRec19[0]) + (fConst100 * fRec19[1])) + (fConst68 * fRec19[2]))))));
			fbargraph2 = (fSlow0 + (20 * log10f(fRec18[0])));
			fRec31[0] = (fTemp8 - (fConst117 * ((fConst120 * fRec31[2]) + (fConst121 * fRec31[1]))));
			fRec30[0] = ((fConst117 * (((fConst119 * fRec31[0]) + (fConst122 * fRec31[1])) + (fConst119 * fRec31[2]))) - (fConst115 * ((fConst123 * fRec30[2]) + (fConst124 * fRec30[1]))));
			fRec29[0] = ((fConst115 * (((fConst116 * fRec30[0]) + (fConst125 * fRec30[1])) + (fConst116 * fRec30[2]))) - (fConst113 * ((fConst126 * fRec29[2]) + (fConst127 * fRec29[1]))));
			float fTemp9 = (fConst113 * (((fConst114 * fRec29[0]) + (fConst128 * fRec29[1])) + (fConst114 * fRec29[2])));
			fRec28[0] = (fTemp9 - (fConst110 * ((fConst129 * fRec28[2]) + (fConst131 * fRec28[1]))));
			fRec27[0] = ((fConst110 * (((fConst112 * fRec28[0]) + (fConst132 * fRec28[1])) + (fConst112 * fRec28[2]))) - (fConst107 * ((fConst133 * fRec27[2]) + (fConst134 * fRec27[1]))));
			fRec26[0] = ((fConst107 * (((fConst109 * fRec27[0]) + (fConst135 * fRec27[1])) + (fConst109 * fRec27[2]))) - (fConst103 * ((fConst136 * fRec26[2]) + (fConst137 * fRec26[1]))));
			fRec25[0] = ((fSlow1 * fRec25[1]) + (fSlow2 * fabsf((fConst103 * (((fConst106 * fRec26[0]) + (fConst138 * fRec26[1])) + (fConst106 * fRec26[2]))))));
			fbargraph3 = (fSlow0 + (20 * log10f(fRec25[0])));
			fRec38[0] = (fTemp9 - (fConst155 * ((fConst158 * fRec38[2]) + (fConst159 * fRec38[1]))));
			fRec37[0] = ((fConst155 * (((fConst157 * fRec38[0]) + (fConst160 * fRec38[1])) + (fConst157 * fRec38[2]))) - (fConst153 * ((fConst161 * fRec37[2]) + (fConst162 * fRec37[1]))));
			fRec36[0] = ((fConst153 * (((fConst154 * fRec37[0]) + (fConst163 * fRec37[1])) + (fConst154 * fRec37[2]))) - (fConst151 * ((fConst164 * fRec36[2]) + (fConst165 * fRec36[1]))));
			float fTemp10 = (fConst151 * (((fConst152 * fRec36[0]) + (fConst166 * fRec36[1])) + (fConst152 * fRec36[2])));
			fRec35[0] = (fTemp10 - (fConst148 * ((fConst167 * fRec35[2]) + (fConst169 * fRec35[1]))));
			fRec34[0] = ((fConst148 * (((fConst150 * fRec35[0]) + (fConst170 * fRec35[1])) + (fConst150 * fRec35[2]))) - (fConst145 * ((fConst171 * fRec34[2]) + (fConst172 * fRec34[1]))));
			fRec33[0] = ((fConst145 * (((fConst147 * fRec34[0]) + (fConst173 * fRec34[1])) + (fConst147 * fRec34[2]))) - (fConst141 * ((fConst174 * fRec33[2]) + (fConst175 * fRec33[1]))));
			fRec32[0] = ((fSlow1 * fRec32[1]) + (fSlow2 * fabsf((fConst141 * (((fConst144 * fRec33[0]) + (fConst176 * fRec33[1])) + (fConst144 * fRec33[2]))))));
			fbargraph4 = (fSlow0 + (20 * log10f(fRec32[0])));
			fRec45[0] = (fTemp10 - (fConst193 * ((fConst196 * fRec45[2]) + (fConst197 * fRec45[1]))));
			fRec44[0] = ((fConst193 * (((fConst195 * fRec45[0]) + (fConst198 * fRec45[1])) + (fConst195 * fRec45[2]))) - (fConst191 * ((fConst199 * fRec44[2]) + (fConst200 * fRec44[1]))));
			fRec43[0] = ((fConst191 * (((fConst192 * fRec44[0]) + (fConst201 * fRec44[1])) + (fConst192 * fRec44[2]))) - (fConst189 * ((fConst202 * fRec43[2]) + (fConst203 * fRec43[1]))));
			float fTemp11 = (fConst189 * (((fConst190 * fRec43[0]) + (fConst204 * fRec43[1])) + (fConst190 * fRec43[2])));
			fRec42[0] = (fTemp11 - (fConst186 * ((fConst205 * fRec42[2]) + (fConst207 * fRec42[1]))));
			fRec41[0] = ((fConst186 * (((fConst188 * fRec42[0]) + (fConst208 * fRec42[1])) + (fConst188 * fRec42[2]))) - (fConst183 * ((fConst209 * fRec41[2]) + (fConst210 * fRec41[1]))));
			fRec40[0] = ((fConst183 * (((fConst185 * fRec41[0]) + (fConst211 * fRec41[1])) + (fConst185 * fRec41[2]))) - (fConst179 * ((fConst212 * fRec40[2]) + (fConst213 * fRec40[1]))));
			fRec39[0] = ((fSlow1 * fRec39[1]) + (fSlow2 * fabsf((fConst179 * (((fConst182 * fRec40[0]) + (fConst214 * fRec40[1])) + (fConst182 * fRec40[2]))))));
			fbargraph5 = (fSlow0 + (20 * log10f(fRec39[0])));
			fRec52[0] = (fTemp11 - (fConst231 * ((fConst234 * fRec52[2]) + (fConst235 * fRec52[1]))));
			fRec51[0] = ((fConst231 * (((fConst233 * fRec52[0]) + (fConst236 * fRec52[1])) + (fConst233 * fRec52[2]))) - (fConst229 * ((fConst237 * fRec51[2]) + (fConst238 * fRec51[1]))));
			fRec50[0] = ((fConst229 * (((fConst230 * fRec51[0]) + (fConst239 * fRec51[1])) + (fConst230 * fRec51[2]))) - (fConst227 * ((fConst240 * fRec50[2]) + (fConst241 * fRec50[1]))));
			float fTemp12 = (fConst227 * (((fConst228 * fRec50[0]) + (fConst242 * fRec50[1])) + (fConst228 * fRec50[2])));
			fRec49[0] = (fTemp12 - (fConst224 * ((fConst243 * fRec49[2]) + (fConst245 * fRec49[1]))));
			fRec48[0] = ((fConst224 * (((fConst226 * fRec49[0]) + (fConst246 * fRec49[1])) + (fConst226 * fRec49[2]))) - (fConst221 * ((fConst247 * fRec48[2]) + (fConst248 * fRec48[1]))));
			fRec47[0] = ((fConst221 * (((fConst223 * fRec48[0]) + (fConst249 * fRec48[1])) + (fConst223 * fRec48[2]))) - (fConst217 * ((fConst250 * fRec47[2]) + (fConst251 * fRec47[1]))));
			fRec46[0] = ((fSlow1 * fRec46[1]) + (fSlow2 * fabsf((fConst217 * (((fConst220 * fRec47[0]) + (fConst252 * fRec47[1])) + (fConst220 * fRec47[2]))))));
			fbargraph6 = (fSlow0 + (20 * log10f(fRec46[0])));
			fRec59[0] = (fTemp12 - (fConst269 * ((fConst272 * fRec59[2]) + (fConst273 * fRec59[1]))));
			fRec58[0] = ((fConst269 * (((fConst271 * fRec59[0]) + (fConst274 * fRec59[1])) + (fConst271 * fRec59[2]))) - (fConst267 * ((fConst275 * fRec58[2]) + (fConst276 * fRec58[1]))));
			fRec57[0] = ((fConst267 * (((fConst268 * fRec58[0]) + (fConst277 * fRec58[1])) + (fConst268 * fRec58[2]))) - (fConst265 * ((fConst278 * fRec57[2]) + (fConst279 * fRec57[1]))));
			float fTemp13 = (fConst265 * (((fConst266 * fRec57[0]) + (fConst280 * fRec57[1])) + (fConst266 * fRec57[2])));
			fRec56[0] = (fTemp13 - (fConst262 * ((fConst281 * fRec56[2]) + (fConst283 * fRec56[1]))));
			fRec55[0] = ((fConst262 * (((fConst264 * fRec56[0]) + (fConst284 * fRec56[1])) + (fConst264 * fRec56[2]))) - (fConst259 * ((fConst285 * fRec55[2]) + (fConst286 * fRec55[1]))));
			fRec54[0] = ((fConst259 * (((fConst261 * fRec55[0]) + (fConst287 * fRec55[1])) + (fConst261 * fRec55[2]))) - (fConst255 * ((fConst288 * fRec54[2]) + (fConst289 * fRec54[1]))));
			fRec53[0] = ((fSlow1 * fRec53[1]) + (fSlow2 * fabsf((fConst255 * (((fConst258 * fRec54[0]) + (fConst290 * fRec54[1])) + (fConst258 * fRec54[2]))))));
			fbargraph7 = (fSlow0 + (20 * log10f(fRec53[0])));
			fRec66[0] = (fTemp13 - (fConst307 * ((fConst310 * fRec66[2]) + (fConst311 * fRec66[1]))));
			fRec65[0] = ((fConst307 * (((fConst309 * fRec66[0]) + (fConst312 * fRec66[1])) + (fConst309 * fRec66[2]))) - (fConst305 * ((fConst313 * fRec65[2]) + (fConst314 * fRec65[1]))));
			fRec64[0] = ((fConst305 * (((fConst306 * fRec65[0]) + (fConst315 * fRec65[1])) + (fConst306 * fRec65[2]))) - (fConst303 * ((fConst316 * fRec64[2]) + (fConst317 * fRec64[1]))));
			float fTemp14 = (fConst303 * (((fConst304 * fRec64[0]) + (fConst318 * fRec64[1])) + (fConst304 * fRec64[2])));
			fRec63[0] = (fTemp14 - (fConst300 * ((fConst319 * fRec63[2]) + (fConst321 * fRec63[1]))));
			fRec62[0] = ((fConst300 * (((fConst302 * fRec63[0]) + (fConst322 * fRec63[1])) + (fConst302 * fRec63[2]))) - (fConst297 * ((fConst323 * fRec62[2]) + (fConst324 * fRec62[1]))));
			fRec61[0] = ((fConst297 * (((fConst299 * fRec62[0]) + (fConst325 * fRec62[1])) + (fConst299 * fRec62[2]))) - (fConst293 * ((fConst326 * fRec61[2]) + (fConst327 * fRec61[1]))));
			fRec60[0] = ((fSlow1 * fRec60[1]) + (fSlow2 * fabsf((fConst293 * (((fConst296 * fRec61[0]) + (fConst328 * fRec61[1])) + (fConst296 * fRec61[2]))))));
			fbargraph8 = (fSlow0 + (20 * log10f(fRec60[0])));
			fRec73[0] = (fTemp14 - (fConst345 * ((fConst348 * fRec73[2]) + (fConst349 * fRec73[1]))));
			fRec72[0] = ((fConst345 * (((fConst347 * fRec73[0]) + (fConst350 * fRec73[1])) + (fConst347 * fRec73[2]))) - (fConst343 * ((fConst351 * fRec72[2]) + (fConst352 * fRec72[1]))));
			fRec71[0] = ((fConst343 * (((fConst344 * fRec72[0]) + (fConst353 * fRec72[1])) + (fConst344 * fRec72[2]))) - (fConst341 * ((fConst354 * fRec71[2]) + (fConst355 * fRec71[1]))));
			float fTemp15 = (fConst341 * (((fConst342 * fRec71[0]) + (fConst356 * fRec71[1])) + (fConst342 * fRec71[2])));
			fRec70[0] = (fTemp15 - (fConst338 * ((fConst357 * fRec70[2]) + (fConst359 * fRec70[1]))));
			fRec69[0] = ((fConst338 * (((fConst340 * fRec70[0]) + (fConst360 * fRec70[1])) + (fConst340 * fRec70[2]))) - (fConst335 * ((fConst361 * fRec69[2]) + (fConst362 * fRec69[1]))));
			fRec68[0] = ((fConst335 * (((fConst337 * fRec69[0]) + (fConst363 * fRec69[1])) + (fConst337 * fRec69[2]))) - (fConst331 * ((fConst364 * fRec68[2]) + (fConst365 * fRec68[1]))));
			fRec67[0] = ((fSlow1 * fRec67[1]) + (fSlow2 * fabsf((fConst331 * (((fConst334 * fRec68[0]) + (fConst366 * fRec68[1])) + (fConst334 * fRec68[2]))))));
			fbargraph9 = (fSlow0 + (20 * log10f(fRec67[0])));
			fRec80[0] = (fTemp15 - (fConst383 * ((fConst386 * fRec80[2]) + (fConst387 * fRec80[1]))));
			fRec79[0] = ((fConst383 * (((fConst385 * fRec80[0]) + (fConst388 * fRec80[1])) + (fConst385 * fRec80[2]))) - (fConst381 * ((fConst389 * fRec79[2]) + (fConst390 * fRec79[1]))));
			fRec78[0] = ((fConst381 * (((fConst382 * fRec79[0]) + (fConst391 * fRec79[1])) + (fConst382 * fRec79[2]))) - (fConst379 * ((fConst392 * fRec78[2]) + (fConst393 * fRec78[1]))));
			float fTemp16 = (fConst379 * (((fConst380 * fRec78[0]) + (fConst394 * fRec78[1])) + (fConst380 * fRec78[2])));
			fRec77[0] = (fTemp16 - (fConst376 * ((fConst395 * fRec77[2]) + (fConst397 * fRec77[1]))));
			fRec76[0] = ((fConst376 * (((fConst378 * fRec77[0]) + (fConst398 * fRec77[1])) + (fConst378 * fRec77[2]))) - (fConst373 * ((fConst399 * fRec76[2]) + (fConst400 * fRec76[1]))));
			fRec75[0] = ((fConst373 * (((fConst375 * fRec76[0]) + (fConst401 * fRec76[1])) + (fConst375 * fRec76[2]))) - (fConst369 * ((fConst402 * fRec75[2]) + (fConst403 * fRec75[1]))));
			fRec74[0] = ((fSlow1 * fRec74[1]) + (fSlow2 * fabsf((fConst369 * (((fConst372 * fRec75[0]) + (fConst404 * fRec75[1])) + (fConst372 * fRec75[2]))))));
			fbargraph10 = (fSlow0 + (20 * log10f(fRec74[0])));
			fRec87[0] = (fTemp16 - (fConst421 * ((fConst424 * fRec87[2]) + (fConst425 * fRec87[1]))));
			fRec86[0] = ((fConst421 * (((fConst423 * fRec87[0]) + (fConst426 * fRec87[1])) + (fConst423 * fRec87[2]))) - (fConst419 * ((fConst427 * fRec86[2]) + (fConst428 * fRec86[1]))));
			fRec85[0] = ((fConst419 * (((fConst420 * fRec86[0]) + (fConst429 * fRec86[1])) + (fConst420 * fRec86[2]))) - (fConst417 * ((fConst430 * fRec85[2]) + (fConst431 * fRec85[1]))));
			float fTemp17 = (fConst417 * (((fConst418 * fRec85[0]) + (fConst432 * fRec85[1])) + (fConst418 * fRec85[2])));
			fRec84[0] = (fTemp17 - (fConst414 * ((fConst433 * fRec84[2]) + (fConst435 * fRec84[1]))));
			fRec83[0] = ((fConst414 * (((fConst416 * fRec84[0]) + (fConst436 * fRec84[1])) + (fConst416 * fRec84[2]))) - (fConst411 * ((fConst437 * fRec83[2]) + (fConst438 * fRec83[1]))));
			fRec82[0] = ((fConst411 * (((fConst413 * fRec83[0]) + (fConst439 * fRec83[1])) + (fConst413 * fRec83[2]))) - (fConst407 * ((fConst440 * fRec82[2]) + (fConst441 * fRec82[1]))));
			fRec81[0] = ((fSlow1 * fRec81[1]) + (fSlow2 * fabsf((fConst407 * (((fConst410 * fRec82[0]) + (fConst442 * fRec82[1])) + (fConst410 * fRec82[2]))))));
			fbargraph11 = (fSlow0 + (20 * log10f(fRec81[0])));
			fRec94[0] = (fTemp17 - (fConst459 * ((fConst462 * fRec94[2]) + (fConst463 * fRec94[1]))));
			fRec93[0] = ((fConst459 * (((fConst461 * fRec94[0]) + (fConst464 * fRec94[1])) + (fConst461 * fRec94[2]))) - (fConst457 * ((fConst465 * fRec93[2]) + (fConst466 * fRec93[1]))));
			fRec92[0] = ((fConst457 * (((fConst458 * fRec93[0]) + (fConst467 * fRec93[1])) + (fConst458 * fRec93[2]))) - (fConst455 * ((fConst468 * fRec92[2]) + (fConst469 * fRec92[1]))));
			float fTemp18 = (fConst455 * (((fConst456 * fRec92[0]) + (fConst470 * fRec92[1])) + (fConst456 * fRec92[2])));
			fRec91[0] = (fTemp18 - (fConst452 * ((fConst471 * fRec91[2]) + (fConst473 * fRec91[1]))));
			fRec90[0] = ((fConst452 * (((fConst454 * fRec91[0]) + (fConst474 * fRec91[1])) + (fConst454 * fRec91[2]))) - (fConst449 * ((fConst475 * fRec90[2]) + (fConst476 * fRec90[1]))));
			fRec89[0] = ((fConst449 * (((fConst451 * fRec90[0]) + (fConst477 * fRec90[1])) + (fConst451 * fRec90[2]))) - (fConst445 * ((fConst478 * fRec89[2]) + (fConst479 * fRec89[1]))));
			fRec88[0] = ((fSlow1 * fRec88[1]) + (fSlow2 * fabsf((fConst445 * (((fConst448 * fRec89[0]) + (fConst480 * fRec89[1])) + (fConst448 * fRec89[2]))))));
			fbargraph12 = (fSlow0 + (20 * log10f(fRec88[0])));
			fRec101[0] = (fTemp18 - (fConst497 * ((fConst500 * fRec101[2]) + (fConst501 * fRec101[1]))));
			fRec100[0] = ((fConst497 * (((fConst499 * fRec101[0]) + (fConst502 * fRec101[1])) + (fConst499 * fRec101[2]))) - (fConst495 * ((fConst503 * fRec100[2]) + (fConst504 * fRec100[1]))));
			fRec99[0] = ((fConst495 * (((fConst496 * fRec100[0]) + (fConst505 * fRec100[1])) + (fConst496 * fRec100[2]))) - (fConst493 * ((fConst506 * fRec99[2]) + (fConst507 * fRec99[1]))));
			float fTemp19 = (fConst493 * (((fConst494 * fRec99[0]) + (fConst508 * fRec99[1])) + (fConst494 * fRec99[2])));
			fRec98[0] = (fTemp19 - (fConst490 * ((fConst509 * fRec98[2]) + (fConst511 * fRec98[1]))));
			fRec97[0] = ((fConst490 * (((fConst492 * fRec98[0]) + (fConst512 * fRec98[1])) + (fConst492 * fRec98[2]))) - (fConst487 * ((fConst513 * fRec97[2]) + (fConst514 * fRec97[1]))));
			fRec96[0] = ((fConst487 * (((fConst489 * fRec97[0]) + (fConst515 * fRec97[1])) + (fConst489 * fRec97[2]))) - (fConst483 * ((fConst516 * fRec96[2]) + (fConst517 * fRec96[1]))));
			fRec95[0] = ((fSlow1 * fRec95[1]) + (fSlow2 * fabsf((fConst483 * (((fConst486 * fRec96[0]) + (fConst518 * fRec96[1])) + (fConst486 * fRec96[2]))))));
			fbargraph13 = (fSlow0 + (20 * log10f(fRec95[0])));
			fRec105[0] = (fTemp19 - (fConst523 * ((fConst526 * fRec105[2]) + (fConst527 * fRec105[1]))));
			fRec104[0] = ((fConst523 * (((fConst525 * fRec105[0]) + (fConst528 * fRec105[1])) + (fConst525 * fRec105[2]))) - (fConst521 * ((fConst529 * fRec104[2]) + (fConst530 * fRec104[1]))));
			fRec103[0] = ((fConst521 * (((fConst522 * fRec104[0]) + (fConst531 * fRec104[1])) + (fConst522 * fRec104[2]))) - (fConst519 * ((fConst532 * fRec103[2]) + (fConst533 * fRec103[1]))));
			fRec102[0] = ((fSlow1 * fRec102[1]) + (fSlow2 * fabsf((fConst519 * (((fConst520 * fRec103[0]) + (fConst534 * fRec103[1])) + (fConst520 * fRec103[2]))))));
			fbargraph14 = (fSlow0 + (20 * log10f(fRec102[0])));
			float fTemp20 = fTemp6;
			output0[i] = (FAUSTFLOAT)fTemp20;
			output1[i] = (FAUSTFLOAT)fTemp20;
			// post processing
			fRec102[1] = fRec102[0];
			fRec103[2] = fRec103[1]; fRec103[1] = fRec103[0];
			fRec104[2] = fRec104[1]; fRec104[1] = fRec104[0];
			fRec105[2] = fRec105[1]; fRec105[1] = fRec105[0];
			fRec95[1] = fRec95[0];
			fRec96[2] = fRec96[1]; fRec96[1] = fRec96[0];
			fRec97[2] = fRec97[1]; fRec97[1] = fRec97[0];
			fRec98[2] = fRec98[1]; fRec98[1] = fRec98[0];
			fRec99[2] = fRec99[1]; fRec99[1] = fRec99[0];
			fRec100[2] = fRec100[1]; fRec100[1] = fRec100[0];
			fRec101[2] = fRec101[1]; fRec101[1] = fRec101[0];
			fRec88[1] = fRec88[0];
			fRec89[2] = fRec89[1]; fRec89[1] = fRec89[0];
			fRec90[2] = fRec90[1]; fRec90[1] = fRec90[0];
			fRec91[2] = fRec91[1]; fRec91[1] = fRec91[0];
			fRec92[2] = fRec92[1]; fRec92[1] = fRec92[0];
			fRec93[2] = fRec93[1]; fRec93[1] = fRec93[0];
			fRec94[2] = fRec94[1]; fRec94[1] = fRec94[0];
			fRec81[1] = fRec81[0];
			fRec82[2] = fRec82[1]; fRec82[1] = fRec82[0];
			fRec83[2] = fRec83[1]; fRec83[1] = fRec83[0];
			fRec84[2] = fRec84[1]; fRec84[1] = fRec84[0];
			fRec85[2] = fRec85[1]; fRec85[1] = fRec85[0];
			fRec86[2] = fRec86[1]; fRec86[1] = fRec86[0];
			fRec87[2] = fRec87[1]; fRec87[1] = fRec87[0];
			fRec74[1] = fRec74[0];
			fRec75[2] = fRec75[1]; fRec75[1] = fRec75[0];
			fRec76[2] = fRec76[1]; fRec76[1] = fRec76[0];
			fRec77[2] = fRec77[1]; fRec77[1] = fRec77[0];
			fRec78[2] = fRec78[1]; fRec78[1] = fRec78[0];
			fRec79[2] = fRec79[1]; fRec79[1] = fRec79[0];
			fRec80[2] = fRec80[1]; fRec80[1] = fRec80[0];
			fRec67[1] = fRec67[0];
			fRec68[2] = fRec68[1]; fRec68[1] = fRec68[0];
			fRec69[2] = fRec69[1]; fRec69[1] = fRec69[0];
			fRec70[2] = fRec70[1]; fRec70[1] = fRec70[0];
			fRec71[2] = fRec71[1]; fRec71[1] = fRec71[0];
			fRec72[2] = fRec72[1]; fRec72[1] = fRec72[0];
			fRec73[2] = fRec73[1]; fRec73[1] = fRec73[0];
			fRec60[1] = fRec60[0];
			fRec61[2] = fRec61[1]; fRec61[1] = fRec61[0];
			fRec62[2] = fRec62[1]; fRec62[1] = fRec62[0];
			fRec63[2] = fRec63[1]; fRec63[1] = fRec63[0];
			fRec64[2] = fRec64[1]; fRec64[1] = fRec64[0];
			fRec65[2] = fRec65[1]; fRec65[1] = fRec65[0];
			fRec66[2] = fRec66[1]; fRec66[1] = fRec66[0];
			fRec53[1] = fRec53[0];
			fRec54[2] = fRec54[1]; fRec54[1] = fRec54[0];
			fRec55[2] = fRec55[1]; fRec55[1] = fRec55[0];
			fRec56[2] = fRec56[1]; fRec56[1] = fRec56[0];
			fRec57[2] = fRec57[1]; fRec57[1] = fRec57[0];
			fRec58[2] = fRec58[1]; fRec58[1] = fRec58[0];
			fRec59[2] = fRec59[1]; fRec59[1] = fRec59[0];
			fRec46[1] = fRec46[0];
			fRec47[2] = fRec47[1]; fRec47[1] = fRec47[0];
			fRec48[2] = fRec48[1]; fRec48[1] = fRec48[0];
			fRec49[2] = fRec49[1]; fRec49[1] = fRec49[0];
			fRec50[2] = fRec50[1]; fRec50[1] = fRec50[0];
			fRec51[2] = fRec51[1]; fRec51[1] = fRec51[0];
			fRec52[2] = fRec52[1]; fRec52[1] = fRec52[0];
			fRec39[1] = fRec39[0];
			fRec40[2] = fRec40[1]; fRec40[1] = fRec40[0];
			fRec41[2] = fRec41[1]; fRec41[1] = fRec41[0];
			fRec42[2] = fRec42[1]; fRec42[1] = fRec42[0];
			fRec43[2] = fRec43[1]; fRec43[1] = fRec43[0];
			fRec44[2] = fRec44[1]; fRec44[1] = fRec44[0];
			fRec45[2] = fRec45[1]; fRec45[1] = fRec45[0];
			fRec32[1] = fRec32[0];
			fRec33[2] = fRec33[1]; fRec33[1] = fRec33[0];
			fRec34[2] = fRec34[1]; fRec34[1] = fRec34[0];
			fRec35[2] = fRec35[1]; fRec35[1] = fRec35[0];
			fRec36[2] = fRec36[1]; fRec36[1] = fRec36[0];
			fRec37[2] = fRec37[1]; fRec37[1] = fRec37[0];
			fRec38[2] = fRec38[1]; fRec38[1] = fRec38[0];
			fRec25[1] = fRec25[0];
			fRec26[2] = fRec26[1]; fRec26[1] = fRec26[0];
			fRec27[2] = fRec27[1]; fRec27[1] = fRec27[0];
			fRec28[2] = fRec28[1]; fRec28[1] = fRec28[0];
			fRec29[2] = fRec29[1]; fRec29[1] = fRec29[0];
			fRec30[2] = fRec30[1]; fRec30[1] = fRec30[0];
			fRec31[2] = fRec31[1]; fRec31[1] = fRec31[0];
			fRec18[1] = fRec18[0];
			fRec19[2] = fRec19[1]; fRec19[1] = fRec19[0];
			fRec20[2] = fRec20[1]; fRec20[1] = fRec20[0];
			fRec21[2] = fRec21[1]; fRec21[1] = fRec21[0];
			fRec22[2] = fRec22[1]; fRec22[1] = fRec22[0];
			fRec23[2] = fRec23[1]; fRec23[1] = fRec23[0];
			fRec24[2] = fRec24[1]; fRec24[1] = fRec24[0];
			fRec11[1] = fRec11[0];
			fRec12[2] = fRec12[1]; fRec12[1] = fRec12[0];
			fRec13[2] = fRec13[1]; fRec13[1] = fRec13[0];
			fRec14[2] = fRec14[1]; fRec14[1] = fRec14[0];
			fRec15[2] = fRec15[1]; fRec15[1] = fRec15[0];
			fRec16[2] = fRec16[1]; fRec16[1] = fRec16[0];
			fRec17[2] = fRec17[1]; fRec17[1] = fRec17[0];
			fRec0[1] = fRec0[0];
			fRec1[2] = fRec1[1]; fRec1[1] = fRec1[0];
			fRec2[2] = fRec2[1]; fRec2[1] = fRec2[0];
			fRec3[2] = fRec3[1]; fRec3[1] = fRec3[0];
			fRec8[1] = fRec8[0];
			fVec1[1] = fVec1[0];
			fRec10[1] = fRec10[0];
			fRec9[1] = fRec9[0];
			fRec6[1] = fRec6[0];
			fRec5[1] = fRec5[0];
			fRec7[1] = fRec7[0];
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
