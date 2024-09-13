#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include <gl2d/gl2d.h>
#include <fstream>
#include <shader.h>
#include <camera.h>
#include <deque>


gl2d::Texture colorT;
Shader shader;
Camera c;

#pragma region data


float cubeData[] = {
	//front
	0.5, 0.5, 0.5,    1, 1,   0,0,1,
	-0.5, 0.5, 0.5,   0, 1,   0,0,1,
	-0.5, -0.5, 0.5,  0, 0,   0,0,1,
	-0.5, -0.5, 0.5,  0, 0,   0,0,1,
	0.5, -0.5, 0.5,   1, 0,   0,0,1,
	0.5, 0.5, 0.5,    1, 1,   0,0,1,

	//back
	-0.5, -0.5, -0.5, 0, 0,   0,0,-1,
	-0.5, 0.5, -0.5,  0, 1,   0,0,-1,
	0.5, 0.5, -0.5,   1, 1,   0,0,-1,
	0.5, 0.5, -0.5,   1, 1,   0,0,-1,
	0.5, -0.5, -0.5,  1, 0,   0,0,-1,
	-0.5, -0.5, -0.5, 0, 0,   0,0,-1,

	//top
	-0.5, 0.5, -0.5,  1, 1,   0,1,0,
	-0.5, 0.5, 0.5,   0, 1,   0,1,0,
	0.5, 0.5, 0.5,    0, 0,   0,1,0,
	0.5, 0.5, 0.5,    0, 0,   0,1,0,
	0.5, 0.5, -0.5,   1, 0,   0,1,0,
	-0.5, 0.5, -0.5,  1, 1,   0,1,0,
	 
	//bottom
	0.5, -0.5, 0.5,   1, 1,   0,-1,0,
	-0.5, -0.5, 0.5,  0, 1,   0,-1,0,
	-0.5, -0.5, -0.5, 0, 0,   0,-1,0,
	-0.5, -0.5, -0.5, 0, 0,   0,-1,0,
	0.5, -0.5, -0.5,  1, 0,   0,-1,0,
	0.5, -0.5, 0.5,   1, 1,   0,-1,0,

	//left
	-0.5, -0.5, 0.5,  1, 0,  -1,0,0,
	-0.5, 0.5, 0.5,   1, 1,  -1,0,0,
	-0.5, 0.5, -0.5,  0, 1,  -1,0,0,
	-0.5, 0.5, -0.5,  0, 1,  -1,0,0,
	-0.5, -0.5, -0.5, 0, 0,  -1,0,0,
	-0.5, -0.5, 0.5,  1, 0,  -1,0,0,

	//right
	0.5, 0.5, -0.5,   1, 1,  1,0,0,
	0.5, 0.5, 0.5,    0, 1,  1,0,0,
	0.5, -0.5, 0.5,   0, 0,  1,0,0,
	0.5, -0.5, 0.5,   0, 0,  1,0,0,
	0.5, -0.5, -0.5,  1, 0,  1,0,0,
	0.5, 0.5, -0.5,   1, 1,  1,0,0,
};


#pragma endregion


GLuint cubeVao = 0;
GLuint cubeVbo = 0;

GLint u_viewProjection = 0;
GLint u_modelMatrix = 0;


bool initGame()
{

	glEnable(GL_DEPTH_TEST);


	colorT.loadFromFile(RESOURCES_PATH "cobblestone.png", true, true);

	glGenVertexArrays(1, &cubeVao);
	glBindVertexArray(cubeVao);

	glGenBuffers(1, &cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVao);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *)(sizeof(float) * 5));


	shader.loadShaderProgramFromFile(RESOURCES_PATH "other/cube.vert", RESOURCES_PATH "other/cube.frag");

	u_viewProjection = shader.getUniform("u_viewProjection");
	u_modelMatrix = shader.getUniform("u_modelMatrix");


	return true;
}


//orientates something facing at (0,0,-1) to be at direction
glm::mat4 createTBNMatrix(const glm::vec3 &direction, const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f))
{
	// Normalize the direction vector
	glm::vec3 forward = -glm::normalize(direction);

	// Calculate the right vector
	glm::vec3 right = glm::normalize(glm::cross(up, forward));

	// Recalculate the up vector as it's perpendicular to both forward and right vectors
	glm::vec3 newUp = glm::cross(forward, right);

	// Create a 3x3 rotation matrix using the right, up, and forward vectors
	glm::mat3 rotationMatrix = glm::mat3(right, newUp, forward);

	// Convert to a 4x4 matrix
	return glm::mat4(rotationMatrix);
}


