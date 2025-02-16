#include "hzpch.h"
#include "Calibur/Core/Application.h"

#include "Calibur/Core/Log.h"

#include "Calibur/Renderer/Renderer.h"
#include "Calibur/Renderer/Renderer2D.h"

#include "Calibur/Core/Input.h"

#include <glfw/glfw3.h>

namespace Calibur {
	Application* Application::s_Instance = nullptr;


	Application::Application(const std::string& name, ApplicationCommandLineArgs args)
		: m_CommandLineArgs(args)
	{
		HZ_PROFILE_FUNCTION();

		HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create(WindowProps(name));
		m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();
		Renderer2D::Init();
		
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application() 
	{
		HZ_PROFILE_FUNCTION();

		Renderer2D::Shutdown();
		Renderer::Shutdown();
	}
	
	void Application::PushLayer(Layer* layer)
	{
		HZ_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		HZ_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		HZ_PROFILE_FUNCTION();

		EventDispatcher dispathcher(e);
		dispathcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(Application::OnWindowClosed));
		dispathcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(Application::OnWindowResize));

		//HZ_CORE_TRACE("{0}", e);

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); it++)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::Run()
	{
		HZ_PROFILE_FUNCTION();

		while (m_Running)
		{
			HZ_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime();
			TimeStep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
			
			if (m_Minimized == false)
			{
				{
					HZ_PROFILE_SCOPE("LayerStack OnUpdate");

					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(timestep);
				}

				m_ImGuiLayer->Begin();
				{
					HZ_PROFILE_SCOPE("LayerStack OnImGuiRender");

					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();
			}

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		HZ_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true; 
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());


		return false;
	}
}
