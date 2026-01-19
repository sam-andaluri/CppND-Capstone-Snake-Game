#include "ai_snake.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

AISnake::AISnake(int grid_width, int grid_height)
    : Snake(grid_width, grid_height),
      food_target_{0, 0},
      player_head_{0, 0},
      grid_width_(grid_width),
      grid_height_(grid_height) {
  // Start AI snake in a different position (bottom-right quadrant)
  head_x = grid_width * 3 / 4;
  head_y = grid_height * 3 / 4;
  speed = 0.1f;  // Same speed as player
}

AISnake::~AISnake() {
  StopAI();
}

AISnake::AISnake(AISnake&& other) noexcept
    : Snake(std::move(other)),
      running_(other.running_.load()),
      food_target_(other.food_target_),
      obstacles_(std::move(other.obstacles_)),
      player_snake_body_(std::move(other.player_snake_body_)),
      player_head_(other.player_head_),
      current_path_(std::move(other.current_path_)),
      path_index_(other.path_index_),
      grid_width_(other.grid_width_),
      grid_height_(other.grid_height_) {
  other.running_ = false;
}

AISnake& AISnake::operator=(AISnake&& other) noexcept {
  if (this != &other) {
    StopAI();
    Snake::operator=(std::move(other));
    running_ = other.running_.load();
    food_target_ = other.food_target_;
    obstacles_ = std::move(other.obstacles_);
    player_snake_body_ = std::move(other.player_snake_body_);
    player_head_ = other.player_head_;
    current_path_ = std::move(other.current_path_);
    path_index_ = other.path_index_;
    grid_width_ = other.grid_width_;
    grid_height_ = other.grid_height_;
    other.running_ = false;
  }
  return *this;
}

void AISnake::StartAI() {
  if (running_) return;

  running_ = true;
  path_promise_ = std::promise<std::vector<SDL_Point>>();
  path_future_ = path_promise_.get_future();

  pathfinding_thread_ = std::thread(&AISnake::PathfindingThread, this);
}

void AISnake::StopAI() {
  if (!running_) return;

  running_ = false;

  // Wake up thread if waiting
  {
    std::lock_guard<std::mutex> lock(mutex_);
    path_requested_ = true;
  }
  path_cv_.notify_one();

  if (pathfinding_thread_.joinable()) {
    pathfinding_thread_.join();
  }
}

void AISnake::SetFoodTarget(int x, int y) {
  std::lock_guard<std::mutex> lock(mutex_);
  food_target_.x = x;
  food_target_.y = y;
  path_requested_ = true;
  path_cv_.notify_one();
}

void AISnake::SetObstacles(const std::vector<SDL_Point>& obstacles) {
  std::lock_guard<std::mutex> lock(mutex_);
  obstacles_ = obstacles;
}

void AISnake::SetPlayerSnakeBody(const std::vector<SDL_Point>& body,
                                  int head_x, int head_y) {
  std::lock_guard<std::mutex> lock(mutex_);
  player_snake_body_ = body;
  player_head_.x = head_x;
  player_head_.y = head_y;
}

bool AISnake::HasValidPath() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return !current_path_.empty() && path_index_ < current_path_.size();
}

void AISnake::PathfindingThread() {
  while (running_) {
    // Wait for path request using condition variable
    std::unique_lock<std::mutex> lock(mutex_);
    path_cv_.wait(lock, [this] { return path_requested_ || !running_; });

    if (!running_) break;

    path_requested_ = false;

    // Copy data needed for pathfinding
    SDL_Point start{static_cast<int>(head_x), static_cast<int>(head_y)};
    SDL_Point goal = food_target_;
    (void)obstacles_;        // Used via member access in IsWalkable
    (void)player_snake_body_; // Used via member access in IsWalkable

    lock.unlock();

    // Calculate path (outside of lock)
    auto new_path = CalculatePath(start, goal);

    // Update the path using promise/future pattern
    lock.lock();
    current_path_ = std::move(new_path);
    path_index_ = 0;
    lock.unlock();
  }
}

void AISnake::UpdateAI() {
  if (!alive) return;

  std::lock_guard<std::mutex> lock(mutex_);

  if (current_path_.empty() || path_index_ >= current_path_.size()) {
    // No valid path, move randomly to avoid getting stuck
    return;
  }

  // Get next position in path
  SDL_Point next = current_path_[path_index_];
  int current_x = static_cast<int>(head_x);
  int current_y = static_cast<int>(head_y);

  // Determine direction to next cell
  int dx = next.x - current_x;
  int dy = next.y - current_y;

  // Handle wrapping
  if (dx > grid_width_ / 2) dx -= grid_width_;
  if (dx < -grid_width_ / 2) dx += grid_width_;
  if (dy > grid_height_ / 2) dy -= grid_height_;
  if (dy < -grid_height_ / 2) dy += grid_height_;

  // Set direction based on delta
  if (std::abs(dx) > std::abs(dy)) {
    if (dx > 0) direction = Direction::kRight;
    else if (dx < 0) direction = Direction::kLeft;
  } else {
    if (dy > 0) direction = Direction::kDown;
    else if (dy < 0) direction = Direction::kUp;
  }

  // Check if we've reached the next waypoint
  if (current_x == next.x && current_y == next.y) {
    path_index_++;
  }

  // Call parent Update (handles actual movement)
  // Note: Update() is called separately in Game::Update()
}

