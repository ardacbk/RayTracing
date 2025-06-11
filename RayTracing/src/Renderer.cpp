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
	
	Ray ray;
	ray.Origin = camera.GetPosition();

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
 
			
			glm::vec4 color = TraceRay(scene,ray);

			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));

			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);

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


}


glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
	
	if (scene.Spheres.size() == 0)
		return glm::vec4(0, 0, 0, 1);

	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max(); // or just FLT_MAX

	for (const Sphere& sphere : scene.Spheres) {

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

		if (closestT < hitDistance) {
			hitDistance = closestT;
			closestSphere = &sphere;
		}
	}

	if (closestSphere==nullptr)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glm::vec3 origin = ray.Origin - closestSphere->Position;
	glm::vec3 hitPoint = ray.Direction * hitDistance + origin;

	// normal = hitpoint - origin but origin = 0,0,0
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 sphereColor = closestSphere->Albedo;


	m_lightDir = glm::normalize(m_lightDir);

	float d = glm::max(glm::dot(normal, -m_lightDir), 0.0f);

	sphereColor *= d;

	return glm::vec4(sphereColor,1);
}
