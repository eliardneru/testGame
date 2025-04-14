//thank you lazyfoo!
#include <stdio.h>
#include <string>
#include <SDL_Image.h>
#include <SDL.h>

//screen dimension constants, using this one because its divisible by 8 or something, plus its not that big
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 576;


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

LTexture undertaleMan;

bool loadMedia()
{
	bool worked = true;

	if (!undertaleMan.loadFromFile("assets/undertaleman.png"))
	{
		printf("failed to foo (whatever that means) texture file!\n");
		worked = false;
	}
	return worked;
}



//closes window and frees stuff
void close()
{
	undertaleMan.free();


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
	//start the game
	if (!init()) // note that it probably returns false or something when it fails
	{
		printf("failed to intialize! \n");
	}
	else
	{
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			bool quitted = false;

			SDL_Event e; //event thingy

			int manPosX = 100;
			int manPosY = 100;
			bool manDirx = false;
			bool manDiry = true;

			//from this point on, its essentially the "game loop", for the lack of a better term
			while (!quitted)
			{
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						quitted = true;
					}
					
				}
				if (manPosX + undertaleMan.getWidth() / 2 > SCREEN_WIDTH || manPosX == 0)
				{
					manDirx = !manDirx;
				}

				if (manPosY + undertaleMan.getHeight() / 2 > SCREEN_HEIGHT || manPosY == 0)
				{
					manDiry = !manDiry;
				}

				if (manDirx)
				{
					manPosX = manPosX + 2;
				}
				else
				{
					manPosX = manPosX - 2;
				}

				if (manDiry)
				{
					manPosY = manPosY + 2;
				}
				else
				{
					manPosY = manPosY - 2;
				}

				SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
				SDL_RenderClear(gRenderer);
				undertaleMan.render(manPosX, manPosY, 0.5);
				SDL_RenderPresent(gRenderer);
				SDL_Delay(16);
			}

			close();

			return 0;
		}
	}
}