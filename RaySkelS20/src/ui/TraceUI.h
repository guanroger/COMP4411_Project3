//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __rayUI_h__
#define __rayUI_h__

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>

#include <FL/fl_file_chooser.H>		// FLTK file chooser

#include "TraceGLWindow.h"

class TraceUI {
public:
	TraceUI();

	// The FLTK widgets
	Fl_Window*			m_mainWindow;
	Fl_Menu_Bar*		m_menubar;

	Fl_Slider*			m_sizeSlider;
	Fl_Slider*			m_depthSlider;

	//Added

	Fl_Slider*			m_auteunationConstantSlider;
	Fl_Slider*			m_auteunationLinearSlider;
	Fl_Slider*			m_auteunationQuadricSlider;
	Fl_Slider*			m_ambientLightSlider;
	Fl_Slider*			m_thresholdSlider;

	Fl_Button*			m_renderButton;
	Fl_Button*			m_stopButton;

	//Added 
	Fl_Check_Button*	m_adaptiveCheckButton;
	Fl_Check_Button* m_jitteringCheckButton;

	TraceGLWindow*		m_traceGlWindow;

	// member functions
	void show();

	void		setRayTracer(RayTracer *tracer);

	int			getSize();
	int			getDepth();

	bool		Jittering() { return m_nJittering; }
	bool		Adaptive() { return m_nAdaptive; }
private:
	RayTracer*	raytracer;

	int			m_nSize;
	int			m_nDepth;
	float m_nAtteunConstant;
	float m_nAtteunLinear;
	float m_nAtteunQuadric;
	float m_nAmbientLight;
	float m_nThreshold;
	bool m_nJittering = false;
	bool m_nAdaptive = false;
// static class members
	static Fl_Menu_Item menuitems[];

	static TraceUI* whoami(Fl_Menu_* o);

	static void cb_load_scene(Fl_Menu_* o, void* v);
	static void cb_save_image(Fl_Menu_* o, void* v);
	static void cb_exit(Fl_Menu_* o, void* v);
	static void cb_about(Fl_Menu_* o, void* v);

	static void cb_exit2(Fl_Widget* o, void* v);

	static void cb_sizeSlides(Fl_Widget* o, void* v);
	static void cb_depthSlides(Fl_Widget* o, void* v);
	static void cb_atteunConstantSlides(Fl_Widget* o, void* v);
	static void cb_atteunLinearSlides(Fl_Widget* o, void* v);
	static void cb_atteunQuadricSlides(Fl_Widget* o, void* v);
	static void cb_ambientLightSlides(Fl_Widget* o, void* v);
	static void cb_thresholdSlides(Fl_Widget* o, void* v);

	static void cd_jitteringLightButton(Fl_Widget* o, void* v);
	static void cd_AdaptiveLightButton(Fl_Widget* o, void* v);


	static void cb_render(Fl_Widget* o, void* v);
	static void cb_stop(Fl_Widget* o, void* v);
};

#endif
