//thank you lazyfoo!
#include <stdio.h>
#include <string>
#include <SDL_Image.h>
#include <SDL.h>

//global constants 
const int SCREEN_WIDTH = 1024; //screen dimension constants, using this one because its divisible by 8 or something, plus its not that big
const int SCREEN_HEIGHT = 576;
const float GRAVITY = 980; //FIXME: 9.8 m/s * 2, or not, from what it seems, i fucked up something but i want too lazy to find out what right now as i just refactored this whole thing
const float TERMINAL_VELOCITY = 10720; // 120 mph in centimeters * 2
const float FIXED_UPDATE_TIME =  1.0f / 60.0f;

//global sdl stuff
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;


struct gameVars //every game global thing, including physics, fps, etc
{
	Uint32 lastTick; //used for calculating deltaTime
	float deltaTime; //time since last frame

	float fps; //frames per second
	int framesSinceFrameCheck; //used in calculating fps with a delay so you don't do it every frame
	float frameSincePhysicsCheck; //used for calculating physics outisde of framerate
	float alpha;  //used in lerp, may get deprecated if larp does nothing
}; gameVars game;
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
#pragma region Texture Class
class LTexture
{
public:
	LTexture();

	~LTexture();

	bool loadFromFile(std::string path);

	void free();

	void render(int x, int y, float size);

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

void LTexture::render(int x, int y, float size)
{
	int newSize[2] = { mWidth * size, mHeight * size };
	SDL_Rect renderQuad = { x, y, newSize[0], newSize[1] };
	SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

#pragma endregion

LTexture angryPlayer; //i wonder if texture and player should be one thing

class player {
public:
	float x, y; //final position of player, used to render, will be lerped
	float velPlayerX = 0;   float velPlayerY = 0; // velocity, how much should the player move essentially
	float acelPlayerX = 0;  float acelPlayerY = 0; //acceleration, different from velocity cuz pysichs, gets added to velocity

	float prePlayerPosX; float prePlayerPosY; // used in interpolation

	player(float startX, float startY)
	: x(startX), y(startY){}
	void update(float deltaTime);
	void render(float size);
};

void player::update(float deltaTime) // updates player physics, should be called as many times as needed per second, use timestep if you want it consistent
{
	y += velPlayerY * deltaTime * 0.5f;
	x += velPlayerX * deltaTime * 0.5f;
	velPlayerX *= 0.97f; //air resistence
	acelPlayerX *= 0.97f; acelPlayerY *= 0.97f;
	velPlayerY += GRAVITY * deltaTime;
	velPlayerX += acelPlayerX * deltaTime;
	velPlayerY += acelPlayerY * deltaTime;
	if (velPlayerY >= TERMINAL_VELOCITY) { velPlayerY = TERMINAL_VELOCITY; }
	y += velPlayerY * deltaTime * 0.5f;
	x += velPlayerX * deltaTime * 0.5f;
}

void player::render(float size) //FIXME: a function that just calls another function is kinda of retarded, really retarded actually but what can we do
{
	angryPlayer.render(x, y, size);
}

bool loadMedia() //FIXME?: i think this is ok if i load in sprite sheets, otherwise it will suck ass 
{
	bool worked = true;

	if (!angryPlayer.loadFromFile("assets/angrydude.png"))
	{
		printf("failed to foo (whatever that means) texture file!\n");
		worked = false;
	}
	return worked;
}



//closes window and frees stuff
void close() //closes the game
{
	angryPlayer.free(); //another bad thing, should just free all possible, maybe add some sort of function that just frees as much stuff as possible?


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

Uint32 calcDeltaTime(gameVars &game) //calculates deltaTime
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

void gameLoop()
{
	bool quitted = false; //check if the game is running
	SDL_Event e; //event thingy
	player angryPlayer(200, -200); //this essentially spawns the player
	int secondsPassed = 0;


	while (!quitted) //while its running, do stuff
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quitted = true;
			}

		}
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0); SDL_RenderClear(gRenderer); // clear screen so we can start again
		calcDeltaTime(game); // calculate deltaTime duh,

		game.alpha = game.frameSincePhysicsCheck / FIXED_UPDATE_TIME; //calculate alpha, used in lerp, might get removed
		game.frameSincePhysicsCheck += game.deltaTime; //add to physics so we can known when to update physics stuff

		while (game.frameSincePhysicsCheck >= FIXED_UPDATE_TIME) //calc physics in fixed time step
		{
			angryPlayer.update(FIXED_UPDATE_TIME); //bad thing, should put all players into a vector and then spawn then on demand
			game.frameSincePhysicsCheck -= FIXED_UPDATE_TIME; //reset
		}

		if (calcFps(game, 60)) { printf("fps: %.0f\n", game.fps); printf("dt: %.0f\n", game.deltaTime); printf("y: %.0f\n", angryPlayer.y); printf("vel: %.0f\n", angryPlayer.velPlayerY); secondsPassed++; printf("seconds: %d\n", secondsPassed); };


		
		game.lastTick = SDL_GetTicks(); //
		angryPlayer.render(1); //renders player with size 1
		SDL_RenderPresent(gRenderer); //do all of that stuff
	}
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
