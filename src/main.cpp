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
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF); //color

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

//closes window
void close()
{

	SDL_DestroyWindow(gWindow);

	gWindow = NULL;

	SDL_Quit();
	//just kill everything to make sure there are no memory leaks
	//what would happen if we didn't? good question!
	//good questions are hard to answer
}

//stupid gay thing we need for textures
/*
class LTexture
{
public:
	LTexture();

	~LTexture();

	bool loadFromFile(std::string path);

	void free();

	void render(int x, int y);

	int getWidth();

	int getHeight();

private:
	SDL_Texture* mTexture;
};

LTexture undertaleMan
*/

int main(int argc, char* args[])
{
	//start the game
	if (!init()) // note that it probably returns false or something when it fails
	{
		printf("failed to intialize! \n");
	}
	else
	{
		bool quitted = false;

		SDL_Event e; //event thingy

		//from this point on, its essentially the "game loop", for the lack of a better term
		while (!quitted)
		{
			while (SDL_PollEvent(&e) != 0)
			{
				if (e.type == SDL_QUIT)
				{
					quitted = true;
				}

				SDL_RenderClear(gRenderer);

			}
		}

		close();


		return 0;
	}
}