#include <gl2d/gl2dParticleSystem.h>

namespace gl2d
{

	static ShaderProgram defaultParticleShader = {};

	static const char *defaultParticleVertexShader =
		GL2D_OPNEGL_SHADER_VERSION "\n"
		GL2D_OPNEGL_SHADER_PRECISION "\n"
		"in vec2 quad_positions;\n"
		"in vec4 quad_colors;\n"
		"in vec2 texturePositions;\n"
		"out vec4 v_color;\n"
		"out vec2 v_texture;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(quad_positions, 0, 1);\n"
		"	v_color = quad_colors;\n"
		"	v_texture = texturePositions;\n"
		"}\n";

	static const char *defaultParcileFragmentShader =
		GL2D_OPNEGL_SHADER_VERSION "\n"
		GL2D_OPNEGL_SHADER_PRECISION "\n"
		R"(out vec4 color;
			in vec4 v_color;
			in vec2 v_texture;
			uniform sampler2D u_sampler;
			
			vec3 rgbTohsv(vec3 c)
			{
				vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
				vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
				vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
			
				float d = q.x - min(q.w, q.y);
				float e = 1.0e-10;
				return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
			}
			
			vec3 hsvTorgb(vec3 c)
			{
				vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
				vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
				return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
			}
			
			const float cFilter = 5.f;
			
			void main()
			{
				color = v_color * texture2D(u_sampler, v_texture);
				
				if(color.a <0.01)discard;
				//color.a = 1.f;
			
				//color.a = pow(color.a, 0.2); 
			
				color.rgb *= cFilter;				//
				color.rgb = floor(color.rgb);		//remove color quality to get a retro effect
				color.rgb /= cFilter;				//
			
				//color.rgb = rgbTohsv(color.rgb);
			
				//color.rgb = hsvTorgb(color.rgb);
			
			})";


void ParticleSystem::initParticleSystem(int size)
{
	cleanup();


	//simdize size
	size += 4 - (size % 4);
	this->size = size;


#pragma region allocations


	int size32Aligned = size + (4 - (size % 4));

	posX = new float[size32Aligned];
	posY = new float[size32Aligned];
	directionX = new float[size32Aligned];
	directionY = new float[size32Aligned];
	rotation = new float[size32Aligned];
	sizeXY = new float[size32Aligned];
	dragX = new float[size32Aligned];
	dragY = new float[size32Aligned];
	duration = new float[size32Aligned];
	durationTotal = new float[size32Aligned];
	color = new glm::vec4[size];
	rotationSpeed = new float[size32Aligned];
	rotationDrag = new float[size32Aligned];
	deathRattle = new ParticleSettings * [size32Aligned];
	thisParticleSettings = new ParticleSettings * [size32Aligned];
	emitParticle = new ParticleSettings * [size32Aligned];
	tranzitionType = new char[size32Aligned];
	textures = new gl2d::Texture * [size32Aligned];
	emitTime = new float[size32Aligned];

#pragma endregion

	for (int i = 0; i < size; i++)
	{
		duration[i] = 0;
		sizeXY[i] = 0;
		deathRattle[i] = 0;
		textures[i] = nullptr;
		thisParticleSettings[i] = nullptr;
		emitParticle[i] = nullptr;
	}

	fb.create(100, 100);

}

#if GL2D_SIMD != 0

#if defined(_MSC_VER)
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
/* GCC-compatible compiler, targeting x86/x86-64 */
#include <x86intrin.h>
#elif defined(__GNUC__) && defined(__ARM_NEON__)
/* GCC-compatible compiler, targeting ARM with NEON */
#include <arm_neon.h>
#elif defined(__GNUC__) && defined(__IWMMXT__)
/* GCC-compatible compiler, targeting ARM with WMMX */
#include <mmintrin.h>
#elif (defined(__GNUC__) || defined(__xlC__)) && (defined(__VEC__) || defined(__ALTIVEC__))
/* XLC or GCC-compatible compiler, targeting PowerPC with VMX/VSX */
#include <altivec.h>
#elif defined(__GNUC__) && defined(__SPE__)
/* GCC-compatible compiler, targeting PowerPC with SPE */
#include <spe.h>
#elif
#undef GL2D_SIMD
#define GL2D_SIMD 0
#endif

#endif

