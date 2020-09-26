#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include "DataObjects.h"
#include<glm.hpp>
#include <algorithm>

using namespace std;

struct Tuple {
	float t1;
	float t2;
};

struct PolyTuple {
	bool hit;
	float distance;
};

Tuple IntersectRaySphere(glm::vec3 rayStart, glm::vec3 rayDir, Sphere sphere);
PolyTuple IntersectTriangle(glm::vec3 rayStart, glm::vec3 rayDir, Triangle triangle);
bool rayIntersectTriangle(glm::vec3 rayStart, glm::vec3 rayDir, Triangle* inTriangle, glm::vec3& outIntersectionPoint, float& t);

string printPixel(glm::vec3 color) {
	string colorString;
	colorString;
	string r = to_string(color.x);
	string g = to_string(color.y);
	string b = to_string(color.z);
	colorString = r + ' ' + g + ' ' + b + '\n';
	return colorString;
}

glm::vec3 canvasToViewPort(float x, float y, float viewX, float viewY, float canvasWidth, float canvasHeight) {
	//cout << x << " " << y << " " << viewX << " " << viewY << " " << canvasWidth << " " << canvasHeight << endl;
	glm::vec3 rayDir(x * (viewX / canvasWidth), y * (viewY / canvasHeight), -1);
	//cout << printPixel(rayDir) << "here!" << endl;
	return rayDir;
}

glm::vec3 getIntersect(float t, glm::vec3 rayStart, glm::vec3 rayDir, Sphere sphere) {
	glm::vec3 intersect = rayStart + (rayDir * t);
	return intersect;
}

glm::vec3 getNormal(Sphere* sphere, glm::vec3 intersect) {
	glm::vec3 normal = intersect - sphere->getCenter();
	normal = glm::normalize(normal);
	return normal;
}

glm::vec3 getReflection(glm::vec3 normal, glm::vec3 vector) {
	return ((float)2 * normal * dot(normal, vector) - vector);
}

Sphere* getClosestSphere(glm::vec3 rayStart, glm::vec3 rayDir, float t_min, float t_max, vector<Sphere> spheres) {
	float closest_t = 300000;
	Sphere* closestSphere = nullptr;
	glm::vec3 intersect;
	for (int i = 0; i < spheres.size(); i++) {
		Tuple intersections = IntersectRaySphere(rayStart, rayDir, spheres.at(i));
		//cout << intersections.t1 << " " << intersections.t2 << endl;
		if (intersections.t1 > t_min&&
			intersections.t1 < t_max &&
			intersections.t1 < closest_t) {
			closest_t = intersections.t1;
			closestSphere = &spheres.at(i);
			intersect = getIntersect(intersections.t1, rayStart, rayDir, spheres.at(i));
		}
		if (intersections.t2 >= t_min &&
			intersections.t2 < t_max &&
			intersections.t2 < closest_t) {
			closest_t = intersections.t2;
			closestSphere = &spheres.at(i);
			intersect = getIntersect(intersections.t2, rayStart, rayDir, spheres.at(i));
		}
	}
	return closestSphere;
}

Triangle* getClosestTriangle(glm::vec3 rayStart, glm::vec3 rayDir, float t_min, float t_max, vector<Triangle> triangles) {
	PolyTuple polyHit;
	Triangle* closestTriangle = nullptr;
	float poly_closest_t = 350000;
	for (int i = 0; i < triangles.size(); i++) {
		polyHit = IntersectTriangle(rayStart, rayDir, triangles.at(i));
		if (polyHit.distance < poly_closest_t && polyHit.hit) {
			poly_closest_t = polyHit.distance;
			closestTriangle = &triangles.at(i);
		}
		/*if (polyHit.hit) {
			return glm::vec3(255, 255, 255);
		}*/
	}
	return closestTriangle;
}

