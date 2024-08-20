﻿#include "ReflectionTest.hpp"
#include "ObserverTest.hpp"
#include "GCTest.hpp"
#include "SingletonTest.hpp"

#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    int result = RUN_ALL_TESTS();
    return result;
}