#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>
#include <rendering/model.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp> // for glm::make_mat4
#include <glm/gtx/quaternion.hpp>
#include <fstream>
#include <map>
#include <set>

glm::mat4 aiToGlm(const aiMatrix4x4 &matrix)
{
	return glm::make_mat4(&matrix.a1); // a1 represents the first element of the matrix
}

const aiNode *findNodeContainingMesh(const aiNode *node, const aiMesh *mesh)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		if (node->mMeshes[i] == mesh->mPrimitiveTypes)
		{
			return node;
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		const aiNode *foundNode = findNodeContainingMesh(node->mChildren[i], mesh);
		if (foundNode)
			return foundNode;
	}

	return nullptr;
}


bool areStringsSameToLower(const char *a, const char *b)
{
	int i = 0;
	while (a[i] != 0 && b[i] != 0)
	{
		if (tolower(a[i]) != tolower(b[i]))
		{
			return false;
		}
		
		i++;
	}
	if (a[i] == 0 && b[i] == 0) { return true; }
	return false;
}


void ModelsManager::loadAllModels(std::string path, bool reportErrors)
{

	bool appendMode = texturesIds.empty();


	//load default texture
	if(appendMode)
	{
		unsigned char data[16] = {};

		{
			int i = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 255;

			data[i++] = 146;
			data[i++] = 52;
			data[i++] = 235;
			data[i++] = 255;

			data[i++] = 146;
			data[i++] = 52;
			data[i++] = 235;
			data[i++] = 255;

			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 255;
		}

		gl2d::Texture t;
		t.createFromBuffer((char *)data, 2, 2, true, false);

		texturesIds.push_back(t.id);
		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);
		gpuIds.push_back(handle);
	}

	auto loadTexture = [&](const char *path, bool appendMode, int index, bool isPlayerSkin = 0)
	{

		if (!appendMode && texturesIds[index] != texturesIds[0]) { return; } //we already have the texture

		gl2d::Texture texture = {};
		GLuint64 handle = 0;

		if (isPlayerSkin)
		{
			texture = loadPlayerSkin(path);
		}
		else
		{
			texture.loadFromFile(path, true, true);
		}

		if (texture.id)
		{

			texture.bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 6.f);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 2.f);

			glGenerateMipmap(GL_TEXTURE_2D);

			handle = glGetTextureHandleARB(texture.id);
			glMakeTextureHandleResidentARB(handle);
		}
		else
		{
			//todo error report
			std::cout << "Error loading: " << path << "\n";

			handle = gpuIds[0];
			texture.id = texturesIds[0];
		}

		if (appendMode)
		{
			gpuIds.push_back(handle);
			texturesIds.push_back(texture.id);
		}
		else
		{
			gpuIds[index] = handle;
			texturesIds[index] = texture.id;
		}


	};


	//load textures
	{
		int index = 1;
		//the order matters!!!!
		//loadTexture((path+"steve.png").c_str());
		loadTexture((path + "steve.png").c_str(), appendMode, index++, true);
		loadTexture((path + "zombie.png").c_str(), appendMode, index++, true);
		loadTexture((path + "pig.png").c_str(), appendMode, index++);
		loadTexture((path + "cat.png").c_str(), appendMode, index++);
		loadTexture((path + "goblin.png").c_str(), appendMode, index++);
		loadTexture((path+ "helmetTest.png").c_str(), appendMode, index++);
		

	}


	Assimp::Importer importer;

	// Step 2: Specify Import Options
	unsigned int flags = aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords | aiProcess_TransformUVCoords 
		| aiProcess_FindInstances | aiProcess_GenNormals;

	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Remove lines and points
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS);

	struct Data
	{
		glm::vec3 position = {};
		glm::vec3 normal = {};
		glm::vec2 uv = {};
		short boneIndex = 0;
		short textureIndex = 0;
	};

	std::vector<Data> vertexes;
	vertexes.reserve(400);

	std::vector<unsigned int> indices;
	indices.reserve(400);

	auto loadModel = [&](const char *path, Model &model, bool multipleTextures = 0)
	{
		vertexes.clear();
		indices.clear();

		const aiScene *scene = importer.ReadFile(path, flags);

		if (scene)
		{

			glGenVertexArrays(1, &model.vao);
			glBindVertexArray(model.vao);

			glGenBuffers(1, &model.geometry);
			glGenBuffers(1, &model.indexBuffer);

			glBindBuffer(GL_ARRAY_BUFFER, model.geometry);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);

			unsigned int vertexOffset = 0;

			int boneIndex = 0;
			for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; ++i)
			{

				const aiNode *node = scene->mRootNode->mChildren[i];

				const char *name = node->mName.C_Str();

				short textureIndex = 0;

				if (areStringsSameToLower(name, "Head")) { model.headIndex = i; }else
				if (areStringsSameToLower(name, "Body")) { model.bodyIndex = i; }else
				if (areStringsSameToLower(name, "RLeg")) { model.rLegIndex = i; }else
				if (areStringsSameToLower(name, "LLeg")) { model.lLefIndex = i; }else
				if (areStringsSameToLower(name, "RArm")) { model.rArmIndex = i; }else
				if (areStringsSameToLower(name, "LArm")) { model.lArmIndex = i; }else
				if (areStringsSameToLower(name, "Pupils")) { model.pupilsIndex = i; }else
				if (areStringsSameToLower(name, "LEye")) { model.lEyeIndex = i; }else
				if (areStringsSameToLower(name, "REye")) { model.rEyeIndex = i; }else
				if (areStringsSameToLower(name, "HeadArmour")) { model.headArmourIndex = i; textureIndex = 1; }else
				if (areStringsSameToLower(name, "BodyArmour")) { model.bodyArmourIndex = i; textureIndex = 2; }else
				if (areStringsSameToLower(name, "RLegArmour")) { model.rLegArmourIndex = i; textureIndex = 3; }else
				if (areStringsSameToLower(name, "LLegArmour")) { model.lLefArmourIndex = i; textureIndex = 3; }else
				if (areStringsSameToLower(name, "RArmArmour")) { model.rArmArmourIndex = i; textureIndex = 2; }else
				if (areStringsSameToLower(name, "LArmArmour")) { model.lArmArmourIndex = i; textureIndex = 2; }

				if (!multipleTextures) { textureIndex = 0; }

				for (int m = 0; m < node->mNumMeshes; m++)
				{
					const aiMesh *mesh = scene->mMeshes[node->mMeshes[m]];

					//const char *name2 = mesh->mName.C_Str();

					aiMatrix4x4 transform = node->mTransformation;
					model.transforms.push_back(glm::transpose(aiToGlm(transform)));

					for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
					{
						Data vertex;
						vertex.textureIndex = textureIndex;

						vertex.boneIndex = boneIndex;

						// Positions
						vertex.position.x = mesh->mVertices[v].x;
						vertex.position.y = mesh->mVertices[v].y;
						vertex.position.z = mesh->mVertices[v].z;

						// Normals
						vertex.normal.x = mesh->mNormals[v].x;
						vertex.normal.y = mesh->mNormals[v].y;
						vertex.normal.z = mesh->mNormals[v].z;

						// UVs (if present)
						if (mesh->HasTextureCoords(0))
						{
							vertex.uv.x = mesh->mTextureCoords[0][v].x;
							vertex.uv.y = mesh->mTextureCoords[0][v].y;
						}
						else
						{
							vertex.uv = glm::vec2(0.0f, 0.0f);
						}

						vertexes.push_back(vertex);
					}

					for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
					{
						const aiFace &face = mesh->mFaces[i];
						for (unsigned int j = 0; j < face.mNumIndices; ++j)
						{
							indices.push_back(face.mIndices[j] + vertexOffset);
						}
					}

					vertexOffset += mesh->mNumVertices;
				}

				if (node->mNumMeshes)
				{
					boneIndex++;
				}
			}


			model.vertexCount = indices.size();

			if (!model.vertexCount) { std::cout << "!!!!Error wrong model format!!!!!!!!!!!!!\n"; }

			glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(Data), vertexes.data(), GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)(sizeof(float) * 3));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)(sizeof(float) * 6));
			glEnableVertexAttribArray(3); //bone
			glVertexAttribIPointer(3, 1, GL_SHORT, sizeof(float) * 9, (void *)(sizeof(float) * 8));

			glEnableVertexAttribArray(4); //texture id
			glVertexAttribIPointer(4, 1, GL_SHORT, sizeof(float) * 9, (void *)(sizeof(float) * 8 + sizeof(short)));



			glBindVertexArray(0);

		}
		else if(reportErrors)
		{
			std::cout << "noSchene in" << path << "\n";
			std::cout << importer.GetErrorString() << "\n";
		}


	};


	if(!human.vertexCount)
		loadModel((path + "human.glb").c_str(), human, true);

	if (!pig.vertexCount)
		loadModel((path + "pig.glb").c_str(), pig);
	
	if (!cat.vertexCount)
		loadModel((path + "cat.glb").c_str(), cat);

	if (!rightHand.vertexCount)
		loadModel((path + "rightHand.glb").c_str(), rightHand);

	if (!goblin.vertexCount)
		loadModel((path + "goblin.glb").c_str(), goblin);


	
	flags = aiProcess_ImproveCacheLocality 
		| aiProcess_JoinIdenticalVertices 
		| aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_FindInstances;

	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Remove lines and points
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS);


	struct Quad
	{
		int a = 0;
		int b = 0;
		int c = 0;
		int d = 0;
	};

	std::vector<Quad> quads;
	quads.reserve(50);

	auto loadBlockModel = [&](const char *path, BlockModel &blockModel)
	{
		blockModel.cleanup();

		const aiScene *scene = importer.ReadFile(path, flags);

		if (scene)
		{
			for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; ++i)
			{
				const aiNode *node = scene->mRootNode->mChildren[i];
				aiMatrix4x4 transform = node->mTransformation;

				for (int m = 0; m < node->mNumMeshes; m++)
				{
					const aiMesh *mesh = scene->mMeshes[node->mMeshes[m]];
					quads.clear();

					struct Triangle
					{
						unsigned int indices[3] = {};
						aiVector3D normal = {};
						int adjacentIndex = -1; // Store the index of the triangle it merges with
					};

					std::vector<Triangle> triangles;
					triangles.reserve(mesh->mNumFaces);

					// Step 1: Collect triangles and compute normals
					for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
					{
						const aiFace &face = mesh->mFaces[i];

						if (face.mNumIndices != 3)
						{
							std::cout << "ERROR wrong model format!\n";
							continue;
						}

						Triangle tri;
						tri.indices[0] = face.mIndices[0];
						tri.indices[1] = face.mIndices[1];
						tri.indices[2] = face.mIndices[2];

						// Compute normal using cross product
						aiVector3D v0 = mesh->mVertices[tri.indices[0]];
						aiVector3D v1 = mesh->mVertices[tri.indices[1]];
						aiVector3D v2 = mesh->mVertices[tri.indices[2]];
						tri.normal = (v1 - v0) ^ (v2 - v0); // Cross product
						tri.normal.Normalize();

						triangles.push_back(tri);
					}

					// Step 2: Find adjacent triangle pairs
					std::map<std::pair<unsigned int, unsigned int>, int> edgeMap; // Edge -> Triangle index

					for (size_t i = 0; i < triangles.size(); i++)
					{
						auto &tri = triangles[i];

						for (int e = 0; e < 3; e++)
						{
							unsigned int a = tri.indices[e];
							unsigned int b = tri.indices[(e + 1) % 3];

							if (a > b)
								std::swap(a, b);

							auto edge = std::make_pair(a, b);
							if (edgeMap.count(edge))
							{
								int otherIndex = edgeMap[edge];
								auto &otherTri = triangles[otherIndex];

								// Step 3: Check for planarity
								if (tri.normal * otherTri.normal > 0.99f) // Almost parallel normals
								{
									// Step 4: Merge into quad
									tri.adjacentIndex = otherIndex;
									otherTri.adjacentIndex = i;
								}
							}
							else
							{
								edgeMap[edge] = i;
							}
						}
					}

					// Step 5: Store quads
					std::vector<char> merged(triangles.size(), false);

					for (size_t i = 0; i < triangles.size(); i++)
					{
						if (merged[i] || triangles[i].adjacentIndex == -1)
							continue;

						int j = triangles[i].adjacentIndex;
						if (merged[j])
							continue;

						// Find the fourth vertex not shared
						std::set<unsigned int> quadVertices = {
							triangles[i].indices[0],
							triangles[i].indices[1],
							triangles[i].indices[2],
							triangles[j].indices[0],
							triangles[j].indices[1],
							triangles[j].indices[2],
						};

						if (quadVertices.size() == 4)
						{
							auto it = quadVertices.begin();
							Quad q;
							q.a = *it++;
							q.b = *it++;
							q.c = *it++;
							q.d = *it++;

							quads.push_back(q);
							merged[i] = merged[j] = true;
						}
					}

					// Step 6: Push quads to blockModel
					for (const Quad &q : quads)
					{

						aiVector3D v = mesh->mVertices[q.d];
						v *= transform;
						blockModel.vertices.push_back(v.x);
						blockModel.vertices.push_back(v.y - 0.5);
						blockModel.vertices.push_back(v.z);

						v = mesh->mVertices[q.b];
						v *= transform;
						blockModel.vertices.push_back(v.x);
						blockModel.vertices.push_back(v.y - 0.5);
						blockModel.vertices.push_back(v.z);

						v = mesh->mVertices[q.a];
						v *= transform;
						blockModel.vertices.push_back(v.x);
						blockModel.vertices.push_back(v.y - 0.5);
						blockModel.vertices.push_back(v.z);

						v = mesh->mVertices[q.c];
						v *= transform;
						blockModel.vertices.push_back(v.x);
						blockModel.vertices.push_back(v.y - 0.5);
						blockModel.vertices.push_back(v.z);

						if (mesh->mTextureCoords[0]) // Has UVs
						{

							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.d].x);
							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.d].y);

							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.b].x);
							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.b].y);


							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.a].x);
							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.a].y);

							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.c].x);
							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.c].y);

						}
					}
				}
			}


			if (blockModel.vertices.size() % 3 != 0 && 
				blockModel.vertices.size() % 12 != 0 &&
				blockModel.uvs.size() %2 != 0 &&
				(blockModel.vertices.size()/3) != (blockModel.uvs.size() /2)
				)
			{
				std::cout << "ERROR LOADING MODEL!\n";
				blockModel.cleanup();
			}


		}
	};


	if (!chairModel.vertices.size())
		loadBlockModel((path + "chair.glb").c_str(), chairModel);

	if (!mugModel.vertices.size())
		loadBlockModel((path + "aleMug.glb").c_str(), mugModel);


	//todo check if it frees all of them
	importer.FreeScene();


	setupSSBO();
}

