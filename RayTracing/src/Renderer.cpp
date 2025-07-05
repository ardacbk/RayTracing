#include "Renderer.h"

#include "Walnut/Random.h"


#include <execution>

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color) {
		uint8_t r = (uint8_t) (color.r * 255.0f);
		uint8_t g = (uint8_t) (color.g * 255.0f);
		uint8_t b = (uint8_t) (color.b * 255.0f);
		uint8_t a = (uint8_t) (color.a * 255.0f);

		return (a << 24) | (b << 16) | (g << 8) | (r);
	}
}


void Renderer::Render(const Scene& scene, const Camera& camera) {

	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

#define MT 1
#if MT
	// MULTITHREADING
	std::for_each(std::execution::par,m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y) {
			std::for_each(std::execution::par,m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y](uint32_t x) {

					glm::vec4 color = PerPixel(x, y);

					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
					glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
					accumulatedColor /= (float)m_FrameIndex;

					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));

					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
				});
		});

#else
	// NO MULTITHREADING
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {

			glm::vec4 color = PerPixel(x, y);

			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= (float)m_FrameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));

			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}
#endif

	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;
}

void Renderer::OnResize(uint32_t width, uint32_t height) {

	if (m_FinalImage) {
		// Dont need to resize
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	// Used for multithreading
	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);

	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;
	
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;
}


glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 light(0.0f);

	glm::vec3 throughput(1.0f);

	int bounces = 5;
	for (int i = 0; i < bounces; i++) {
		Renderer::HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0.0f) {
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			light += skyColor * throughput;
			break;
		}



		const Hittable* hittable = m_ActiveScene->Hittables[payload.ObjectIndex].get();
		const Material& material = m_ActiveScene->Materials[hittable->MaterialIndex];

		// light += material.Albedo * throughput;

		throughput *= material.Albedo;

		light += material.GetEmission() * material.Albedo;
		
		// Increase bounce origin to provent new hitpoint being inside the object
		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		// ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f,0.5f));
		ray.Direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
	}



	return glm::vec4(light, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{

	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max(); // or just FLT_MAX

	for (size_t i = 0; i < m_ActiveScene->Hittables.size(); i++) {

		const Hittable* hittable = m_ActiveScene->Hittables[i].get();

		float closestT = hittable->Intersect(ray);

		if (closestT > 0.0f && closestT < hitDistance) {
			hitDistance = closestT;
			closestSphere = (int)i;
		}
	}

	if (closestSphere < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestSphere);

}


Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{

	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Hittable* hittable = m_ActiveScene->Hittables[objectIndex].get();
	glm::vec3 origin = ray.Origin - hittable->Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);
	
	payload.WorldPosition += hittable->Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}
