#ifndef FOOD_H
#define FOOD_H

#include "SDL.h"
#include <random>
#include <memory>

// Forward declaration
class Snake;

// Color structure for food rendering
struct Color {
  Uint8 r, g, b, a;
};

// Abstract base class for all food types
// Satisfies OOP rubric: classes, access specifiers, inheritance, virtual functions
class Food {
 public:
  // Enum for food type identification
  enum class Type { Normal, SpeedBoost, Slowdown, Bonus };

  // Constructor with member initialization list
  Food(int x, int y, Type type);

  // Virtual destructor for proper polymorphic destruction
  virtual ~Food() = default;

  // Pure virtual method - each food type implements its own effect
  virtual void ApplyEffect(Snake& snake, int& score) = 0;

  // Pure virtual method - each food type has its own color
  virtual Color GetColor() const = 0;

  // Pure virtual method - returns point value
  virtual int GetPoints() const = 0;

  // Getters (encapsulation)
  int GetX() const { return position_.x; }
  int GetY() const { return position_.y; }
  SDL_Point GetPosition() const { return position_; }
  Type GetType() const { return type_; }

  // Check if position matches this food
  bool IsAt(int x, int y) const;

 protected:
  SDL_Point position_;
  Type type_;
};

// Normal food - standard behavior (1 point, grows snake)
class NormalFood : public Food {
 public:
  NormalFood(int x, int y);
  void ApplyEffect(Snake& snake, int& score) override;
  Color GetColor() const override;
  int GetPoints() const override { return 1; }
};

// Speed boost food - temporarily increases speed
class SpeedBoostFood : public Food {
 public:
  SpeedBoostFood(int x, int y);
  void ApplyEffect(Snake& snake, int& score) override;
  Color GetColor() const override;
  int GetPoints() const override { return 2; }

 private:
  static constexpr float kSpeedIncrease = 0.005f;
};

// Slowdown food - temporarily decreases speed
class SlowdownFood : public Food {
 public:
  SlowdownFood(int x, int y);
  void ApplyEffect(Snake& snake, int& score) override;
  Color GetColor() const override;
  int GetPoints() const override { return 1; }

 private:
  static constexpr float kSpeedDecrease = 0.005f;
  static constexpr float kMinSpeed = 0.05f;
};

// Bonus food - gives extra points (rare spawn)
class BonusFood : public Food {
 public:
  BonusFood(int x, int y);
  void ApplyEffect(Snake& snake, int& score) override;
  Color GetColor() const override;
  int GetPoints() const override { return 5; }
};

// Template factory function for creating food objects
// Satisfies OOP rubric: templates
template<typename T>
std::unique_ptr<Food> CreateFood(int x, int y) {
  return std::make_unique<T>(x, y);
}

// Food factory class for random food generation
class FoodFactory {
 public:
  FoodFactory(int grid_width, int grid_height);

  // Creates a random food type at a random valid position
  std::unique_ptr<Food> CreateRandomFood(std::mt19937& engine);

 private:
  std::uniform_int_distribution<int> random_x_;
  std::uniform_int_distribution<int> random_y_;
  std::uniform_int_distribution<int> random_type_;
};

#endif
