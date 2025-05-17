#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/any.hpp"

class AnyValueTest : public testing::Test {
protected:
    Any any;
};

// Value set tests
TEST_F(AnyValueTest, ValueSetTest) {
    any.emplace<int>(20);
    ASSERT_TRUE(any.has_value());
}

TEST_F(AnyValueTest, ResetAfterValueSetTest) {
    any.emplace<int>(20);
    any.reset();
    ASSERT_FALSE(any.has_value());
}

TEST_F(AnyValueTest, ValueSetTwoTimesTest) {
    any.emplace<std::string>("Wotofak");
    any.emplace<int>(99);
    ASSERT_EQ(anycast<int>(any), 99);
}

TEST_F(AnyValueTest, ResetSeveralTimesTest) {
    any.reset();
    any.reset();
}

TEST_F(AnyValueTest, TypeEqualsValueTypeTest) {
    any.emplace<int>(20);
    ASSERT_EQ(typeid(int), any.type());
    any.emplace<std::string>("something");
    ASSERT_EQ(typeid(std::string), any.type());
}

TEST_F(AnyValueTest, EmplaceDifferentTypesTest) {
    any.emplace<int>(42);
    ASSERT_EQ(anycast<int>(any), 42);
    any.emplace<double>(3.14);
    ASSERT_EQ(anycast<double>(any), 3.14);
}
TEST_F(AnyValueTest, EmplaceAfterResetTest) {
    any.emplace<int>(42);
    any.reset();
    ASSERT_FALSE(any.has_value());
    any.emplace<std::string>("Hello");
    ASSERT_TRUE(any.has_value());
    ASSERT_EQ(anycast<std::string>(any), "Hello");
}
TEST_F(AnyValueTest, EmplaceExceptionHandlingTest) {
    try {
        any.emplace<std::string>(std::string(1000000, 'A')); // Large string
    } catch (const std::bad_alloc&) {
        // Expected exception
    }
    ASSERT_TRUE(any.has_value()); // Ensure no value was set
}
TEST_F(AnyValueTest, MultipleResetsTest) {
    any.emplace<int>(42);
    any.reset();
    any.reset(); // Should not crash or throw
    ASSERT_FALSE(any.has_value());
}
TEST_F(AnyValueTest, EmplaceSameTypeTest) {
    any.emplace<int>(10);
    ASSERT_EQ(anycast<int>(any), 10);
    any.emplace<int>(20);
    ASSERT_EQ(anycast<int>(any), 20);
}

TEST(AnyCtorsAssmsTest, DefaultContructorTest) {
    Any any;
    ASSERT_FALSE(any.has_value());
}
TEST(AnyCtorsAssmsTest, CopyConstructorTest) {
    Any any{10};
    Any any2{any};
    ASSERT_TRUE(any.has_value()); ASSERT_TRUE(any2.has_value());
    auto a = anycast<int>(any);
    auto b = anycast<int>(any2);
}
TEST(AnyCtorsAssmsTest, MoveContructorTest) {
    Any any{10};
    Any any2{std::move(any)};
    ASSERT_NO_THROW(anycast<int>(any2));
    ASSERT_THROW(anycast<int>(any), std::bad_cast);
    ASSERT_TRUE(any2.has_value());
    ASSERT_FALSE(any.has_value());
}
TEST(AnyCtorsAssmsTest, CopyAssigmentTest) {
    Any any{10};
    Any any2;
    ASSERT_TRUE(any.has_value());
    ASSERT_FALSE(any2.has_value());

    any2 = any;
    ASSERT_NO_THROW(anycast<int>(any2));
    ASSERT_NO_THROW(anycast<int>(any));
    ASSERT_TRUE(any2.has_value());
    ASSERT_TRUE(any.has_value());
}
TEST(AnyCtorsAssmsTest, MoveAssigmentTest) {
    Any any{10};
    Any any2;
    ASSERT_TRUE(any.has_value());
    ASSERT_FALSE(any2.has_value());

    any2 = std::move(any);
    ASSERT_NO_THROW(anycast<int>(any2));
    ASSERT_THROW(anycast<int>(any), std::bad_cast);
    ASSERT_TRUE(any2.has_value());
    ASSERT_FALSE(any.has_value());
}
TEST(AnyCtorsAssmsTest, CopyScopesTest) {
    Any any;
    {
        Any any2{10};
        any = any2;
        ASSERT_NO_THROW(anycast<int>(any));
        ASSERT_NO_THROW(anycast<int>(any2));
    }
    ASSERT_NO_THROW(anycast<int>(any));
}
TEST(AnyCtorsAssmsTest, MoveScopesTest) {
    Any any;
    {
        Any any2{10};
        any = std::move(any2);
        ASSERT_NO_THROW(anycast<int>(any));
        ASSERT_THROW(anycast<int>(any2), std::bad_cast);
    }
    ASSERT_NO_THROW(anycast<int>(any));
}

TEST(AnyCastTest, WrongTypeCastThrows) {
    Any any{42};
    ASSERT_THROW(anycast<double>(any), std::bad_cast);
}
TEST(AnyCastTest, CastToConstReference) {
    Any any{std::string("Hello")};
    const std::string& ref = anycast<const std::string&>(any);
    ASSERT_EQ(ref, "Hello");
}
TEST(AnyCastTest, CastToMutableReference) {
    Any any{std::string("Hi")};
    std::string& ref = anycast<std::string&>(any);
    ref = "Changed";
    ASSERT_EQ(anycast<std::string>(any), "Changed");
}

TEST_F(AnyValueTest, SwapTwoValues) {
    Any a{42}, b{std::string("hello")};
    std::swap(a, b);
    ASSERT_EQ(anycast<std::string>(a), "hello");
    ASSERT_EQ(anycast<int>(b), 42);
}
TEST_F(AnyValueTest, LargeVectorTest) {
    std::vector<int> big_vec(100000, 1);
    Any any;
    any.emplace<std::vector<int>>(big_vec);
    ASSERT_EQ(anycast<std::vector<int>>(any).size(), 100000);
}
TEST_F(AnyValueTest, ReplaceSameTypeDifferentValue) {
    any.emplace<int>(10);
    ASSERT_EQ(anycast<int>(any), 10);
    any.emplace<int>(20);
    ASSERT_EQ(anycast<int>(any), 20);
}

TEST(AnyCtorsAssmsTest, MoveFromEmpty) {
    Any a;
    Any b = std::move(a);
    ASSERT_FALSE(b.has_value());
    ASSERT_FALSE(a.has_value());
}