void ParticleSystem::applyMovement(float deltaTime)
{

#pragma region newParticles

	int recreatedParticlesThisFrame = 0;

	//if(createdPosition < size && createTimeCountdown <= 0 && emitParticles)
	//{
	//	createTimeCountdown = rand(ps.emisSpeed);
	//
	//	for(int i=createdPosition; i<createdPosition+ maxCreatePerEvent; i++)
	//	{
	//		//reset particle
	//		posX[i] = position.x;
	//		posY[i] = position.y;
	//		directionX[i] = rand(ps.directionX);
	//		directionY[i] = rand(ps.directionY);
	//		rotation[i] = rand(ps.rotation);;
	//		sizeXY[i] = rand(ps.createApearence.size);
	//		dragX[i] = rand(ps.dragX);
	//		dragY[i] = rand(ps.dragY);
	//		color[i].x = rand({ ps.createApearence.color1.x, ps.createApearence.color2.x });
	//		color[i].y = rand({ ps.createApearence.color1.y, ps.createApearence.color2.y });
	//		color[i].z = rand({ ps.createApearence.color1.z, ps.createApearence.color2.z });
	//		color[i].w = rand({ ps.createApearence.color1.w, ps.createApearence.color2.w });
	//		rotationSpeed[i] = rand(ps.rotationSpeed);
	//		rotationDrag[i] = rand(ps.rotationDrag);
	//		deathRattle[i] = 1;
	//
	//		duration[i] = rand(ps.particleLifeTime);
	//		durationTotal[i] = duration[i];
	//
	//		recreatedParticlesThisFrame++;
	//	}
	//	
	//	createdPosition++;
	//
	//
	//}


#pragma endregion


	for (int i = 0; i < size; i++)
	{

		if (duration[i] > 0)
			duration[i] -= deltaTime;

		if (emitTime[i] > 0 && emitParticle[i])
			emitTime[i] -= deltaTime;

		if (duration[i] <= 0)
		{
			if (deathRattle[i] != nullptr && deathRattle[i]->onCreateCount)
			{

				this->emitParticleWave(deathRattle[i], {posX[i], posY[i]});

			}

			deathRattle[i] = nullptr;
			duration[i] = 0;
			sizeXY[i] = 0;
			emitParticle[i] = nullptr;

		}
		else if (emitTime[i] <= 0 && emitParticle[i])
		{
			emitTime[i] = rand(thisParticleSettings[i]->subemitParticleTime);

			//emit particle
			this->emitParticleWave(emitParticle[i], {posX[i], posY[i]});

		}

	}

	__m128 _deltaTime = _mm_set1_ps(deltaTime);

#pragma region applyDrag

#if GL2D_SIMD == 0
	for (int i = 0; i < size; i++)
	{
		//if (duration[i] > 0)
		directionX[i] += deltaTime * dragX[i];
	}

	for (int i = 0; i < size; i++)
	{
		//if (duration[i] > 0)
		directionY[i] += deltaTime * dragY[i];

	}

	for (int i = 0; i < size; i++)
	{

		//if (duration[i] > 0)
		rotationSpeed[i] += deltaTime * rotationDrag[i];
	}
#else

	for (int i = 0; i < size; i += 4)
	{
		//directionX[i] += deltaTime * dragX[i];

		__m128 *dir = (__m128 *) & (directionX[i]);
		__m128 *drag = (__m128 *) & (dragX[i]);

		*dir = _mm_fmadd_ps(_deltaTime, *drag, *dir);
	}

	for (int i = 0; i < size; i += 4)
	{
		//directionY[i] += deltaTime * dragY[i];

		__m128 *dir = (__m128 *) & (directionY[i]);
		__m128 *drag = (__m128 *) & (dragY[i]);

		*dir = _mm_fmadd_ps(_deltaTime, *drag, *dir);
	}

	for (int i = 0; i < size; i += 4)
	{
		//rotationSpeed[i] += deltaTime * rotationDrag[i];

		__m128 *dir = (__m128 *) & (rotationSpeed[i]);
		__m128 *drag = (__m128 *) & (rotationDrag[i]);

		*dir = _mm_fmadd_ps(_deltaTime, *drag, *dir);
	}
#endif



#pragma endregion


#pragma region apply movement


#if GL2D_SIMD == 0
	for (int i = 0; i < size; i++)
	{
		//if (duration[i] > 0)
		posX[i] += deltaTime * directionX[i];

	}


	for (int i = 0; i < size; i++)
	{
		//if (duration[i] > 0)
		posY[i] += deltaTime * directionY[i];

	}

	for (int i = 0; i < size; i++)
	{
		//if (duration[i] > 0)
		rotation[i] += deltaTime * rotationSpeed[i];

	}
#else 
	for (int i = 0; i < size; i += 4)
	{
		//posX[i] += deltaTime * directionX[i];
		__m128 *dir = (__m128 *) & (posX[i]);
		__m128 *drag = (__m128 *) & (directionX[i]);

		*dir = _mm_fmadd_ps(_deltaTime, *drag, *dir);
	}


	for (int i = 0; i < size; i++)
	{
		//posY[i] += deltaTime * directionY[i];
		__m128 *dir = (__m128 *) & (posY[i]);
		__m128 *drag = (__m128 *) & (directionY[i]);

		*dir = _mm_fmadd_ps(_deltaTime, *drag, *dir);
	}

	for (int i = 0; i < size; i++)
	{
		//rotation[i] += deltaTime * rotationSpeed[i];
		__m128 *dir = (__m128 *) & (rotation[i]);
		__m128 *drag = (__m128 *) & (rotationSpeed[i]);

		*dir = _mm_fmadd_ps(_deltaTime, *drag, *dir);
	}

#endif

#pragma endregion



}

