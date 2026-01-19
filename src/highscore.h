#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <string>
#include <vector>
#include <algorithm>

// Represents a single high score entry
struct ScoreEntry {
  std::string name;
  int score;

  // Comparison operator for sorting (highest score first)
  bool operator>(const ScoreEntry& other) const {
    return score > other.score;
  }
};

// Manages high score persistence and retrieval
// Satisfies I/O rubric requirements: file I/O, data structures
class HighScoreManager {
 public:
  // Constructor with configurable filename and max entries
  explicit HighScoreManager(const std::string& filename = "highscores.txt",
                           std::size_t max_entries = 5);

  // Prompts user for name via console input
  static std::string GetPlayerName();

  // Loads high scores from file
  void LoadScores();

  // Saves high scores to file
  void SaveScores() const;

  // Adds a new score entry and maintains sorted order
  void AddScore(const std::string& name, int score);

  // Displays high scores to console
  void DisplayScores() const;

  // Returns const reference to scores vector (immutable access)
  const std::vector<ScoreEntry>& GetScores() const { return scores_; }

  // Check if score qualifies for high score list
  bool IsHighScore(int score) const;

 private:
  std::string filename_;
  std::size_t max_entries_;
  std::vector<ScoreEntry> scores_;

  // Sorts scores in descending order using standard algorithm
  void SortScores();
};

#endif