void ModelsManager::clearAllModels()
{
	human.cleanup();

	pig.cleanup();

	cat.cleanup();

	rightHand.cleanup();

	goblin.cleanup();

	chairModel.cleanup();

	if (texturesIds.size())
	{
		auto defaultTexture = texturesIds[0];

		for (int i = 1; i < texturesIds.size(); i++)
		{
			if (texturesIds[i] != defaultTexture)
			{
				glDeleteTextures(1, &texturesIds[i]);
			}
		}
		glDeleteTextures(1, &texturesIds[0]);
	}

	texturesIds.clear();
	gpuIds.clear();

}

void ModelsManager::setupSSBO()
{

	if (!texturesSSBO)
	{
		glGenBuffers(1, &texturesSSBO);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturesSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, gpuIds.size() * sizeof(gpuIds[0]), gpuIds.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, texturesSSBO); //todo add some constatns here

}

void animatePlayerLegs(glm::mat4 *poseVector,
	float &currentAngle, int &direction, float deltaTime)
{

	if (direction == 0)
	{
		if (currentAngle)
		{
			if (currentAngle > 0)
			{
				currentAngle -= deltaTime;
				if (currentAngle < 0)
				{
					currentAngle = 0;
				}
			}
			else
			{
				currentAngle += deltaTime;
				if (currentAngle > 0)
				{
					currentAngle = 0;
				}
			}
		
		}
	}
	{
		currentAngle += deltaTime * direction;

		if (direction == 1)
		{
			if (currentAngle >= glm::radians(40.f))
			{
				currentAngle = glm::radians(40.f);
				direction = -1;
			}
		}
		else
		{
			if (currentAngle <= glm::radians(-40.f))
			{
				currentAngle = glm::radians(-40.f);
				direction = 1;
			}
		}

	}

	
	poseVector[0]; //head
	poseVector[1]; //torso
	poseVector[2]; //right hand
	poseVector[3]; //left hand
	poseVector[4]; //right leg
	poseVector[5]; //left leg

	poseVector[4] = poseVector[4] * glm::rotate(currentAngle, glm::vec3{1.f,0.f,0.f});
	poseVector[5] = poseVector[5] * glm::rotate(-currentAngle, glm::vec3{1.f,0.f,0.f});

}

