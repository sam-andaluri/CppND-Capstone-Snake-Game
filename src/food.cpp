#include "food.h"
#include "snake.h"

// Base Food class implementation
Food::Food(int x, int y, Type type)
    : position_{x, y}, type_(type) {}

bool Food::IsAt(int x, int y) const {
  return position_.x == x && position_.y == y;
}

// NormalFood implementation
NormalFood::NormalFood(int x, int y)
    : Food(x, y, Type::Normal) {}

void NormalFood::ApplyEffect(Snake& snake, int& score) {
  score += GetPoints();
  snake.GrowBody();
  snake.speed += 0.002f;
}

Color NormalFood::GetColor() const {
  return {0xFF, 0xFF, 0x00, 0xFF};  // Bright yellow
}

// SpeedBoostFood implementation
SpeedBoostFood::SpeedBoostFood(int x, int y)
    : Food(x, y, Type::SpeedBoost) {}

void SpeedBoostFood::ApplyEffect(Snake& snake, int& score) {
  score += GetPoints();
  snake.GrowBody();
  snake.speed += kSpeedIncrease;  // Extra speed boost
}

Color SpeedBoostFood::GetColor() const {
  return {0xFF, 0x44, 0x44, 0xFF};  // Bright red - speed boost
}

// SlowdownFood implementation
SlowdownFood::SlowdownFood(int x, int y)
    : Food(x, y, Type::Slowdown) {}

void SlowdownFood::ApplyEffect(Snake& snake, int& score) {
  score += GetPoints();
  snake.GrowBody();
  // Decrease speed but maintain minimum
  snake.speed = std::max(kMinSpeed, snake.speed - kSpeedDecrease);
}

Color SlowdownFood::GetColor() const {
  return {0x00, 0xFF, 0x88, 0xFF};  // Cyan-green - slowdown
}

// BonusFood implementation
BonusFood::BonusFood(int x, int y)
    : Food(x, y, Type::Bonus) {}

void BonusFood::ApplyEffect(Snake& snake, int& score) {
  score += GetPoints();
  snake.GrowBody();
  // No speed change for bonus food
}

Color BonusFood::GetColor() const {
  return {0xFF, 0x66, 0xFF, 0xFF};  // Bright pink - bonus points
}

// FoodFactory implementation
FoodFactory::FoodFactory(int grid_width, int grid_height)
    : random_x_(0, grid_width - 1),
      random_y_(0, grid_height - 1),
      random_type_(0, 99) {}

std::unique_ptr<Food> FoodFactory::CreateRandomFood(std::mt19937& engine) {
  int x = random_x_(engine);
  int y = random_y_(engine);
  int type_roll = random_type_(engine);

  // Probability distribution:
  // 60% Normal, 15% SpeedBoost, 15% Slowdown, 10% Bonus
  if (type_roll < 60) {
    return CreateFood<NormalFood>(x, y);
  } else if (type_roll < 75) {
    return CreateFood<SpeedBoostFood>(x, y);
  } else if (type_roll < 90) {
    return CreateFood<SlowdownFood>(x, y);
  } else {
    return CreateFood<BonusFood>(x, y);
  }
}
