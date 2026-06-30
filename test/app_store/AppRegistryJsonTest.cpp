#include <gtest/gtest.h>

#include <ArduinoJson.h>

#include "lib/AppStore/AppRegistryJson.h"

TEST(AppRegistryJsonTest, ParsesEmptyAppsArray) {
  std::vector<AppRegistryEntry> entries;
  uint8_t version = 0;
  ASSERT_TRUE(AppRegistryJson::parse(R"({"version":1,"apps":[]})", entries, version));
  EXPECT_EQ(version, 1);
  EXPECT_TRUE(entries.empty());
}

TEST(AppRegistryJsonTest, RoundTripPreservesEntries) {
  std::vector<AppRegistryEntry> entries;
  AppRegistryEntry entry;
  entry.id = "hello";
  entry.name = "Hello App";
  entry.version = "1.0.0";
  entry.installedAt = "2026-06-28T12:00:00Z";
  entries.push_back(entry);

  const std::string json = AppRegistryJson::serialize(entries);
  std::vector<AppRegistryEntry> parsed;
  uint8_t version = 0;
  ASSERT_TRUE(AppRegistryJson::parse(json.c_str(), parsed, version));
  ASSERT_EQ(parsed.size(), 1u);
  EXPECT_EQ(parsed[0].id, "hello");
  EXPECT_EQ(parsed[0].name, "Hello App");
  EXPECT_EQ(parsed[0].version, "1.0.0");
  EXPECT_EQ(parsed[0].installedAt, "2026-06-28T12:00:00Z");
}