glm::vec3 traceRay(glm::vec3 rayStart, glm::vec3 rayDir, float t_min, float t_max, vector<Sphere> spheres, vector<Triangle> triangles) {
	float sphere_closest_t = 300000;
	Sphere* closestSphere = nullptr;
	Triangle* closestTriangle = nullptr;
	glm::vec3 sphereIntersect(0, 0, 0);
	for (int i = 0; i < spheres.size(); i++) {
		Tuple intersections = IntersectRaySphere(rayStart, rayDir, spheres.at(i));
		//cout << intersections.t1 << " " << intersections.t2 << endl;
		if (intersections.t1 > t_min&&
			intersections.t1 < t_max &&
			intersections.t1 < sphere_closest_t) {
			sphere_closest_t = intersections.t1;
			closestSphere = &spheres.at(i);
			sphereIntersect = getIntersect(intersections.t1, rayStart, rayDir, spheres.at(i));
		}
		if (intersections.t2 >= t_min &&
			intersections.t2 < t_max &&
			intersections.t2 < sphere_closest_t) {
			sphere_closest_t = intersections.t2;
			closestSphere = &spheres.at(i);
			sphereIntersect = getIntersect(intersections.t2, rayStart, rayDir, spheres.at(i));
		}
	}
	PolyTuple polyHit;
	float poly_closest_t = 350000;
	for (int i = 0; i < triangles.size(); i++) {
		glm::vec3 intersectPoint;
		float t;
		bool hit = rayIntersectTriangle(rayStart, rayDir, &triangles.at(i), intersectPoint, t);
		if (hit && t < poly_closest_t) {
			poly_closest_t = t;
			closestTriangle = &triangles.at(i);
		}
	}
	if (closestSphere == nullptr && closestTriangle == nullptr) {
		return glm::vec3(50, 50, 50);
	}
	if (poly_closest_t < sphere_closest_t) {
		glm::vec3 normal = closestTriangle->getNormal();
		normal = normalize(normal);
		glm::vec3 color(0, 0, 0);

		glm::vec3 ambient(25, 25, 25);
		glm::vec3 dirLight(1, 0, 0);

		glm::vec3 diffuse;
		glm::vec3 phong;

		dirLight = glm::normalize(dirLight);
		color += ambient;

		Sphere* shadow_sphere = getClosestSphere(rayStart + rayDir * poly_closest_t + (-normal * (float).01), dirLight, t_min, t_max, spheres);
		if (shadow_sphere != nullptr) {
			return color;
		}
		Triangle* shadow_triangle = getClosestTriangle(rayStart + rayDir * poly_closest_t + (-normal * (float).01), dirLight, t_min, t_max, triangles);
		if (shadow_triangle != nullptr) {
			return color;
		}

		color = glm::clamp(color, (float)0, (float)255);
		diffuse = closestTriangle->getDiffuse() * (dot(normal, dirLight) / (glm::length(-normal) * glm::length(dirLight)));
		diffuse = glm::clamp(diffuse, (float)0, (float)255);
		glm::vec3 viewDir = rayDir;
		glm::vec3 reflection = getReflection(-normal, dirLight);
		reflection = glm::normalize(reflection);
		phong = closestTriangle->getSpecularColor() * pow(std::max((float)0, dot(reflection, glm::normalize(viewDir))) / (length(reflection) * length(viewDir)), closestTriangle->getPhong());
		phong = glm::clamp(phong, (float)0, (float)255);
		color += phong;
		color = glm::clamp(color, (float)0, (float)255);
		color += diffuse;
		color = glm::clamp(color, (float)0, (float)255);
		return color;
	}
	else {
		glm::vec3 normal = getNormal(closestSphere, sphereIntersect);
		glm::vec3 color(0, 0, 0);

		glm::vec3 ambient(25, 25, 25);
		glm::vec3 dirLight(1, 0, 0);

		glm::vec3 diffuse;
		glm::vec3 phong;

		dirLight = glm::normalize(dirLight);
		color += ambient;

		Sphere* shadow_sphere = getClosestSphere(sphereIntersect + (normal * (float).01), dirLight, t_min, t_max, spheres);
		if (shadow_sphere != nullptr) {
			return color;
		}
		Triangle* shadow_triangle = getClosestTriangle(sphereIntersect + (normal * (float).01), dirLight, t_min, t_max, triangles);
		if (shadow_triangle != nullptr) {
			return color;
		}

		color = glm::clamp(color, (float)0, (float)255);
		diffuse = closestSphere->getDiffuse() * (dot(normal, dirLight) / (glm::length(normal) * glm::length(dirLight)));
		diffuse = glm::clamp(diffuse, (float)0, (float)255);
		glm::vec3 viewDir = -rayDir;
		glm::vec3 reflection = getReflection(normal, dirLight);
		reflection = glm::normalize(reflection);
		phong = closestSphere->getSpecularColor() * pow(std::max((float)0, dot(reflection, glm::normalize(viewDir))) / (length(reflection) * length(viewDir)), closestSphere->getPhong());
		phong = glm::clamp(phong, (float)0, (float)255);
		color += phong;
		color = glm::clamp(color, (float)0, (float)255);
		color += diffuse;
		color = glm::clamp(color, (float)0, (float)255);
		return color;
	}
}

