#include "catch2/catch_test_macros.hpp"
#include "lstl/Vector.hpp"
#include <lstl/Map.hpp>
#include <spdlog/spdlog.h>

TEST_CASE("ctors1", "[vector]") {
    spdlog::info("info");
    const int a[] = {0, 1, 2, 3, 4};
    lstl::Vector<int> v(a, a + 5);
    REQUIRE(v.data() == &v.front());
    int* pi = v.data();
    REQUIRE(*pi == 0);
}

TEST_CASE("ctors2", "[vector]") {
    const int a[] = {4, 3, 2, 1, 0};
    lstl::Vector<int> v(a, a + 5);
    REQUIRE(v.data() == &v.front());
    int* pi = v.data();
    REQUIRE(*pi == 4);
}

TEST_CASE("init", "[map]") {
    lstl::Set set;
    set.insert(1);
    set.insert(2);
    set.insert(3);
    set.insert(4);
    REQUIRE(set.find(3)->value == 3);
    REQUIRE(set.find(4)->value == 4);
    REQUIRE(set.find(5) == nullptr);
}

TEST_CASE("LL", "[map]") {
    lstl::Set set;
    set.insert(3);
    set.insert(2);
    set.insert(1);
    set.right_rotate(set.root);
    REQUIRE(set.root->left->value == 1);
    REQUIRE(set.root->value == 2);
    REQUIRE(set.root->right->value == 3);
}

TEST_CASE("RR", "[map]") {
    lstl::Set set;
    set.insert(1);
    set.insert(2);
    set.insert(3);
    set.left_rotate(set.root);
    REQUIRE(set.root->left->value == 1);
    REQUIRE(set.root->value == 2);
    REQUIRE(set.root->right->value == 3);
}

TEST_CASE("LR", "[map]") {
    lstl::Set set;
    set.insert(3);
    set.insert(1);
    set.insert(2);
    set.left_rotate(set.root->left);
    set.right_rotate(set.root);
    REQUIRE(set.root->left->value == 1);
    REQUIRE(set.root->value == 2);
    REQUIRE(set.root->right->value == 3);
}

TEST_CASE("RL", "[map]") {
    lstl::Set set;
    set.insert(1);
    set.insert(3);
    set.insert(2);
    set.right_rotate(set.root->right);
    set.left_rotate(set.root);
    REQUIRE(set.root->left->value == 1);
    REQUIRE(set.root->value == 2);
    REQUIRE(set.root->right->value == 3);
}