gl2d::Texture loadPlayerSkin(const char *path)
{
	//todo implement
	std::ifstream file(path, std::ios::binary);

	if (!file.is_open())
	{
		return {};
	}

	int fileSize = 0;
	file.seekg(0, std::ios::end);
	fileSize = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	unsigned char *fileData = new unsigned char[fileSize];
	file.read((char *)fileData, fileSize);
	file.close();

	gl2d::Texture texture;
	{
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channels = 0;

		const unsigned char *decodedImage = stbi_load_from_memory(fileData, (int)fileSize, &width, &height, &channels, 4);

		if (width == height && width == PLAYER_SKIN_SIZE)
		{
			texture.createFromBuffer((const char *)decodedImage, width, height, true, true);
		}
		else if(width == height * 2 && width == PLAYER_SKIN_SIZE)
		{
			std::vector<char> newData;
			newData.resize(width * height * 4 * 2);
			memcpy(newData.data() + width * height * 4, decodedImage, width * height * 4);
			texture.createFromBuffer(newData.data(), width, height * 2, true, true);
		}

		STBI_FREE(decodedImage);

	}

	delete[] fileData;

	return texture;
}

void Model::cleanup()
{
	glDeleteBuffers(1, &indexBuffer);
	glDeleteBuffers(1, &geometry);
	glDeleteVertexArrays(1, &vao);

	*this = {};
}

