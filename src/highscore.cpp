#include "highscore.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

HighScoreManager::HighScoreManager(const std::string& filename,
                                   std::size_t max_entries)
    : filename_(filename), max_entries_(max_entries) {
  LoadScores();
}

std::string HighScoreManager::GetPlayerName() {
  std::string name;
  std::cout << "\n=== SNAKE GAME ===\n";
  std::cout << "Enter your name: ";
  std::getline(std::cin, name);

  // Trim whitespace and limit length
  if (name.empty()) {
    name = "Player";
  }
  if (name.length() > 20) {
    name = name.substr(0, 20);
  }
  return name;
}

void HighScoreManager::LoadScores() {
  scores_.clear();
  std::ifstream file(filename_);

  if (!file.is_open()) {
    // File doesn't exist yet, that's okay
    return;
  }

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    ScoreEntry entry;

    // Parse format: "score name" (score first for easier parsing)
    if (iss >> entry.score) {
      // Rest of line is the name
      std::getline(iss >> std::ws, entry.name);
      if (!entry.name.empty()) {
        scores_.push_back(entry);
      }
    }
  }

  file.close();
  SortScores();
}

void HighScoreManager::SaveScores() const {
  std::ofstream file(filename_);

  if (!file.is_open()) {
    std::cerr << "Error: Could not save high scores to " << filename_ << "\n";
    return;
  }

  for (const auto& entry : scores_) {
    file << entry.score << " " << entry.name << "\n";
  }

  file.close();
}

void HighScoreManager::AddScore(const std::string& name, int score) {
  ScoreEntry entry{name, score};
  scores_.push_back(entry);
  SortScores();

  // Keep only top entries
  if (scores_.size() > max_entries_) {
    scores_.resize(max_entries_);
  }

  SaveScores();
}

void HighScoreManager::DisplayScores() const {
  std::cout << "\n=== HIGH SCORES ===\n";

  if (scores_.empty()) {
    std::cout << "No high scores yet!\n";
    return;
  }

  int rank = 1;
  for (const auto& entry : scores_) {
    std::cout << std::setw(2) << rank << ". "
              << std::setw(20) << std::left << entry.name
              << " - " << entry.score << "\n";
    rank++;
  }
  std::cout << "==================\n\n";
}

bool HighScoreManager::IsHighScore(int score) const {
  if (scores_.size() < max_entries_) {
    return true;
  }
  // Check if score beats the lowest high score
  return score > scores_.back().score;
}

void HighScoreManager::SortScores() {
  // Use standard algorithm to sort in descending order
  std::sort(scores_.begin(), scores_.end(),
            [](const ScoreEntry& a, const ScoreEntry& b) {
              return a > b;  // Uses our operator>
            });
}