struct UndoRedo
{
	glm::ivec3 newColor = {};
	glm::ivec3 oldColor = {};
	glm::ivec2 position = {};
	int textureIndex = 0;
};

std::deque<UndoRedo> undoRedo;
int undoRedoPosition = 0;

void writeAtIndex(glm::ivec3 color, glm::ivec2 position, int index)
{

	gl2d::Texture t;
	if (index == 0)
	{
		t = colorT;
	}

	glm::ivec2 size = {};
	std::vector<unsigned char> data = t.readTextureData(0, &size);

	int inTexture = (position.y * size.x + position.x) * 4;
	data[inTexture + 0] = color.x; // R
	data[inTexture + 1] = color.y; // G
	data[inTexture + 2] = color.z; // B

	t.cleanup();
	t.createFromBuffer((char *)data.data(), size.x, size.y, true, true);

}

void imageEditor(const char *name, gl2d::Texture &t, int textureIndex)
{
	glm::ivec2 size = {};
	std::vector<unsigned char> data = t.readTextureData(0, &size);
	bool changed = false;

	// Variables for drawing
	static ImVec2 draw_start_pos;
	static bool is_drawing = false;

	// Color picker for brush color
	static ImVec4 brush_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red by default

	ImGui::PushID(name);

	// Display the texture with a fixed size of 512x512
	ImGui::Text("%s", name);
	ImGui::Image((void *)(intptr_t)t.id, ImVec2(512, 512), {0,1}, {1,0}, {1,1,1,1}, 
		{0.5,0.5,0.5,1});

	// Get the image position on the window
	ImVec2 image_pos = ImGui::GetItemRectMin();

	// Color picker
	ImGui::ColorEdit3("Brush Color", (float *)&brush_color);


	// Handle mouse interaction
	//if (ImGui::IsItemHovered())
	{
		ImVec2 mouse_pos = ImGui::GetMousePos();
		ImVec2 uv((mouse_pos.x - image_pos.x) / 512.f, (mouse_pos.y - image_pos.y) / 512.f);
		uv.y = 1 - uv.y;

		int x = static_cast<int>(uv.x * size.x);
		int y = static_cast<int>(uv.y * size.y);

		// Check if the mouse is within the image bounds
		if (x >= 0 && x < size.x && y >= 0 && y < size.y)
		{
			int index = (y * size.x + x) * 4;

			// Left mouse button for painting
			if (ImGui::IsMouseDown(0) || ImGui::IsMouseDragging(0))
			{

				//color picker
				if (platform::isKeyHeld(platform::Button::LeftCtrl))
				{
					brush_color.x = data[index + 0] / 255.f;
					brush_color.y = data[index + 1] / 255.f;
					brush_color.z = data[index + 2] / 255.f;
				}
				else
				{

					if (
						data[index + 0] != brush_color.x * 255.0f &&
						data[index + 1] != brush_color.y * 255.0f &&
						data[index + 2] != brush_color.z * 255.0f
						)
					{
						UndoRedo undo;
						undo.textureIndex = textureIndex;
						undo.position = {x, y};
						undo.oldColor.x = data[index + 0];
						undo.oldColor.y = data[index + 1];
						undo.oldColor.z = data[index + 2];
						undo.newColor.x = brush_color.x * 255.0f;
						undo.newColor.y = brush_color.y * 255.0f;
						undo.newColor.z = brush_color.z * 255.0f;

						if (undoRedoPosition < undoRedo.size())
						{
							undoRedo.erase(undoRedo.begin() + undoRedoPosition, undoRedo.end());
						}

						undoRedo.push_back(undo);
						undoRedoPosition = undoRedo.size();

						//std::cout << "Yess\n";
						is_drawing = true;

						// Set the pixel color
						data[index + 0] = static_cast<unsigned char>(brush_color.x * 255.0f); // R
						data[index + 1] = static_cast<unsigned char>(brush_color.y * 255.0f); // G
						data[index + 2] = static_cast<unsigned char>(brush_color.z * 255.0f); // B
						data[index + 3] = 255; // A
						changed = true;
					}
					
				}

				
			}
		}
	}

	ImGui::PopID();

	if (changed)
	{
		// Recreate the texture from the updated buffer
		t.cleanup();
		t.createFromBuffer((char *)data.data(), size.x, size.y, true, true);
	}
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getWindowSizeX(); //window w
	h = platform::getWindowSizeY(); //window h

	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear screen
#pragma endregion

	c.aspectRatio = ((float)w) / h;
	c.position = {0,0,2};


#pragma region rotate

	{
		static glm::vec2 velocity = {};
		glm::vec2 currentMousePos = platform::getRelMousePosition();

		float speed = 12 * deltaTime;
		c.aspectRatio = (float)w / h;

		glm::vec2 drag = {0.85, 0.85};

		static glm::vec2 angleFromPlanet = {};
		static float distanceFromPlanet = 3.f;

		if (platform::isKeyHeld(platform::Button::W))
		{
			distanceFromPlanet -= deltaTime * 7;
		}
		else if (platform::isKeyHeld(platform::Button::S))
		{
			distanceFromPlanet += deltaTime * 7;
		}

		distanceFromPlanet = std::min(distanceFromPlanet, 10.f);
		distanceFromPlanet = std::max(distanceFromPlanet, 2.f);

		glm::vec2 delta = {};
		{
			static glm::vec2 lastMousePos = {};
			if (platform::isRMouseHeld() || platform::isRMousePressed() || platform::isRMouseReleased())
			{

				delta = lastMousePos - currentMousePos;

			}
			lastMousePos = currentMousePos;
		}

		if (glm::length(velocity) < 1.f)
		{
			float speed = 0.1f;
			velocity += delta * speed * deltaTime;
		}

		velocity *= drag;

		angleFromPlanet -= velocity;

		angleFromPlanet.y = std::min(angleFromPlanet.y, 3.141592f / 2.f);
		angleFromPlanet.y = std::max(angleFromPlanet.y, -3.141592f / 2.f);

		c.position.x = std::cos(angleFromPlanet.x) * std::cos(angleFromPlanet.y);
		c.position.z = std::sin(angleFromPlanet.x) * std::cos(angleFromPlanet.y);
		c.position.y = std::sin(angleFromPlanet.y);
		c.position = glm::normalize(c.position);
		c.position *= distanceFromPlanet;

		c.viewDirection = glm::normalize(-c.position);
	}

#pragma endregion


#pragma region imgui

	ImGui::Begin("Editor");


	//ImGui::Image((ImTextureID)t.id, {512, 512}, {0,1}, {1,0});
	imageEditor("Color", colorT, 0);


	ImGui::End();

	while (undoRedo.size() > 200)
	{
		undoRedo.pop_front();
	}

#pragma endregion

#pragma region undo redo

	if (platform::isKeyHeld(platform::Button::LeftCtrl) &&
		platform::isKeyReleased(platform::Button::Z)
		)
	{
		if (undoRedoPosition > 0)
		{
			undoRedoPosition--;
			writeAtIndex(undoRedo[undoRedoPosition].oldColor, undoRedo[undoRedoPosition].position,
				undoRedo[undoRedoPosition].textureIndex);
		}
	}

	if (platform::isKeyHeld(platform::Button::LeftCtrl) &&
		platform::isKeyReleased(platform::Button::Y))
	{
		if (undoRedoPosition < undoRedo.size())
		{
			writeAtIndex(undoRedo[undoRedoPosition].newColor, undoRedo[undoRedoPosition].position,
				undoRedo[undoRedoPosition].textureIndex);
			undoRedoPosition++;

		}
	}

	if (undoRedoPosition < 0)
	{
		undoRedoPosition = 0;
	}

	if (undoRedoPosition > undoRedo.size())
	{
		undoRedoPosition = undoRedo.size();
	}

#pragma endregion


#pragma region rendering
	shader.bind();
	glBindVertexArray(cubeVao);

	//glm::mat4 model = createTBNMatrix(rotateVector);
	glm::mat4 model = glm::mat4(1.f);
	glm::mat4 viewProj = c.getViewProjectionWithPositionMatrix();
	
	glUniformMatrix4fv(u_modelMatrix, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(u_viewProjection, 1, GL_FALSE, glm::value_ptr(viewProj));
	
	colorT.bind(0);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{


}
