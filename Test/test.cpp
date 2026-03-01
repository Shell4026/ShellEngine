#include "ReflectionTest.hpp"
#include "ObserverTest.hpp"
#include "GCTest.hpp"
#include "SingletonTest.hpp"
#include "ContainerTest.hpp"
#include "AllocatorTest.hpp"
#include "AABBTest.hpp"
#include "OctreeTest.hpp"
#include "ShaderParserTest.hpp"
#include "SpinLockTest.hpp"
#include "ThreadPoolTest.hpp"
#include "EventBusTest.hpp"
#ifdef Bool
#undef Bool
#endif

#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    int result = RUN_ALL_TESTS();
    return result;
}
