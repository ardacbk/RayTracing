#pragma once

#include <memory>

#include "Walnut/Image.h"
#include "glm/glm.hpp"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

class Renderer {
public:
	struct Settings {
		bool Accumulate = true;
	};

public:
	Renderer() = default;


	void Render(const Scene& scene,const Camera &camera);
	void OnResize(uint32_t width, uint32_t height);
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void ResetFrameIndex() { m_FrameIndex = 1; }
	Settings& GetSettings() { return m_Settings; }
	struct HitPayload {
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		uint32_t ObjectIndex;
	};
private:

	glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen in Vulkan.

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);
private:

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;
	Settings m_Settings;

	// Multithreading iterators
	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;

	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;
	
	// Used for calculating the average
	uint32_t m_FrameIndex = 1;
};