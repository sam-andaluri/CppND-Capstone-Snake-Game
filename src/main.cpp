#include <iostream>
#include "controller.h"
#include "game.h"
#include "renderer.h"
#include "highscore.h"

int main() {
  constexpr std::size_t kFramesPerSecond{60};
  constexpr std::size_t kMsPerFrame{1000 / kFramesPerSecond};
  constexpr std::size_t kScreenWidth{640};
  constexpr std::size_t kScreenHeight{640};
  constexpr std::size_t kGridWidth{32};
  constexpr std::size_t kGridHeight{32};

  // Initialize high score manager
  HighScoreManager highscores;

  // Display existing high scores
  highscores.DisplayScores();

  // Get player name
  std::string player_name = HighScoreManager::GetPlayerName();

  // Ask if user wants AI opponent
  std::cout << "\nPlay with AI opponent? (y/n): ";
  std::string ai_choice;
  std::getline(std::cin, ai_choice);
  bool enable_ai = (ai_choice == "y" || ai_choice == "Y" || ai_choice == "yes");

  std::cout << "\nWelcome, " << player_name << "!\n";
  std::cout << "Controls: Arrow keys to move\n";
  std::cout << "\nSnake colors:\n";
  std::cout << "  YOUR SNAKE:  Blue head, White body\n";
  if (enable_ai) {
    std::cout << "  AI SNAKE:    Purple head, Orange body\n";
  }
  std::cout << "\nFood types:\n";
  std::cout << "  Yellow    - Normal (+1 point)\n";
  std::cout << "  Red       - Speed Boost (+2 points, faster)\n";
  std::cout << "  Cyan      - Slowdown (+1 point, slower)\n";
  std::cout << "  Pink      - Bonus (+5 points)\n";
  std::cout << "\nGray blocks are obstacles - avoid them!\n";
  std::cout << "\nStarting game...\n\n";

  Renderer renderer(kScreenWidth, kScreenHeight, kGridWidth, kGridHeight);
  Controller controller;
  Game game(kGridWidth, kGridHeight, enable_ai);
  game.SetPlayerName(player_name);

  game.Run(controller, renderer, kMsPerFrame);

  std::cout << "\nGame has terminated!\n";
  std::cout << "Your Score: " << game.GetScore() << "\n";
  std::cout << "Your Size: " << game.GetSize() << "\n";

  // Check if player made the high score list
  int final_score = game.GetScore();
  if (highscores.IsHighScore(final_score)) {
    std::cout << "\nCongratulations! You made the high score list!\n";
  }

  // Add score to high scores
  highscores.AddScore(player_name, final_score);

  // Display updated high scores
  highscores.DisplayScores();

  return 0;
}
