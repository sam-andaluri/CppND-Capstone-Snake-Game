#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <memory>
#include "SDL.h"
#include "snake.h"
#include "ai_snake.h"
#include "food.h"
#include "obstacle.h"

class Renderer {
 public:
  Renderer(const std::size_t screen_width, const std::size_t screen_height,
           const std::size_t grid_width, const std::size_t grid_height);
  ~Renderer();

  // Updated render method to handle all game entities
  void Render(Snake const &player_snake, AISnake const &ai_snake,
              const std::vector<std::unique_ptr<Food>>& foods,
              ObstacleManager const &obstacles, bool render_ai = true);

  // Updated to show both player and AI scores
  void UpdateWindowTitle(int player_score, int ai_score, int fps);

 private:
  SDL_Window *sdl_window;
  SDL_Renderer *sdl_renderer;

  const std::size_t screen_width;
  const std::size_t screen_height;
  const std::size_t grid_width;
  const std::size_t grid_height;

  // Helper methods for rendering different entities
  void RenderSnake(Snake const &snake, bool is_player);
  void RenderFoods(const std::vector<std::unique_ptr<Food>>& foods);
  void RenderObstacles(ObstacleManager const &obstacles);
};

#endif