Tuple IntersectRaySphere(glm::vec3 rayStart, glm::vec3 rayDir, Sphere sphere) {
	glm::vec3 oc = rayStart - sphere.getCenter();
	float k1 = dot(rayDir, rayDir);
	float k2 = 2 * dot(oc, rayDir);
	float k3 = dot(oc, oc) - sphere.getRadius() * sphere.getRadius();
	float disc = k2 * k2 - 4 * k1 * k3;
	Tuple t;
	if (disc < 0) {
		t.t1 = 20000;
		t.t2 = 20000;
		return t;
	}
	t.t1 = (-k2 + sqrt(disc)) / (2 * k1);
	t.t2 = (-k2 - sqrt(disc)) / (2 * k1);
	return t;
}

bool rayIntersectTriangle(glm::vec3 rayStart, glm::vec3 rayDir, Triangle* inTriangle, glm::vec3& outIntersectionPoint, float& outT) {
	float epsilon = 0.0000001;
	glm::vec3 vertex0 = inTriangle->getPoint1();
	glm::vec3 vertex1 = inTriangle->getPoint2();
	glm::vec3 vertex2 = inTriangle->getPoint3();
	glm::vec3 edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = glm::cross(rayDir, edge2);
	a = glm::dot(edge1, h);
	if (a > -epsilon && a < epsilon) {
		return false;
	}
	f = 1.0 / a;
	s = rayStart - vertex0;
	u = f * glm::dot(s, h);
	if (u < 0.0 || u > 1.0) {
		return false;
	}
	q = glm::cross(s, edge1);
	v = f * glm::dot(rayDir, q);
	if (v < 0.0 || u + v > 1.0) {
		return false;
	}
	float t = f * glm::dot(edge2, q);
	outT = t;
	if (t > epsilon) {
		outIntersectionPoint = rayStart + rayDir * t;
		return true;
	}
	else {
		return false;
	}
}

