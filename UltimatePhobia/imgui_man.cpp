#include "imgui_man.hpp"
#include "global_state.hpp"
#include "safe_path.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>

namespace ImGuiMan {
namespace {
constexpr ImVec4 clear_color = ImVec4(0.1372549019607843f, 0.1215686274509804f, 0.1254901960784314f, 1.00f); // #231f20

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
} // namespace

void init() {
    g.logger->info("Initializing GUI...");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) != 0) {
        g.logger->error("SDL_Init Error: {}", SDL_GetError());
        return;
    }

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow("Phasmophobia", 1280, 800, window_flags);

    if (!window) {
        g.logger->error("SDL_CreateWindow Error: {}", SDL_GetError());
        SDL_Quit();
        return;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        g.logger->error("SDL_CreateRenderer Error: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Set window position and show
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);
}

void deinit() {
    g.logger->info("Deinitializing GUI...");

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}

bool pre_update() {
    // Poll and handle events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
            return false;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
            return false;
    }

    // Handle minimized window
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
        SDL_Delay(10);
        return false;
    }

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    static std::string iniFilename = (SafePath::get() / "imgui.ini").string();
    ImGui::GetIO().IniFilename = iniFilename.c_str();
    ImGui::NewFrame();

    return true;
}

void post_update() {
    // Rendering
    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    SDL_RenderClear(renderer);

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}
} // namespace ImGuiMan