std::vector<SDL_Point> AISnake::CalculatePath(SDL_Point start, SDL_Point goal) {
  // A* pathfinding implementation
  std::vector<std::unique_ptr<PathNode>> all_nodes;
  auto hash = [this](const SDL_Point& p) {
    return p.y * grid_width_ + p.x;
  };

  std::unordered_map<int, PathNode*> node_map;
  std::priority_queue<PathNode*, std::vector<PathNode*>,
                      std::function<bool(PathNode*, PathNode*)>> open_set(
      [](PathNode* a, PathNode* b) { return a->f_cost() > b->f_cost(); });
  std::unordered_set<int> closed_set;

  // Create start node
  auto start_node = std::make_unique<PathNode>();
  start_node->x = start.x;
  start_node->y = start.y;
  start_node->g_cost = 0;
  start_node->h_cost = Heuristic(start.x, start.y, goal.x, goal.y);
  start_node->parent = nullptr;

  int start_hash = hash(start);
  node_map[start_hash] = start_node.get();
  open_set.push(start_node.get());
  all_nodes.push_back(std::move(start_node));

  while (!open_set.empty()) {
    PathNode* current = open_set.top();
    open_set.pop();

    int current_hash = current->y * grid_width_ + current->x;

    if (closed_set.count(current_hash)) continue;
    closed_set.insert(current_hash);

    // Check if reached goal
    if (current->x == goal.x && current->y == goal.y) {
      return ReconstructPath(current);
    }

    // Explore neighbors
    for (const auto& neighbor_pos : GetNeighbors(current->x, current->y)) {
      int neighbor_hash = hash(neighbor_pos);

      if (closed_set.count(neighbor_hash)) continue;
      if (!IsWalkable(neighbor_pos.x, neighbor_pos.y)) continue;

      int tentative_g = current->g_cost + 1;

      PathNode* neighbor_node = nullptr;
      if (node_map.count(neighbor_hash)) {
        neighbor_node = node_map[neighbor_hash];
        if (tentative_g >= neighbor_node->g_cost) continue;
      } else {
        auto new_node = std::make_unique<PathNode>();
        new_node->x = neighbor_pos.x;
        new_node->y = neighbor_pos.y;
        neighbor_node = new_node.get();
        node_map[neighbor_hash] = neighbor_node;
        all_nodes.push_back(std::move(new_node));
      }

      neighbor_node->g_cost = tentative_g;
      neighbor_node->h_cost = Heuristic(neighbor_pos.x, neighbor_pos.y,
                                        goal.x, goal.y);
      neighbor_node->parent = current;
      open_set.push(neighbor_node);
    }
  }

  // No path found, return empty
  return {};
}

int AISnake::Heuristic(int x1, int y1, int x2, int y2) const {
  // Manhattan distance with wrapping consideration
  int dx = std::abs(x2 - x1);
  int dy = std::abs(y2 - y1);

  // Account for wrapping
  dx = std::min(dx, grid_width_ - dx);
  dy = std::min(dy, grid_height_ - dy);

  return dx + dy;
}

std::vector<SDL_Point> AISnake::GetNeighbors(int x, int y) const {
  std::vector<SDL_Point> neighbors;

  // Four cardinal directions with wrapping
  neighbors.push_back({(x + 1) % grid_width_, y});
  neighbors.push_back({(x - 1 + grid_width_) % grid_width_, y});
  neighbors.push_back({x, (y + 1) % grid_height_});
  neighbors.push_back({x, (y - 1 + grid_height_) % grid_height_});

  return neighbors;
}

bool AISnake::IsWalkable(int x, int y) const {
  // Check against own body
  for (const auto& segment : body) {
    if (segment.x == x && segment.y == y) return false;
  }

  // Check against obstacles
  for (const auto& obstacle : obstacles_) {
    if (obstacle.x == x && obstacle.y == y) return false;
  }

  // Check against player snake body
  for (const auto& segment : player_snake_body_) {
    if (segment.x == x && segment.y == y) return false;
  }

  // Check against player head
  if (player_head_.x == x && player_head_.y == y) return false;

  return true;
}

std::vector<SDL_Point> AISnake::ReconstructPath(PathNode* end_node) {
  std::vector<SDL_Point> path;
  PathNode* current = end_node;

  while (current != nullptr) {
    path.push_back({current->x, current->y});
    current = current->parent;
  }

  // Reverse to get path from start to goal
  std::reverse(path.begin(), path.end());

  // Skip first node (current position)
  if (!path.empty()) {
    path.erase(path.begin());
  }

  return path;
}
