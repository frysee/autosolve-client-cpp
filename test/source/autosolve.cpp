#include <autosolve/AutoSolve.h>
#include <autosolve/version.h>
#include <doctest/doctest.h>

#include <string>

// TEST_CASE("AutoSolve") {
//   // TODO
// }

TEST_CASE("AutoSolve version") {
  static_assert(std::string_view(AUTOSOLVE_VERSION) == std::string_view("1.0"));
  CHECK(std::string(AUTOSOLVE_VERSION) == std::string("1.0"));
}