void ParticleSystem::cleanup()
{
	delete[] posX;
	delete[] posY;

	delete[] directionX;
	delete[] directionY;
	delete[] rotation;

	delete[] sizeXY;

	delete[] dragY;
	delete[] dragX;
	delete[] duration;
	delete[] durationTotal;
	delete[] color;
	delete[] rotationSpeed;
	delete[] rotationDrag;
	delete[] emitTime;
	delete[] tranzitionType;
	delete[] deathRattle;
	delete[] thisParticleSettings;
	delete[] emitParticle;
	delete[] textures;


	posX = 0;
	posY = 0;
	directionX = 0;
	directionY = 0;
	rotation = 0;
	sizeXY = 0;
	dragX = 0;
	dragY = 0;
	duration = 0;
	durationTotal = 0;
	color = 0;
	rotationSpeed = 0;
	rotationDrag = 0;
	emitTime = 0;
	tranzitionType = 0;
	deathRattle = 0;
	thisParticleSettings = 0;
	emitParticle = 0;
	textures = 0;

	size = 0;


	fb.cleanup();
}

void ParticleSystem::emitParticleWave(ParticleSettings *ps, glm::vec2 pos)
{
	int recreatedParticlesThisFrame = 0;

	for (int i = 0; i < size; i++)
	{

		if (recreatedParticlesThisFrame < ps->onCreateCount &&
			sizeXY[i] == 0)
		{

			duration[i] = rand(ps->particleLifeTime);
			durationTotal[i] = duration[i];

			//reset particle
			posX[i] = pos.x + rand(ps->positionX);
			posY[i] = pos.y + rand(ps->positionY);
			directionX[i] = rand(ps->directionX);
			directionY[i] = rand(ps->directionY);
			rotation[i] = rand(ps->rotation);;
			sizeXY[i] = rand(ps->createApearence.size);
			dragX[i] = rand(ps->dragX);
			dragY[i] = rand(ps->dragY);
			color[i].x = rand({ps->createApearence.color1.x, ps->createApearence.color2.x});
			color[i].y = rand({ps->createApearence.color1.y, ps->createApearence.color2.y});
			color[i].z = rand({ps->createApearence.color1.z, ps->createApearence.color2.z});
			color[i].w = rand({ps->createApearence.color1.w, ps->createApearence.color2.w});
			rotationSpeed[i] = rand(ps->rotationSpeed);
			rotationDrag[i] = rand(ps->rotationDrag);
			textures[i] = ps->texturePtr;
			deathRattle[i] = ps->deathRattle;
			tranzitionType[i] = ps->tranzitionType;
			thisParticleSettings[i] = ps;
			emitParticle[i] = ps->subemitParticle;
			emitTime[i] = rand(thisParticleSettings[i]->subemitParticleTime);

			recreatedParticlesThisFrame++;
		}



	}


}

float interpolate(float a, float b, float perc)
{
	return a * perc + b * (1 - perc);

}

