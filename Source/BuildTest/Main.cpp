#include <iostream>

#include <SDL3/SDL.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <glad/gl.h>
#include <windows.h>
#include <imgui.h>

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = SDL_CreateWindow(
        "[glad] GL with SDL",
        800, 600,
        SDL_WINDOW_OPENGL
    );

    SDL_GLContext context = SDL_GL_CreateContext(window);

    int version = gladLoadGL(SDL_GL_GetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    std::cout << glGetString(GL_VENDOR) << "\n";
    std::cout << glGetString(GL_RENDERER) << "\n";
    std::cout << glGetString(GL_VERSION) << std::endl;

    int exit = 0;
    while(!exit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT:
                    exit = 1;
                break;
                case SDL_EVENT_KEY_UP:
                    if (event.key.key == SDLK_ESCAPE) {
                        exit = 1;
                    }
                break;
                default:
                    break;
            }
        }

        glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);
        SDL_Delay(1);
    }

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}