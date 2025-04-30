//thank you lazyfoo!
#include <stdio.h>
#include <string>
#include <SDL_Image.h>
#include <SDL.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <atomic>
std::atomic<bool> running(true);


//hADSUASODASJDA IOFUCK YOU


//global constants 
const int SCREEN_WIDTH = 1024; //screen dimension constants, using this one because its divisible by 8 or something, plus its not that big
const int SCREEN_HEIGHT = 576;
const float GRAVITY = 780; //FIXME: 9.8 m/s * 2, or not, from what it seems, i fucked up something but i want too lazy to find out what right now as i just refactored this whole thing
const float TERMINAL_VELOCITY = 10720; // 120 mph in centimeters * 2
const float FIXED_UPDATE_TIME = 1.0f / 60.0f;

//global sdl stuff
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

struct gameVars //every game global thing, including physics, fps, etc
{
	Uint32 lastTick = 0; //used for calculating deltaTime
	float deltaTime = 0; //time since last frame

	float fps = 0; //frames per second
	int framesSinceFrameCheck = 0; //used in calculating fps with a delay so you don't do it every frame
	float frameSincePhysicsCheck = 0; //used for calculating physics outisde of framerate
	float alpha;  //used in lerp, may get deprecated if larp does nothing
}; gameVars game;

struct pivot //stupid gay struct we need to declare to make things rotate
{
	int x, y;
};
//inits window
bool init()
{
	bool worked = true; //named it worked instead of success because its easier to type

	if (SDL_Init(SDL_INIT_VIDEO) < 0) //if its less than 0 it means it did not start
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError()); //error handling, duh, quite repetitive piece of code
		bool worked = false;
	}
	else
	{
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) //something something make things look less worse
		{
			printf("Warning: Linear texture filtering is not enabled");
		}
		//Create window
		gWindow = SDL_CreateWindow("TestGame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL) //check if it actually created
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			worked = false;
		}
		else
		{
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED); //-1 just means 'choose whatever'
			if (gRenderer == NULL) //same thing
			{ 
				printf("renderer could not be created! SDL error: %s\n", SDL_GetError());
				worked = false;
			}
			else
			{
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags)) //this is weird but basically it says something like 'if this is not what i wanted, error
				{
					printf("SDL_Image could not be initialized! SDL_Image Error: %s\n", IMG_GetError());
					worked = false;
				}
			}
		}
	}
	return worked;
}

//stupid gay thing we need for textures
#pragma Texture Class{
class LTexture
{
public:
	LTexture();

	~LTexture();

	bool loadFromFile(std::string path);

	void free();

	void render(float x, float y, float size);

	void renderEx(float x, float y, float size, float ang, SDL_Point pivot);

	int getWidth();

	int getHeight();

private:
	SDL_Texture* mTexture;

	int mWidth;
	int mHeight;
};

LTexture::LTexture()
{
	//inits texture
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	free();

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path.c_str());

	if (loadedSurface == NULL)
	{
		printf("unable to load image %s! SDL_Image error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
			
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);

		if (newTexture == NULL)
		{
			printf("unable to load image %s! SDL_Image error: %s\n", path.c_str(), IMG_GetError());
		}
		else
		{
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}
		SDL_FreeSurface(loadedSurface);
	}

	mTexture = newTexture;
	if (!newTexture) { return false; }
	return mTexture != NULL;
}

void LTexture::free()
{
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render(float x, float y, float size)
{
	if (!mTexture) {
		SDL_Log("Attempting to render a null texture!");
		return;
	}

	int newSize[2] = { mWidth * size, mHeight * size };
	SDL_Rect renderQuad = { x, y, newSize[0], newSize[1] };

	if (SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad) < 0)
	{
		printf("\n\nsomething went to shit\n\n");
		printf(SDL_GetError());
	}
}

void LTexture::renderEx(float x, float y, float size, float ang, SDL_Point pivot)
{
	if (!mTexture) {
		SDL_Log("Attempting to render a null texture!");
		return;
	}

	int newSize[2] = { mWidth * size, mHeight * size };
	SDL_Rect renderQuad = { x, y, newSize[0], newSize[1] };

	if (SDL_RenderCopyEx(gRenderer, mTexture, nullptr, &renderQuad, ang, &pivot, SDL_FLIP_NONE) < 0)
	{
		printf("\n\nsomething went to shit\n\n");
		printf(SDL_GetError());
	}

}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

