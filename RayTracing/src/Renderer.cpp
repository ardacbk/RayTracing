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


void Renderer::Render(){
	
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };

			coord = coord * 2.0f - 1.0f ;
 
			glm::vec4 color = PerPixel(coord);

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

glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	uint8_t r = (uint8_t)(coord.x * 255.0f);
	uint8_t g = (uint8_t)(coord.y * 255.0f);

	glm::vec3 rayOrigin = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 rayDirection = glm::vec3(coord.x, coord.y, -1.0f);
	
	float radius = 0.5f;

	// (bx^2 + by^2)t^2 + 2(axbx + ayby)t + (ax^2 + ay^2 - r^2) = 0
 
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2 * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;


	// Calculate the discriminant
	// b^2 - 4ac

	float disc = b * b - 4.0f * a * c;


	// No hit return
	if (disc < 0)
		return glm::vec4(0,0,0,1);

	// (-b -+ sqrt(disc)) / 2a 

	float t0 = (-b + glm::sqrt(disc)) / (2 * a);
	float closestT = (-b - glm::sqrt(disc)) / (2 * a);

	glm::vec3 hitPoint = rayDirection * closestT + rayOrigin;

	// normal = hitpoint - origin but origin = 0,0,0
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 sphereColor = glm::vec3(1,0,1);


	glm::vec3 ligthDir = glm::normalize(glm::vec3(-1, -1, -1));

	float d = glm::max(glm::dot(normal, -ligthDir), 0.0f);

	sphereColor *= d;

	return glm::vec4(sphereColor,1);
}
