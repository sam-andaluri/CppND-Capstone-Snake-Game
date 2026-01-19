#include "renderer.h"
#include <iostream>
#include <string>

Renderer::Renderer(const std::size_t screen_width,
                   const std::size_t screen_height,
                   const std::size_t grid_width, const std::size_t grid_height)
    : screen_width(screen_width),
      screen_height(screen_height),
      grid_width(grid_width),
      grid_height(grid_height) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create Window
  sdl_window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, screen_width,
                                screen_height, SDL_WINDOW_SHOWN);

  if (nullptr == sdl_window) {
    std::cerr << "Window could not be created.\n";
    std::cerr << " SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create renderer
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  if (nullptr == sdl_renderer) {
    std::cerr << "Renderer could not be created.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }
}

Renderer::~Renderer() {
  SDL_DestroyWindow(sdl_window);
  SDL_Quit();
}

void Renderer::Render(Snake const &player_snake, AISnake const &ai_snake,
                      const std::vector<std::unique_ptr<Food>>& foods,
                      ObstacleManager const &obstacles, bool render_ai) {
  // Clear screen
  SDL_SetRenderDrawColor(sdl_renderer, 0x1E, 0x1E, 0x1E, 0xFF);
  SDL_RenderClear(sdl_renderer);

  // Render obstacles first (background layer)
  RenderObstacles(obstacles);

  // Render all food items
  RenderFoods(foods);

  // Render AI snake only if enabled
  if (render_ai) {
    RenderSnake(ai_snake, false);
  }

  // Render player snake (on top)
  RenderSnake(player_snake, true);

  // Update Screen
  SDL_RenderPresent(sdl_renderer);
}

void Renderer::RenderSnake(Snake const &snake, bool is_player) {
  SDL_Rect block;
  block.w = screen_width / grid_width;
  block.h = screen_height / grid_height;

  // Render snake's body
  if (is_player) {
    // Player body: white
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  } else {
    // AI body: orange
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xA5, 0x00, 0xFF);
  }

  for (SDL_Point const &point : snake.body) {
    block.x = point.x * block.w;
    block.y = point.y * block.h;
    SDL_RenderFillRect(sdl_renderer, &block);
  }

  // Render snake's head
  block.x = static_cast<int>(snake.head_x) * block.w;
  block.y = static_cast<int>(snake.head_y) * block.h;

  if (snake.alive) {
    if (is_player) {
      // Player head: bright blue
      SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x99, 0xFF, 0xFF);
    } else {
      // AI head: purple (clearly different from player)
      SDL_SetRenderDrawColor(sdl_renderer, 0x99, 0x00, 0xFF, 0xFF);
    }
  } else {
    // Dead head: red
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
  }
  SDL_RenderFillRect(sdl_renderer, &block);
}

void Renderer::RenderFoods(const std::vector<std::unique_ptr<Food>>& foods) {
  SDL_Rect block;
  block.w = screen_width / grid_width;
  block.h = screen_height / grid_height;

  for (const auto& food : foods) {
    // Get food color based on type
    Color color = food->GetColor();
    SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, color.a);

    block.x = food->GetX() * block.w;
    block.y = food->GetY() * block.h;
    SDL_RenderFillRect(sdl_renderer, &block);
  }
}

void Renderer::RenderObstacles(ObstacleManager const &obstacles) {
  SDL_Rect block;
  block.w = screen_width / grid_width;
  block.h = screen_height / grid_height;

  for (const auto &obstacle : obstacles.GetObstacles()) {
    // Fixed obstacles: dark gray, Moving obstacles: lighter gray
    if (dynamic_cast<const FixedObstacle*>(obstacle.get())) {
      SDL_SetRenderDrawColor(sdl_renderer, 0x44, 0x44, 0x44, 0xFF);
    } else {
      SDL_SetRenderDrawColor(sdl_renderer, 0x66, 0x66, 0x66, 0xFF);
    }

    for (const auto &cell : obstacle->GetOccupiedCells()) {
      block.x = cell.x * block.w;
      block.y = cell.y * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }
}

void Renderer::UpdateWindowTitle(int player_score, int ai_score, int fps) {
  std::string title{"Snake - You: " + std::to_string(player_score) +
                    " | AI: " + std::to_string(ai_score) +
                    " | FPS: " + std::to_string(fps)};
  SDL_SetWindowTitle(sdl_window, title.c_str());
}
