#ifndef DATA_OBJECTS
#define DATA_OBJECTS
#pragma once
#include<glm.hpp>
class Camera {
	private:
		glm::vec3 lookAt;
		glm::vec3 lookFrom;
		glm::vec3 lookUp;
		float fieldOfView;
	public:
		Camera(glm::vec3 lookAt, glm::vec3 lookFrom, glm::vec3 lookUp, float fieldOfView);

		glm::vec3 getLookAt() { return lookAt; }

		void setLookAt(glm::vec3 lookAt) { this->lookAt = lookAt; }

		glm::vec3 getLookFrom() { return lookFrom; }

		void setLookFrom(glm::vec3 lookFrom) { this->lookFrom = lookFrom; }

		glm::vec3 getLookUp() { return lookUp; }

		void setLookUp(glm::vec3 lookUp) { this->lookUp = lookUp; }
};

class Light {
	private:
		glm::vec3 directionToLight;
		glm::vec3 lightColor;
		glm::vec3 ambientLight;
public:
	Light(glm::vec3 directionToLight, glm::vec3 lightColor, glm::vec3 ambientLight);
	glm::vec3 getDirectionToLight() { return directionToLight; }
	void setDirectionToLight(glm::vec3 directionToLight) { this->directionToLight = directionToLight; }
	glm::vec3 getLightColor() { return lightColor; }
	void setLightColor(glm::vec3 lightColor) { this->lightColor = lightColor; }
	glm::vec3 getAmbientLight() { return ambientLight; }
	void setAmbientLight(glm::vec3 ambientLight) { this->ambientLight = ambientLight; }
};

class Sphere {
	private:
		glm::vec3 center;
		float radius;
		glm::vec3 diffuse;
		glm::vec3 specularColor;
		float phong;
public:
	Sphere(glm::vec3 center, float radius, glm::vec3 diffuse, glm::vec3 specularColor, float phong);
	Sphere();
	glm::vec3 getCenter() { return center; }
	void setCenter(glm::vec3 center) { this->center = center; }
	float getRadius() { return radius; }
	void setRadius(float radius) { this->radius = radius; }
	glm::vec3 getDiffuse() { return diffuse; }
	void setDiffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
	glm::vec3 getSpecularColor() { return specularColor; }
	void setSpecularColor(glm::vec3 specularColor) { this->specularColor = specularColor; }
	float getPhong() { return phong; }
	void setPhong(float phong) { this->phong = phong; }
};

class Triangle {
private:
	glm::vec3 point1;
	glm::vec3 point2;
	glm::vec3 point3;
	glm::vec3 diffuse;
	glm::vec3 specularColor;
	glm::vec3 normal;
	float phong;
	glm::vec3 center;
public:
	Triangle(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 diffuse, glm::vec3 specular, float phong);
	glm::vec3 getPoint1() { return point1; }
	void setPoint1(glm::vec3 point1) { this->point1 = point1; }
	glm::vec3 getPoint2() { return point2; }
	void setPoint2(glm::vec3 point2) { this->point2 = point2; }
	glm::vec3 getPoint3() { return point3; }
	void setPoint3(glm::vec3 point3) { this->point3 = point3; }
	glm::vec3 getDiffuse() { return diffuse; }
	void setDiffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
	glm::vec3 getSpecularColor() { return specularColor; }
	void setSpecularColor(glm::vec3 specularColor) { this->specularColor = specularColor; }
	float getPhong() { return phong; }
	void setPhong(float phong) { this->phong = phong; }
	glm::vec3 getNormal() { return normal; }
	void setNormal(glm::vec3 normal) { this->normal = normal; }
	glm::vec3 getCenter() { return center; }
	void setCenter(glm::vec3 center) { this->center = center; }
};

#endif
