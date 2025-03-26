#include "MyGame.h"

MyGame::MyGame(SDL_Renderer* renderer)
{
    mainFont = TTF_OpenFont(MAIN_FONT_DIRECTORY, 50);

    backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundTempSurface);
    if (!backgroundTexture) std::cerr << "couldn't load background textrue";
    SDL_FreeSurface(backgroundTempSurface);

    ballTexture = SDL_CreateTextureFromSurface(renderer, ballTempSurface);
    if (!ballTexture) std::cerr << "couldn't load ball texture";
    SDL_FreeSurface(ballTempSurface);

    paddleTexture = SDL_CreateTextureFromSurface(renderer, paddleTempSurface);
    if (!paddleTexture) std::cerr << "couldn't load paddle textrue";
    SDL_FreeSurface(paddleTempSurface);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    //std::cout << "dafuq? : " << mainFont;
}

void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {
    if (cmd == "GAME_DATA") {
        // we should have exactly 4 arguments
        if (args.size() == 4) {
            game_data.player1Y = stoi(args.at(0));
            game_data.player2Y = stoi(args.at(1));
            game_data.ballX = stoi(args.at(2));
            game_data.ballY = stoi(args.at(3));
        }
    }
    else if (cmd == "SCORES")
    {
        // we should have exactly 2 scores (duh)
        if (args.size() == 2)
        {
            game_data.player1Score = stoi(args.at(0));
            game_data.player2Score = stoi(args.at(1));
        }
    }
    else if (cmd.find("GAME_WIN") != std::string::npos)
    {
        std::cout << "Game won" << std::endl;
        game_data.gameSequence = 2;
        std::cout << game_data.gameSequence;
        
    }
    else if (cmd.find("GAME_LOSE") != std::string::npos)
    {
        std::cout << "Game lost" << std::endl;
        game_data.gameSequence = 3;
        std::cout << game_data.gameSequence;
        
    }
    else if (cmd.find("GAME_END") != std::string::npos)
    {
        std::cout << "Game ended" << std::endl;
        game_data.gameSequence = 4;
    }
    else if ((cmd == "HIT_WALL_RIGHT" || cmd == "HIT_WALL_LEFT") && getSequence() == 1)
    {
        std::cout << "Played goal sfx";
        sounds = Mix_LoadWAV(SCORE_GOAL_SOUND_DIRECTORY);
        Mix_PlayChannel(-1, sounds, 0);
    }
    else
    {
        std::cout << "Received: " << cmd << std::endl;
    }
}

void MyGame::send(std::string message) {
    messages.push_back(message);
}

void MyGame::input(SDL_Event& event) {
    switch (event.key.keysym.sym) {
    case SDLK_w:
    {
        send(event.type == SDL_KEYDOWN ? "STRT_UP" : "STOP_UP");
        break;
    }
    case SDLK_s:
    {
        send(event.type == SDL_KEYDOWN ? "STRT_DOWN" : "STOP_DOWN");
    }
    }


}

int MyGame::getSequence()
{
    return game_data.gameSequence;
}
void MyGame::setSequence(int value)
{
    game_data.gameSequence = value;
}

void MyGame::update() {
    player1.y = game_data.player1Y;
    player2.y = game_data.player2Y;
    ball.x = game_data.ballX;
    ball.y = game_data.ballY;

    loadText(std::to_string(game_data.player1Score) + "                           " + std::to_string(game_data.player2Score));

    //std::cout << game_data.gameSequence;

    //std::cout << player1.x;

}

void MyGame::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderCopy(renderer, backgroundTexture, &backgroundImgRect, &backgroundRect);
    SDL_RenderCopy(renderer, paddleTexture, &paddleImgSrcRect, &player1);
    SDL_RenderCopy(renderer, paddleTexture, &paddleImgSrcRect, &player2);
    SDL_RenderCopy(renderer, ballTexture, &ballImgSrcRect, &ball);
    renderText(renderer);
}

void MyGame::renderText(SDL_Renderer* renderer)
{
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    //std::cout << textTexture;

    //std::cout << "Surface Width: " << textSurface->w
    //    << ", Height: " << textSurface->h << std::endl;

    if (!textTexture)
        std::cerr << "Failed to load text texture" << SDL_GetError() << std::endl;

    SDL_QueryTexture(textTexture, nullptr, nullptr, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    SDL_FreeSurface(textSurface); SDL_DestroyTexture(textTexture);

}

void MyGame::loadText(const std::string& text)
{
    //std::cout << text;
    //std::cout << mainFont;
    textSurface = TTF_RenderText_Blended_Wrapped(mainFont, text.c_str(), scoreFontColour, 1000);
}