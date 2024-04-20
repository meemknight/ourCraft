#include <rendering/model.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp> // for glm::make_mat4

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

void ModelsManager::loadAllModels()
{


	//load textures
	{

		steveTexture.loadFromFile(RESOURCES_PATH "models/steve.png", true, false);
		//steveTexture.loadFromFile(RESOURCES_PATH "models/otherskin.png", true, false);
		
		if (steveTexture.id)
		{

			steveTexture.bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 6.f);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 6.f);

			glGenerateMipmap(GL_TEXTURE_2D);

			steveTextureHandle = glGetTextureHandleARB(steveTexture.id);
			glMakeTextureHandleResidentARB(steveTextureHandle);
			std::cout << "Loaded steve texture!\n";
		}
		else
		{
			std::cout << "Error steve texture!\n";
		}

	}


	Assimp::Importer importer;

	// Step 2: Specify Import Options
	unsigned int flags = aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_FindInstances;

	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Remove lines and points
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS);



	//const aiScene *scene = importer.ReadFile(RESOURCES_PATH "models/easymodel.glb", flags);
	const aiScene *scene = importer.ReadFile(RESOURCES_PATH "models/human.glb", flags);


	struct Data
	{
		glm::vec3 position = {};
		glm::vec3 normal = {};
		glm::vec2 uv = {};
		int boneIndex = 0;
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
		glGenBuffers(1, &human.indexBuffer);

		glBindBuffer(GL_ARRAY_BUFFER, human.geometry);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, human.indexBuffer);

		
		
		std::cout << "count : " << scene->mNumMeshes << "\n";

		std::cout << "childrens : " << scene->mRootNode->mNumChildren << "\n";

		unsigned int vertexOffset = 0;

		for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; ++i)
		{
			
			const aiNode *node = scene->mRootNode->mChildren[i];
			//const aiMesh *mesh = scene->mMeshes[i];
			const aiMesh *mesh = scene->mMeshes[node->mMeshes[0]];

			//const aiNode *node = scene->mRootNode->FindNode(mesh->mName);
			//const aiNode *node = findNodeContainingMesh(scene->mRootNode, mesh);
			if (!node)
			{ std::cout << "no no no!\n"; }
			aiMatrix4x4 transform = node->mTransformation;
			human.transforms.push_back(glm::transpose(aiToGlm(transform)));

			std::cout << node << "\n";
			std::cout << aiToGlm(transform)[0][0] << " " << aiToGlm(transform)[0][1] << " " << aiToGlm(transform)[0][2] << " " << aiToGlm(transform)[0][3] << '\n';
			std::cout << aiToGlm(transform)[1][0] << " " << aiToGlm(transform)[1][1] << " " << aiToGlm(transform)[1][2] << " " << aiToGlm(transform)[1][3] << '\n';
			std::cout << aiToGlm(transform)[2][0] << " " << aiToGlm(transform)[2][1] << " " << aiToGlm(transform)[2][2] << " " << aiToGlm(transform)[2][3] << '\n';
			std::cout << aiToGlm(transform)[3][0] << " " << aiToGlm(transform)[3][1] << " " << aiToGlm(transform)[3][2] << " " << aiToGlm(transform)[3][3] << '\n';

			std::cout << "-----\n";

			for (unsigned int m = 0; m < mesh->mNumVertices; ++m)
			{
				Data vertex;

				vertex.boneIndex = i;

				// Positions
				vertex.position.x = mesh->mVertices[m].x;
				vertex.position.y = mesh->mVertices[m].y;
				vertex.position.z = mesh->mVertices[m].z;

				// Normals
				vertex.normal.x = mesh->mNormals[m].x;
				vertex.normal.y = mesh->mNormals[m].y;
				vertex.normal.z = mesh->mNormals[m].z;

				// UVs (if present)
				if (mesh->HasTextureCoords(0))
				{
					vertex.uv.x = mesh->mTextureCoords[0][m].x;
					vertex.uv.y = mesh->mTextureCoords[0][m].y;
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
		

		human.vertexCount = indices.size();

		std::cout << "Vertexes: " << vertexes.size() << "\n";
		std::cout << "Indices: " << indices.size() << "\n";
		std::cout << "Matrixes: " << human.transforms.size() << "\n";
		glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(Data), vertexes.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*9, 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 6));
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 1, GL_INT, sizeof(float) * 9, (void *)(sizeof(float) * 8));


		glBindVertexArray(0);

	}
	else
	{
		std::cout << "noSchene:(((((\n";
		std::cout << importer.GetErrorString() << "\n";
	}


	importer.FreeScene();


}
