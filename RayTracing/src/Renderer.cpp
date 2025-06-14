#include "Renderer.h"

#include "Walnut/Random.h"


namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color) {
		uint8_t r = (uint8_t) (color.r * 255.0f);
		uint8_t g = (uint8_t) (color.g * 255.0f);
		uint8_t b = (uint8_t) (color.b * 255.0f);
		uint8_t a = (uint8_t) (color.a * 255.0f);

		return (a << 24) | (b << 16) | (g << 8) | (r);
	}
}


void Renderer::Render(const Scene& scene,const Camera& camera){
	
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			
			glm::vec4 color = PerPixel(x,y);

			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= (float)m_FrameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));

			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}

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


}


glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 color(0.0f);

	float multiplier = 1.0f;

	int bounces = 5;
	for (int i = 0; i < bounces; i++) {
		Renderer::HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0.0f) {
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			color += skyColor * multiplier;
			break;
		}

		m_lightDir = glm::normalize(m_lightDir);

		float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -m_lightDir), 0.0f);

		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

		glm::vec3 sphereColor = material.Albedo;

		sphereColor *= lightIntensity;

		color += sphereColor * multiplier;

		multiplier *= 0.5f;
		// Increase bounce origin to provent new hitpoint being inside the object
		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f,0.5f));
	}



	return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{

	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max(); // or just FLT_MAX

	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++) {

		const Sphere& sphere = m_ActiveScene->Spheres[i];

		glm::vec3 origin = ray.Origin - sphere.Position;

		// (bx^2 + by^2)t^2 + 2(axbx + ayby)t + (ax^2 + ay^2 - r^2) = 0

		// a = ray origin
		// b = ray direction
		// r = radius
		// t = hit distance


		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2 * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;


		// Calculate the discriminant
		// b^2 - 4ac

		float disc = b * b - 4.0f * a * c;


		// No hit return
		if (disc < 0)
			continue;

		// (-b -+ sqrt(disc)) / 2a 

		float t0 = (-b + glm::sqrt(disc)) / (2 * a); // Further hit point (unused for now)

		float closestT = (-b - glm::sqrt(disc)) / (2 * a);

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

	const Sphere& sphere = m_ActiveScene->Spheres[objectIndex];
	glm::vec3 origin = ray.Origin - sphere.Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);
	
	payload.WorldPosition += sphere.Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}
