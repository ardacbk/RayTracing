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
	virtual glm::vec3 CalculateWorldNormal(const glm::vec3& hitPoint) const = 0;
	int MaterialIndex;
	glm::vec3 Position;
};


struct Plane : public Hittable {
	glm::vec3 Normal{ 0.0f };
	
	float Intersect(const Ray& ray) const override {
		
		float denom = glm::dot(ray.Direction, glm::normalize(this->Normal));
		if (glm::abs(denom) < 1e-6)
			return -1;
		glm::vec3 k = this->Position - ray.Origin;
		float t = glm::dot(k,glm::normalize(this->Normal)) / denom;
		if (t < 0)
			return -1;
		return t;
	}

	glm::vec3 CalculateWorldNormal(const glm::vec3& hitPoint) const override {
		return glm::normalize(this->Normal);
	}

	void DrawImGui(int i, int materialSize) override {
		ImGui::PushID(i);
		ImGui::Text("Plane %d", i);
		ImGui::DragFloat3("Position", glm::value_ptr(this->Position), 0.1f);
		ImGui::DragFloat3("Normal", glm::value_ptr(this->Normal), 0.1f);
		ImGui::DragInt("Material", &MaterialIndex, 1.0f, 0, (int)materialSize - 1);
		ImGui::Separator();
		ImGui::PopID();

	}



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

	glm::vec3 CalculateWorldNormal(const glm::vec3& hitPoint) const override {
		return glm::normalize(hitPoint - this->Position);
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