void ParticleSystem::draw(Renderer2D &r)
{

	unsigned int w = r.windowW;
	unsigned int h = r.windowH;

	auto cam = r.currentCamera;

	if (postProcessing)
	{

		r.flush();

		if (fb.texture.GetSize() != glm::ivec2{w / pixelateFactor,h / pixelateFactor})
		{
			fb.resize(w / pixelateFactor, h / pixelateFactor);

		}

		r.updateWindowMetrics(w / pixelateFactor, h / pixelateFactor);

	}


	for (int i = 0; i < size; i++)
	{
		if (sizeXY[i] == 0) { continue; }

		float lifePerc = duration[i] / durationTotal[i]; //close to 0 when gone, 1 when full

		switch (this->tranzitionType[i])
		{
		case gl2d::TRANZITION_TYPES::none:
		lifePerc = 1;
		break;
		case gl2d::TRANZITION_TYPES::linear:

		break;
		case gl2d::TRANZITION_TYPES::curbe:
		lifePerc *= lifePerc;
		break;
		case gl2d::TRANZITION_TYPES::abruptCurbe:
		lifePerc *= lifePerc * lifePerc;
		break;
		case gl2d::TRANZITION_TYPES::wave:
		lifePerc = (std::cos(lifePerc * 5 * 3.141592) * lifePerc + lifePerc) / 2.f;
		break;
		case gl2d::TRANZITION_TYPES::wave2:
		lifePerc = std::cos(lifePerc * 5 * 3.141592) * std::sqrt(lifePerc) * 0.9f + 0.1f;
		break;
		case gl2d::TRANZITION_TYPES::delay:
		lifePerc = (std::cos(lifePerc * 3.141592 * 2) * std::sin(lifePerc * lifePerc)) / 2.f;
		break;
		case gl2d::TRANZITION_TYPES::delay2:
		lifePerc = (std::atan(2 * lifePerc * lifePerc * lifePerc * 3.141592)) / 2.f;
		break;
		default:
		break;
		}

		glm::vec4 pos = {};
		glm::vec4 c;

		if (thisParticleSettings[i])
		{
			pos.x = posX[i];
			pos.y = posY[i];
			pos.z = interpolate(sizeXY[i], thisParticleSettings[i]->createEndApearence.size.x, lifePerc);
			pos.w = pos.z;

			c.x = interpolate(color[i].x, thisParticleSettings[i]->createEndApearence.color1.x, lifePerc);
			c.y = interpolate(color[i].y, thisParticleSettings[i]->createEndApearence.color1.y, lifePerc);
			c.z = interpolate(color[i].z, thisParticleSettings[i]->createEndApearence.color1.z, lifePerc);
			c.w = interpolate(color[i].w, thisParticleSettings[i]->createEndApearence.color1.w, lifePerc);
		}
		else
		{
			pos.x = posX[i];
			pos.y = posY[i];
			pos.z = sizeXY[i];
			pos.w = pos.z;

			c.x = color[i].x;
			c.y = color[i].y;
			c.z = color[i].z;
			c.w = color[i].w;
		}

		glm::vec4 p;

		if (postProcessing)
		{
			r.currentCamera = cam;

			p = pos / pixelateFactor;

			//p.x += 200;
			//p.y += 200;

			p.x -= r.currentCamera.position.x / pixelateFactor;
			p.y -= r.currentCamera.position.y / pixelateFactor;
			//

			r.currentCamera.position = {};
			//r.currentCamera.position.x += w / (2.f );
			//r.currentCamera.position.y += h / (2.f );
			//
			//r.currentCamera.position /= pixelateFactor/2.f;
			//
			//r.currentCamera.position.x -= w / (2.f);
			//r.currentCamera.position.y -= h / (2.f);


			//r.currentCamera.position += glm::vec2{w / (pixelateFactor * 2.f), h / (pixelateFactor*2.f)};
			//r.currentCamera.position *= pixelateFactor;
			//c.w = sqrt(c.w);
			// c.w = 1;
		}
		else
		{
			p = pos;
		}


		if (textures[i] != nullptr)
		{
			r.renderRectangle(p, *textures[i], c, { 0, 0 }, rotation[i]);
		}
		else
		{
			r.renderRectangle(p, c, {0,0}, rotation[i]);
		}


	}


	if (postProcessing)
	{
		fb.clear();
		r.flushFBO(fb);



		r.updateWindowMetrics(w, h);
		r.currentCamera.setDefault();


		auto s = r.currentShader;

		r.renderRectangle({0,0,w,h}, fb.texture);

		r.setShaderProgram(defaultParticleShader);
		r.flush();

		r.setShaderProgram(s);

	}

	r.currentCamera = cam;

}

float ParticleSystem::rand(glm::vec2 v)
{
	if (v.x > v.y)
	{
		std::swap(v.x, v.y);
	}

	std::uniform_real_distribution<float> dist(v.x, v.y);

	return dist(random);
}

void initgl2dParticleSystem()
{
	defaultParticleShader = createShaderProgram(defaultParticleVertexShader, defaultParcileFragmentShader);
}

void cleanupgl2dParticleSystem()
{
	glDeleteShader(defaultParticleShader.id);
}


};
