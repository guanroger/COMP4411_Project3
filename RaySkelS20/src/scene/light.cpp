#include <cmath>

#include "light.h"
#include "../ui/TraceUI.h"

#define PI 3.14159265

extern TraceUI *traceUI;

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
	vec3f result(1.0f, 1.0f, 1.0f);
	const vec3f dir = this->getDirection(P);	// Direction from intersection to light source
	vec3f p = P + dir * RAY_EPSILON;	// Offset the point a little bit to prevent intersection with itself

	// Check will the light ray hit the intersection point
	while (result[0] > NORMAL_EPSILON && result[1] > NORMAL_EPSILON && !result[2] > NORMAL_EPSILON) {
		isect i;
		ray shadow_ray(p, dir);

		if (!this->scene->intersect(shadow_ray, i)) {
			// No blocking object in between
			return result;
		}

		if (i.getMaterial().kt.iszero()) {
			// Totally non transparent object
			return vec3f(0.0f, 0.0f, 0.0f);
		}

		result = prod(result, i.getMaterial().kt);

		// Prevent hitting itself
		p = shadow_ray.at(i.t) + dir * RAY_EPSILON;

		cout << result << endl;
	}
	return result;
}

vec3f DirectionalLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f DirectionalLight::getDirection( const vec3f& P ) const
{
	return -orientation;
}

double PointLight::distanceAttenuation( const vec3f& P ) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, I assume no attenuation and just return 1.0
	
	const double a = this->const_coeff;
	const double b = this->linear_coeff;
	const double c = this->quad_coeff;

	// Get the distance between intersection point and the light source
	const double dis = P.distanceTo(position);

	double divisor		= a + b * dis + c * dis * dis;
	if (divisor == 0) return 1;
	double dis_atten	= 1 / divisor;

	return dis_atten < 1 ? dis_atten : 1;
}

vec3f PointLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f PointLight::getDirection( const vec3f& P ) const
{
	return (position - P).normalize();
}

PointLight::PointLight(Scene *scene, const vec3f &pos, const vec3f &color) 
: Light(scene, color), position(pos), const_coeff(0), linear_coeff(0), quad_coeff(0)
{

}

PointLight::PointLight(Scene *scene, const vec3f &pos, const vec3f &color, const double &const_coeff, const double &linear_coeff, const double &quad_coeff)
: Light(scene, color), position(pos), const_coeff(const_coeff), linear_coeff(linear_coeff), quad_coeff(quad_coeff) {}

vec3f PointLight::shadowAttenuation(const vec3f& P) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.

	vec3f result(1.0f, 1.0f, 1.0f);

	const vec3f dir = this->getDirection(P);	// Direction from intersection to light source
	vec3f p			= P + dir * RAY_EPSILON;	// Offset the point a little bit to prevent intersection with itself

	// Check will the light ray hit the intersection point
	while (result[0] > NORMAL_EPSILON || result[1] > NORMAL_EPSILON || result[2] > NORMAL_EPSILON) {
		isect i;
		ray shadow_ray(p, dir);

		const double t = (position - p).length();
		if (!this->scene->intersect(shadow_ray, i) || i.t >= t) {
			// No blocking object in between OR the intersection point is behind the light
			return result;
		}

		result = prod(result, i.getMaterial().kt);

		// Prevent hitting itself
		p = shadow_ray.at(i.t) + dir * RAY_EPSILON;
	}

	return result;
}


// For ambient light, shadow attenuation is not important.
// Therefore, we just return a default vector
vec3f AmbientLight::shadowAttenuation(const vec3f &P) const {
	return vec3f();
}

// For ambient light, distance attenuation is not important.
// Therefore, we just return a default distance
double AmbientLight::distanceAttenuation(const vec3f &P) const {
	return 0.0;
}

// Return the color of the ambient light
vec3f AmbientLight::getColor(const vec3f &P) const {
	return this->color;
}

// For ambient light, direction is not important.
// Therefore, we just return a default direction
vec3f AmbientLight::getDirection(const vec3f &P) const {
	return vec3f();
}

SpotLight::SpotLight(Scene *scene, const vec3f &color, const vec3f &dir, const vec3f &pos, const vec3f &edge)
	: Light(scene, color), direction(dir), position(pos), coneAngle(edge[0]), const_coeff(0), linear_coeff(0), quad_coeff(0) {}

SpotLight::SpotLight(Scene *scene, const vec3f &color, const vec3f &dir, const vec3f &pos, const vec3f &edge, const double &const_coeff, const double &linear_coeff, const double &quad_coeff)
: Light(scene, color), direction(dir), position(pos), coneAngle(edge[0]), const_coeff(const_coeff), linear_coeff(linear_coeff), quad_coeff(quad_coeff) {}

