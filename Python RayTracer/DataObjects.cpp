#include "DataObjects.h"
#include<glm.hpp>


Camera::Camera(glm::vec3 lookAt, glm::vec3 lookFrom, glm::vec3 lookUp, float fieldOfView) {
	this->lookAt = lookAt;
	this->lookFrom = lookFrom;
	this->lookUp = lookUp;
	this->fieldOfView = fieldOfView;
}

Light::Light(glm::vec3 directionToLight, glm::vec3 lightColor, glm::vec3 ambientLight) {
	this->directionToLight = directionToLight;
	this->lightColor = lightColor;
	this->ambientLight = ambientLight;
}

Sphere::Sphere(glm::vec3 center, float radius, glm::vec3 diffuse, glm::vec3 specularColor, float phong) {
	this->center = center;
	this->radius = radius;
	this->diffuse = diffuse;
	this->specularColor = specularColor;
	this->phong = phong;
}

Sphere::Sphere() {
	//nothing
}

Triangle::Triangle(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 diffuse, glm::vec3 specular, float phong) {
	this->point1 = point1;
	this->point2 = point2;
	this->point3 = point3;
	glm::vec3 edge1 = point1 - point2;
	glm::vec3 edge2 = point3 - point2;
	this->normal = glm::normalize(glm::cross(edge1, edge2));
	this->diffuse = diffuse;
	this->specularColor = specular;
	this->phong = phong;

	this->center = glm::vec3((point1.x + point2.x + point3.x) / 3,
		(point1.y + point2.y + point3.y) / 3,
		(point1.z + point2.z + point3.z) / 3);

}