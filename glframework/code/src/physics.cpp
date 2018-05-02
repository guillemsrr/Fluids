#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\quaternion.hpp>
#include <iostream>
#include <time.h>
#include <SDL.h>

#pragma region Variables
//matrices:
glm::mat4 cubeTransform;
glm::mat4 cubePosition;
glm::mat4 cubeRotation;

//kinematics:
struct stateVector
{
	glm::vec3 position;
};

stateVector sphereState;

//Time:
float resetTime;
float deltaTime;

bool renderSphere = true;
#pragma endregion

namespace Sphere
{
	void cleanupSphere();
	void updateSphere(glm::vec3 pos, float radius = 1.f);
}

#pragma region Functions
//various:
void PhysicsInit();
float randomFloat(float min, float max)
{
	return ((max - min) * ((float)rand() / RAND_MAX)) + min;
}
//physics:
void setSphereTransform();

#pragma endregion

#pragma region GUI Variables
static bool playSimulation = true;
int clicked = 0;
float totalResetTime = 15.0f;
glm::vec3 gravityAccel = { 0.0f,-9.81,0.0f };

bool useCollisions = true;
float elasticCoefficient = 0.2f;
float frictionCoefficient = 0.1f;
#pragma endregion

bool show_test_window = false;
void GUI()
{
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		//ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		ImGui::Checkbox("Play simulation", &playSimulation);
		if (ImGui::Button("Reset Simulation"))
		{
			clicked++;
		}
		if (clicked & 1)
		{
			PhysicsInit();
			clicked--;
		}
		ImGui::DragFloat("Reset Time", &totalResetTime, 0.05f);
		ImGui::InputFloat3("Gravity Accel", (float*)&gravityAccel);

		if (ImGui::TreeNode("Collisions"))
		{
			ImGui::Checkbox("Use Collisions", &useCollisions);
			ImGui::DragFloat("Elastic Coefficient", &elasticCoefficient, 0.005f);
			ImGui::DragFloat("Friction Coefficient", &frictionCoefficient, 0.005f);

			ImGui::TreePop();
		}
	}
	// .........................

	ImGui::End();

	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}


int loop;
void PhysicsInit()
{
	loop = 0;
	system("cls");

	//Time:
	resetTime = 0.0f;

	//Seed
	srand(static_cast<unsigned int>(_getpid()) ^ static_cast<unsigned int>(clock()) ^ static_cast<unsigned int>(time(NULL)));

	//Random position:
	sphereState.position = glm::vec3(randomFloat(-5.0f, 5.0f), randomFloat(0.f, 10.0f), randomFloat(-5.0f, 5.0f));
	Sphere::updateSphere(sphereState.position, 1.f );
}

void PhysicsUpdate(float dt)
{
	if (playSimulation)
	{
		if (resetTime >= totalResetTime)
		{
			clicked++;
		}
		else
		{
			resetTime += dt;
			deltaTime = dt;
			
			//collisions:
			if (useCollisions)
			{
				
			}
			else
			{
				
			}

		}
		Sphere::updateSphere(sphereState.position, 1.f);
	}
}

void PhysicsCleanup()
{
	Sphere::cleanupSphere();
}

