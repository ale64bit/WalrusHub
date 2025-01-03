#pragma once

#include <cstdint>
#include <optional>
#include <stack>
#include <unordered_set>
#include <utility>
#include <vector>

namespace wq {

enum class Color {
  kNone = 0,
  kBlack = 1,
  kWhite = 2,
};

const char *color_string(Color col);
const char *color_short_string(Color col);

using Point = std::pair<int, int>;
using PointList = std::vector<Point>;
using Move = std::pair<Color, Point>;
using MoveList = std::vector<Move>;

constexpr Point kPass{-1, -1};

class Board {
 public:
  Board(int row_count, int col_count);
  int row_count() const;
  int col_count() const;
  Color at(int r, int c) const;
  std::optional<Move> last_move() const;
  bool move(Color col, int r, int c, PointList &removed);
  bool undo(int &r_out, int &c_out, PointList &added);

 private:
  const int row_count_;
  const int col_count_;
  std::vector<std::vector<Color>> state_;
  uint64_t traversal_tag_ = 0;
  std::vector<std::vector<uint64_t>> tag_;
  uint64_t cur_hash_ = 0;
  std::unordered_set<uint64_t> prev_hash_;
  std::stack<Move, std::vector<Move>> prev_move_;
  std::stack<PointList, std::vector<PointList>> prev_removed_;

  bool inside(int r, int c) const;
  bool has_liberties(int r, int c);
  uint64_t hash_group(wq::Color col, int r, int c);
  void remove_group(int r, int c, PointList &removed);
};

}  // namespace wq
