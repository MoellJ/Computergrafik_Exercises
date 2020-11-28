#include <cglib/rt/renderer.h>
#include <cglib/rt/intersection_tests.h>
#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/scene.h>
#include <cglib/rt/light.h>
#include <cglib/rt/material.h>
#include <cglib/rt/render_data.h>

/*
 * DONE: implement a ray-sphere intersection test here.
 * The sphere is defined by its center and radius.
 *
 * Return true, if (and only if) the ray intersects the sphere.
 * In this case, also fill the parameter t with the distance such that
 *    ray_origin + t * ray_direction
 * is the intersection point.
 */
bool intersect_sphere(
    glm::vec3 const& ray_origin,    // starting point of the ray
    glm::vec3 const& ray_direction, // direction of the ray
    glm::vec3 const& center,        // position of the sphere
    float radius,                   // radius of the sphere
    float* t)                       // output parameter which contains distance to the hit point
{
    cg_assert(t);
    cg_assert(std::fabs(glm::length(ray_direction) - 1.f) < EPSILON);

    glm::vec3 oc = ray_origin - center;
    float a = glm::dot(ray_direction, ray_direction);
    float b = 2.0 * glm::dot(oc, ray_direction);
    float c = glm::dot(oc,oc) - radius*radius;
    float discriminant = b*b - 4*a*c;
    if(discriminant < 0){
        return false;
    }
    else{
        *t = (-b - sqrt(discriminant)) / (2.0*a);
        if(*t < 0){
            return false;
        }
        return true;
    }
}

/*
 * emission characteristic of a spotlight
 */
glm::vec3 SpotLight::getEmission(
		glm::vec3 const& omega // world space direction
		) const
{
	cg_assert(std::fabs(glm::length(omega) - 1.f) < EPSILON);
 
	// DONE: implement a spotlight emitter as specified on the exercise sheet
	return power * (falloff + 2) * (float)pow(fmax(0, glm::dot(direction, omega * -1.f)),falloff);
}

glm::vec3 evaluate_phong(
	RenderData &data,			// class containing raytracing information
	MaterialSample const& mat,	// the material at position
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V)			// view vector (already normalized)
{
	cg_assert(std::fabs(glm::length(N) - 1.f) < EPSILON);
	cg_assert(std::fabs(glm::length(V) - 1.f) < EPSILON);

	glm::vec3 contribution(0.f);

	// iterate over lights and sum up their contribution
	for (auto& light_uptr : data.context.get_active_scene()->lights) 
	{
		// DONE: calculate the (normalized) direction to the light
		const Light *light = light_uptr.get();
		glm::vec3 L = light->getPosition();
        glm::vec3 lightDir = glm::normalize(L - P);
        float cosTheta = glm::dot(N, lightDir);

		float visibility = 1.f;
		if (data.context.params.shadows) {
		    if(!visible(data, L, P)){
                visibility = 0.f;
		    }
			// DONE: check if light source is visible
		}

		glm::vec3 diffuse(0.f);
		if (data.context.params.diffuse) {
		    diffuse = mat.k_d * (float)fmax(0, cosTheta);
			// DONE: compute diffuse component of phong model
		}

		glm::vec3 specular(0.f);
		if (data.context.params.specular) {
		    float cosPsi = glm::dot(V, glm::reflect(lightDir * -1.f, N));
		    specular =   mat.k_s * (float)pow(fmax(0, cosPsi),mat.n);
			// DONE: compute specular component of phong model
		}

        float distanceToLight2 = pow(glm::length(L - P),2);
		float angleCheck = 0;
		if(cosTheta > 0){
            angleCheck = 1;
		}

        glm::vec3 IL = (light->getEmission(lightDir) * visibility * angleCheck)/distanceToLight2;

		glm::vec3 ambient = data.context.params.ambient ? mat.k_a : glm::vec3(0.0f);
		// DONE: modify this and implement the phong model as specified on the exercise sheet
		contribution += IL * (diffuse + specular) + ambient * light->getPower() / distanceToLight2;
	}

	return contribution;
}


glm::vec3 evaluate_reflection(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V)			// view vector (already normalized)
{
	// DONE: calculate reflective contribution by constructing and shooting a reflection ray.
	glm::vec3 newDirection = glm::vec3(reflect(V, N));
	Ray newRay = Ray(glm::vec3(P + data.context.params.ray_epsilon * newDirection), newDirection);
	return trace_recursive(data, newRay, depth + 1);
}

glm::vec3 evaluate_transmission(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V,			// view vector (already normalized)
	float eta)					// the relative refraction index
{
	// TODO: calculate transmissive contribution by constructing and shooting a transmission ray.
	glm::vec3 contribution(0.f);
	return contribution;
}

glm::vec3 handle_transmissive_material_single_ior(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V,			// view vector (already normalized)
	float eta)					// the relative refraction index
{
	if (data.context.params.fresnel) {
		// TODO: replace with proper fresnel handling.
		return evaluate_transmission(data, depth, P, N, V, eta);
	}
	else {
		// just regular transmission
		return evaluate_transmission(data, depth, P, N, V, eta);
	}
}

glm::vec3 handle_transmissive_material(
	RenderData &data,					// class containing raytracing information
	int depth,							// the current recursion depth
	glm::vec3 const& P,					// world space position
	glm::vec3 const& N,					// normal at the position (already normalized)
	glm::vec3 const& V,					// view vector (already normalized)
	glm::vec3 const& eta_of_channel)	// relative refraction index of red, green and blue color channel
{
	if (data.context.params.dispersion && !(eta_of_channel[0] == eta_of_channel[1] && eta_of_channel[0] == eta_of_channel[2])) {
		// TODO: split ray into 3 rays (one for each color channel) and implement dispersion here
		glm::vec3 contribution(0.f);
		return contribution;
	}
	else {
		// dont handle transmission, take average refraction index instead.
		const float eta = 1.f/3.f*(eta_of_channel[0]+eta_of_channel[1]+eta_of_channel[2]);
		return handle_transmissive_material_single_ior(data, depth, P, N, V, eta);
	}
	return glm::vec3(0.f);
}
// CG_REVISION f0d732361be3e2b0495d47edcd0f0acf37b5c1f4
