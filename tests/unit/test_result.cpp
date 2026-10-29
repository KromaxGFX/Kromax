// Unit test: KxResult
#include <kromax/kromax.h>
#include <string>
#include <doctest.h>

TEST_CASE("result_to_string") {
    const char* s = kxResultToString(KX_SUCCESS);
    CHECK(std::string(s) == "KX_SUCCESS");
}