#pragma endregion }

LTexture angryPlayerTex; //it would be good to load it before anything else so its public, honestly this is so fucking retarded its not funny
LTexture shotgunTex;

#pragma region crosshair {

class crosshairA
{
public:
	float x, y; //those are just so i can return the value of the mouse with the crosshair class

	//void update();
	void draw(Uint32 x, Uint32 y);
};

void crosshairA::draw(Uint32 mosx, Uint32 mosy)
{
	x = mosx; y = mosy;
	SDL_Rect r = { mosx, mosy, 5, 5 };
	SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
	SDL_RenderFillRect(gRenderer, &r);
}

#pragma endregion }

#pragma region collider stuff {
class collider
{
public:
	float x, y, w, h;
	collider(float startx, float starty, float startw, float starth)
		: x(startx), y(starty), w(startw), h(starth) {} //this is so retarded [broken heart emoji]
	void draw(float& x, float& y, float& w, float& h, int isVisible);

};

void collider::draw(float& x, float& y, float& w, float& h, int isVisible)
{
	int v;
	SDL_Rect col = { x, y, w, h }; //FIXME: we should probably only set this once, not every frame
	if (isVisible >= 1) { v = 255; }
	else if (isVisible < 0) { v = 0; }
	else { v = 100; } //make so the collider object is visible, partially visible or insivible if asked, ugly hack
	SDL_SetRenderDrawColor(gRenderer, 211, 211, 211, v);
	SDL_RenderFillRect(gRenderer, &col);
}
#pragma endregion }

#pragma region system stuff {
bool loadMedia() //FIXME?: i think this is ok if i load in sprite sheets, otherwise it will suck ass 
{
	bool worked = true;
	if (!angryPlayerTex.loadFromFile("assets/angrydude.png"))
	{
		printf("failed to foo (whatever that means) texture file!\n");
		worked = false;
	}
	if (!shotgunTex.loadFromFile("assets/mossberg590A1.png"))
	{
		printf("failed to foo (whatever that means) texture file!\n");
		worked = false;
	}

	return worked;
}



//closes window and frees stuff
void close() //closes the game
{
	angryPlayerTex.free(); //another bad thing, should just free all possible, maybe add some sort of function that just frees as much stuff as possible?
	shotgunTex.free();
	SDL_DestroyWindow(gWindow);
	SDL_DestroyRenderer(gRenderer);
	gWindow = NULL;
	gRenderer = NULL;

	SDL_Quit();
	IMG_Quit();
	//just kill everything to make sure there are no memory leaks
	//what would happen if we didn't? good question!
	//good questions are hard to answer
}

float lerp(float a, float b, float t) { //deprecated i think, will see if i will remove it when colision and controls are added
	return a + (b - a) * t;
}

Uint32 calcDeltaTime(gameVars& game) //calculates deltaTime
{
	Uint32 currentTick = SDL_GetTicks(); //same thing but in the game loop
	game.deltaTime = (currentTick - game.lastTick) / 1000.0f; //get dt
	game.lastTick = currentTick; //update when the last tick was
	return game.deltaTime;

}

bool calcFps(gameVars& game, int timeToUpdate) //calculates fps using deltaTime, might need to be rounded as its too precise
{
	game.fps = 1.0f / game.deltaTime;

	if (game.framesSinceFrameCheck >= timeToUpdate) { game.framesSinceFrameCheck = 0; return true; }
	game.framesSinceFrameCheck++;
	return false;
}



bool getInputsKB(SDL_Scancode key)
{
	const Uint8* keystates = SDL_GetKeyboardState(nullptr);
	return keystates[key];
}

#pragma endregion }

#pragma region player stuff {
class player {
public:
	float x, y; //final position of player, used to render, will be lerped
	float velPlayerX = 0;   float velPlayerY = 0; // velocity, how much should the player move essentially
	float acelPlayerX = 0;  float acelPlayerY = 0; //acceleration, different from velocity cuz pysichs, gets added to velocity
	bool hasJumped = true;

