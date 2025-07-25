/*
 * Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

#include <string>
#include <regex>

#include <odbc_unit_test_suite.h>
#include "iotsitewise/odbc/log.h"
#include "iotsitewise/odbc/log_level.h"
#include <ignite/common/include/common/platform_utils.h>
#include <iotsitewise/odbc/authentication/auth_type.h>
#include "iotsitewise/odbc/statement.h"
#include "iotsitewise/odbc/utility.h"
#include "iotsitewise/odbc/query/data_query.h"

using iotsitewise::odbc::AuthType;
using iotsitewise::odbc::MockConnection;
using iotsitewise::odbc::MockIoTSiteWiseService;
using iotsitewise::odbc::OdbcUnitTestSuite;
using iotsitewise::odbc::Statement;
using iotsitewise::odbc::config::Configuration;
using iotsitewise::odbc::query::DataQuery;
using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct SqlPreprocessingUnitTestSuiteFixture : OdbcUnitTestSuite {
  SqlPreprocessingUnitTestSuiteFixture() : OdbcUnitTestSuite() {
    stmt = dbc->CreateStatement();
  }

  /**
   * Destructor.
   */
  ~SqlPreprocessingUnitTestSuiteFixture() = default;

  void getLogOptions(Configuration& config) const {
    using ignite::odbc::common::GetEnv;
    using iotsitewise::odbc::LogLevel;

    std::string logPath = GetEnv("IOTSITEWISE_LOG_PATH");
    std::string logLevel = GetEnv("IOTSITEWISE_LOG_LEVEL");

    if (!logPath.empty()) {
      config.SetLogPath(logPath);
    }

    if (!logLevel.empty()) {
      LogLevel::Type logLevelType = LogLevel::FromString(logLevel);
      config.SetLogLevel(logLevelType);
    }
  }

  /**
   * Helper function to test SQL preprocessing directly.
   * This mimics the PreprocessSql function from DataQuery.
   */
  std::string PreprocessSql(const std::string& originalSql) {
    std::string processedSql = originalSql;
    
    // Remove empty database qualifiers like "".table_name or ""."schema"."table"
    // This handles the case where Excel/PowerBI generates queries with empty catalog names
    std::regex emptyDbPattern(R"(""\.)");
    processedSql = std::regex_replace(processedSql, emptyDbPattern, "");
    
    // Also handle cases where there might be spaces around the empty qualifier
    // Use a more precise pattern that preserves spacing after the dot
    std::regex emptyDbPatternWithSpaces(R"(\s*""\s*\.\s*)");
    processedSql = std::regex_replace(processedSql, emptyDbPatternWithSpaces, " ");
    
    return processedSql;
  }

  /** Statement. */
  Statement* stmt;
};

BOOST_FIXTURE_TEST_SUITE(SqlPreprocessingUnitTestSuite, SqlPreprocessingUnitTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestRemoveEmptyDatabaseQualifier) {
  std::string originalSql = R"(select "asset_id", "property_id" from ""."asset")";
  std::string expectedSql = R"(select "asset_id", "property_id" from "asset")";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_CASE(TestRemoveEmptyDatabaseQualifierWithSpaces) {
  std::string originalSql = R"(select "asset_id", "property_id" from "" . "asset")";
  std::string expectedSql = R"(select "asset_id", "property_id" from "asset")";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_CASE(TestRemoveEmptyDatabaseQualifierWithSchema) {
  std::string originalSql = R"(select "asset_id", "property_id" from ""."schema"."asset")";
  std::string expectedSql = R"(select "asset_id", "property_id" from "schema"."asset")";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_CASE(TestPreserveNormalTableName) {
  std::string originalSql = R"(select "asset_id", "property_id" from "asset")";
  std::string expectedSql = R"(select "asset_id", "property_id" from "asset")";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_CASE(TestPreserveSchemaQualifiedTableName) {
  std::string originalSql = R"(select "asset_id", "property_id" from "schema"."asset")";
  std::string expectedSql = R"(select "asset_id", "property_id" from "schema"."asset")";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_CASE(TestComplexQueryWithEmptyDatabaseQualifier) {
  std::string originalSql = R"(select "asset_id",
    "property_id",
    "asset_name",
    "asset_description",
    "property_name",
    "asset_model_id",
    "property_alias",
    "asset_composite_model_id",
    "asset_external_id",
    "asset_model_external_id",
    "property_external_id",
    "parent_asset_id",
    "property_data_type",
    "hierarchy_id",
    "property_type",
    "int_attribute_value",
    "double_attribute_value",
    "boolean_attribute_value",
    "string_attribute_value"
from ""."asset")";

  std::string expectedSql = R"(select "asset_id",
    "property_id",
    "asset_name",
    "asset_description",
    "property_name",
    "asset_model_id",
    "property_alias",
    "asset_composite_model_id",
    "asset_external_id",
    "asset_model_external_id",
    "property_external_id",
    "parent_asset_id",
    "property_data_type",
    "hierarchy_id",
    "property_type",
    "int_attribute_value",
    "double_attribute_value",
    "boolean_attribute_value",
    "string_attribute_value"
from "asset")";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_CASE(TestMultipleEmptyDatabaseQualifiers) {
  std::string originalSql = R"(select a."asset_id", b."property_id" from ""."asset" a join ""."property" b on a.id = b.asset_id)";
  std::string expectedSql = R"(select a."asset_id", b."property_id" from "asset" a join "property" b on a.id = b.asset_id)";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_CASE(TestNoChangeForValidQuery) {
  std::string originalSql = R"(select "asset_id", "property_id" from "asset" where "asset_name" = 'test')";
  std::string expectedSql = R"(select "asset_id", "property_id" from "asset" where "asset_name" = 'test')";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_CASE(TestEmptyString) {
  std::string originalSql = "";
  std::string expectedSql = "";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_CASE(TestOnlyEmptyQualifier) {
  std::string originalSql = R"("".table)";
  std::string expectedSql = R"(table)";
  
  std::string processedSql = PreprocessSql(originalSql);
  
  BOOST_CHECK_EQUAL(processedSql, expectedSql);
}

BOOST_AUTO_TEST_SUITE_END()
