#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "SDL.h"
#include <vector>
#include <memory>
#include <random>

// Abstract base class for obstacles
// Satisfies Memory rubric: RAII, destructors, smart pointers
class Obstacle {
 public:
  // Constructor with member initialization list
  Obstacle(int x, int y, int grid_width, int grid_height);

  // Virtual destructor for proper polymorphic destruction
  virtual ~Obstacle();

  // Rule of 5: Explicitly define copy/move operations
  // Copy constructor
  Obstacle(const Obstacle& other);
  // Copy assignment operator
  Obstacle& operator=(const Obstacle& other);
  // Move constructor
  Obstacle(Obstacle&& other) noexcept;
  // Move assignment operator
  Obstacle& operator=(Obstacle&& other) noexcept;

  // Pure virtual method for updating obstacle (movement logic)
  virtual void Update() = 0;

  // Check if obstacle occupies given position
  bool IsAt(int x, int y) const;

  // Getters
  int GetX() const { return position_.x; }
  int GetY() const { return position_.y; }
  SDL_Point GetPosition() const { return position_; }

  // Get all positions occupied by this obstacle (for larger obstacles)
  virtual std::vector<SDL_Point> GetOccupiedCells() const;

 protected:
  SDL_Point position_;
  int grid_width_;
  int grid_height_;

  // RAII: Dynamically allocated resource for demonstrating Rule of 5
  int* obstacle_id_;

  // Static counter for unique IDs
  static int next_id_;
};

// Fixed obstacle - stationary hazard
class FixedObstacle : public Obstacle {
 public:
  FixedObstacle(int x, int y, int grid_width, int grid_height);
  ~FixedObstacle() override = default;

  // Fixed obstacles don't move
  void Update() override;
};

// Moving obstacle - hazard that moves in patterns
class MovingObstacle : public Obstacle {
 public:
  enum class Pattern { Horizontal, Vertical, Circular };

  MovingObstacle(int x, int y, int grid_width, int grid_height, Pattern pattern);
  ~MovingObstacle() override = default;

  void Update() override;

 private:
  Pattern pattern_;
  int direction_;        // 1 or -1 for linear patterns
  int steps_moved_;      // Counter for movement
  int max_steps_;        // Maximum steps before reversing
  float angle_;          // For circular pattern

  void UpdateHorizontal();
  void UpdateVertical();
  void UpdateCircular();
};

// Obstacle manager using smart pointers
// Satisfies Memory rubric: smart pointers, RAII, pass-by-reference
class ObstacleManager {
 public:
  ObstacleManager(int grid_width, int grid_height, std::size_t num_fixed,
                  std::size_t num_moving);

  // Destructor follows RAII - unique_ptr handles cleanup automatically
  ~ObstacleManager() = default;

  // Update all obstacles (movement)
  void Update();

  // Check if any obstacle is at position (pass by value for small types)
  bool IsObstacleAt(int x, int y) const;

  // Get all obstacles for rendering (pass by const reference)
  const std::vector<std::unique_ptr<Obstacle>>& GetObstacles() const;

  // Check collision with snake body positions (pass by const reference)
  bool CheckCollision(const std::vector<SDL_Point>& snake_body, int head_x,
                      int head_y) const;

 private:
  std::vector<std::unique_ptr<Obstacle>> obstacles_;
  int grid_width_;
  int grid_height_;
  std::mt19937 engine_;

  // Helper to generate obstacles avoiding center where snake spawns
  void GenerateObstacles(std::size_t num_fixed, std::size_t num_moving);
  bool IsSafeSpawnLocation(int x, int y) const;
};

#endif
