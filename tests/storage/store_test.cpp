#include "storage/store.h"

#include <gtest/gtest.h>

#include <string>

namespace redis_clone
{

TEST(StoreTest, StartsEmpty)
{
    const Store store;

    EXPECT_TRUE(store.empty());
    EXPECT_EQ(store.size(), 0U);
}

TEST(StoreTest, StoresAndRetrievesValue)
{
    Store store;

    store.set("name", "redis");

    const auto value = store.get("name");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "redis");
}

TEST(StoreTest, ReturnsNoValueForMissingKey)
{
    const Store store;

    EXPECT_FALSE(store.get("missing").has_value());
}

TEST(StoreTest, OverwritesExistingValue)
{
    Store store;

    store.set("name", "redis");
    store.set("name", "redis-clone");

    const auto value = store.get("name");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "redis-clone");
    EXPECT_EQ(store.size(), 1U);
}

TEST(StoreTest, ErasesExistingKey)
{
    Store store;
    store.set("name", "redis");

    EXPECT_TRUE(store.erase("name"));
    EXPECT_FALSE(store.contains("name"));
    EXPECT_TRUE(store.empty());
}

TEST(StoreTest, ErasingMissingKeyReturnsFalse)
{
    Store store;

    EXPECT_FALSE(store.erase("missing"));
}

TEST(StoreTest, SupportsBinarySafeValues)
{
    Store store;
    const std::string value("a\0b", 3);

    store.set("binary", value);

    const auto retrieved = store.get("binary");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(*retrieved, value);
}

} // namespace redis_clone
