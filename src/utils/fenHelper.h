#pragma once

#include <string>

struct BoardState;

void parseFenString(std::string fen, BoardState& state);
