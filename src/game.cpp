#include "game.h"
#include <iostream>
#include <cmath>
#include "SDL.h"

Game::Game(std::size_t grid_width, std::size_t grid_height, bool enable_ai)
    : snake_(grid_width, grid_height),
      ai_snake_(grid_width, grid_height),
      engine_(dev_()),
      random_w_(0, static_cast<int>(grid_width - 1)),
      random_h_(0, static_cast<int>(grid_height - 1)),
      ai_enabled_(enable_ai),
      food_factory_(grid_width, grid_height) {
  // Create obstacle manager with 5 fixed and 3 moving obstacles
  obstacles_ = std::make_unique<ObstacleManager>(
      grid_width, grid_height, 5, 3);

  // Place initial food items
  for (std::size_t i = 0; i < 3; ++i) {
    PlaceFood();
  }

  // Start AI snake pathfinding thread only if enabled
  if (ai_enabled_) {
    ai_snake_.StartAI();
    UpdateAIFoodTarget();
  } else {
    ai_snake_.alive = false;  // Disable AI snake
  }
}

Game::~Game() {
  // Stop AI thread before destruction
  ai_snake_.StopAI();
}

void Game::Run(Controller const &controller, Renderer &renderer,
               std::size_t target_frame_duration) {
  Uint32 title_timestamp = SDL_GetTicks();
  Uint32 frame_start;
  Uint32 frame_end;
  Uint32 frame_duration;
  int fps_frame_count = 0;
  bool running = true;

  while (running) {
    frame_start = SDL_GetTicks();

    // Input, Update, Render - the main game loop.
    controller.HandleInput(running, snake_);
    Update();
    renderer.Render(snake_, ai_snake_, foods_, *obstacles_, ai_enabled_);

    frame_end = SDL_GetTicks();

    // Keep track of how long each loop through the input/update/render cycle
    // takes.
    fps_frame_count++;
    frame_duration = frame_end - frame_start;

    // After every second, update the window title.
    if (frame_end - title_timestamp >= 1000) {
      renderer.UpdateWindowTitle(score_, ai_score_, fps_frame_count);
      fps_frame_count = 0;
      title_timestamp = frame_end;
    }

    // If the time for this frame is too small (i.e. frame_duration is
    // smaller than the target ms_per_frame), delay the loop to
    // achieve the correct frame rate.
    if (frame_duration < target_frame_duration) {
      SDL_Delay(target_frame_duration - frame_duration);
    }
  }
}

void Game::PlaceFood() {
  if (foods_.size() >= kMaxFoodItems) return;

  int x, y;
  int attempts = 0;
  const int max_attempts = 100;
  std::unique_ptr<Food> new_food;

  do {
    new_food = food_factory_.CreateRandomFood(engine_);
    x = new_food->GetX();
    y = new_food->GetY();
    attempts++;
  } while (!IsValidFoodPosition(x, y) && attempts < max_attempts);

  if (attempts < max_attempts) {
    foods_.push_back(std::move(new_food));
  }
}

bool Game::IsValidFoodPosition(int x, int y) const {
  // Check against player snake
  if (snake_.SnakeCell(x, y)) {
    return false;
  }

  // Check against AI snake
  if (ai_snake_.SnakeCell(x, y)) {
    return false;
  }

  // Check against obstacles
  if (obstacles_->IsObstacleAt(x, y)) {
    return false;
  }

  // Check against existing food
  for (const auto& food : foods_) {
    if (food->IsAt(x, y)) {
      return false;
    }
  }

  return true;
}

void Game::UpdateAIFoodTarget() {
  if (foods_.empty()) return;

  // Find closest food to AI snake
  int ai_x = static_cast<int>(ai_snake_.head_x);
  int ai_y = static_cast<int>(ai_snake_.head_y);

  int closest_dist = WINT_MAX;
  int target_x = 0, target_y = 0;

  for (const auto& food : foods_) {
    int dx = std::abs(food->GetX() - ai_x);
    int dy = std::abs(food->GetY() - ai_y);
    int dist = dx + dy;
    if (dist < closest_dist) {
      closest_dist = dist;
      target_x = food->GetX();
      target_y = food->GetY();
    }
  }

  ai_snake_.SetFoodTarget(target_x, target_y);
}

void Game::Update() {
  bool player_active = snake_.alive;
  bool ai_active = ai_enabled_ && ai_snake_.alive;

  if (!player_active && !ai_active) return;

  frame_count_++;

  // Spawn new food every kFoodSpawnInterval frames
  if (frame_count_ % kFoodSpawnInterval == 0) {
    PlaceFood();
  }

  // Update obstacles periodically
  if (frame_count_ % kObstacleUpdateInterval == 0) {
    obstacles_->Update();
  }

  // Update player snake
  if (player_active) {
    snake_.Update();

    int new_x = static_cast<int>(snake_.head_x);
    int new_y = static_cast<int>(snake_.head_y);

    // Check collision with obstacles
    if (obstacles_->CheckCollision(snake_.body, new_x, new_y)) {
      snake_.alive = false;
    }

    // Check collision with AI snake (only if AI enabled)
    if (ai_active && ai_snake_.SnakeCell(new_x, new_y)) {
      snake_.alive = false;
    }

    // Check if player snake got any food
    if (snake_.alive) {
      for (auto it = foods_.begin(); it != foods_.end(); ) {
        if ((*it)->IsAt(new_x, new_y)) {
          (*it)->ApplyEffect(snake_, score_);
          it = foods_.erase(it);
          if (ai_enabled_) UpdateAIFoodTarget();
        } else {
          ++it;
        }
      }
    }
  }

  // Update AI snake only if enabled
  if (ai_enabled_) {
    UpdateAISnake();
  }
}

void Game::UpdateAISnake() {
  if (!ai_snake_.alive) return;

  // Collect obstacle positions for pathfinding
  std::vector<SDL_Point> obstacle_positions;
  for (const auto& obstacle : obstacles_->GetObstacles()) {
    for (const auto& cell : obstacle->GetOccupiedCells()) {
      obstacle_positions.push_back(cell);
    }
  }
  ai_snake_.SetObstacles(obstacle_positions);

  // Update AI with player snake position
  ai_snake_.SetPlayerSnakeBody(snake_.body,
                               static_cast<int>(snake_.head_x),
                               static_cast<int>(snake_.head_y));

  // Update AI direction based on path
  ai_snake_.UpdateAI();

  // Update AI snake movement
  ai_snake_.Update();

  int ai_x = static_cast<int>(ai_snake_.head_x);
  int ai_y = static_cast<int>(ai_snake_.head_y);

  // Check AI collision with obstacles
  if (obstacles_->CheckCollision(ai_snake_.body, ai_x, ai_y)) {
    ai_snake_.alive = false;
  }

  // Check AI collision with player snake
  if (snake_.alive && snake_.SnakeCell(ai_x, ai_y)) {
    ai_snake_.alive = false;
  }

  // Check if AI snake got any food
  if (ai_snake_.alive) {
    for (auto it = foods_.begin(); it != foods_.end(); ) {
      if ((*it)->IsAt(ai_x, ai_y)) {
        (*it)->ApplyEffect(ai_snake_, ai_score_);
        it = foods_.erase(it);
        UpdateAIFoodTarget();
      } else {
        ++it;
      }
    }
  }
}

int Game::GetScore() const { return score_; }
int Game::GetSize() const { return snake_.size; }