	float prePlayerPosX; float prePlayerPosY; // used in interpolation, probably deprecated

	player(float startX, float startY, LTexture& playerTex)
		: x(startX), y(startY), playerTex(playerTex) {}
	void update(float deltaTime);
	void render(float size);
	void drawArmsA(LTexture& tex, crosshairA& c);
	void updateInputs(); //updates the player based on inputs
	LTexture playerTex;
	LTexture getTexture();
};

LTexture player::getTexture()
{
	return this->playerTex;
}

void player::update(float deltaTime) // updates player physics, should be called as many times as needed per second, use timestep if you want it consistent
{
	y += velPlayerY * deltaTime * 0.5f;  //add half before and half after, too lazy to explain why go watch a video on deltaTime
	x += velPlayerX * deltaTime * 0.5f;
	velPlayerX *= 0.97f; //air resistence
	acelPlayerX *= 0.97f; acelPlayerY *= 0.97f; //air resistence but acceleration, this is not accurate but it does the job
	velPlayerY += GRAVITY * deltaTime;
	velPlayerX += acelPlayerX * deltaTime;
	velPlayerY += acelPlayerY * deltaTime;
	if (velPlayerY >= TERMINAL_VELOCITY) { velPlayerY = TERMINAL_VELOCITY; }
	y += velPlayerY * deltaTime * 0.5f;
	x += velPlayerX * deltaTime * 0.5f;
}



void player::render(float size) //FIXME: a function that just calls another function is kinda of retarded, really retarded actually but what can we do
{
	angryPlayerTex.render(x, y, size);
}


void player::updateInputs() //updates player based on inputs
{
		//printf("is trying to jump?%d\n",  getInputsKB(SDL_SCANCODE_W));
		SDL_PumpEvents();
		//printf("ok");
		bool wantsWalkR = false;
		bool wantsWalkL = false;
		bool wantsJump = false;
		if (getInputsKB(SDL_SCANCODE_D)) { wantsWalkR = true; }
		if (getInputsKB(SDL_SCANCODE_A)) { wantsWalkL = true; }
		if (getInputsKB(SDL_SCANCODE_W)) { wantsJump = true; }
		if (hasJumped == false && wantsJump == true)
		{
			velPlayerY -= 330;
			hasJumped = true; //TODO: add some sort of delay in jumps, the character being able to bunny hop like a retard is just gay
		}
		if (wantsWalkR) { velPlayerX += 25; }
		if (wantsWalkL) { velPlayerX -= 25; }
		SDL_Delay(16);
}

void player::drawArmsA(LTexture& tex, crosshairA& c)
{
	int rx = x + 16, ry = y + 22;//note arm pos x: x + 16, y = y + 22;
	//FIX ME: this rect is just a place holder variable thingy
	float dx = c.x - rx;
	float dy = c.y - ry;
	float ang = std::atan2(dy, dx);
	float dgr = ang * 180.0f / M_PI;

	SDL_Point pivot;
	pivot.x = 20;
	pivot.y = 4;

	tex.renderEx(rx, ry, 1, dgr, pivot);
}

//all this functions are pretty retarded, but because they work, we are letting then stay here and never touching then again


#pragma endregion }


void checkCollision(player& p, collider& c) //in theory we could just use this function with a loop in all objects that need coliding
{
	//we will be using AABB, its not elegant, please do not put too many colliders, i do not like O(n^2)
	if (p.x < c.x + c.w && p.x + p.playerTex.getWidth() > c.x && p.y < c.y + c.h && p.y + p.playerTex.getHeight() > c.y)
	{
		float aRight = p.x + p.playerTex.getWidth();
		float bRight = c.x + c.w;
		float aLeft = p.x;
		float bLeft = c.x;
		float aBottom = p.y + p.playerTex.getHeight();
		float bBottom = c.y + c.h;
		float aTop = p.y;
		float bTop = c.y;

		//calculate overlap
		float overlapX = std::min(aRight, bRight) - std::max(aLeft, bLeft);
		float overlapY = std::min(aBottom, bBottom) - std::max(aTop, bTop);
		
		if (fabs(overlapX) < fabs(overlapY)) { if (p.velPlayerX > 0) {p.x -= overlapX;} else {p.x+=overlapX;} p.velPlayerX = 0; }
		else { if(p.velPlayerY > 0){p.y -= overlapY;} else {p.y += overlapY;} p.velPlayerY *= 0.1f; p.hasJumped = false; }
	}

}