vec3f SpotLight::shadowAttenuation(const vec3f &P) const {
	vec3f result(1.0f, 1.0f, 1.0f);

	const vec3f dir = this->getDirection(P);	// Direction from intersection to light source
	vec3f p = P + dir * RAY_EPSILON;	// Offset the point a little bit to prevent intersection with itself

	// Check will the light ray hit the intersection point
	while (result[0] >= NORMAL_EPSILON && result[1] >= NORMAL_EPSILON && result[2] >= NORMAL_EPSILON) {
		isect i;
		ray shadow_ray(p, dir);

		const double t = (position - p).length();
		if (!this->scene->intersect(shadow_ray, i) || i.t >= t) {
			// No blocking object in between OR the intersection point is behind the light
			return result;
		}

		if (i.getMaterial().kt.iszero()) {
			// Totally non-transparent object
			return vec3f(0.0f, 0.0f, 0.0f);
		}

		result = prod(result, i.getMaterial().kt);

		// Prevent hitting itself
		p = shadow_ray.at(i.t) + dir * RAY_EPSILON;
	}

	return result;
}

double SpotLight::distanceAttenuation(const vec3f &P) const {
	const double a = this->const_coeff;
	const double b = this->linear_coeff;
	const double c = this->quad_coeff;

	// Get the distance between intersection point and the light source
	const double dis = P.distanceTo(position);

	double divisor = a + b * dis + c * dis * dis;
	if (divisor == 0) return 1;
	double dis_atten = 1 / divisor;

	return dis_atten < 1 ? dis_atten : 1;
}

vec3f SpotLight::getColor(const vec3f &P) const {
	return this->color;
}

vec3f SpotLight::getDirection(const vec3f &P) const {
	return (position - P).normalize();
}

double SpotLight::getConeAngle() const {
	return this->coneAngle;
}

WarnLight::WarnLight(Scene *scene, const vec3f &pos, const vec3f &dir, const vec3f &color, const vec3f &type)
: PointLight(scene, pos, color, 0, 0, 0), direction(dir), type(static_cast<Type>(static_cast<int>(type[0]))) 
{
	this->setUpMatrix(dir, pos);
	this->size = 0.5;
}

WarnLight::WarnLight(Scene *scene, const vec3f &pos, const vec3f &dir, const vec3f &color, const vec3f &type, const double &const_coeff, const double &linear_coeff, const double &quad_coeff)
: PointLight(scene, pos, color, const_coeff, linear_coeff, quad_coeff), direction(dir), type(static_cast<Type>(static_cast<int>(type[0]))) 
{
	this->setUpMatrix(dir, pos);
	this->size = 0.5;
}

double WarnLight::distanceAttenuation(const vec3f &p) const {
	vec4f proj = this->matrix * vec3f({p[0], p[1], p[2], 1});
	const double x = proj[0];
	const double y = proj[1];

	bool show = false;

	// Determine the light exceed the area of shape or not
	double m = 0.0;
	switch (this->type) {
		case Type::kCircle:
			cout << "circle" << endl;
			show = x * x + y * y < this->size *this->size; 
			break;
		case Type::kSquare:
			show = x > -this->size && x < this->size && y > -this->size && y < this->size;
			break;
		case Type::kTriangle:
			m = tan(PI / 3);
			show = y > -this->size / 2 && y < m *x + this->size && y < -m * x + this->size;
			break;
		case Type::kStar:
			m = tan(PI / 3);
			show = y > -size / 2 && y < m *x + this->size && y < -m * x + this->size;
			show = show || (y < size / 2 && y > m * x - this->size && y > -m * x - this->size);
			break;
		default:
			show = false;
			break;
	}

	return show ? PointLight::distanceAttenuation(p) : 0;
}

void WarnLight::setUpMatrix(const vec3f &dir, const vec3f& pos) {
	u = dir.cross({0,1,0}).normalize();
	v = dir.cross(u).normalize();
	u = dir.cross(v).normalize();
	mat4f translate({1,0,0,-pos[0]}, {0,1,0,-pos[1]}, {0,0,1,-pos[2]}, {0,0,0,0});
	mat4f rotate(vec3f({u[0],u[1],u[2],0}), vec3f({v[0],v[1],v[2],0}), vec3f({dir[0],dir[1],dir[2],0}), vec3f({0,0,0,1}));
	mat4f project({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,1,0});
	this->matrix = project * rotate * translate;
}
