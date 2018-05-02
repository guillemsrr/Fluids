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

//Sphere
glm::vec3 spherePosition;
float sphereRadius;
const float mass = 1.0f;

//Cloth:
glm::vec3 posCloth[18][14];

//Wave:
//struct wave
float amplitude;
float frequency;
glm::vec3 waveVector;
glm::vec3 waveDirection;
float lambda;


//Time:
float resetTime;
float deltaTime;

bool renderSphere = false;
bool renderCloth = true;
#pragma endregion

namespace Sphere
{
	void cleanupSphere();
	void updateSphere(glm::vec3 pos, float radius = 1.f);
}
namespace ClothMesh
{
	void cleanupClothMesh();
	void updateClothMesh(float* array_data);
}

#pragma region Functions
//various:
void PhysicsInit();
float randomFloat(float min, float max)
{
	return ((max - min) * ((float)rand() / RAND_MAX)) + min;
}
//physics:

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


void PhysicsInit()
{
	system("cls");

	//Time:
	resetTime = 0.0f;

	//Seed
	srand(static_cast<unsigned int>(_getpid()) ^ static_cast<unsigned int>(clock()) ^ static_cast<unsigned int>(time(NULL)));

	//Initialize Sphere at random position
	if (renderSphere)
	{
		spherePosition = { -5.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))), static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (5.0f))), -5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))) };
		sphereRadius = 1.f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0f)));
		Sphere::updateSphere(spherePosition, sphereRadius);
	}

	//Initialize Cloth:
	//wave variables:
	amplitude = randomFloat(0.2f, 2.0f);
	frequency = randomFloat(0.2f,1.0f);
	waveVector = glm::vec3{randomFloat(0.0f,1.0f), 0.0f, randomFloat(0.0f,1.0f)};

	//initial Position:
	glm::vec3 auxPos;
	auxPos.z = -5.0f - 10.0f / 18.0f;
	for (int i = 0; i<18; i++)
	{
		auxPos.z += 10.0f/18.0f;
		auxPos.x = -5.f - 10.0f / 14.0f;
		for (int j = 0; j<14; j++)
		{
			//Position:
			auxPos.x += 10.0f/14.0f;
			posCloth[i][j] = { auxPos.x , 1.0f , auxPos.z };
		}
	}

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
			
			//GERSTNER WAVES:

			ClothMesh::updateClothMesh((float*)posCloth);

			//Sphere Buoyancy:
			if (useCollisions)
			{
				Sphere::updateSphere(spherePosition, 1.f);
			}
			
		}
	}
}

void PhysicsCleanup()
{
	ClothMesh::cleanupClothMesh();
	Sphere::cleanupSphere();
}

void gerstnerWave(glm::vec3 pos)
{
	pos.x -= amplitude*sin(waveVector)
}