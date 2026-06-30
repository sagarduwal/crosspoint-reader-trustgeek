#include <gtest/gtest.h>

#include "lib/AppStore/AppPathSanitizer.h"
#include "lib/AppStore/AppVersion.h"

TEST(AppVersionTest, ParsesSimpleVersion) {
  const auto version = AppVersion::parse("1.2.3");
  ASSERT_TRUE(version.has_value());
  EXPECT_EQ(version->text(), "1.2.3");
}

TEST(AppVersionTest, ParsesLeadingV) {
  const auto version = AppVersion::parse("v2.0.1");
  ASSERT_TRUE(version.has_value());
  EXPECT_TRUE(version->isNewerThan(*AppVersion::parse("1.9.9")));
}

TEST(AppVersionTest, RejectsInvalidVersion) {
  EXPECT_FALSE(AppVersion::parse("").has_value());
  EXPECT_FALSE(AppVersion::parse("1.2").has_value());
  EXPECT_FALSE(AppVersion::parse("a.b.c").has_value());
}

TEST(AppVersionTest, ComparesPatchLevels) {
  const auto older = AppVersion::parse("1.0.0");
  const auto newer = AppVersion::parse("1.0.1");
  ASSERT_TRUE(older.has_value());
  ASSERT_TRUE(newer.has_value());
  EXPECT_TRUE(newer->isNewerThan(*older));
  EXPECT_FALSE(older->isNewerThan(*newer));
}

TEST(AppPathSanitizerTest, AcceptsSafeRelativePath) {
  const auto result = AppPathSanitizer::sanitizeRelativePath("notes/todo.txt");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->normalized, "notes/todo.txt");
}

TEST(AppPathSanitizerTest, RejectsTraversal) {
  EXPECT_FALSE(AppPathSanitizer::sanitizeRelativePath("../secret").has_value());
  EXPECT_FALSE(AppPathSanitizer::sanitizeRelativePath("a/../b").has_value());
}

TEST(AppPathSanitizerTest, RejectsAbsolutePath) {
  EXPECT_FALSE(AppPathSanitizer::sanitizeRelativePath("/etc/passwd").has_value());
}

TEST(AppPathSanitizerTest, ValidatesAppId) {
  EXPECT_TRUE(AppPathSanitizer::isValidAppId("pomodoro"));
  EXPECT_TRUE(AppPathSanitizer::isValidAppId("flash_cards-v2"));
  EXPECT_FALSE(AppPathSanitizer::isValidAppId(""));
  EXPECT_FALSE(AppPathSanitizer::isValidAppId("../bad"));
}
