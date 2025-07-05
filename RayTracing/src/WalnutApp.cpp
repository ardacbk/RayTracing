#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"
#include "glm/gtc/type_ptr.hpp"

#include "Renderer.h"
#include "Camera.h"

using namespace Walnut;

class RayTracingLayer : public Walnut::Layer
{
public:
	RayTracingLayer() 
		: m_Camera(45.0f,0.1f,100.0f)
	{
		Material& pinkSphere = m_Scene.Materials.emplace_back();
		pinkSphere.Albedo = { 1.0f, 0, 1.0f };
		pinkSphere.Roughness = 0.0f;

		Material& blueSphere= m_Scene.Materials.emplace_back();
		blueSphere.Albedo = { 0.2f, 0.3f, 1.0f };
		blueSphere.Roughness = 0.1f;


		Material& orangeSphere = m_Scene.Materials.emplace_back();
		orangeSphere.Albedo = { 0.8f, 0.5f, 0.2f };
		orangeSphere.Roughness = 0.1f;
		orangeSphere.EmisisonColor = orangeSphere.Albedo;
		orangeSphere.EmissionPower = 2.0f;

			//Sphere 1
			{
				Sphere sphere;
				sphere.Position = { 0.0f, 0.0f , 0.0f };
				sphere.Radius = 1.0f;
				sphere.MaterialIndex = 0;
				m_Scene.Hittables.push_back(std::make_unique<Sphere>(sphere));
		}

			//Sphere 2
			{
				Sphere sphere;
				sphere.Position = { 2.0f, 0.0f , 0.0f };
				sphere.Radius = 1.0f;
				sphere.MaterialIndex = 2;
				m_Scene.Hittables.push_back(std::make_unique<Sphere>(sphere));
			}
			
			//Ground
			{
				Sphere sphere;
				sphere.Position = { 0.0f, -101.0f , 0.0f };
				sphere.Radius = 100.0f;
				sphere.MaterialIndex = 1;
				m_Scene.Hittables.push_back(std::make_unique<Sphere>(sphere));
			}

	}
	virtual void OnUpdate(float ts) override {
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();
	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);

		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset"))
			m_Renderer.ResetFrameIndex();

		ImGui::End();



		ImGui::Begin("Scene");
		ImGui::Separator();

		for (size_t i = 0; i < m_Scene.Hittables.size(); i++) {
			m_Scene.Hittables[i]->DrawImGui(i, m_Scene.Materials.size());
		}


		ImGui::Text("MATERIALS");
		for (size_t i = 0; i < m_Scene.Materials.size(); i++) {
			ImGui::PushID(i);
			ImGui::Text("Material %d", i);
			Material& material = m_Scene.Materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo), 0.1f);
			ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f );
			ImGui::DragFloat("Metallic", &material.Metallic, 0.05f, 0.0f, 1.0f);
			ImGui::ColorEdit3("EmissionColor", glm::value_ptr(material.EmisisonColor));
			ImGui::DragFloat("EmissionPower", &material.EmissionPower, 0.05f, 0.0f, FLT_MAX);
			ImGui::Separator();
			ImGui::PopID();
		}
		ImGui::End();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;


		auto image = m_Renderer.GetFinalImage();
		if (image)
			ImGui::Image(image->GetDescriptorSet(), {(float)image->GetWidth(), (float)image->GetHeight()},ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();
		
		Render();
	}


	void Render() {
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene,m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	Camera m_Camera;
	Scene m_Scene;

	float m_LastRenderTime = 0.0f;
}; 

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<RayTracingLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}