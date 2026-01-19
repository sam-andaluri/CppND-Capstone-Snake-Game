#ifndef GAME_H
#define GAME_H

#include <random>
#include <memory>
#include <string>
#include <vector>
#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "snake.h"
#include "food.h"
#include "obstacle.h"
#include "ai_snake.h"

class Game {
 public:
  Game(std::size_t grid_width, std::size_t grid_height, bool enable_ai = true);
  ~Game();

  void Run(Controller const &controller, Renderer &renderer,
           std::size_t target_frame_duration);
  int GetScore() const;
  int GetSize() const;

  // Set player name for high score
  void SetPlayerName(const std::string& name) { player_name_ = name; }
  std::string GetPlayerName() const { return player_name_; }

  // Get AI snake for rendering
  const AISnake& GetAISnake() const { return ai_snake_; }
  bool IsAIEnabled() const { return ai_enabled_; }

  // Get obstacles for rendering
  const ObstacleManager& GetObstacles() const { return *obstacles_; }

  // Get all food items for rendering
  const std::vector<std::unique_ptr<Food>>& GetFoods() const { return foods_; }

 private:
  Snake snake_;
  AISnake ai_snake_;
  std::vector<std::unique_ptr<Food>> foods_;
  std::unique_ptr<ObstacleManager> obstacles_;

  std::random_device dev_;
  std::mt19937 engine_;
  std::uniform_int_distribution<int> random_w_;
  std::uniform_int_distribution<int> random_h_;

  int score_{0};
  int ai_score_{0};
  std::string player_name_;
  bool ai_enabled_{true};

  // Food factory for creating different food types
  FoodFactory food_factory_;

  // Frame counter for food spawning and obstacle updates
  int frame_count_{0};
  static constexpr int kFoodSpawnInterval = 5;
  static constexpr int kObstacleUpdateInterval = 15;
  static constexpr std::size_t kMaxFoodItems = 5;

  void PlaceFood();
  void Update();
  void UpdateAISnake();
  bool IsValidFoodPosition(int x, int y) const;
  void UpdateAIFoodTarget();
};

#endif
