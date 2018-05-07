#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\constants.hpp>
#include <glm\gtx\quaternion.hpp>
#include <iostream>
#include <time.h>
#include <SDL.h>

#pragma region Variables
//matrices:

//Sphere
glm::vec3 spherePosition;
float sphereRadius;
float mass = 1.0f; //kg


//Cloth:
glm::vec3 posCloth[18][14];
//Waves:
struct wave
{
	float amplitude;
	float frequency;
	glm::vec3 waveDirection;//k
	float lambda;
	float phi;
};
const int numWaves = 1;
wave allWaves[numWaves];
//wave *allWaves;

//Fluid:
float density = 997.f;//  kg/m^3

//Time:
float resetTime;
float deltaTime;

bool renderSphere = true;
bool renderCloth = true;
#pragma endregion

namespace Sphere
{
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
}
namespace ClothMesh
{
	void cleanupClothMesh();
	void updateClothMesh(float* array_data);
}

#pragma region Functions
void PhysicsInit();
float randomFloat(float min, float max)
{
	return ((max - min) * ((float)rand() / RAND_MAX)) + min;
}
void gerstnerWave(glm::vec3 &pos, wave w);
void sphereCollision(glm::vec3 &pos, float dt);

#pragma endregion

#pragma region GUI Variables
static bool playSimulation = true;
int clicked = 0;
float totalResetTime = 15.0f;
glm::vec3 gravityAccel = { 0.0f,-9.81,0.0f };

bool useCollisions = true;
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
			ImGui::DragFloat("Mass", &mass, 0.005f);
			ImGui::DragFloat("Water density", &density, 0.005f);

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
		spherePosition = { -5.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))), 5.f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (5.0f))), -5 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))) };
		std::cout << "spherePosition: " << spherePosition.x << " " << spherePosition.y << " " << spherePosition.z << std::endl << std::endl;
		sphereRadius = 1.f;// +static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0f)));
		Sphere::updateSphere(spherePosition, sphereRadius);
	}

	//Initialize Cloth:
	//int numWaves = rand() % 3;
	//allWaves = new wave[numWaves];
	//allWaves = new wave[2];
	for (int i = 1; i <= numWaves; i++)
	{
		allWaves[i].amplitude = randomFloat(0.1f, 1.0f);
		//allWaves[i].amplitude = 0.4f;
		std::cout << "wave " << i << " amplitude: " << allWaves[i].amplitude << std::endl;

		allWaves[i].frequency = randomFloat(0.1f, 1.0f);
		//allWaves[i].frequency = 0.8f;
		std::cout << "wave " << i << " frequency: " << allWaves[i].frequency << std::endl;

		allWaves[i].waveDirection = glm::vec3{ 1,0,0 };//randomFloat(0.0f,1.0f), 0.f, randomFloat(0.0f,1.0f)};
		std::cout << "wave " << i << " direction: (" << allWaves[i].waveDirection.x << ", " << allWaves[i].waveDirection.y << ", " << allWaves[i].waveDirection.z << ") " << std::endl;

		allWaves[i].lambda = randomFloat(0.1f, 1.0f);
		//allWaves[i].lambda = 0.2f;
		std::cout << "wave "<<i<<" lambda: "<<allWaves[i].lambda << std::endl;

		allWaves[i].phi = randomFloat(0.1f, 1.0f);
		//allWaves[i].phi = 0.2f;
		std::cout << "wave " << i << " phi: " << allWaves[i].phi << std::endl;
	}

	//initial Cloth Position:
	glm::vec3 auxPos;
	auxPos.z = -5.0f;// -10.0f / 18.0f;
	for (int i = 0; i<18; i++)
	{
		auxPos.z += 10.0f/18.0f;
		auxPos.x = -5.f;// -10.0f / 14.0f;
		for (int j = 0; j<14; j++)
		{
			//Position:
			auxPos.x += 10.0f/14.0f;
			posCloth[i][j] = { auxPos.x , 2.0f , auxPos.z };
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
			for (int i = 0; i < 18; i++)
			{
				for (int j = 0; j < 14; j++)
				{
					for (int w = 1; w <= numWaves; w++)
					{
						posCloth[i][j].y = 2.f;
						gerstnerWave(posCloth[i][j], allWaves[w]);
					}
				}
			}
			ClothMesh::updateClothMesh((float*)posCloth);

			//Sphere Buoyancy:
			if (useCollisions)
			{
				//compute with temporal position:
				glm::vec3 auxSpherePosition = spherePosition;

				//gravity force:
				auxSpherePosition += dt * gravityAccel;

				//check collisions:
				//sphereCollision(auxSpherePosition, dt);

				spherePosition = auxSpherePosition;
				//Sphere::updateSphere(spherePosition, sphereRadius);
			}
		}
	}
}

void PhysicsCleanup()
{
	ClothMesh::cleanupClothMesh();
	Sphere::cleanupSphere();
}

void gerstnerWave(glm::vec3 &pos, wave wave)
{
	pos -= wave.waveDirection * (wave.lambda / (2 * glm::pi<float>()))* wave.amplitude * sin(glm::dot(wave.waveDirection,pos) - wave.frequency * resetTime + wave.phi);
	pos.y +=wave.amplitude * cos(glm::dot(wave.waveDirection,pos) - wave.frequency* resetTime + wave.phi);
}

void sphereCollision(glm::vec3 &pos, float dt)
{
	//Cerca binaria:
	glm::vec3 auxLast;
	glm::vec3 auxPos;
	glm::vec3 cuttingPoint;

	for (int i = 0; i < 18; i++)
	{
		for (int j = 0; j < 14; j++)
		{
			if (glm::distance(posCloth[i][j], pos) < sphereRadius)
			{
				//Cerca dicotòmica:
				auxLast = spherePosition;
				auxPos = pos;
				cuttingPoint = (auxLast + auxPos) / 2.f;

				while (glm::distance(cuttingPoint, pos) <= sphereRadius - 0.001f || glm::distance(cuttingPoint, pos) >= sphereRadius + 0.001f)
				{
					if (glm::distance(cuttingPoint, pos) > sphereRadius)
					{
						auxLast = cuttingPoint;
					}
					else
					{
						auxPos = cuttingPoint;
					}
					cuttingPoint = (auxLast + auxPos) / 2.f;
				}

				//buoyancy force:
				cuttingPoint += dt*density * gravityAccel * sphereRadius;
				pos = cuttingPoint;
			}
		}
	}
}