void gameLoop()
{
	bool quitted = false; //check if the game is running
	SDL_Event e; //event thingy
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND); //makes so transparency works
	player angryPlayer(400, 370, angryPlayerTex); //this essentially spawns the player, i wonder if texture and player should be one thing
	std::vector<collider> colliders = 
	{
		collider(200, 400, 50, 800),
		collider(600, 200, 50, 800),
		collider(SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, 400, 100),
		collider(200, 0, 50, 700),
		collider(200, 350, 700, 20)
	};


	

	crosshairA crosshairA;
	int secondsPassed = 0;
	bool wantsJump = false;
	bool wantsWalkR = false;
	bool wantsWalkL = false;
	int mouseX, mouseY;
	game.lastTick = SDL_GetTicks();
	SDL_ShowCursor(SDL_DISABLE);
	SDL_SetWindowGrab(gWindow, SDL_TRUE);

	while (!quitted) //while its running, do stuff
	{
		while (SDL_PollEvent(&e) != 0)
		{
			//SDL_GetKeyboardState();
			if (e.type == SDL_QUIT)
			{
				quitted = true;
			}

			}
		
		//SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
		//SDL_PumpEvents();

		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0); SDL_RenderClear(gRenderer); // clear screen so we can start again
		calcDeltaTime(game); // calculate deltaTime duh, 
		if (game.deltaTime == 0.0f) { game.deltaTime = 0.0000001f; } //possible fix to window freezes if dragged bug
#ifdef _DEBUG
		game.deltaTime = FIXED_UPDATE_TIME; //DEBUG: if in debug, simulate 60 fps
#endif
		game.frameSincePhysicsCheck = std::min(game.frameSincePhysicsCheck + game.deltaTime, 0.16f); //limit stuff so shit does not go crazy
		game.alpha = game.frameSincePhysicsCheck / FIXED_UPDATE_TIME; //calculate alpha, used in lerp, might get removed
		game.frameSincePhysicsCheck += game.deltaTime; //add to physics so we can known when to update physics stuff

		while (game.frameSincePhysicsCheck >= FIXED_UPDATE_TIME) //calc physics in fixed time step
		{
			angryPlayer.update(FIXED_UPDATE_TIME); //bad thing, should put all players into a vector and then spawn then on demand
			game.frameSincePhysicsCheck -= FIXED_UPDATE_TIME; //reset
		}

		if (calcFps(game, 60)) { printf("\nfps: %.0f\n", game.fps); printf("dt: %f\n", game.deltaTime); printf("y: %.0f\n", 1.0f); printf("vel: %.0f\n", angryPlayer.velPlayerY); secondsPassed++; printf("seconds: %d\n", secondsPassed); };

		for (collider c : colliders)
		{
			c.draw(c.x, c.y, c.w, c.h, 1);
			checkCollision(angryPlayer, c);
		}

		SDL_GetMouseState(&mouseX, &mouseY);
		crosshairA.draw(mouseX, mouseY);
		angryPlayer.render(1); //renders player with size 1
		angryPlayer.drawArmsA(shotgunTex, crosshairA);
		angryPlayer.updateInputs();
		SDL_RenderPresent(gRenderer); //do all of that stuff

	}
	running = false;
}


int main(int argc, char* args[])
{
	

	if (!init()) //start the game
	{
		printf("failed to intialize! \n");
	}
	else
	{
		if (!loadMedia()) //load stuff
		{
			printf("Failed to load media!\n");

		}
		else
		{
			SDL_RenderSetVSync(gRenderer, 1); // could check if vsync failed but i won't

		}

		gameLoop();
		
		close();

		return 0;
	}
}
