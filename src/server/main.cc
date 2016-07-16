#include "server/game.h"

int main(const int argc, const char* const argv[]) {
  return std::unique_ptr<GameServer>(new GameServer())->Run(ParseCommandLine(argc, argv));
}
