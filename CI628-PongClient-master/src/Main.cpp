#include "SDL_net.h"

#include "MyGame.h"

using namespace std;

void startServer(bool close);

const char* IP_NAME = "localhost";
const Uint16 PORT = 55555;

MyGame* game;
//GameData gameDataInst;

bool is_running = true;
//int gameSequence = 0;

static int on_receive(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;

    const int message_length = 1024;

    char message[message_length];
    int received;

    // TODO: while(), rather than do
    if (game->getSequence() != 1)
        return 0;

    do {
        received = SDLNet_TCP_Recv(socket, message, message_length);
        message[received] = '\0';

        char* pch = strtok(message, ",");

        // get the command, which is the first string in the message
        string cmd(pch);

        // then get the arguments to the command
        vector<string> args;

        while (pch != NULL) {
            pch = strtok(NULL, ",");

            if (pch != NULL) {
                args.push_back(string(pch));
            }
        }

        game->on_receive(cmd, args);

        if (cmd == "exit") {
            break;
        }

    } while (received > 0 && is_running);

    return 0;
}

static int on_send(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;

    if (game->getSequence() != 1)
        return 0;

    while (is_running) {
        if (game->messages.size() > 0) {
            string message = "CLIENT_DATA";

            for (auto m : game->messages) {
                message += "," + m;
            }

            game->messages.clear();

            cout << "Sending_TCP: " << message << endl;

            SDLNet_TCP_Send(socket, message.c_str(), message.length());
        }

        SDL_Delay(1);
    }

    return 0;
}

void loop(SDL_Renderer* renderer) {
    SDL_Event event;

    while (is_running) {
        
        //std::cout << game_data.gameSequence;


        while (SDL_PollEvent(&event)) {
            
            if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
                game->input(event);

                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        is_running = false;
                        break;

                    default:
                        break;
                }
            }
            if(game->getSequence() == 0 || game->getSequence() == 2 || game->getSequence() == 3 || game->getSequence() == 4)
            {
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
                {
                    game->setSequence(1);
                    std::cout << "server started";
                    startServer(false);
                }
            }

            if (event.type == SDL_QUIT) {
                is_running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        switch (game->getSequence())
        {
        case 0:
        {
            startServer(true);

            game->loadText("\n\n\nPress enter to start game...");
            game->renderText(renderer);
            break;
        }
        case 2:
        {
            startServer(true);

            game->loadText("You Win!!\n\n\n\nPress enter to play again...");
            game->renderText(renderer);
            break;
        }
        case 3:
        {
            startServer(true);

            game->loadText("You lose :(\n\n\n\nPress enter to try again...");
            game->renderText(renderer);
            break;
        }
        case 4:
        {
            startServer(true);

            game->loadText("Player " + std::to_string(game_data.player1Score > game_data.player2Score ? 1 : 2) + " wins!\n\n\n\nPress enter to start game...");
            game->renderText(renderer);
            break;
        }
        case 1:
        {
            //game->renderBackground(renderer);
            game->update();
            game->render(renderer);
            break;
        }
        }

        

        //game->update();
        //game->render(renderer);

        //std::cout << game_data.gameSequence;

        SDL_RenderPresent(renderer);

        SDL_Delay(17);
    }
}

int run_game() {
    SDL_Window* window = SDL_CreateWindow(
        "Multiplayer Pong Client",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    if (nullptr == window) {
        std::cout << "Failed to create window" << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    game = new MyGame(renderer);

    if (nullptr == renderer) {
        std::cout << "Failed to create renderer" << SDL_GetError() << std::endl;
        return -1;
    }

    loop(renderer);

    return 0;
}

void startServer(bool close)
{
    TCPsocket socket = nullptr;

    if (!close)
    {
        IPaddress ip;

        // Resolve host (ip name + port) into an IPaddress type
        if (SDLNet_ResolveHost(&ip, IP_NAME, PORT) == -1) {
            printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
            exit(3);
        }

        // Open the connection to the server
        socket = SDLNet_TCP_Open(&ip);

        if (!socket) {
            printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
            exit(4);
        }

        SDL_CreateThread(on_receive, "ConnectionReceiveThread", (void*)socket);
        SDL_CreateThread(on_send, "ConnectionSendThread", (void*)socket);

        game->send("NEW_CONNECTION");

        std::cout << socket << " < socket";

    }

    if (close)
    {
        // Close connection to the server
        if(socket != nullptr)
            SDLNet_TCP_Close(socket);
        return;
    }

}

int main(int argc, char** argv) {


    // Initialize SDL
    if (SDL_Init(0) == -1) {
        printf("SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // Initialize SDL_net
    if (SDLNet_Init() == -1) {
        printf("SDLNet_Init: %s\n", SDLNet_GetError());
        exit(2);
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        return -1;
    }

    //game = new MyGame(renderer);

    //GameData data;
   

    run_game();

    game->shutdownSounds();

    delete game;

    startServer(true);

    // Shutdown SDL_net
    SDLNet_Quit();
    Mix_CloseAudio();

    TTF_Quit();
    SDL_Quit();

    // Shutdown SDL
    SDL_Quit();

    return 0;
}