glm::mat4 BoneTransform::getPoseMatrix()
{
	auto poseMatrix = glm::mat4(1.f);
	poseMatrix = glm::translate(position) 
		* glm::toMat4(rotation);

	return poseMatrix;
}

bool BoneTransform::goTowards(BoneTransform &other, float speed)
{
	// 1. Move position towards the target at a constant speed
	glm::vec3 direction = other.position - position;
	float distanceToMove = speed;
	float distanceToTarget = glm::length(direction);

	bool ret = false;

	if (distanceToMove >= distanceToTarget)
	{
		position = other.position; // Clamp to target
		ret = true;
	}
	else
	{
		position += glm::normalize(direction) * distanceToMove;
	}

	// 2. Rotate towards the target at a constant angular speed
	float dot = glm::dot(rotation, other.rotation);

	// Clamp dot to prevent errors due to floating point precision
	dot = glm::clamp(dot, -1.0f, 1.0f);

	// Calculate the angle between the quaternions
	float angle = std::acos(dot) * 2.0f;

	float angleToMove = speed;

	if (angleToMove >= angle)
	{
		rotation = other.rotation; // Clamp to target
	}
	else
	{
		rotation = glm::slerp(rotation, other.rotation, angleToMove / angle);
		ret = false;
	}

	return ret;
}
