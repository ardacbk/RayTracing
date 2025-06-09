#include "Renderer.h"

#include "Walnut/Random.h"



void Renderer::Render(){
	
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };

			coord = coord * 2.0f - 1.0f ;
 
			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
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

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	uint8_t r = (uint8_t)(coord.x * 255.0f);
	uint8_t g = (uint8_t)(coord.y * 255.0f);

	glm::vec3 rayOrigin = glm::vec3(0.0f, 0.0f, 2.0f);
	glm::vec3 rayDirection = glm::vec3(coord.x, coord.y, -1.0f);
	rayDirection = glm::normalize(rayDirection);
	
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

	if (disc >= 0)
		return 0xffff00ff;
	return 0xff000000;
}
