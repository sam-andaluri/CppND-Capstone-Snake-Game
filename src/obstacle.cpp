#include "obstacle.h"
#include <cmath>

// Initialize static member
int Obstacle::next_id_ = 0;

// Base Obstacle class implementation

Obstacle::Obstacle(int x, int y, int grid_width, int grid_height)
    : position_{x, y},
      grid_width_(grid_width),
      grid_height_(grid_height),
      obstacle_id_(new int(next_id_++)) {}

Obstacle::~Obstacle() {
  delete obstacle_id_;
  obstacle_id_ = nullptr;
}

// Rule of 5: Copy constructor
Obstacle::Obstacle(const Obstacle& other)
    : position_(other.position_),
      grid_width_(other.grid_width_),
      grid_height_(other.grid_height_),
      obstacle_id_(new int(*other.obstacle_id_)) {}

// Rule of 5: Copy assignment operator
Obstacle& Obstacle::operator=(const Obstacle& other) {
  if (this != &other) {
    position_ = other.position_;
    grid_width_ = other.grid_width_;
    grid_height_ = other.grid_height_;

    // Deep copy the dynamically allocated resource
    delete obstacle_id_;
    obstacle_id_ = new int(*other.obstacle_id_);
  }
  return *this;
}

// Rule of 5: Move constructor
Obstacle::Obstacle(Obstacle&& other) noexcept
    : position_(other.position_),
      grid_width_(other.grid_width_),
      grid_height_(other.grid_height_),
      obstacle_id_(other.obstacle_id_) {
  // Transfer ownership, leave other in valid state
  other.obstacle_id_ = nullptr;
  other.position_ = {0, 0};
}

// Rule of 5: Move assignment operator
Obstacle& Obstacle::operator=(Obstacle&& other) noexcept {
  if (this != &other) {
    // Clean up existing resource
    delete obstacle_id_;

    // Transfer ownership
    position_ = other.position_;
    grid_width_ = other.grid_width_;
    grid_height_ = other.grid_height_;
    obstacle_id_ = other.obstacle_id_;

    // Leave other in valid state
    other.obstacle_id_ = nullptr;
    other.position_ = {0, 0};
  }
  return *this;
}

bool Obstacle::IsAt(int x, int y) const {
  for (const auto& cell : GetOccupiedCells()) {
    if (cell.x == x && cell.y == y) {
      return true;
    }
  }
  return false;
}

std::vector<SDL_Point> Obstacle::GetOccupiedCells() const {
  return {position_};
}

// FixedObstacle implementation

FixedObstacle::FixedObstacle(int x, int y, int grid_width, int grid_height)
    : Obstacle(x, y, grid_width, grid_height) {}

void FixedObstacle::Update() {
  // Fixed obstacles don't move - intentionally empty
}

// MovingObstacle implementation

MovingObstacle::MovingObstacle(int x, int y, int grid_width, int grid_height,
                               Pattern pattern)
    : Obstacle(x, y, grid_width, grid_height),
      pattern_(pattern),
      direction_(1),
      steps_moved_(0),
      max_steps_(5),
      angle_(0.0f) {}

void MovingObstacle::Update() {
  switch (pattern_) {
    case Pattern::Horizontal:
      UpdateHorizontal();
      break;
    case Pattern::Vertical:
      UpdateVertical();
      break;
    case Pattern::Circular:
      UpdateCircular();
      break;
  }
}

void MovingObstacle::UpdateHorizontal() {
  steps_moved_++;
  if (steps_moved_ >= max_steps_) {
    direction_ *= -1;
    steps_moved_ = 0;
  }

  position_.x += direction_;

  // Wrap around grid boundaries
  if (position_.x < 0) position_.x = grid_width_ - 1;
  if (position_.x >= grid_width_) position_.x = 0;
}

void MovingObstacle::UpdateVertical() {
  steps_moved_++;
  if (steps_moved_ >= max_steps_) {
    direction_ *= -1;
    steps_moved_ = 0;
  }

  position_.y += direction_;

  // Wrap around grid boundaries
  if (position_.y < 0) position_.y = grid_height_ - 1;
  if (position_.y >= grid_height_) position_.y = 0;
}

void MovingObstacle::UpdateCircular() {
  // Store original position as center
  static int center_x = position_.x;
  static int center_y = position_.y;
  const int radius = 3;

  angle_ += 0.1f;
  if (angle_ > 2 * M_PI) angle_ -= 2 * M_PI;

  int new_x = center_x + static_cast<int>(radius * std::cos(angle_));
  int new_y = center_y + static_cast<int>(radius * std::sin(angle_));

  // Clamp to grid boundaries
  position_.x = std::max(0, std::min(grid_width_ - 1, new_x));
  position_.y = std::max(0, std::min(grid_height_ - 1, new_y));
}

// ObstacleManager implementation

ObstacleManager::ObstacleManager(int grid_width, int grid_height,
                                 std::size_t num_fixed, std::size_t num_moving)
    : grid_width_(grid_width),
      grid_height_(grid_height),
      engine_(std::random_device{}()) {
  GenerateObstacles(num_fixed, num_moving);
}

void ObstacleManager::Update() {
  for (auto& obstacle : obstacles_) {
    obstacle->Update();
  }
}

bool ObstacleManager::IsObstacleAt(int x, int y) const {
  for (const auto& obstacle : obstacles_) {
    if (obstacle->IsAt(x, y)) {
      return true;
    }
  }
  return false;
}

const std::vector<std::unique_ptr<Obstacle>>& ObstacleManager::GetObstacles() const {
  return obstacles_;
}

bool ObstacleManager::CheckCollision(const std::vector<SDL_Point>& snake_body,
                                     int head_x, int head_y) const {
  // Check head collision
  if (IsObstacleAt(head_x, head_y)) {
    return true;
  }
  return false;
}

void ObstacleManager::GenerateObstacles(std::size_t num_fixed,
                                        std::size_t num_moving) {
  std::uniform_int_distribution<int> dist_x(0, grid_width_ - 1);
  std::uniform_int_distribution<int> dist_y(0, grid_height_ - 1);
  std::uniform_int_distribution<int> dist_pattern(0, 2);

  // Generate fixed obstacles
  for (std::size_t i = 0; i < num_fixed; ++i) {
    int x, y;
    do {
      x = dist_x(engine_);
      y = dist_y(engine_);
    } while (!IsSafeSpawnLocation(x, y));

    obstacles_.push_back(
        std::make_unique<FixedObstacle>(x, y, grid_width_, grid_height_));
  }

  // Generate moving obstacles
  for (std::size_t i = 0; i < num_moving; ++i) {
    int x, y;
    do {
      x = dist_x(engine_);
      y = dist_y(engine_);
    } while (!IsSafeSpawnLocation(x, y));

    auto pattern = static_cast<MovingObstacle::Pattern>(dist_pattern(engine_));
    obstacles_.push_back(
        std::make_unique<MovingObstacle>(x, y, grid_width_, grid_height_, pattern));
  }
}

bool ObstacleManager::IsSafeSpawnLocation(int x, int y) const {
  // Avoid center area where snake spawns (with margin)
  int center_x = grid_width_ / 2;
  int center_y = grid_height_ / 2;
  int margin = 4;

  if (std::abs(x - center_x) <= margin && std::abs(y - center_y) <= margin) {
    return false;
  }

  // Avoid existing obstacle positions
  for (const auto& obstacle : obstacles_) {
    if (obstacle->IsAt(x, y)) {
      return false;
    }
  }

  return true;
}
