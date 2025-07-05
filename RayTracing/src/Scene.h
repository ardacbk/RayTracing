#pragma once
#include "Walnut/Application.h"
#include <vector>
#include <glm/glm.hpp>
#include "Renderer.h"
#include "glm/gtc/type_ptr.hpp"


struct Material {
	glm::vec3 Albedo{ 1.0f };
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	float EmissionPower = 0.0f;
	glm::vec3 EmisisonColor{ 0.0f };

	glm::vec3 GetEmission() const { return EmisisonColor * EmissionPower; }
};


struct Hittable {
	virtual float Intersect(const Ray& ray) const = 0;
	virtual void DrawImGui(int i, int materialSize) = 0;
	int MaterialIndex;
	glm::vec3 Position;
};


struct Sphere : public Hittable {
	float Radius = 0.5f;

	float Intersect(const Ray& ray) const override {

		glm::vec3 origin = ray.Origin - this->Position;


		// a = ray origin
		// b = ray direction
		// r = radius
		// t = hit distance


		// (bx^2 + by^2)t^2 + 2(axbx + ayby)t + (ax^2 + ay^2 - r^2) = 0


		// at^2 + 2bt + c = 0
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2 * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - Radius * Radius;

		// disc = b^2 -4ac
		float disc = (b * b) - (4.0f * a * c);

		if (disc < 0)
			return -1.0f;


		// (-b -+ sqrt(disc)) / 2a 

		float t0 = (-b + glm::sqrt(disc)) / (2 * a); // Further hit point (unused for now)

		float t1 = (-b - glm::sqrt(disc)) / (2 * a);

		return t1;

	}

	void DrawImGui(int i,int materialSize) override {
		ImGui::PushID(i);
		ImGui::Text("Sphere %d", i);
		ImGui::DragFloat3("Position", glm::value_ptr(this->Position), 0.1f);
		ImGui::DragFloat("Radius", &Radius, 0.1f);
		ImGui::DragInt("Material", &MaterialIndex, 1.0f, 0, (int)materialSize - 1);
		ImGui::Separator();
		ImGui::PopID();

	}

};


struct Scene {
	std::vector<std::unique_ptr<Hittable>> Hittables;
	std::vector<Material> Materials;
};