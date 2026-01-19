#ifndef AI_SNAKE_H
#define AI_SNAKE_H

#include "snake.h"
#include "SDL.h"
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>
#include <queue>
#include <unordered_set>

// Node for A* pathfinding
struct PathNode {
  int x, y;
  int g_cost;  // Cost from start
  int h_cost;  // Heuristic cost to goal
  int f_cost() const { return g_cost + h_cost; }
  PathNode* parent;

  bool operator>(const PathNode& other) const {
    return f_cost() > other.f_cost();
  }
};

// AI-controlled snake using A* pathfinding
// Satisfies Concurrency rubric: multithreading, mutex, condition variable, promise/future
class AISnake : public Snake {
 public:
  AISnake(int grid_width, int grid_height);
  ~AISnake();

  // Disable copy (threaded object)
  AISnake(const AISnake&) = delete;
  AISnake& operator=(const AISnake&) = delete;

  // Allow move
  AISnake(AISnake&& other) noexcept;
  AISnake& operator=(AISnake&& other) noexcept;

  // Start the AI pathfinding thread
  void StartAI();

  // Stop the AI thread
  void StopAI();

  // Update AI snake movement based on calculated path
  void UpdateAI();

  // Set the current food target (thread-safe)
  void SetFoodTarget(int x, int y);

  // Set obstacles for path avoidance (thread-safe)
  void SetObstacles(const std::vector<SDL_Point>& obstacles);

  // Set player snake body for avoidance (thread-safe)
  void SetPlayerSnakeBody(const std::vector<SDL_Point>& body, int head_x, int head_y);

  // Check if AI has calculated a valid path
  bool HasValidPath() const;

 private:
  // Pathfinding thread function
  void PathfindingThread();

  // A* pathfinding algorithm
  std::vector<SDL_Point> CalculatePath(SDL_Point start, SDL_Point goal);

  // Helper functions for A*
  int Heuristic(int x1, int y1, int x2, int y2) const;
  std::vector<SDL_Point> GetNeighbors(int x, int y) const;
  bool IsWalkable(int x, int y) const;
  std::vector<SDL_Point> ReconstructPath(PathNode* end_node);

  // Thread management
  std::thread pathfinding_thread_;
  std::atomic<bool> running_{false};

  // Mutex for thread safety (protects shared state)
  mutable std::mutex mutex_;

  // Condition variable for synchronization
  std::condition_variable path_cv_;
  bool path_requested_{false};

  // Promise/Future for passing path results
  std::promise<std::vector<SDL_Point>> path_promise_;
  std::future<std::vector<SDL_Point>> path_future_;

  // Shared state (protected by mutex)
  SDL_Point food_target_;
  std::vector<SDL_Point> obstacles_;
  std::vector<SDL_Point> player_snake_body_;
  SDL_Point player_head_;

  // Current calculated path
  std::vector<SDL_Point> current_path_;
  std::size_t path_index_{0};

  // Grid boundaries
  int grid_width_;
  int grid_height_;
};

#endif
