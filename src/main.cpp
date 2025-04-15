//thank you lazyfoo!
#include <stdio.h>
#include <string>
#include <SDL_Image.h>
#include <SDL.h>

//screen dimension constants, using this one because its divisible by 8 or something, plus its not that big
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 576;
const float GRAVITY = 1960; //9.8 m/s * 2
const float TERMINAL_VELOCITY = 10720; // 120 mph in centimeters * 2

//global vars, some say its bad but i think its ok
SDL_Window* gWindow = NULL;

SDL_Renderer* gRenderer = NULL;

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
LTexture angryPlayer;

bool loadMedia()
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
void close()
{
	angryPlayer.free();


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


int main(int argc, char* args[])
{
	printf("im cumming!!!!\n");
	Uint32 lastTick = SDL_GetTicks(); //check current tick, useful for timers and fps
									  //ticks and frames are kinda of the same here
									  
	
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

			bool quitted = false; //check if the game is running

			SDL_Event e; //event thingy

			Uint32 currentTick = SDL_GetTicks(); //get current tick
			float deltaTime = (currentTick - lastTick) / 1000.0f; //calculate delta time by calculating time between ticks
			lastTick = currentTick; //update when the last tick was

			float fpsAvg = 0; //our fps
			float tempAvgFps = 0; //gay thing to see fps
			float secondCnt = 0; // how long since the last second

			float playerPosX = 100; float playerPosY = 0; //player position, duh
			bool changedX = false;  bool changedY = false; //depecrated, used in undertaleman
			float velPlayerX = 0;   float velPlayerY = 0; // velocity, how much should the player move essentially
			float acelPlayerX = 0;  float acelPlayerY = 0; //acceleration, different from velocity cuz pysichs

			
			while (!quitted) //while its running, do stuff
			{
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						quitted = true;
					}

				}
				//from this point on, its essentially the "game loop", for the lack of a better term

				/*
				changedX = (manPosX < 0) || (manPosX + angryPlayer.getWidth() > SCREEN_WIDTH);
				changedY = (manPosY < 0) || (manPosY + angryPlayer.getHeight() > SCREEN_HEIGHT);
				if (changedX) { velPlayerX *= -1; changedX = false; }
				if (changedY) { velPlayerY *= -1; changedY = false; }
				manPosX = manPosX + velPlayerX; manPosY = manPosY + velPlayerY;
				*/

				currentTick = SDL_GetTicks(); //same thing but in the game loop
				deltaTime = (currentTick - lastTick) / 1000.0f; //get dt
				lastTick = currentTick; //update when the last tick was

				if (secondCnt <= 1) { secondCnt += deltaTime; tempAvgFps++; } //if a second hasn't passed, add a frame
				else { fpsAvg = tempAvgFps; secondCnt = 0; tempAvgFps = 0;  } //else, this is our fps, reset

				SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0); SDL_RenderClear(gRenderer);

				playerPosY += velPlayerY * deltaTime * 0.5f;
				velPlayerY += GRAVITY * deltaTime;
				playerPosY += velPlayerY * deltaTime * 0.5f;
				//i add the velocity before and afterwards to "avarage" how much i need
				//im too lazy to explain what that means, just watch jonas tyroller's video on delta time
				//
				printf("%f\n", fpsAvg);
				angryPlayer.render(playerPosX, playerPosY, 1);

				SDL_RenderPresent(gRenderer);
			}

			close();

			return 0;
		}
	}
}