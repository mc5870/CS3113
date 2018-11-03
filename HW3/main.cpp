

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include <vector>

ShaderProgram program;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
SDL_Event event;

bool done = false;

float elapsed;
float lastFrameTicks = 0.0f;
SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
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

class SheetSprite {
public:
	SheetSprite() {}
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size)
		: textureID(textureID), u(u), v(v), width(width), height(height) {}

	void DrawSpriteSheetSprite(ShaderProgram &program, int index, int spriteCountX, int spriteCountY) {     
		float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;     
		float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;     
		float spriteWidth = 0.8/(float)spriteCountX;     
		float spriteHeight = 1.0/(float)spriteCountY;          
		float texCoords[] = {         
			u, v+spriteHeight,         
			u+spriteWidth, v,         
			u, v,         
			u+spriteWidth, v,         
			u, v+spriteHeight,         
			u+spriteWidth, v+spriteHeight     
			};      
		float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,  -0.5f, -0.5f, 0.5f, -0.5f}; 
		// draw our arrays

		program.SetModelMatrix(modelMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);
		
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

	}
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);

};

class Entity {
public:
	Entity(){}
	Entity(float x, float y, float z):position(x,y,z){
		size.x = 1.0;
		size.y = 1.0;
		size.z = 1.0;
	}

	void Draw(ShaderProgram &program){		
		sprite.DrawSpriteSheetSprite(program,size.x,size.y,size.z);
	}

	SheetSprite sprite;

	glm::vec3 position;
	glm::vec3 size;
	glm::vec3 velocity;	
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	
	bool isDead = false;
};

class Text :public Entity {
public:
	Text() :Entity() {}
	Text(unsigned int texture) {
		textureID = texture;
	}

	GLuint textureID;

	void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
		float character_size = 1.0 / 16.0f;
		std::vector<float> vertexData;
		std::vector<float> texCoordData;

		for (int i = 0; i < text.size(); i++) {
			int spriteIndex = (int)text[i];
			float texture_x = (float)(spriteIndex % 16) / 16.0f;
			float texture_y = (float)(spriteIndex / 16) / 16.0f;
			vertexData.insert(vertexData.end(), { ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
				((size + spacing) * i) + (-0.5f * size), -0.5f * size,
				((size + spacing) * i) + (0.5f * size), 0.5f * size,
				((size + spacing) * i) + (0.5f * size), -0.5f * size,
				((size + spacing) * i) + (0.5f * size), 0.5f * size,
				((size + spacing) * i) + (-0.5f * size), -0.5f * size, });

			texCoordData.insert(texCoordData.end(), { texture_x, texture_y,
				texture_x, texture_y + character_size,
				texture_x + character_size, texture_y,
				texture_x + character_size, texture_y + character_size,
				texture_x + character_size, texture_y,
				texture_x, texture_y + character_size, });
		}
		glBindTexture(GL_TEXTURE_2D, fontTexture);

		// draw this data (use the .data() method of std::vector to get pointer to data) 
		// draw this yourself, use text.size() * 6 or vertexData.size()/2 to get number of vertices } 

		program.SetModelMatrix(modelMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, text.length() * 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}
};

Text text;

class Player : public Entity {
public:
	Player(){}
	Player(float x, float y, float z) : Entity(x, y, z) {
		size.x = x;
		size.y = y;
		size.z = z;
	}
};

class Enemy : public Entity {
public:
	Enemy(){}
	Enemy(float x, float y, float z) : Entity(x, y, z) {
		size.x = x;
		size.y = y;
		size.z = z;
	}

	void Update(float elapsed) {
		
	}
};

class Bullet : public Entity{
public:
	Bullet() {}
	Bullet(float x, float y, float z) : Entity(x, y, z) {
		size.x = x;
		size.y = y;
		size.z = z;
	}
};

#define MAXBULLETS 30
int bulletIndex = 0;

class GameState {
public:
	GameState() {}
	Player player;
	Enemy enemies[12];
	Bullet bullets[MAXBULLETS];
	int score;

	void ProcessEvents();

	void shootBullet(Entity &entity) {
		bullets[bulletIndex].position.x = entity.position.x;
		bullets[bulletIndex].position.y = entity.position.y;

		bulletIndex++;
		if (bulletIndex > MAXBULLETS - 1) {
			bulletIndex = 0;
		}
		if (&entity == &player) {
			//player shot the bullet, check if enemy got hit
			bullets[bulletIndex].position.y = elapsed * 2.0f;
			for (int i = 0; i < sizeof(enemies); i++){
				if (!enemies[i].isDead) {
					// loop through every enemy that is still alive and check if any were hit by the bullet
					if (bullets[bulletIndex].position.x == enemies[i].size.x / 2
						&& bullets[bulletIndex].position.x <= enemies[i].position.x + enemies[i].size.x / 2
						&& bullets[bulletIndex].position.y >= enemies[i].position.y - enemies[i].size.x / 2
						&& bullets[bulletIndex].position.y <= enemies[i].position.y + enemies[i].size.y / 2) {
						enemies[i].isDead = true;
						bullets[bulletIndex].position.y = -2000.0f;
						bullets[bulletIndex].velocity.y = 0.0f;
					}
				}
				else if (bullets[bulletIndex].position.y >= 1.25f) {
					//bullet did not hit any enemy, make it disappear
					bullets[bulletIndex].position.y = -2000.0f;
				}
			}
		}
		else{
			//if entity is not player, then it is one of the enemies shooting at player. Check if it hits
			bullets[bulletIndex].position.y -= elapsed * 2.0f;
			if (bullets[bulletIndex].position.x == player.size.x / 2
				&& bullets[bulletIndex].position.x <= player.position.x + player.size.x / 2
				&& bullets[bulletIndex].position.y >= player.position.y - player.size.x / 2
				&& bullets[bulletIndex].position.y <= player.position.y + player.size.y / 2) {
				player.isDead = true;
				bullets[bulletIndex].position.y = -2000.0f;
				bullets[bulletIndex].velocity.y = 0.0f;
				text.DrawText(program, text.textureID, "Game Over", 0.1, 0.1);
				
			}
			else if (bullets[bulletIndex].position.y >= 1.25f) {
				bullets[bulletIndex].position.y = -2000.0f;
		}
	}
}