PolyTuple IntersectTriangle(glm::vec3 rayStart, glm::vec3 rayDir, Triangle triangle) {
	glm::vec3 pNormal = triangle.getNormal();
	float vd = dot(pNormal, rayDir);
	PolyTuple result;
	result.hit = false;
	result.distance = 30000;
	/*if (vd >= 0) {
		return result;
	}*/
	if (vd < 0) {
		pNormal = -pNormal;
	}
	float v0 = (float)0 - (dot(pNormal, rayStart) + glm::length(triangle.getCenter()));
	float t = v0 / vd;
	if (t < 0) {
		return result;
	}
	glm::vec3 intersect = rayStart + rayDir * t;
	//pNormal = -pNormal;
	int dominantCoord = 0;
	float highest = pNormal[0];
	if (pNormal[1] > highest) {
		dominantCoord = 1;
		highest = pNormal[1];
	}
	if (pNormal[2] > highest) {
		dominantCoord = 2;
		highest = pNormal[2];
	}

	glm::vec2 point1;
	glm::vec2 point2;
	glm::vec2 point3;

	glm::vec2 planeIntersect;
	int index = 0;
	for (int i = 0; i < 3; i++) {
		if (i != dominantCoord) {
			point1[index] = triangle.getPoint1()[i];
			point2[index] = triangle.getPoint2()[i];
			point3[index] = triangle.getPoint3()[i];
			planeIntersect[index] = intersect[i];
			index++;
		}
	}
	point1 = point1 - planeIntersect;
	point2 = point2 - planeIntersect;
	point3 = point3 - planeIntersect;
	vector<glm::vec2> points;
	points.push_back(point1);
	points.push_back(point2);
	points.push_back(point3);
	planeIntersect -= planeIntersect;
	int numCrossing = 0;
	int signholder;
	int nextSignholder;
	if (point1[1] < 0) {
		signholder = -1;
	}
	else {
		signholder = 1;
	}
	int len = points.size();
	len = 3;
	for (int i = 0; i < len; i++) {
		if (points[(i + 1) % len][1] < 0) {
			nextSignholder = -1;
		}
		else {
			nextSignholder = 1;
		}
		if (signholder != nextSignholder) {
			if (points[i][0] > 0 && points[(i + 1) % len][0] > 0) {
				numCrossing++;
			}
			else if (points[i][0] > 0 && points[(i + 1) % len][0]) {
				float uCross;
				uCross = points[i][0] - (points[i][1] * (points[(i + 1) % len][0] - points[i][0]) / (points[(i + 1) % len][1] - points[i][1]));
				if (uCross > 0) {
					numCrossing++;
				}
			}
		}
		signholder = nextSignholder;
	}
	if (numCrossing % 2 == 1) { //if numCrossing is odd
		result.hit = true;
		result.distance = t;
		return result;
	}

	return result;
}


void main() {

	int canvasWidth = 800;
	int canvasHeight = 800;
	ofstream myfile;
	myfile.open("render.ppm");
	myfile << "P3\n" << endl;
	myfile << canvasWidth << " " << canvasHeight << endl;
	myfile << "255" << endl;

	//set up camera
	glm::vec3 lookAt(0, 0, 0);
	glm::vec3 lookFrom(0, 0, 1);
	glm::vec3 lookUp(0, 1, 0);
	float fieldOfView = 28;
	Camera camera(lookAt, lookFrom, lookUp, fieldOfView);

	glm::vec3 directionToLight(1, 0, 0);
	glm::vec3 lightColor(255, 255, 255);
	glm::vec3 ambientLight(25.5, 25.5, 25.5);
	Light light(directionToLight, lightColor, ambientLight);

	Sphere sphere1 = Sphere(glm::vec3(.35, 0, -.1), .05, glm::vec3(255, 255, 255), glm::vec3(255, 255, 255), 4);
	Sphere sphere2 = Sphere(glm::vec3(.2, 0, -.1), .075, glm::vec3(255, 0, 0), glm::vec3(123, 255, 123), 32);
	Sphere sphere3 = Sphere(glm::vec3(-.6, 0, 0), .3, glm::vec3(0, 255, 0), glm::vec3(123, 255, 123), 32);

	vector<Sphere> spheres;
	spheres.push_back(sphere1);
	spheres.push_back(sphere2);
	spheres.push_back(sphere3);

	Triangle triangle1 = Triangle(glm::vec3(0, .3, -.1), glm::vec3(.3, -.3, -.4), glm::vec3(-.3, -.3, .2), glm::vec3(0, 0, 255), glm::vec3(255, 255, 255), 32);
	Triangle triangle2 = Triangle(glm::vec3(-.2, .1, -.3), glm::vec3(-.2, -.5, .2), glm::vec3(-.2, .1, .1), glm::vec3(255, 255, 0), glm::vec3(255, 255, 255), 4);
	vector<Triangle> triangles;
	triangles.push_back(triangle1);
	triangles.push_back(triangle2);

	for (int column = canvasHeight / 2; column > -canvasHeight / 2; column--) {
		for (int row = -canvasWidth / 2; row < canvasWidth / 2; row++) {
			glm::vec3 rayDir = canvasToViewPort(row, column, 1.0, 1.0, (float)canvasWidth, (float)canvasHeight);
			// - glm::vec3(0,0,-1)
			glm::vec3 color = traceRay(camera.getLookFrom(), rayDir, .001, 20000, spheres, triangles);
			myfile << printPixel(color);
		}
	}
}