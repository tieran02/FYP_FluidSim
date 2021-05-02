#include <util/Log.h>
#include "Simulation.h"

Simulation::Simulation() : m_solver{nullptr}, m_renderer{nullptr}, m_isPaused(false)
{
	
}

void Simulation::Init()
{
	m_renderer = std::make_unique<FluidRenderer>(Window::Width(), Window::Height(), SPHERE_COUNT);

	m_solver = std::make_unique<PCISPHSolverGPU>( 1.0f / 60.0f, SPHERE_COUNT, boxCollider, m_openCLContext, 
		m_renderer->GetPositionStorageBuffer(), m_renderer->GetPressureStorageBuffer() );
}

void Simulation::Update()
{
	CORE_ASSERT(m_solver, "Solver is nullptr");
	CORE_ASSERT(m_renderer, "Renderer is nullptr");
	if(!m_solver || !m_renderer)
		return;
	
	//run simulation
	if(!m_isPaused)
		m_solver->Update();

	m_renderer->Render();
}

void Simulation::restart(Solver::Scenario scenario)
{
	CORE_ASSERT(m_solver, "Solver is nullptr");
	if (!m_solver)
		return;
	
	m_solver->Reset(scenario);
}


void Simulation::KeyCallback(int key, int action, int mode)
{
	Camera& camera = m_renderer->GetCamera();
	switch (key)
	{
	case GLFW_KEY_W:
		camera.Move(camera.Forward());
		break;
	case GLFW_KEY_S:
		camera.Move(-camera.Forward());
		break;
	case GLFW_KEY_A:
		camera.Move(-glm::cross(camera.Forward(), camera.Up()));
		break;
	case GLFW_KEY_D:
		camera.Move(glm::cross(camera.Forward(), camera.Up()));
		break;
	case GLFW_KEY_F9:
		restart(Solver::Scenario::Fill);
		break;
	case GLFW_KEY_F10:
		restart(Solver::Scenario::OneSided);
		break;
	case GLFW_KEY_F11:
		restart(Solver::Scenario::TwoSided);
		break;
	case GLFW_KEY_F1:
		m_renderer->ChangeRenderMode(FluidRenderer::RenderMode::SPHERE);
		break;
	case GLFW_KEY_F2:
		m_renderer->ChangeRenderMode(FluidRenderer::RenderMode::DEPTH);
		break;
	case GLFW_KEY_F3:
		m_renderer->ChangeRenderMode(FluidRenderer::RenderMode::NORMAL);
		break;
	case GLFW_KEY_F4:
		m_renderer->ChangeRenderMode(FluidRenderer::RenderMode::THICKNESS);
		break;
	case GLFW_KEY_F5:
		m_renderer->ChangeRenderMode(FluidRenderer::RenderMode::FLUID);
		break;
	case GLFW_KEY_P:
		if(action == GLFW_RELEASE)
			m_isPaused = !m_isPaused;
		break;
	}
}

void Simulation::CursorCallback(double xPos, double yPos)
{
	Camera& camera = m_renderer->GetCamera();
	
	camera.MousePosition(xPos,yPos);
}

void Simulation::MouseButtonCallback(int button, int action, int mod)
{

}
