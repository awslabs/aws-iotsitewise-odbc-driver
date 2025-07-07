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

#ifdef _WIN32
#include <windows.h>
#endif

#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <utility>

#include "iotsitewise/odbc/meta/column_meta.h"
#include "iotsitewise/odbc/type_traits.h"
#include "odbc_test_suite.h"

using iotsitewise::odbc::OdbcTestSuite;
using iotsitewise::odbc::meta::ColumnMeta;
using iotsitewise::odbc::meta::Nullability;
using namespace boost::unit_test;

BOOST_AUTO_TEST_CASE(TestGetAttribute) {
  // Only SQL_DESC_* fields are tested in this test.
  // This is because those are the fields that would be passed to
  // SQLColAttribute function.
  using namespace iotsitewise::odbc::type_traits;

  std::string database("database");
  std::string table("table");
  std::string column("column");

  ColumnMeta columnMeta(database, table, column,
                        static_cast< int16_t >(ScalarType::STRING),
                        Nullability::NULLABLE);

  SQLLEN intVal;
  std::string resVal;
  bool found;

  // test retrieving std::string value

  // test SQL_DESC_LABEL
  found = columnMeta.GetAttribute(SQL_DESC_LABEL, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_BASE_COLUMN_NAME
  found = columnMeta.GetAttribute(SQL_DESC_BASE_COLUMN_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_NAME
  found = columnMeta.GetAttribute(SQL_DESC_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_TABLE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_TABLE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, table);

  // test SQL_DESC_BASE_TABLE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_BASE_TABLE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, table);

  // test SQL_DESC_SCHEMA_NAME
  found = columnMeta.GetAttribute(SQL_DESC_SCHEMA_NAME, resVal);
  BOOST_CHECK(found);
  if (DATABASE_AS_SCHEMA) {
    BOOST_CHECK_EQUAL(resVal, database);
  } else {
    BOOST_CHECK_EQUAL(resVal, "");
  }

  // test SQL_DESC_CATALOG_NAME
  found = columnMeta.GetAttribute(SQL_DESC_CATALOG_NAME, resVal);
  BOOST_CHECK(found);
  if (DATABASE_AS_SCHEMA)
    BOOST_CHECK_EQUAL(resVal, "");
  else
    BOOST_CHECK_EQUAL(resVal, database);

  // test SQL_DESC_LITERAL_PREFIX
  found = columnMeta.GetAttribute(SQL_DESC_LITERAL_PREFIX, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, "'");

  // test SQL_DESC_LITERAL_SUFFIX
  found = columnMeta.GetAttribute(SQL_DESC_LITERAL_SUFFIX, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, "'");

  // test SQL_DESC_TYPE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_TYPE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, SqlTypeName::VARCHAR);

  // test SQL_DESC_LOCAL_TYPE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_LOCAL_TYPE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, SqlTypeName::VARCHAR);

  // fields SQL_COLUMN_PRECISION and SQL_DESC_SCALE are not tested
  // for retrieving string values

  // test retrieving SQLLEN value

  // test SQL_DESC_FIXED_PREC_SCALE
  found = columnMeta.GetAttribute(SQL_DESC_FIXED_PREC_SCALE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_FALSE);

  // test SQL_DESC_AUTO_UNIQUE_VALUE
  found = columnMeta.GetAttribute(SQL_DESC_AUTO_UNIQUE_VALUE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_FALSE);

  // test SQL_DESC_CASE_SENSITIVE
  found = columnMeta.GetAttribute(SQL_DESC_CASE_SENSITIVE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_TRUE);

  // test SQL_DESC_CONCISE_TYPE
  found = columnMeta.GetAttribute(SQL_DESC_CONCISE_TYPE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_VARCHAR);

  // test SQL_DESC_TYPE
  found = columnMeta.GetAttribute(SQL_DESC_TYPE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_VARCHAR);

  // test SQL_DESC_DISPLAY_SIZE
  found = columnMeta.GetAttribute(SQL_DESC_DISPLAY_SIZE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, IOTSITEWISE_SQL_MAX_LENGTH);

  // test SQL_DESC_LENGTH
  found = columnMeta.GetAttribute(SQL_DESC_LENGTH, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, IOTSITEWISE_SQL_MAX_LENGTH);

  // test SQL_DESC_OCTET_LENGTH
  found = columnMeta.GetAttribute(SQL_DESC_OCTET_LENGTH, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, IOTSITEWISE_SQL_MAX_LENGTH);

  // test SQL_DESC_NULLABLE
  found = columnMeta.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NULLABLE);

  // test SQL_DESC_NUM_PREC_RADIX
  found = columnMeta.GetAttribute(SQL_DESC_NUM_PREC_RADIX, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, 0);

  // test SQL_DESC_PRECISION
  found = columnMeta.GetAttribute(SQL_DESC_PRECISION, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, IOTSITEWISE_SQL_MAX_LENGTH);

  // test SQL_DESC_SCALE
  found = columnMeta.GetAttribute(SQL_DESC_SCALE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, -1);

  // test SQL_DESC_SEARCHABLE
  found = columnMeta.GetAttribute(SQL_DESC_SEARCHABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_PRED_BASIC);

  // test SQL_DESC_UNNAMED
  found = columnMeta.GetAttribute(SQL_DESC_UNNAMED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NAMED);

  // test SQL_DESC_UNSIGNED
  found = columnMeta.GetAttribute(SQL_DESC_UNSIGNED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_TRUE);

  // test SQL_DESC_UPDATABLE
  found = columnMeta.GetAttribute(SQL_DESC_UPDATABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_ATTR_READWRITE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLiteralPrefix) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, std::string > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     std::string("'")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     std::string(""))};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving std::string value

    // test SQL_DESC_LITERAL_PREFIX
    found = columnMeta.GetAttribute(SQL_DESC_LITERAL_PREFIX, resVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(resVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLiteralSuffix) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, std::string > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     std::string("'")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     std::string(""))};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving std::string value

    // test SQL_DESC_LITERAL_SUFFIX
    found = columnMeta.GetAttribute(SQL_DESC_LITERAL_SUFFIX, resVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(resVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLocalTypeName) {
  using namespace iotsitewise::odbc::type_traits;

  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, std::string > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     iotsitewise::odbc::type_traits::SqlTypeName::VARCHAR),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     iotsitewise::odbc::type_traits::SqlTypeName::BIT),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     iotsitewise::odbc::type_traits::SqlTypeName::DOUBLE),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     iotsitewise::odbc::type_traits::SqlTypeName::TIMESTAMP),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     iotsitewise::odbc::type_traits::SqlTypeName::INTEGER),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     iotsitewise::odbc::type_traits::SqlTypeName::NOT_SET)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving std::string value

    // test SQL_DESC_LOCAL_TYPE_NAME
    found = columnMeta.GetAttribute(SQL_DESC_LOCAL_TYPE_NAME, resVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(resVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeCaseSensitive) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     true),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     false)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_CASE_SENSITIVE
    found = columnMeta.GetAttribute(SQL_DESC_CASE_SENSITIVE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeConciseTypeAndType) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     SQL_VARCHAR),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     SQL_BIT),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     SQL_DOUBLE),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     SQL_TYPE_TIMESTAMP),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     SQL_INTEGER),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     SQL_VARCHAR)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_CONCISE_TYPE
    found = columnMeta.GetAttribute(SQL_DESC_CONCISE_TYPE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);

    // test SQL_DESC_TYPE
    found = columnMeta.GetAttribute(SQL_DESC_TYPE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeDisplaySize) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     IOTSITEWISE_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     1),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     24),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     20),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     11),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     IOTSITEWISE_SQL_MAX_LENGTH)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_DISPLAY_SIZE
    found = columnMeta.GetAttribute(SQL_DESC_DISPLAY_SIZE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLength) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     IOTSITEWISE_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     1),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     24),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     20),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     11),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     IOTSITEWISE_SQL_MAX_LENGTH)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value
    // test SQL_DESC_LENGTH
    found = columnMeta.GetAttribute(SQL_DESC_LENGTH, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeOctetLength) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;
  size_t size_of_char = sizeof(char);

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     IOTSITEWISE_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     1),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     8),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     16),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     4),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     IOTSITEWISE_SQL_MAX_LENGTH)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_OCTET_LENGTH
    found = columnMeta.GetAttribute(SQL_DESC_OCTET_LENGTH, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeNullable) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;
  ColumnMeta columnMetaNullable(database, table, column, ScalarType::NOT_SET,
                                Nullability::NULLABLE);

  // test SQL_DESC_NULLABLE
  found = columnMetaNullable.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NULLABLE);

  ColumnMeta columnMetaNoNulls(database, table, column, ScalarType::NOT_SET,
                               Nullability::NO_NULL);

  // test SQL_DESC_NULLABLE
  found = columnMetaNoNulls.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NO_NULLS);

  ColumnMeta columnMetaUnknown(database, table, column, ScalarType::NOT_SET,
                               Nullability::NULLABILITY_UNKNOWN);

  // test SQL_DESC_NULLABLE
  found = columnMetaUnknown.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NULLABLE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestGetAttributeNumPrecRadix) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     0),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     10),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     2),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     0),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     10),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     0)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_NUM_PREC_RADIX
    found = columnMeta.GetAttribute(SQL_DESC_NUM_PREC_RADIX, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributePrecision) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     IOTSITEWISE_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     1),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     15),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     19),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     10),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     IOTSITEWISE_SQL_MAX_LENGTH)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_PRECISION
    found = columnMeta.GetAttribute(SQL_DESC_PRECISION, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeScale) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     -1),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     -1),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     15),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     -1),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     0),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     -1)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_SCALE
    found = columnMeta.GetAttribute(SQL_DESC_SCALE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeUnnamed) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;
  ColumnMeta columnMetaUnnamed(database, table, std::string(""),
                               ScalarType::NOT_SET, Nullability::NULLABLE);

  // test SQL_DESC_UNNAMED
  found = columnMetaUnnamed.GetAttribute(SQL_DESC_UNNAMED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_UNNAMED);

  ColumnMeta columnMetaNamed(database, table, column, ScalarType::NOT_SET,
                             Nullability::NULLABLE);

  // test SQL_DESC_UNNAMED
  found = columnMetaNamed.GetAttribute(SQL_DESC_UNNAMED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NAMED);
}

BOOST_AUTO_TEST_CASE(TestGetAttributeUnsigned) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::STRING),
                     true),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::BOOLEAN),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::DOUBLE),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP),
                     true),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::INT),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::IoTSiteWise::Model::ScalarType::NOT_SET),
                     true)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_UNSIGNED
    found = columnMeta.GetAttribute(SQL_DESC_UNSIGNED, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}
