#include <rendering/model.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

void ModelsManager::loadAllModels()
{


	Assimp::Importer importer;

	// Step 2: Specify Import Options
	unsigned int flags = aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_FindInstances;

	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Remove lines and points
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS);



	// Step 3: Load the Model
	const aiScene *scene = importer.ReadFile(RESOURCES_PATH "models/human.glb", flags);


	struct Data
	{
		glm::vec3 position = {};
		glm::vec3 normal = {};
		glm::vec2 uv = {};
	};

	std::vector<Data> vertexes;
	vertexes.reserve(400);

	std::vector<unsigned int> indices;
	indices.reserve(400);

	if (scene)
	{

		glGenVertexArrays(1, &human.vao);
		glBindVertexArray(human.vao);

		glGenBuffers(1, &human.geometry);
		glGenBuffers(1, &human.vao);

		glBindBuffer(GL_ARRAY_BUFFER, human.geometry);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, human.vao);

		
		std::cout << "yessss :)))\n";
		
		
		std::cout << "count : " << scene->mNumMeshes << "\n";

		unsigned int vertexOffset = 0;

		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			const aiMesh *mesh = scene->mMeshes[i];

			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
			{
				Data vertex;

				// Positions
				vertex.position.x = mesh->mVertices[i].x;
				vertex.position.y = mesh->mVertices[i].y;
				vertex.position.z = mesh->mVertices[i].z;

				// Normals
				vertex.normal.x = mesh->mNormals[i].x;
				vertex.normal.y = mesh->mNormals[i].y;
				vertex.normal.z = mesh->mNormals[i].z;

				// UVs (if present)
				if (mesh->HasTextureCoords(0))
				{
					vertex.uv.x = mesh->mTextureCoords[0][i].x;
					vertex.uv.y = mesh->mTextureCoords[0][i].y;
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

		human.vertexCount = vertexOffset;

		glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(Data), vertexes.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));


		glBindVertexArray(0);

	}
	else
	{
		std::cout << "noSchene:(((((\n";
		std::cout << importer.GetErrorString() << "\n";
	}


	importer.FreeScene();


}
