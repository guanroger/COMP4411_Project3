// The main ray tracer.

#include <Fl/fl_ask.h>
#include <deque>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include "ui/TraceUI.h"

extern TraceUI *traceUI;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );
	const float threshold	= traceUI->getTreshold();
	const vec3f thresh(threshold, threshold, threshold);
	const int	depth		= traceUI->getDepth();
	return traceRay( scene, r, thresh, depth ).clamp();
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, ray& r, 
	const vec3f& thresh, int depth )
{

	// Recursion end condition
	if (depth < 0) return vec3f(0.0f, 0.0f, 0.0f);


	isect i;

	if( scene->intersect( r, i ) ) {
		// YOUR CODE HERE

		// An intersection occurred!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

		const Material& m	= i.getMaterial();
		vec3f intensity		= m.shade(scene, r, i);

		// ======================== Handle reflection =======================================
		
		// Get the point of intersection
		// Note that the point is shifted a little bit to prevent self intersection
		const vec3f point	= r.at(i.t) + i.N.normalize() * NORMAL_EPSILON;
		
		// Get the direction of reflection
		const vec3f L			= r.getDirection().normalize();
		const vec3f N			= i.N.normalize();
		const vec3f reflect_dir = this->getReflectedDir(-L, N);

		// Get the reflection intensity
		ray reflected_ray(point, reflect_dir);
		vec3f reflect_i = this->traceRay(scene, reflected_ray, thresh, depth-1);

		// Get the index of refraction
		// We also need to consider the entering material
		const bool entering = this->isEntering(L, N);
		double n_i = 0.0f, n_t = 0.0f;
		if (entering) {
			n_i = r.getPrevMaterialIndex();
			n_t = m.index;
		} else {
			n_i = m.index;
			n_t = 1;
		}

		// ======================== Handle refraction =======================================

		vec3f refraction_i(0.0f, 0.0f, 0.0f);
		if (!this->TIR(-L, N, n_i, n_t)) {
			// Total Internal Reflection doesn't occur
			const vec3f refract_dir = this->getRefrationDir(-L, entering ? N : -N, n_i, n_t);
			const vec3f point		= r.at(i.t) - N * NORMAL_EPSILON * (entering ? 1 : -1);
			ray refract_ray(point, refract_dir);
			refract_ray.setPrevMaterial(&m);
			refraction_i = this->traceRay(scene, refract_ray, thresh, depth - 1);
		}

		return intensity + prod(m.kr, reflect_i) + prod(m.kt, refraction_i);
	
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.
		return vec3f( 0.0, 0.0, 0.0 );
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}


RayTracer::~RayTracer()
{
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

// Get the direction of reflection
// Apply the reflection formula
vec3f RayTracer::getReflectedDir(const vec3f &L, const vec3f &N) const {
	return (2.0 * N.dot(L) * N - L).normalize();
}

// Get the direction of refraction
// Apply the vector form of Snell's law
vec3f RayTracer::getRefrationDir(const vec3f &L, const vec3f &N, const double &n_i, const double &n_t) const {
	const double mu = n_i / n_t;
	const double NL = N.dot(L);

	const double root = 1 - mu * mu * (1 - NL * NL);
	if (root < 0.0f) return vec3f();

	const double coeff = mu * NL - sqrt(root);

	return (coeff * N - mu * L).normalize();
}

// Note that if the angle between L and N is greater than 90 degree, 
// then the inner product will return a negative value
bool RayTracer::isEntering(const vec3f &L, const vec3f &N) const {
	return L.dot(N) < 0;
}

// Total Internal Reflection occur when incident angle is greater than the critical angle
// We modify the formula n_i * sin(theta_in) = n_t * sin(theta_refrac) here
bool RayTracer::TIR(const vec3f &L, const vec3f &N, const double &n_i, const double &n_t) const {
	const double in_angle = L.dot(N);
	return pow(in_angle, 2) <= 1 - pow(n_t / n_t, 2);
}

bool RayTracer::loadScene( char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( ParseError pe )
	{
		fl_alert( "ParseError: %s\n", pe );
		return false;
	}

	if( !scene )
		return false;
	
	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];
	
	// separate objects into bounded and unbounded
	scene->initScene();
	
	// Add any specialized scene loading code here
	
	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;

	if( !scene )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	col = trace( scene,x,y );

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}