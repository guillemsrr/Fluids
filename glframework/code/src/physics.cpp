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


//Cloth:
glm::vec3 posCloth[18][14];
glm::vec3 initialPosCloth[18][14];

//Waves:
struct wave
{
	float amplitude;
	float frequency;
	glm::vec3 waveDirection;//k
	float lambda;
	float phi;
};
const int numWaves = 2;
wave allWaves[numWaves];
//wave *allWaves;

//Fluid:
float density = 1.f;// 997.f;//  kg/m^3
float dragCoefficient = 1.f;

//Time:
float resetTime;
float deltaTime;

bool renderSphere = true;
bool renderCloth = true;
#pragma endregion

namespace Sphere
{
	//Sphere
	glm::vec3 position;
	glm::vec3 velocity;
	float radius;
	float mass = 1.0f; //kg
	glm::vec3 forceSum;
	glm::vec3 gravityForce;
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
	int iEq = 0;
	int jEq= 0;
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
void gerstnerWave(glm::vec3 &pos, wave w, glm::vec3 x0);
void sphereBuoyancy(glm::vec3 pos, float dt);

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

		ImGui::Checkbox("Use Buoyancy Force", &useCollisions);
		ImGui::DragFloat("Sphere Mass", &Sphere::mass, 0.005f);
		ImGui::DragFloat("Sphere Radius", &Sphere::radius, 0.005f);
		ImGui::DragFloat("Water density", &density, 0.005f);
		ImGui::DragFloat("Drag Coefficient", &dragCoefficient, 0.005f);
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

	//Initialize Waves:
	for (int i = 0; i < numWaves; i++)
	{
		allWaves[i].amplitude = randomFloat(0.1f, 1.0f);
		//allWaves[i].amplitude = 0.4f;
		std::cout << "wave " << i << " amplitude: " << allWaves[i].amplitude << std::endl;

		allWaves[i].frequency = randomFloat(0.1f, 1.0f);
		//allWaves[i].frequency = 0.8f;
		std::cout << "wave " << i << " frequency: " << allWaves[i].frequency << std::endl;

		allWaves[i].waveDirection = {randomFloat(0.0f,1.0f), 0.f, randomFloat(0.0f,1.0f)};
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
	auxPos.z = -5.0f -10.0f / 18.0f;
	for (int i = 0; i<18; i++)
	{
		auxPos.z += 10.0f/18.0f;
		auxPos.x = -5.f -10.0f / 14.0f;
		for (int j = 0; j<14; j++)
		{
			//Position:
			auxPos.x += 10.0f/14.0f;
			posCloth[i][j] = { auxPos.x , 2.0f , auxPos.z };
			initialPosCloth[i][j] = { auxPos.x , 2.0f , auxPos.z };
		}
	}

	//Initialize Sphere at random position
	if (renderSphere)
	{
		Sphere::position = { -4.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (8.0f))), 5.f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (5.0f))), -4.f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (8.0f))) };
		Sphere::velocity = glm::vec3{ 0.f,0.f,0.f };
		Sphere::radius = 0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.2f)));
		Sphere::mass = 0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.5f)));
		Sphere::gravityForce = Sphere::mass * gravityAccel;
		Sphere::forceSum = Sphere::gravityForce;
		Sphere::updateSphere(Sphere::position, Sphere::radius);
	}

	//get the closest x,z point of Sphere - Cloth
	glm::vec3 closestPoint = posCloth[0][0];

	for (int i = 0; i < 18; i++)
	{
		for (int j = 0; j < 14; j++)
		{
			//checking x and z position within closest distance:
			if (glm::distance(glm::vec2{ Sphere::position.x,Sphere::position.z }, glm::vec2{ posCloth[i][j].x,posCloth[i][j].z }) < glm::distance(glm::vec2{ Sphere::position.x,Sphere::position.z }, glm::vec2{ closestPoint.x, closestPoint.z }))
			{
				closestPoint = posCloth[i][j];
				Sphere::iEq = i;
				Sphere::jEq = j;
			}
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
			deltaTime = dt;
			resetTime += dt;

			//GERSTNER WAVES:
			for (int i = 0; i < 18; i++)
			{
				for (int j = 0; j < 14; j++)
				{
					posCloth[i][j] = initialPosCloth[i][j];
					for (int w = 0; w < numWaves; w++)
					{
						gerstnerWave(posCloth[i][j], allWaves[w], initialPosCloth[i][j]);
					}
				}
			}
			ClothMesh::updateClothMesh((float*)posCloth);

			//Sphere Buoyancy:
			if (useCollisions)
			{
				//Euler Solver:
				//update Position:
				Sphere::position += dt * Sphere::velocity;
				//update velocity:
				Sphere::velocity += dt * Sphere::forceSum / Sphere::mass;

				//reinitalize force:
				Sphere::forceSum = Sphere::gravityForce;//this force always applies

				//buoyancy Force:
				sphereBuoyancy(Sphere::position, dt);

				Sphere::updateSphere(Sphere::position, Sphere::radius);
				//std::cout << "spherePosition: " << Sphere::position.x << " " << Sphere::position.y << " " << Sphere::position.z << std::endl << std::endl;
			}
		}
	}
}

void PhysicsCleanup()
{
	ClothMesh::cleanupClothMesh();
	Sphere::cleanupSphere();
}

void gerstnerWave(glm::vec3 &pos, wave wave, glm::vec3 x0)
{
	pos -= wave.waveDirection * (wave.lambda / (2 * glm::pi<float>()))* wave.amplitude * sin(glm::dot(wave.waveDirection, x0) - wave.frequency * resetTime + wave.phi);
	pos.y += wave.amplitude * cos(glm::dot(wave.waveDirection, x0) - wave.frequency* resetTime + wave.phi);
}

void sphereBuoyancy(glm::vec3 pos, float dt)
{
	float height;
	height = pos.y - Sphere::radius - posCloth[Sphere::iEq][Sphere::jEq].y;
	if (height < 0.f)
	{
		float d = pos.y - posCloth[Sphere::iEq][Sphere::jEq].y;
		float r = sqrt(Sphere::radius*Sphere::radius - d*d);//teorema de pitàgores
		if (glm::abs(height) > Sphere::radius * 2)
		{
			height = Sphere::radius * 2;
			r = Sphere::radius;
		}
		//buoyancy force:
		float base = Sphere::radius*Sphere::radius*4.f;
		Sphere::forceSum += glm::abs(density * gravityAccel * base*height);
		std::cout << "buoyancy force: " << Sphere::forceSum.y << std::endl;
		std::cout << "height: " << height << std::endl;

		//drag force:
		float CSArea = glm::pi<float>()*r*r;
		Sphere::forceSum += -0.5f * density*dragCoefficient*CSArea * glm::length(Sphere::velocity)*Sphere::velocity;
	}
}