	void UpdateGame(float elapsed) {
		// move all the enemies based on time elapsed and their velocity
		if (enemies[0].position.x <= -0.85) {
			//if first enemy is near left wall, decrease y position and start moving to the right
			for (int i = 0; i < sizeof(enemies); i++) {
				enemies[i].position.y -= 0.1;
				enemies[i].position.x += elapsed * 0.01;
			}
		}
			
		else if (enemies[11].position.x >= 0.85) {
			//if last enemy is near right wall, decrease all enemies' y position and start moving to the left
			for (int i = 0; i < sizeof(enemies); i++) {
				enemies[i].position.y -= 0.1;
				enemies[i].position.x -= elapsed * 0.01;
			}
		}		
	}

	void RenderGame() {
		// render all the entities in the game
		GLuint spriteSheetTexture = LoadTexture("space_invaders.png");
		for (int i = 0; i < 12; i++) {
			Enemy newEnemy;
			enemies[i] = newEnemy;
		}
		for (int i = 0; i < 30; i++) {
			Bullet newBullet;
			bullets[i] = newBullet;
		}
		Player myPlayer;
		player = myPlayer;

		player.sprite.DrawSpriteSheetSprite(program, 2, 3, 1);

		for (int i = 0; i < sizeof(enemies); i++) {
			enemies[i].sprite.DrawSpriteSheetSprite(program, 1, 4, 1);
		}
		for (int i = 0; i < sizeof(bullets); i++) {
			bullets[i].sprite.DrawSpriteSheetSprite(program, 11, 6, 1);

		}
	}
};

void setUp() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1720, 640, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif		
}

GameState state;

void GameState:: ProcessEvents() {
	bool done = false;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				// DO AN ACTION WHEN SPACE IS PRESSED!
				shootBullet(state.player);
			}
			else if (keys[SDL_SCANCODE_LEFT]) {
				// go left, check if player is touching left side of wall, if it is, don't change position. Otherwise allow movement
				if (player.position.x <= -0.85)
					player.position.x = player.position.x;
				else
					player.position.x -= state.player.position.x * elapsed * 0.01;

			}
			else if (keys[SDL_SCANCODE_RIGHT]) {
				// go right, check if player is touching right side of wall, if it is, don't change position. Otherwise allow movement
				if (player.position.x >= 0.85)
					player.position.x = player.position.x;
				else
					player.position.x += player.position.x * elapsed * 0.01;
			}
		}
	}
}

int mode = 0;

void RenderMainMenu() {
	text.textureID = LoadTexture("pixel_font.png");
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0f);

	text.DrawText(program, text.textureID, "Space Invaders", 0.1f, 0.1f);
	text.DrawText(program, text.textureID, "Press S to begin", 0.1f, 0.1f);

	SDL_GL_SwapWindow(displayWindow);
}

void RenderGameLevel(GameState &state) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0f);
	text.DrawText(program, text.textureID, "Score", 0.1f, 0.1f);
	text.DrawText(program, text.textureID, "0000", 0.1f, 0.1f);
	state.RenderGame();
	SDL_GL_SwapWindow(displayWindow);
}

void UpdateMainMenu(float elapsed) {
	state.player.position.x = 0.0f;
	state.player.position.y = -0.85f;
	state.player.size.x = 0.25f;
	state.player.size.y = 0.25f;
}

void UpdateGameLevel(GameState &state, float elapsed) {
	while (!state.player.isDead) {
		state.UpdateGame(elapsed);
	}
	RenderMainMenu();
}

void ProcessMainMenuInput() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
			mode = 1;
		}
	}
}

void ProcessGameLevelInput(GameState &state) {
	state.ProcessEvents();
}

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };

void Render() {
	switch (mode) {
	case STATE_MAIN_MENU:
		RenderMainMenu();
		break;
	case STATE_GAME_LEVEL:
		RenderGameLevel(state);
		break;
	}
}
void Update(float elapsed) {
	switch (mode) {
	case STATE_MAIN_MENU:
		UpdateMainMenu(elapsed);
		break;
	case STATE_GAME_LEVEL:
		UpdateGameLevel(state, elapsed);
		break;
	}
}

void ProcessInput() { 
	switch (mode) { 
		case STATE_MAIN_MENU:             
			ProcessMainMenuInput();         
		break;         
		case STATE_GAME_LEVEL:             
			ProcessGameLevelInput(state);         
		break; 
	} 
}

int main(int argc, char *argv[]){
	setUp();
	
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glViewport(0, 0, 640, 360);
	
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		ProcessInput();
		
		Update(elapsed);
		Render();
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}


