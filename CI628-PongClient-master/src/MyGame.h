#pragma once
#ifndef __MY_GAME_H__
#define __MY_GAME_H__

#include <iostream>
#include <vector>
#include <string>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL.h>

#include "MainUtils.h"

static struct GameData {
    int player1Y = 0;
    int player2Y = 0;
    int ballX = 0;
    int ballY = 0;
    int player1Score = 0;
    int player2Score = 0;
    int gameSequence = 0;
} game_data;

class MyGame {

    private:
        SDL_Rect player1 = { SCREEN_WIDTH / 5, SCREEN_HEIGHT / 2 - 30, 20, 60};
        SDL_Rect player2 = { 4 * SCREEN_WIDTH / 5 - 20, SCREEN_HEIGHT / 2 - 30, 20, 60 };
        SDL_Rect ball = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 18, 18 };

        //Loads font
        TTF_Font* mainFont = nullptr;
        SDL_Color scoreFontColour = { 255, 255, 255, 255 };

        //char* basePath = SDL_GetBasePath();
        //std::string MAIN_FONT_DIRECTORY = std::string(basePath) + "../Resources/Fonts/pong.ttf";
        const char* MAIN_FONT_DIRECTORY = "Resources/Fonts/pong.ttf";
        const char* BACKGROUND_IMAGE_DIRECTORY = "Resources/Images/BackgroundBeta.png";

        //Loads text texture
	    SDL_Surface* textSurface = nullptr;
	    SDL_Texture* textTexture = nullptr;
	    SDL_Rect textRect = {SCREEN_WIDTH / 20, SCREEN_HEIGHT / 20, SCREEN_WIDTH, SCREEN_HEIGHT};

        //loads background texture
        SDL_Texture* backgroundTexture = nullptr;
        SDL_Surface* backgroundTempSurface = IMG_Load(BACKGROUND_IMAGE_DIRECTORY);
        SDL_Rect backgroundImgRect = { 0, 0, 120, 72 };
        SDL_Rect backgroundRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

        //Loads the ball texture
        const char* BALL_IMAGE_DIRECTORY = "Resources/Images/Ball.png";
        SDL_Texture* ballTexture = nullptr;
        SDL_Surface* ballTempSurface = IMG_Load(BALL_IMAGE_DIRECTORY);
        SDL_Rect ballImgSrcRect = { 0, 0, 8, 8 };

        //Loads the paddle texture
        const char* PADDLE_IMAGE_DIRECTORY = "Resources/Images/Paddle.png";
        SDL_Texture* paddleTexture = nullptr;
        SDL_Renderer* rendererInstance = nullptr;
        SDL_Surface* paddleTempSurface = IMG_Load(PADDLE_IMAGE_DIRECTORY);
        SDL_Rect paddleImgSrcRect = { 0, 0, 8, 64 };

        Mix_Chunk* sounds = nullptr;

        const char* WALL_BOUNCE_SOUND_DIRECTORY = "Resources/Audio/wallBounce.wav";
        const char* PADDLE_BOUNCE_SOUND_DIRECTORY = "Resources/Audio/paddleBounce.wav";
        const char* SCORE_GOAL_SOUND_DIRECTORY = "Resources/Audio/scoreGoal.wav";


    public:
        MyGame(SDL_Renderer* renderer);

        std::vector<std::string> messages;

        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        void update();
        void render(SDL_Renderer* renderer);

        void loadText(const std::string& text);
        void renderText(SDL_Renderer* renderer);
        void setFont();
        int getSequence();
        void setSequence(int value);

        void shutdownSounds() { Mix_FreeChunk(sounds); }

};

#endif