#include <catch2/catch_test_macros.hpp>

#include "Base64.hpp"

TEST_CASE("Base64 - Standard RFC 4648 Encoding Vectors", "[tier1][base64]") {
    SECTION("Empty input produces empty output") {
        std::vector<uint8_t> empty;
        REQUIRE(Base64::encode(empty).empty());
    }

    SECTION("Standard RFC test vectors") {
        // "f" -> "Zg=="
        std::vector<uint8_t> f = {'f'};
        REQUIRE(Base64::encode(f) == "Zg==");

        // "fo" -> "Zm8="
        std::vector<uint8_t> fo = {'f', 'o'};
        REQUIRE(Base64::encode(fo) == "Zm8=");

        // "foo" -> "Zm9v"
        std::vector<uint8_t> foo = {'f', 'o', 'o'};
        REQUIRE(Base64::encode(foo) == "Zm9v");

        // "foob" -> "Zm9vYg=="
        std::vector<uint8_t> foob = {'f', 'o', 'o', 'b'};
        REQUIRE(Base64::encode(foob) == "Zm9vYg==");

        // "fooba" -> "Zm9vYmE="
        std::vector<uint8_t> fooba = {'f', 'o', 'o', 'b', 'a'};
        REQUIRE(Base64::encode(fooba) == "Zm9vYmE=");

        // "foobar" -> "Zm9vYmFy"
        std::vector<uint8_t> foobar = {'f', 'o', 'o', 'b', 'a', 'r'};
        REQUIRE(Base64::encode(foobar) == "Zm9vYmFy");
    }

    SECTION("Binary payload with nulls and full byte range") {
        std::vector<uint8_t> binary = {0x00, 0xFF, 0xAA, 0x55, 0x12, 0x34};
        std::string encoded = Base64::encode(binary);
        REQUIRE(encoded == "AP+qVRI0");
    }
}

TEST_CASE("Base64 - Data URI Construction", "[tier1][base64]") {
    SECTION("JPEG Data URI") {
        std::vector<uint8_t> bytes = {0xFF, 0xD8, 0xFF};
        std::string uri = Base64::to_data_uri(bytes, "image/jpeg");
        REQUIRE(uri == "data:image/jpeg;base64,/9j/");
    }

    SECTION("PNG Data URI") {
        std::vector<uint8_t> bytes = {0x89, 0x50, 0x4E, 0x47};
        std::string uri = Base64::to_data_uri(bytes, "image/png");
        REQUIRE(uri == "data:image/png;base64,iVBORw==");
    }
}
