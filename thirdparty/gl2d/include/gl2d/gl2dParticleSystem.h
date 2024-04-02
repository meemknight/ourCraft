#pragma once
#include "gl2d.h"

namespace gl2d
{

	///////////////////// ParticleSysyem /////////////////////
#pragma region ParticleSysyem

	void initgl2dParticleSystem();
	
	void cleanupgl2dParticleSystem();

	struct ParticleApearence
	{
		glm::vec2 size = {};
		glm::vec4 color1 = {};
		glm::vec4 color2 = {};
	};

	enum TRANZITION_TYPES
	{
		none = 0,
		linear,
		curbe,
		abruptCurbe,
		wave,
		wave2,
		delay,
		delay2
	};


	struct ParticleSettings
	{
		ParticleSettings *deathRattle = nullptr;
		ParticleSettings *subemitParticle = nullptr;

		int onCreateCount = 0;

		glm::vec2 subemitParticleTime = {};

		glm::vec2 positionX = {};
		glm::vec2 positionY = {};

		glm::vec2 particleLifeTime = {}; // move
		glm::vec2 directionX = {};
		glm::vec2 directionY = {};
		glm::vec2 dragX = {};
		glm::vec2 dragY = {};

		glm::vec2 rotation = {};
		glm::vec2 rotationSpeed = {};
		glm::vec2 rotationDrag = {};

		ParticleApearence createApearence = {};
		ParticleApearence createEndApearence = {};

		gl2d::Texture *texturePtr = 0;

		int tranzitionType = TRANZITION_TYPES::linear;
	};


	struct ParticleSystem
	{
		void initParticleSystem(int size);
		void cleanup();

		void emitParticleWave(ParticleSettings *ps, glm::vec2 pos);


		void applyMovement(float deltaTime);

		void draw(gl2d::Renderer2D &r);

		bool postProcessing = true;
		float pixelateFactor = 2;

	private:

		int size = 0;

		float *posX = 0;
		float *posY = 0;

		float *directionX = 0;
		float *directionY = 0;

		float *rotation = 0;

		float *sizeXY = 0;

		float *dragX = 0;
		float *dragY = 0;

		float *duration = 0;
		float *durationTotal = 0;

		glm::vec4 *color = 0;

		float *rotationSpeed = 0;
		float *rotationDrag = 0;

		float *emitTime = 0;

		char *tranzitionType = 0;
		ParticleSettings **deathRattle = 0;
		ParticleSettings **thisParticleSettings = 0;
		ParticleSettings **emitParticle = 0;

		gl2d::Texture **textures = 0;

		std::mt19937 random{std::random_device{}()};

		gl2d::FrameBuffer fb = {};

		float rand(glm::vec2 v);
	};


#pragma endregion

}