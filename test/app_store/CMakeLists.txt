include(FetchContent)

FetchContent_Declare(
  ArduinoJson
  URL https://github.com/bblanchon/ArduinoJson/archive/refs/tags/v7.4.2.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(ArduinoJson)

add_executable(AppStoreCoreTest
  AppStoreCoreTest.cpp
  ${REPO_ROOT}/lib/AppStore/AppVersion.cpp
  ${REPO_ROOT}/lib/AppStore/AppPathSanitizer.cpp
)

target_link_libraries(AppStoreCoreTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(AppStoreCoreTest)

add_executable(AppRegistryJsonTest
  AppRegistryJsonTest.cpp
  ${REPO_ROOT}/lib/AppStore/AppRegistryJson.cpp
)

target_include_directories(AppRegistryJsonTest PRIVATE
  ${arduinojson_SOURCE_DIR}/src
)

target_link_libraries(AppRegistryJsonTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(AppRegistryJsonTest)

add_executable(AppStoreManifestJsonTest
  AppStoreManifestJsonTest.cpp
  ${REPO_ROOT}/lib/AppStore/AppStoreManifestJson.cpp
)

target_include_directories(AppStoreManifestJsonTest PRIVATE
  ${arduinojson_SOURCE_DIR}/src
)

target_link_libraries(AppStoreManifestJsonTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(AppStoreManifestJsonTest)

add_executable(AppStoreManifestFallbackTest
  AppStoreManifestFallbackTest.cpp
  ${REPO_ROOT}/lib/AppStore/AppStoreManifestJson.cpp
  ${REPO_ROOT}/lib/AppStore/AppStoreManifestResolve.cpp
)

target_include_directories(AppStoreManifestFallbackTest PRIVATE
  ${arduinojson_SOURCE_DIR}/src
)

target_link_libraries(AppStoreManifestFallbackTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(AppStoreManifestFallbackTest)
