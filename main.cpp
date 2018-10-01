#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;

GLuint LoadTexture(const char* someImg) {
	int w, h, comp;
	unsigned char* image = stbi_load(someImg, &w, &h, &comp, STBI_rgb_alpha);
	
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Maiky Chen - HW 1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1340, 620, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	//Load images
	GLuint catTexture = LoadTexture(RESOURCE_FOLDER"cat.png");
	GLuint batTexture = LoadTexture(RESOURCE_FOLDER"bat.png");
	GLuint moonTexture = LoadTexture(RESOURCE_FOLDER"moon.png");

	glm::mat4 projectionMatrix = glm::mat4(0.5f);
	glm::mat4 viewMatrix = glm::mat4(0.5f);
	glm::mat4 modelMatrix = glm::mat4(0.5f);

	projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;

	//Enabling blending
	glEnable(GL_BLEND);
	//Set alpha blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//keeping time set up
	float lastFrameTicks = 0.0f;

	//Transformation variable
	float angle = 0.0f;

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		//Keeping time in loop
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		//Set screen color
		glClearColor(0.5f, 0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		program.SetModelMatrix(modelMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);

		//Binding a texture to a texture target
		glBindTexture(GL_TEXTURE_2D, catTexture);
		float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
		
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		//Translations
		modelMatrix = glm::mat4(1.0f);

		//Untextured triangle
		glBegin(GL_TRIANGLES);
		glColor3f(0.5, 0.2, 0.3);
		glVertex3f(0, 0, 0);
		glVertex3f(1, 0, 0);
		glVertex3f(0, 1, 0);
		glEnd();

		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.75f, -0.5f, 1.0f));
		program.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	
		glBindTexture(GL_TEXTURE_2D, moonTexture);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-angle, 1.0f, -1.0f));
		program.SetModelMatrix(modelMatrix);
		angle += elapsed;
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindTexture(GL_TEXTURE_2D, batTexture);
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, 0.5f, -1.0f));
		program.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
