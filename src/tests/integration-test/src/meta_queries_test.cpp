/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modifications Copyright Amazon.com, Inc. or its affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/test/tools/old/interface.hpp>
#ifdef _WIN32
#include <Windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <string>
#include <vector>

#include "iotsitewise/odbc/type_traits.h"
#include "iotsitewise/odbc/utility.h"
#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace iotsitewise;
using namespace iotsitewise_test;
using namespace iotsitewise::odbc;
using namespace iotsitewise::odbc::type_traits;
using namespace iotsitewise::odbc::utility;

using namespace boost::unit_test;

#ifdef __APPLE__
constexpr auto FUNCTION_SEQUENCE_ERROR_STATE = "S1010";
#endif

constexpr auto INVALID_CURSOR_STATE = "24000";

/**
 * Test setup fixture.
 */
struct MetaQueriesTestSuiteFixture : public odbc::OdbcTestSuite {
  const static SQLLEN C_STR_LEN_DEFAULT = 1024;

  /**
   * Converts SQLCHAR[] to std::string
   *
   * @param strBuf SQLCHAR pointer
   * @return buf std::string
   */
  std::string SqlCharToString(SQLCHAR *strBuf) {
    std::stringstream bufStream;
    bufStream << strBuf;
    std::string buf;
    bufStream >> buf;

    return buf;
  }

  /**
   * Checks single row result set for correct work with SQLGetData.
   *
   * @param stmt Statement.
   */
  void CheckSingleRowResultSetWithGetData(
      SQLHSTMT stmt, SQLUSMALLINT columnIndex = 1,
      const std::string expectedValue = "", bool checkOtherValEmpty = false,
#ifdef __APPLE__
      const std::string expectedErrorState =
          FUNCTION_SEQUENCE_ERROR_STATE) const {
#else
      const std::string expectedErrorState = INVALID_CURSOR_STATE) const {
#endif
    SQLRETURN ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret)) {
      std::string sqlMessage = GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt);
      if (sqlMessage.empty()) {
        sqlMessage.append("SQLFetch returned: " + std::to_string(ret));
      }
      // Handle the case of ret equals SQL_NO_DATA
      if (ret == SQL_NO_DATA) {
        sqlMessage = "SQL_NO_DATA is returned from SQLFetch. " + sqlMessage;
      }
      BOOST_FAIL(sqlMessage);
    }

    SQLWCHAR buf[1024];
    SQLLEN bufLen = sizeof(buf);

    columnIndex = columnIndex >= 1 ? columnIndex : 1;
    for (int i = 1; i <= columnIndex; i++) {
      ret = SQLGetData(stmt, i, SQL_C_WCHAR, buf, sizeof(buf), &bufLen);

      if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }

      std::string actualValueStr = utility::SqlWcharToString(buf, bufLen);
      if (i == columnIndex && !expectedValue.empty()) {
        BOOST_CHECK_EQUAL(expectedValue, actualValueStr);
      } else if (checkOtherValEmpty) {
        // check that values at other column indeces are empty strings
        BOOST_CHECK_EQUAL("", actualValueStr);
      }
    }

    ret = SQLFetch(stmt);

    BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);

    ret = SQLGetData(stmt, 1, SQL_C_WCHAR, buf, sizeof(buf), &bufLen);

    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    BOOST_CHECK_EQUAL(GetOdbcErrorState(SQL_HANDLE_STMT, stmt),
                      expectedErrorState);
  }

  /**
   * Constructor.
   */
  MetaQueriesTestSuiteFixture() = default;

  bool WasNull(const SQLLEN length) const {
    return (length == SQL_NULL_DATA);
  }

  void SQLColumnsBindColumns(
      SQLHSTMT stmt, char table_cat[], SQLLEN &table_cat_len,
      char table_schem[], SQLLEN &table_schem_len, char table_name[],
      SQLLEN &table_name_len, char column_name[], SQLLEN &column_name_len,
      SQLSMALLINT &data_type, SQLLEN &data_type_len, char type_name[],
      SQLLEN &type_name_len, SQLINTEGER &column_size, SQLLEN &column_size_len,
      SQLINTEGER &buffer_length, SQLLEN &buffer_length_len,
      SQLSMALLINT &decimal_digits, SQLLEN &decimal_digits_len,
      SQLSMALLINT &num_prec_radix, SQLLEN &num_prec_radix_len,
      SQLSMALLINT &nullable, SQLLEN &nullable_len, char remarks[],
      SQLLEN &remarks_len, char column_def[], SQLLEN &column_def_len,
      SQLSMALLINT &sql_data_type, SQLLEN &sql_data_type_len,
      SQLSMALLINT &sql_datetime_sub, SQLLEN &sql_datetime_sub_len,
      SQLINTEGER &char_octet_length, SQLLEN &char_octet_length_len,
      SQLINTEGER &ordinal_position, SQLLEN &ordinal_position_len,
      char is_nullable[], SQLLEN &is_nullable_len) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLBindCol(stmt, 1, SQL_C_CHAR, table_cat, C_STR_LEN_DEFAULT,
                     &table_cat_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 2, SQL_C_CHAR, table_schem, C_STR_LEN_DEFAULT,
                     &table_schem_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 3, SQL_C_CHAR, table_name, C_STR_LEN_DEFAULT,
                     &table_name_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 4, SQL_C_CHAR, column_name, C_STR_LEN_DEFAULT,
                     &column_name_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 5, SQL_SMALLINT, &data_type, sizeof(data_type),
                     &data_type_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 6, SQL_C_CHAR, type_name, C_STR_LEN_DEFAULT,
                     &type_name_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 7, SQL_INTEGER, &column_size, sizeof(column_size),
                     &column_size_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 8, SQL_INTEGER, &buffer_length,
                     sizeof(buffer_length), &buffer_length_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 9, SQL_SMALLINT, &decimal_digits,
                     sizeof(decimal_digits), &decimal_digits_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 10, SQL_SMALLINT, &num_prec_radix,
                     sizeof(num_prec_radix), &num_prec_radix_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 11, SQL_SMALLINT, &nullable, sizeof(nullable),
                     &nullable_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 12, SQL_C_CHAR, remarks, C_STR_LEN_DEFAULT,
                     &remarks_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 13, SQL_C_CHAR, column_def, C_STR_LEN_DEFAULT,
                     &column_def_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 14, SQL_SMALLINT, &sql_data_type,
                     sizeof(sql_data_type), &sql_data_type_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 15, SQL_SMALLINT, &sql_datetime_sub,
                     sizeof(sql_datetime_sub), &sql_datetime_sub_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 16, SQL_INTEGER, &char_octet_length,
                     sizeof(char_octet_length), &char_octet_length_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 17, SQL_INTEGER, &ordinal_position,
                     sizeof(ordinal_position), &ordinal_position_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 18, SQL_C_CHAR, is_nullable, C_STR_LEN_DEFAULT,
                     &is_nullable_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
  }

  /**
   * Check attribute using SQLColAttribute.
   * The value returned from SQLColAttribute should match the expected value.
   *
   * @param stmt Statement.
   * @param query SQL Query.
   * @param fieldId Field Identifier.
   * @param expectedVal Expected string data.
   */
  void callSQLColAttribute(SQLHSTMT stmt, const SQLCHAR *query,
                           SQLSMALLINT fieldId,
                           const std::string &expectedVal) {
    SQLWCHAR strBuf[1024] = {0};
    std::vector< SQLWCHAR > wQuery =
        MakeSqlBuffer(reinterpret_cast< const char * >(query));

    SQLRETURN ret = SQLExecDirect(stmt, wQuery.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret) && ret != SQL_NO_DATA) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    // resLen (6th parameter of SQLColAttribute) is unused, but it needs to be
    // defined for macOS, as iODBC will attempt to access resLen when strBuf is
    // non-empty string. This behavior is out of the driver's control.
    SQLSMALLINT resLen = 0;
    ret = SQLColAttribute(stmt, 1, fieldId, strBuf, sizeof(strBuf), &resLen,
                          nullptr);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    std::string buf = utility::SqlWcharToString(strBuf);

    BOOST_CHECK(expectedVal == buf);
  }

  /**
   * Check attribute using SQLColAttribute.
   * The value returned from SQLColAttribute should match the expected value.
   *
   * @param stmt Statement.
   * @param query SQL Query.
   * @param fieldId Field Identifier.
   * @param expectedVal Expected int data.
   */
  void callSQLColAttribute(SQLHSTMT stmt, const SQLCHAR *query,
                           SQLSMALLINT fieldId, const int &expectedVal) {
    SQLLEN intVal;
    std::vector< SQLWCHAR > wQuery =
        MakeSqlBuffer(reinterpret_cast< const char * >(query));

    SQLRETURN ret = SQLExecDirect(stmt, wQuery.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret) && ret != SQL_NO_DATA) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    ret = SQLColAttribute(stmt, 1, fieldId, nullptr, 0, nullptr, &intVal);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(intVal, expectedVal);
  }

  /**
   * Check attribute using SQLColAttributes.
   * The value returned from SQLColAttributes should match the expected value.
   *
   * @param stmt Statement.
   * @param query SQL Query.
   * @param fieldId Field Identifier.
   * @param expectedVal Expected string data.
   */
  void callSQLColAttributes(SQLHSTMT stmt, const SQLCHAR *query,
                            SQLSMALLINT fieldId,
                            const std::string &expectedVal) {
    SQLWCHAR strBuf[1024] = {0};
    std::vector< SQLWCHAR > wQuery =
        MakeSqlBuffer(reinterpret_cast< const char * >(query));

    SQLRETURN ret = SQLExecDirect(stmt, wQuery.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret) && ret != SQL_NO_DATA) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    // resLen (6th parameter of SQLColAttributes) is unused, but it needs to be
    // defined for macOS, as iODBC will attempt to access resLen when strBuf is
    // non-empty string. This behavior is out of the driver's control.
    SQLSMALLINT resLen = 0;
    ret = SQLColAttributes(stmt, 1, fieldId, strBuf, sizeof(strBuf), &resLen,
                           nullptr);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    std::string buf = utility::SqlWcharToString(strBuf);

    BOOST_CHECK(expectedVal == buf);
  }

  /**
   * Check attribute using SQLColAttributes.
   * The value returned from SQLColAttributes should match the expected value.
   *
   * @param stmt Statement.
   * @param query SQL Query.
   * @param fieldId Field Identifier.
   * @param expectedVal Expected int data.
   */
  void callSQLColAttributes(SQLHSTMT stmt, const SQLCHAR *query,
                            SQLSMALLINT fieldId, const int &expectedVal) {
    SQLLEN intVal;
    std::vector< SQLWCHAR > wQuery =
        MakeSqlBuffer(reinterpret_cast< const char * >(query));

    SQLRETURN ret = SQLExecDirect(stmt, wQuery.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret) && ret != SQL_NO_DATA) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    ret = SQLColAttributes(stmt, 1, fieldId, nullptr, 0, nullptr, &intVal);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(intVal, expectedVal);
  }

  /**
   * Check result set column metadata using SQLDescribeCol.
   *
   * @param stmt Statement.
   * @param idx Index.
   * @param expName Expected name.
   * @param expDataType Expected data type.
   * @param expSize Expected column size.
   * @param expScale Expected column scale.
   * @param expNullability expected nullability.
   */
  void CheckColumnMetaWithSQLDescribeCol(SQLHSTMT stmt, SQLUSMALLINT idx,
                                         const std::string &expName,
                                         SQLSMALLINT expDataType,
                                         SQLULEN expSize, SQLSMALLINT expScale,
                                         SQLSMALLINT expNullability) const {
    std::vector< SQLWCHAR > name(ODBC_BUFFER_SIZE);
    SQLSMALLINT nameLen = 0;
    SQLSMALLINT dataType = 0;
    SQLULEN size;
    SQLSMALLINT scale;
    SQLSMALLINT nullability;

    SQLRETURN ret =
        SQLDescribeCol(stmt, idx, &name[0], (SQLSMALLINT)name.size(), &nameLen,
                       &dataType, &size, &scale, &nullability);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    BOOST_CHECK_GE(nameLen, 0);
    BOOST_CHECK_LE(nameLen, static_cast< SQLSMALLINT >(ODBC_BUFFER_SIZE));

    BOOST_CHECK_EQUAL(utility::SqlWcharToString(name.data()), expName);
    BOOST_CHECK_EQUAL(dataType, expDataType);
    BOOST_CHECK_EQUAL(size, expSize);
    BOOST_CHECK_EQUAL(scale, expScale);
    BOOST_CHECK_EQUAL(nullability, expNullability);
  }

  /**
   * @param func Function to call before tests. May be PrepareQuery or
   * ExecQuery.
   *
   * 1. Start node.
   * 2. Connect to node using ODBC.
   * 3. Create table with decimal and char columns with specified size and
   * scale.
   * 4. Execute or prepare statement.
   * 5. Check presicion and scale of every column using SQLDescribeCol.
   */
  template < typename F >
  void CheckSQLDescribeColPrecisionAndScale(F func) {
    ConnectToSW();

    SQLRETURN ret = (this->*func)(
        "SELECT 'string', true, 515.0, 1 FROM system.tables LIMIT 1");
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    SQLSMALLINT columnCount = 0;

    ret = SQLNumResultCols(stmt, &columnCount);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    BOOST_CHECK_EQUAL(columnCount, 4);

    CheckColumnMetaWithSQLDescribeCol(stmt, 1, "_1", SQL_VARCHAR,
                                      IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                      SQL_NULLABLE_UNKNOWN);
    CheckColumnMetaWithSQLDescribeCol(stmt, 2, "_2", SQL_BIT, 1, -1,
                                      SQL_NULLABLE_UNKNOWN);
    CheckColumnMetaWithSQLDescribeCol(stmt, 3, "_3", SQL_DOUBLE,
                                      15, 15, SQL_NULLABLE_UNKNOWN);
    CheckColumnMetaWithSQLDescribeCol(stmt, 4, "_4", SQL_INTEGER,
                                      10, 0, SQL_NULLABLE_UNKNOWN);
  }

  /**
   * Check result set column metadata using SQLColAttribute.
   *
   * @param stmt Statement.
   * @param idx Index.
   * @param expName Expected name.
   * @param expDataType Expected data type.
   * @param expSize Expected column size.
   * @param expScale Expected column scale.
   * @param expNullability expected nullability.
   */
  void CheckColumnMetaWithSQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT idx,
                                          const std::string &expName,
                                          SQLLEN expDataType, SQLULEN expSize,
                                          SQLLEN expScale,
                                          SQLLEN expNullability) {
    std::vector< SQLWCHAR > name(ODBC_BUFFER_SIZE);
    SQLSMALLINT nameLen = 0;
    SQLLEN dataType = 0;
    SQLLEN size;
    SQLLEN scale;
    SQLLEN nullability;

    SQLRETURN ret = SQLColAttribute(stmt, idx, SQL_DESC_NAME, &name[0],
                                    (SQLSMALLINT)name.size() * sizeof(SQLWCHAR),
                                    &nameLen, nullptr);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, idx, SQL_DESC_TYPE, 0, 0, 0, &dataType);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, idx, SQL_DESC_PRECISION, 0, 0, 0, &size);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, idx, SQL_DESC_SCALE, 0, 0, 0, &scale);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, idx, SQL_DESC_NULLABLE, 0, 0, 0, &nullability);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    BOOST_CHECK_GE(nameLen, 0);
    BOOST_CHECK_LE(nameLen, static_cast< SQLSMALLINT >(ODBC_BUFFER_SIZE));

    BOOST_CHECK_EQUAL(utility::SqlWcharToString(name.data()), expName);
    BOOST_CHECK_EQUAL(dataType, expDataType);
    BOOST_CHECK_EQUAL(size, expSize);
    BOOST_CHECK_EQUAL(scale, expScale);
    BOOST_CHECK_EQUAL(nullability, expNullability);
  }

  /**
   * @param func Function to call before tests. May be PrepareQuery or
   * ExecQuery.
   *
   * 1. Start node.
   * 2. Connect to node using ODBC.
   * 3. Create table with decimal and char columns with specified size and
   * scale.
   * 4. Execute or prepare statement.
   * 5. Check presicion and scale of every column using SQLColAttribute.
   */
  template < typename F >
  void CheckSQLColAttributePrecisionAndScale(F func) {
    ConnectToSW();

    SQLRETURN ret = (this->*func)(
      "SELECT 'string', true, 515.0, 1 FROM system.tables LIMIT 1");
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    SQLSMALLINT columnCount = 0;

    ret = SQLNumResultCols(stmt, &columnCount);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    BOOST_CHECK_EQUAL(columnCount, 4);

    CheckColumnMetaWithSQLColAttribute(stmt, 1, "_1", SQL_VARCHAR,
                                       IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                       SQL_NULLABLE_UNKNOWN);
    CheckColumnMetaWithSQLColAttribute(stmt, 2, "_2", SQL_BIT, 1, -1,
                                       SQL_NULLABLE_UNKNOWN);
    CheckColumnMetaWithSQLColAttribute(stmt, 3, "_3", SQL_DOUBLE,
                                       15, 15, SQL_NULLABLE_UNKNOWN);
    CheckColumnMetaWithSQLColAttribute(stmt, 4, "_4",
                                       SQL_INTEGER, 10, 0, SQL_NULLABLE_UNKNOWN);
  }

  // check SQLGetTypeInfo result 1st and 2nd column
  void CheckSQLGetTypeInfoResult(const std::string &exptectedTypeName,
                                 int expectedDataType) {
    SQLRETURN ret = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    SQLWCHAR buf[1024];
    SQLLEN bufLen = sizeof(buf);

    ret = SQLGetData(stmt, 1, SQL_C_WCHAR, buf, sizeof(buf), &bufLen);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    std::string actualValueStr = utility::SqlWcharToString(buf, bufLen);
    BOOST_CHECK_EQUAL(exptectedTypeName, actualValueStr);

    SQLSMALLINT dataType;
    SQLLEN dataTypeLen;
    ret = SQLGetData(stmt, 2, SQL_SMALLINT, &dataType, sizeof(dataType),
                     &dataTypeLen);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(expectedDataType, dataType);
  }

  /**
   * Destructor.
   */
  ~MetaQueriesTestSuiteFixture() {
    // No-op.
  }
};

BOOST_FIXTURE_TEST_SUITE(MetaQueriesTestSuite, MetaQueriesTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestSQLDescribeColGetTypeInfo) {
  ConnectToSW();

  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_VARCHAR);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 19);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "TYPE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "DATA_TYPE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "COLUMN_SIZE", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "LITERAL_PREFIX", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "LITERAL_SUFFIX", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "CREATE_PARAMS", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 7, "NULLABLE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 8, "CASE_SENSITIVE", SQL_INTEGER, 10,
                                    0, SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 9, "SEARCHABLE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 10, "UNSIGNED_ATTRIBUTE", SQL_INTEGER,
                                    10, 0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 11, "FIXED_PREC_SCALE", SQL_INTEGER,
                                    10, 0, SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 12, "AUTO_UNIQUE_VALUE", SQL_INTEGER,
                                    10, 0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 13, "LOCAL_TYPE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 14, "MINIMUM_SCALE", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 15, "MAXIMUM_SCALE", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 16, "SQL_DATA_TYPE", SQL_INTEGER, 10,
                                    0, SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 17, "SQL_DATETIME_SUB", SQL_INTEGER,
                                    10, 0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 18, "NUM_PREC_RADIX", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 19, "INTERVAL_PRECISION", SQL_INTEGER,
                                    10, 0, SQL_NULLABLE);
}

BOOST_AUTO_TEST_CASE(TestGetTypeInfoAllTypes) {
  ConnectToSW();

  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_ALL_TYPES);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  CheckSQLGetTypeInfoResult("VARCHAR", SQL_VARCHAR);
  CheckSQLGetTypeInfoResult("BIT", SQL_BIT);
  CheckSQLGetTypeInfoResult("DOUBLE", SQL_DOUBLE);
  CheckSQLGetTypeInfoResult("TIMESTAMP", SQL_TYPE_TIMESTAMP);
  CheckSQLGetTypeInfoResult("INTEGER", SQL_INTEGER);
  CheckSQLGetTypeInfoResult("NOT_SET", SQL_VARCHAR);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnLength) {
  ConnectToSW();

  std::vector< SQLWCHAR > req = MakeSqlBuffer(
      "SELECT 515.0 FROM system.tables LIMIT 1");
  SQLExecDirect(stmt, req.data(), SQL_NTS);

  SQLLEN intVal;
  SQLWCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_LENGTH, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(intVal, 24);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnPresicion) {
  ConnectToSW();

  std::vector< SQLWCHAR > req = MakeSqlBuffer(
    "SELECT 515.0 FROM system.tables LIMIT 1");
  SQLExecDirect(stmt, req.data(), SQL_NTS);

  SQLLEN intVal;
  SQLWCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_PRECISION, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(intVal, 15);
}

BOOST_AUTO_TEST_CASE(TestColAttributeWithOneTable) {
  ConnectToSW();

  std::pair< int16_t, std::string > tests[] = {
      std::make_pair(SQL_VARCHAR, std::string("_1")),
      std::make_pair(SQL_VARCHAR, std::string("_2")),
      std::make_pair(SQL_DOUBLE, std::string("_3"))};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, std::string >);

  std::vector< SQLWCHAR > req = MakeSqlBuffer(
    "SELECT 'asset_id', 'asset_name', 515.0 FROM system.tables LIMIT 1");
  SQLLEN intVal;
  SQLSMALLINT strLen;
  SQLWCHAR strBuf[1024];

  SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret) && ret != SQL_NO_DATA) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  for (int i = 1; i <= numTests; i++) {
    ret = SQLColAttribute(stmt, SQLUSMALLINT(i), SQL_DESC_TYPE, nullptr, 0,
                          nullptr, &intVal);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(intVal, tests[i - 1].first);

    ret = SQLColAttribute(stmt, SQLUSMALLINT(i), SQL_DESC_NAME, strBuf,
                          sizeof(strBuf), &strLen, &intVal);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(utility::SqlWcharToString(strBuf), tests[i - 1].second);
  }
}

BOOST_AUTO_TEST_CASE(TestColAttributeDataTypesAndColumnNames) {
  ConnectToSW();

  std::pair< int16_t, std::string > tests[] = {
      std::make_pair(SQL_VARCHAR, std::string("_1")),
      std::make_pair(SQL_VARCHAR, std::string("_2")),
      std::make_pair(SQL_DOUBLE, std::string("_3"))};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, std::string >);

  std::vector< SQLWCHAR > req = MakeSqlBuffer(
    "SELECT 'asset_id', 'asset_name', 515.0 FROM system.tables LIMIT 1");
  SQLLEN intVal;
  SQLSMALLINT strLen;
  SQLWCHAR strBuf[1024];

  SQLExecDirect(stmt, req.data(), SQL_NTS);

  for (int i = 1; i <= numTests; i++) {
    SQLRETURN ret = SQLColAttribute(stmt, SQLUSMALLINT(i), SQL_DESC_TYPE,
                                    nullptr, 0, nullptr, &intVal);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(intVal, tests[i - 1].first);

    ret = SQLColAttribute(stmt, SQLUSMALLINT(i), SQL_DESC_NAME, strBuf,
                          sizeof(strBuf), &strLen, &intVal);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(utility::SqlWcharToString(strBuf), tests[i - 1].second);
  }
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescAutoUniqueValue) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // only "NO" is returned for IS_AUTOINCREMENT field
  callSQLColAttribute(stmt, req, SQL_DESC_AUTO_UNIQUE_VALUE, SQL_FALSE);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescBaseColumnName) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req, SQL_DESC_BASE_COLUMN_NAME,
                      std::string("table_name"));
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescBaseTableName) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // Table names are empty
  callSQLColAttribute(stmt, req, SQL_DESC_BASE_TABLE_NAME, std::string(""));
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescCaseSensitive) {
  ConnectToSW();

  // test that case sensitive returns true for string field.
  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req1, SQL_DESC_CASE_SENSITIVE, SQL_TRUE);

  // test that case sensitive returns false for double field.
  const SQLCHAR req2[] = "SELECT 515.0 FROM system.tables LIMIT 1";


  callSQLColAttribute(stmt, req2, SQL_DESC_CASE_SENSITIVE, SQL_FALSE);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescCatalogName) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // check that catalog should be empty
  callSQLColAttribute(stmt, req, SQL_DESC_CATALOG_NAME, std::string(""));
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescTypeName) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req, SQL_DESC_TYPE_NAME, std::string("VARCHAR"));
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescConciseType) {
  ConnectToSW();

  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req1, SQL_DESC_CONCISE_TYPE, SQL_VARCHAR);

  const SQLCHAR req2[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  callSQLColAttribute(stmt, req2, SQL_DESC_CONCISE_TYPE, SQL_DOUBLE);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescCount) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // count should be 1
  callSQLColAttribute(stmt, req, SQL_DESC_COUNT, 1);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescDisplaySize) {
  ConnectToSW();

  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // SQL_VARCHAR should have display size IOTSITEWISE_SQL_MAX_LENGTH
  callSQLColAttribute(stmt, req1, SQL_DESC_DISPLAY_SIZE,
                      IOTSITEWISE_SQL_MAX_LENGTH);

  const SQLCHAR req2[] = "SELECT 1 FROM system.tables LIMIT 1";

  // SQL_INTEGER should have display size 11
  callSQLColAttribute(stmt, req2, SQL_DESC_DISPLAY_SIZE, 11);

  const SQLCHAR req3[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  // SQL_DOUBLE should have display size 24
  callSQLColAttribute(stmt, req3, SQL_DESC_DISPLAY_SIZE, 24);

  const SQLCHAR req4[] = "SELECT true FROM system.tables LIMIT 1";

  // SQL_BIT should have display size 1
  callSQLColAttribute(stmt, req4, SQL_DESC_DISPLAY_SIZE, 1);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescFixedPrecScale) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // only SQL_FALSE is returned
  callSQLColAttribute(stmt, req, SQL_DESC_FIXED_PREC_SCALE, SQL_FALSE);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescLabel) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req, SQL_DESC_LABEL, std::string("table_name"));
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescLength) {
  ConnectToSW();

  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // SQL_VARCHAR should have length IOTSITEWISE_SQL_MAX_LENGTH
  callSQLColAttribute(stmt, req1, SQL_DESC_LENGTH, IOTSITEWISE_SQL_MAX_LENGTH);

  const SQLCHAR req2[] = "SELECT 1 FROM system.tables LIMIT 1";

  // SQL_INTEGER should have length 11
  callSQLColAttribute(stmt, req2, SQL_DESC_LENGTH, 11);

  const SQLCHAR req3[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  // SQL_DOUBLE should have length 24
  callSQLColAttribute(stmt, req3, SQL_DESC_LENGTH, 24);

  const SQLCHAR req4[] = "SELECT true FROM system.tables LIMIT 1";

  // SQL_BIT should have length 1
  callSQLColAttribute(stmt, req4, SQL_DESC_LENGTH, 1);

}

BOOST_AUTO_TEST_CASE(TestColAttributeDescLiteralPrefix) {
  ConnectToSW();

  // test that empty string is returned for non-char and non-binary type
  const SQLCHAR req1[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  callSQLColAttribute(stmt, req1, SQL_DESC_LITERAL_PREFIX, std::string(""));

  // test that "'" is returned for VARCHAR type
  const SQLCHAR req2[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req2, SQL_DESC_LITERAL_PREFIX, std::string("'"));
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescLiteralSuffix) {
  ConnectToSW();

  // test that empty string is returned for non-char and non-binary type
  const SQLCHAR req1[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  callSQLColAttribute(stmt, req1, SQL_DESC_LITERAL_SUFFIX, std::string(""));

  // test that "'" is returned for *CHAR type
  const SQLCHAR req2[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req2, SQL_DESC_LITERAL_SUFFIX, std::string("'"));
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescLocalTypeName) {
  ConnectToSW();

  const SQLCHAR req1[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  // SQL_DOUBLE should have type name SqlTypeName::DOUBLE
  callSQLColAttribute(stmt, req1, SQL_DESC_LOCAL_TYPE_NAME,
                      SqlTypeName::DOUBLE);

  const SQLCHAR req2[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // SQL_VARCHAR should have type name SqlTypeName::VARCHAR
  callSQLColAttribute(stmt, req2, SQL_DESC_LOCAL_TYPE_NAME,
                      SqlTypeName::VARCHAR);

  const SQLCHAR req3[] = "SELECT true FROM system.tables LIMIT 1";

  // SQL_BIT should have type name SqlTypeName::BIT
  callSQLColAttribute(stmt, req3, SQL_DESC_LOCAL_TYPE_NAME,
                      SqlTypeName::BIT);

  const SQLCHAR req4[] = "SELECT 1 FROM system.tables LIMIT 1";

  // SQL_INTEGER should have type name SqlTypeName::INTEGER
  callSQLColAttribute(stmt, req4, SQL_DESC_LOCAL_TYPE_NAME,
                      SqlTypeName::INTEGER);

}

BOOST_AUTO_TEST_CASE(TestColAttributeDescName) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req, SQL_DESC_NAME,
                      std::string("table_name"));
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescNullable) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req, SQL_DESC_NULLABLE, SQL_NULLABLE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescNumPrecRadix) {
  ConnectToSW();

  const SQLCHAR req1[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  // SQL_DOUBLE should have precision radix 2
  callSQLColAttribute(stmt, req1, SQL_DESC_NUM_PREC_RADIX, 2);

  const SQLCHAR req2[] = "SELECT 1 FROM system.tables LIMIT 1";

  // SQL_INTEGER should have precision radix 10
  callSQLColAttribute(stmt, req2, SQL_DESC_NUM_PREC_RADIX, 10);

  const SQLCHAR req3[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // SQL_VARCHAR (non-numeric type) should have precision radix 0
  callSQLColAttribute(stmt, req3, SQL_DESC_NUM_PREC_RADIX, 0);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescOctetLength) {
  ConnectToSW();

  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // SQL_VARCHAR should have octet length IOTSITEWISE_SQL_MAX_LENGTH
  callSQLColAttribute(stmt, req1, SQL_DESC_OCTET_LENGTH,
                      IOTSITEWISE_SQL_MAX_LENGTH);

  const SQLCHAR req2[] = "SELECT true FROM system.tables LIMIT 1";

  // SQL_BIT should have octet length 1 * sizeof(char)
  callSQLColAttribute(stmt, req2, SQL_DESC_OCTET_LENGTH, 1);

  const SQLCHAR req3[] = "SELECT 1 FROM system.tables LIMIT 1";

  // SQL_INTEGER should have octet length 4 * sizeof(char)
  callSQLColAttribute(stmt, req3, SQL_DESC_OCTET_LENGTH, 4);

  const SQLCHAR req4[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  // SQL_DOUBLE should have octet length 8 * sizeof(char)
  callSQLColAttribute(stmt, req4, SQL_DESC_OCTET_LENGTH, 8);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescPrecision) {
  ConnectToSW();

  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // SQL_VARCHAR should have precision IOTSITEWISE_SQL_MAX_LENGTH
  callSQLColAttribute(stmt, req1, SQL_DESC_PRECISION,
                      IOTSITEWISE_SQL_MAX_LENGTH);

  const SQLCHAR req2[] = "SELECT true FROM system.tables LIMIT 1";

  // SQL_BIT should have precision 1
  callSQLColAttribute(stmt, req2, SQL_DESC_PRECISION, 1);

  const SQLCHAR req3[] = "SELECT 1 FROM system.tables LIMIT 1";

  // SQL_INTEGER should precision 19
  callSQLColAttribute(stmt, req3, SQL_DESC_PRECISION, 10);

  const SQLCHAR req4[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  // SQL_DOUBLE should have precision 15
  callSQLColAttribute(stmt, req4, SQL_DESC_PRECISION, 15);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescScale) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT 1 FROM system.tables LIMIT 1";

  // default scale value is 0
  callSQLColAttribute(stmt, req, SQL_DESC_SCALE, 0);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescSchemaName) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // schema name is empty
  callSQLColAttribute(stmt, req, SQL_DESC_SCHEMA_NAME, std::string(""));
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescSearchable) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // only SQL_PRED_BASIC is returned
  callSQLColAttribute(stmt, req, SQL_DESC_SEARCHABLE, SQL_PRED_BASIC);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescTableName) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // table name is not set for a column
  callSQLColAttribute(stmt, req, SQL_DESC_TABLE_NAME, "");
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescType) {
  ConnectToSW();

  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttribute(stmt, req1, SQL_DESC_TYPE, SQL_VARCHAR);

  const SQLCHAR req2[] = "SELECT 1 FROM system.tables LIMIT 1";

  callSQLColAttribute(stmt, req2, SQL_DESC_TYPE, SQL_INTEGER);

  const SQLCHAR req3[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  callSQLColAttribute(stmt, req3, SQL_DESC_TYPE, SQL_DOUBLE);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescUnnamed) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // all columns should be named bacause they cannot be null
  callSQLColAttribute(stmt, req, SQL_DESC_UNNAMED, SQL_NAMED);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescUnsigned) {
  ConnectToSW();

  const SQLCHAR req1[] = "SELECT 515.0 FROM system.tables LIMIT 1";

  // numeric type should be signed
  callSQLColAttribute(stmt, req1, SQL_DESC_UNSIGNED, SQL_FALSE);

  const SQLCHAR req2[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // non-numeric types should be unsigned
  callSQLColAttribute(stmt, req2, SQL_DESC_UNSIGNED, SQL_TRUE);
}

BOOST_AUTO_TEST_CASE(TestColAttributeDescUpdatable) {
  ConnectToSW();

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // only SQL_ATTR_READWRITE_UNKNOWN is returned
  callSQLColAttribute(stmt, req, SQL_DESC_UPDATABLE,
                      SQL_ATTR_READWRITE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnScale) {
  ConnectToSW();

  std::vector< SQLWCHAR > req = MakeSqlBuffer("SELECT 515.0 FROM system.tables LIMIT 1");
  SQLExecDirect(stmt, req.data(), SQL_NTS);

  SQLLEN intVal;
  SQLWCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_SCALE, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnLengthPrepare) {
  ConnectToSW();

  std::vector< SQLWCHAR > req = MakeSqlBuffer("SELECT 1 FROM system.tables LIMIT 1");
  SQLPrepare(stmt, req.data(), SQL_NTS);

  SQLLEN intVal;
  SQLWCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_LENGTH, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(intVal, 11);

  ret = SQLExecute(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLColAttribute(stmt, 1, SQL_COLUMN_LENGTH, strBuf, sizeof(strBuf),
                        &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(intVal, 11);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnPresicionPrepare) {
  ConnectToSW();

  std::vector< SQLWCHAR > req = MakeSqlBuffer("SELECT 1 FROM system.tables LIMIT 1");
  SQLPrepare(stmt, req.data(), SQL_NTS);

  SQLLEN intVal;
  SQLWCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_PRECISION, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(intVal, 10);

  ret = SQLExecute(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLColAttribute(stmt, 1, SQL_COLUMN_PRECISION, strBuf, sizeof(strBuf),
                        &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(intVal, 10);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnScalePrepare) {
  ConnectToSW();

  std::vector< SQLWCHAR > req = MakeSqlBuffer("SELECT 1 FROM system.tables LIMIT 1");
  SQLPrepare(stmt, req.data(), SQL_NTS);

  SQLLEN intVal;
  SQLWCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_SCALE, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLExecute(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLColAttribute(stmt, 1, SQL_COLUMN_SCALE, strBuf, sizeof(strBuf),
                        &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestGetDataWithGetTypeInfo) {
  ConnectToSW();

  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_VARCHAR);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  CheckSQLGetTypeInfoResult("VARCHAR", SQL_VARCHAR);
}

BOOST_AUTO_TEST_CASE(TestSQLDescribeColSQLColumns) {
  ConnectToSW();

  std::string dbNameStr = "data_queries_test_db";
  std::vector< SQLWCHAR > table = MakeSqlBuffer("TestScalarTypes");
  std::vector< SQLWCHAR > databaseName = MakeSqlBuffer(dbNameStr);
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, nullptr, 0, databaseName.data(), SQL_NTS,
                     table.data(), SQL_NTS, nullptr, 0);
  } else {
    ret = SQLColumns(stmt, databaseName.data(), SQL_NTS, nullptr, 0,
                     table.data(), SQL_NTS, nullptr, 0);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 18);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "TABLE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "TABLE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "TABLE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "COLUMN_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "DATA_TYPE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "TYPE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 7, "COLUMN_SIZE", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 8, "BUFFER_LENGTH", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 9, "DECIMAL_DIGITS", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 10, "NUM_PREC_RADIX", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 11, "NULLABLE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 12, "REMARKS", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 13, "COLUMN_DEF", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 14, "SQL_DATA_TYPE", SQL_INTEGER, 10,
                                    0, SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 15, "SQL_DATETIME_SUB", SQL_INTEGER,
                                    10, 0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 16, "CHAR_OCTET_LENGTH", SQL_INTEGER,
                                    10, 0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 17, "ORDINAL_POSITION", SQL_INTEGER,
                                    10, 0, SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 18, "IS_NULLABLE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsDataTypes) {
  ConnectToSW();

  std::string dbNameStr = "sample";
  std::vector< SQLWCHAR > table = MakeSqlBuffer("raw_time_series");
  std::vector< SQLWCHAR > databaseName = MakeSqlBuffer(dbNameStr);
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, nullptr, 0, databaseName.data(), SQL_NTS,
                     table.data(), SQL_NTS, nullptr, 0);
  } else {
    ret = SQLColumns(stmt, databaseName.data(), SQL_NTS, nullptr, 0,
                     table.data(), SQL_NTS, nullptr, 0);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  char column_name[C_STR_LEN_DEFAULT]{};
  SQLLEN column_name_len = sizeof(column_name);
  SQLSMALLINT data_type = 0;
  SQLLEN data_type_len = sizeof(data_type);
  char type_name[C_STR_LEN_DEFAULT]{};
  SQLLEN type_name_len = sizeof(type_name);
  SQLSMALLINT nullable = 0;
  SQLLEN nullable_len = sizeof(nullable);

  ret = SQLBindCol(stmt, 4, SQL_C_CHAR, column_name, sizeof(column_name),
                   &column_name_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));
  ret = SQLBindCol(stmt, 5, SQL_SMALLINT, &data_type, sizeof(data_type),
                   &data_type_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));
  ret = SQLBindCol(stmt, 6, SQL_C_CHAR, type_name, sizeof(type_name),
                   &type_name_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));
  ret = SQLBindCol(stmt, 11, SQL_SMALLINT, &nullable, sizeof(nullable),
                   &nullable_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL("asset_id", column_name);         // COLUMN_NAME
  BOOST_CHECK_EQUAL(SQL_VARCHAR, data_type);           // DATA_TYPE
  BOOST_CHECK_EQUAL(SqlTypeName::VARCHAR, type_name);  // TYPE_NAME
  BOOST_CHECK_EQUAL(SQL_NO_NULLS, nullable);           // TYPE_NAME

  // Currently at 1st column in the table, call SQLFetch(stmt) 5 times to go to
  // 5th column
  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL("int_value", column_name);                // COLUMN_NAME
  BOOST_CHECK_EQUAL(SQL_INTEGER, data_type);      // DATA_TYPE
  BOOST_CHECK_EQUAL(SqlTypeName::INTEGER, type_name);  // TYPE_NAME
  BOOST_CHECK_EQUAL(SQL_NULLABLE, nullable);             // TYPE_NAME

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL("boolean_value", column_name);          // COLUMN_NAME
  BOOST_CHECK_EQUAL(SQL_BIT, data_type);           // DATA_TYPE
  BOOST_CHECK_EQUAL(SqlTypeName::BIT, type_name);  // TYPE_NAME
  BOOST_CHECK_EQUAL(SQL_NULLABLE, nullable);       // TYPE_NAME

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL("double_value", column_name);  // COLUMN_NAME
  BOOST_CHECK_EQUAL(SQL_DOUBLE, data_type);             // DATA_TYPE
  BOOST_CHECK_EQUAL(SqlTypeName::DOUBLE, type_name);    // TYPE_NAME
  BOOST_CHECK_EQUAL(SQL_NULLABLE, nullable);            // TYPE_NAME

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL("string_value", column_name);  // COLUMN_NAME
  BOOST_CHECK_EQUAL(SQL_VARCHAR, data_type);              // DATA_TYPE
  BOOST_CHECK_EQUAL(SqlTypeName::VARCHAR, type_name);     // TYPE_NAME
  BOOST_CHECK_EQUAL(SQL_NULLABLE, nullable);             // TYPE_NAME

  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsTableNameOnly) {
  // Test SQLColumns by only passing table names and "%" without specifying
  // database Check that columns from tables with duplicate names are returned
  // correctly with SQLColumns.
  ConnectToSW();

  std::vector< SQLWCHAR > all = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("raw_time_series");
  SQLUSMALLINT databaseColumnIndex;
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    databaseColumnIndex = 2;
    ret = SQLColumns(stmt, all.data(), SQL_NTS, nullptr, 0, table.data(),
                     SQL_NTS, all.data(), SQL_NTS);
  } else {
    databaseColumnIndex = 1;
    ret = SQLColumns(stmt, nullptr, 0, all.data(), SQL_NTS, table.data(),
                     SQL_NTS, all.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  char database_name[C_STR_LEN_DEFAULT]{};
  SQLLEN database_name_len = sizeof(database_name);
  char column_name[C_STR_LEN_DEFAULT]{};
  SQLLEN column_name_len = sizeof(column_name);
  SQLSMALLINT data_type = 0;
  SQLLEN data_type_len = sizeof(data_type);
  char type_name[C_STR_LEN_DEFAULT]{};
  SQLLEN type_name_len = sizeof(type_name);
  SQLSMALLINT nullable = 0;
  SQLLEN nullable_len = sizeof(nullable);

  // databaseColumnIndex = 1 (TABLE_CAT) if database is reported as catalog,
  // 2 (TABLE_SCHEM) if database is reported as schema
  ret = SQLBindCol(stmt, databaseColumnIndex, SQL_C_CHAR, database_name,
                   sizeof(database_name), &database_name_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));
  ret = SQLBindCol(stmt, 4, SQL_C_CHAR, column_name, sizeof(column_name),
                   &column_name_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));
  ret = SQLBindCol(stmt, 5, SQL_SMALLINT, &data_type, sizeof(data_type),
                   &data_type_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));
  ret = SQLBindCol(stmt, 6, SQL_C_CHAR, type_name, sizeof(type_name),
                   &type_name_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));
  ret = SQLBindCol(stmt, 11, SQL_SMALLINT, &nullable, sizeof(nullable),
                   &nullable_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL("asset_id", column_name);             // COLUMN_NAME
  BOOST_CHECK_EQUAL(SQL_VARCHAR, data_type);           // DATA_TYPE
  BOOST_CHECK_EQUAL(SqlTypeName::VARCHAR, type_name);  // TYPE_NAME
  BOOST_CHECK_EQUAL(SQL_NO_NULLS, nullable);           // TYPE_NAME

  // Currently at 1st row in the table, call SQLFetch(stmt) 8 times to go to
  // 9th column
  for (int i = 0; i < 8; i++) {
    ret = SQLFetch(stmt);
    BOOST_TEST_MESSAGE("i = " + std::to_string(i));
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
  }

  BOOST_CHECK_EQUAL("string_value", column_name);             // COLUMN_NAME
  BOOST_CHECK_EQUAL(SQL_VARCHAR, data_type);           // DATA_TYPE
  BOOST_CHECK_EQUAL(SqlTypeName::VARCHAR, type_name);  // TYPE_NAME
  BOOST_CHECK_EQUAL(SQL_NULLABLE, nullable);           // TYPE_NAME
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsNull) {
  ConnectToSW();

  // database is empty case
  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("raw_time_series");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("int_value");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, empty.data(), SQL_NTS, nullptr, 0, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
  } else {
    ret = SQLColumns(stmt, nullptr, 0, empty.data(), SQL_NTS, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  int count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(--count == 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);

  // table is nullptr case
  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, empty.data(), SQL_NTS, nullptr, 0, nullptr, 0,
                     column.data(), SQL_NTS);
  } else {
    ret = SQLColumns(stmt, nullptr, 0, empty.data(), SQL_NTS, nullptr, 0,
                     column.data(), SQL_NTS);
  }

  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(--count > 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);

  // column is nullptr case
  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, empty.data(), SQL_NTS, nullptr, 0, nullptr, 0,
                     nullptr, 0);
  } else {
    ret = SQLColumns(stmt, nullptr, 0, empty.data(), SQL_NTS, nullptr, 0,
                     nullptr, 0);
  }

  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(--count > 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsEmptyMetadataIdTrue) {
  ConnectToSW();
  // Set SQL_ATTR_METADATA_ID to SQL_TRUE
  SQLRETURN ret = SQLSetConnectAttr(
      dbc, SQL_ATTR_METADATA_ID, reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);

  std::vector< SQLWCHAR > any = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > database = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("asset_id");

  // catalogName and schemaName are empty strings case
  // This should always return a warning
  ret = SQLColumns(stmt, empty.data(), SQL_NTS, empty.data(), SQL_NTS,
                   table.data(), SQL_NTS, column.data(), SQL_NTS);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  CheckSQLStatementDiagnosticError("01000");
  BOOST_REQUIRE_EQUAL("01000: catalogName and schemaName are empty strings.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  if (DATABASE_AS_SCHEMA) {
    // catalogName is empty case
    ret = SQLColumns(stmt, empty.data(), SQL_NTS, database.data(), SQL_NTS,
                     table.data(), SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
  } else {
    // catalogName is empty case
    ret = SQLColumns(stmt, empty.data(), SQL_NTS, any.data(), SQL_NTS,
                     table.data(), SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
    CheckSQLStatementDiagnosticError("01000");
    BOOST_REQUIRE_EQUAL("01000: Catalog and table name should not be empty.",
                        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsEmptyMetadataIdFalse) {
  ConnectToSW();

  std::vector< SQLWCHAR > any = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > database = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("asset_id");

  // catalogName and schemaName are empty strings case
  // This should always return a warning
  SQLRETURN ret = SQLColumns(stmt, empty.data(), SQL_NTS, empty.data(), SQL_NTS,
                             table.data(), SQL_NTS, column.data(), SQL_NTS);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  CheckSQLStatementDiagnosticError("01000");
  BOOST_REQUIRE_EQUAL("01000: catalogName and schemaName are empty strings.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  if (DATABASE_AS_SCHEMA) {
    // catalogName is empty case
    ret = SQLColumns(stmt, empty.data(), SQL_NTS, database.data(), SQL_NTS,
                     table.data(), SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
  }
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsNullMetadataIdTrue) {
  ConnectToSW();
  // Set SQL_ATTR_METADATA_ID to SQL_TRUE
  SQLRETURN ret = SQLSetConnectAttr(
      dbc, SQL_ATTR_METADATA_ID, reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);

  std::vector< SQLWCHAR > any = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > database = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("asset_id");

  // catalogName and schemaName are null case
  ret = SQLColumns(stmt, nullptr, 0, nullptr, 0, table.data(), SQL_NTS,
                   column.data(), SQL_NTS);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HY009");

  if (DATABASE_AS_SCHEMA) {
    // Check error message for catalogName and schemaName being null
    BOOST_REQUIRE_EQUAL(
        "HY009: SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, "
        "and "
        "the SchemaName, TableName, or ColumnName argument was a null pointer.",
        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    // catalogName is null case
    ret = SQLColumns(stmt, nullptr, 0, database.data(), SQL_NTS, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

    // schemaName is null case
    ret = SQLColumns(stmt, any.data(), SQL_NTS, nullptr, 0, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    CheckSQLStatementDiagnosticError("HY009");
    BOOST_REQUIRE_EQUAL(
        "HY009: SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, "
        "and "
        "the SchemaName, TableName, or ColumnName argument was a null pointer.",
        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    // tableName is null case
    ret = SQLColumns(stmt, any.data(), SQL_NTS, database.data(), SQL_NTS,
                     nullptr, 0, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    CheckSQLStatementDiagnosticError("HY009");
    BOOST_REQUIRE_EQUAL(
        "HY009: SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, "
        "and "
        "the SchemaName, TableName, or ColumnName argument was a null pointer.",
        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    // columnName is null case
    ret = SQLColumns(stmt, any.data(), SQL_NTS, database.data(), SQL_NTS,
                     table.data(), SQL_NTS, nullptr, 0);
    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    CheckSQLStatementDiagnosticError("HY009");
    BOOST_REQUIRE_EQUAL(
        "HY009: SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, "
        "and "
        "the SchemaName, TableName, or ColumnName argument was a null pointer.",
        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  } else {
    // Check error message for catalogName and schemaName being null
    BOOST_REQUIRE_EQUAL(
        "HY009: SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, "
        "and "
        "the CatalogName, TableName, or ColumnName argument was a null "
        "pointer.",
        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    // catalogName is null case
    ret = SQLColumns(stmt, nullptr, 0, any.data(), SQL_NTS, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    CheckSQLStatementDiagnosticError("HY009");
    BOOST_REQUIRE_EQUAL(
        "HY009: SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, "
        "and "
        "the CatalogName, TableName, or ColumnName argument was a null "
        "pointer.",
        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    // schemaName is null case
    ret = SQLColumns(stmt, database.data(), SQL_NTS, nullptr, 0, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

    // tableName is null case
    ret = SQLColumns(stmt, database.data(), SQL_NTS, any.data(), SQL_NTS,
                     nullptr, 0, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    CheckSQLStatementDiagnosticError("HY009");
    BOOST_REQUIRE_EQUAL(
        "HY009: SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, "
        "and "
        "the CatalogName, TableName, or ColumnName argument was a null "
        "pointer.",
        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    // columnName is null case
    ret = SQLColumns(stmt, database.data(), SQL_NTS, any.data(), SQL_NTS,
                     table.data(), SQL_NTS, nullptr, 0);
    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    CheckSQLStatementDiagnosticError("HY009");
    BOOST_REQUIRE_EQUAL(
        "HY009: SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, "
        "and "
        "the CatalogName, TableName, or ColumnName argument was a null "
        "pointer.",
        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsNullMetadataIdFalse) {
  ConnectToSW();

  std::vector< SQLWCHAR > any = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > database = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("asset_id");

  // catalogName and schemaName are null case
  SQLRETURN ret = SQLColumns(stmt, nullptr, 0, nullptr, 0, table.data(),
                             SQL_NTS, column.data(), SQL_NTS);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  if (DATABASE_AS_SCHEMA) {
    // catalogName is null case
    ret = SQLColumns(stmt, nullptr, 0, database.data(), SQL_NTS, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

    // schemaName is null case
    ret = SQLColumns(stmt, any.data(), SQL_NTS, nullptr, 0, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

    // tableName is null case
    ret = SQLColumns(stmt, any.data(), SQL_NTS, database.data(), SQL_NTS,
                     nullptr, 0, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

    // columnName is null case
    ret = SQLColumns(stmt, any.data(), SQL_NTS, database.data(), SQL_NTS,
                     table.data(), SQL_NTS, nullptr, 0);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
  } else {
    // catalogName is null case
    ret = SQLColumns(stmt, nullptr, 0, any.data(), SQL_NTS, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

    // schemaName is null case
    ret = SQLColumns(stmt, database.data(), SQL_NTS, nullptr, 0, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

    // tableName is null case
    ret = SQLColumns(stmt, database.data(), SQL_NTS, any.data(), SQL_NTS,
                     nullptr, 0, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

    // columnName is null case
    ret = SQLColumns(stmt, database.data(), SQL_NTS, any.data(), SQL_NTS,
                     table.data(), SQL_NTS, nullptr, 0);
    BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
  }
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsEmpty) {
  ConnectToSW();

  std::vector< SQLWCHAR > any = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > database = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("asset_id");

  // database is empty case
  SQLRETURN ret = SQLColumns(stmt, empty.data(), SQL_NTS, empty.data(), SQL_NTS,
                             table.data(), SQL_NTS, column.data(), SQL_NTS);

  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  CheckSQLStatementDiagnosticError("01000");
  BOOST_REQUIRE_EQUAL("01000: catalogName and schemaName are empty strings.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsSearchPattern) {
  ConnectToSW();

  std::vector< SQLWCHAR > databaseName = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("%");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, nullptr, 0, databaseName.data(), SQL_NTS,
                     table.data(), SQL_NTS, column.data(), SQL_NTS);
  } else {
    ret = SQLColumns(stmt, databaseName.data(), SQL_NTS, nullptr, 0,
                     table.data(), SQL_NTS, column.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  int count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(--count > 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);

  databaseName = MakeSqlBuffer("sample");
  table = MakeSqlBuffer("asset");

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, nullptr, 0, databaseName.data(), SQL_NTS,
                     table.data(), SQL_NTS, column.data(), SQL_NTS);
  } else {
    ret = SQLColumns(stmt, databaseName.data(), SQL_NTS, nullptr, 0,
                     table.data(), SQL_NTS, column.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(--count > 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsIdentifier) {
  ConnectToSW();

  SQLRETURN ret = SQLSetConnectAttr(
      dbc, SQL_ATTR_METADATA_ID, reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  std::vector< SQLWCHAR > databaseName = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("%");

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, nullptr, 0, databaseName.data(), SQL_NTS,
                     table.data(), SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
  } else {
    ret = SQLColumns(stmt, databaseName.data(), SQL_NTS, nullptr, 0,
                     table.data(), SQL_NTS, column.data(), SQL_NTS);
    BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
  }

  std::string error = GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt);
  std::string linuxErrPattern = "Cannot find ODBC error message";
  std::string otherErrPattern = "Failed to execute query \"describe \"%\".\"%\"";
  if (error.find(linuxErrPattern) == std::string::npos && error.find(otherErrPattern) == std::string::npos) {
    BOOST_FAIL("'" + error + "' does not match '" + linuxErrPattern + "' or '" + otherErrPattern + "'");
  }
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsNonExist) {
  // test passing nonexistent table/column names to SQLColumns returns
  // no data
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("nonexistent");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("nonexistent_column");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, empty.data(), SQL_NTS, nullptr, 0, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
  } else {
    ret = SQLColumns(stmt, nullptr, 0, empty.data(), SQL_NTS, table.data(),
                     SQL_NTS, column.data(), SQL_NTS);
  }

  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  BOOST_REQUIRE_NE(
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt)
          .find("01000: No table is found with pattern \'nonexistent\'"),
      std::string::npos);
  // The complete error message also mentions the database that the driver
  // searched for, there for a substring match is used here.

  std::vector< SQLWCHAR > correctTable = MakeSqlBuffer("asset_property");
  std::vector< SQLWCHAR > correctColumn = MakeSqlBuffer("asset_id");

  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);

  // test passing empty string databaseName to SQLColumns returns no data
  if (DATABASE_AS_SCHEMA) {
    ret =
        SQLColumns(stmt, empty.data(), SQL_NTS, nullptr, 0, correctTable.data(),
                   SQL_NTS, correctColumn.data(), SQL_NTS);
  } else {
    ret =
        SQLColumns(stmt, nullptr, 0, empty.data(), SQL_NTS, correctTable.data(),
                   SQL_NTS, correctColumn.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  int count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(--count == 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesSearchPatternReturnsMany) {
  ConnectToSW();

  // provide table name pattern that should match many tables
  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > testTablePattern = MakeSqlBuffer("asset");
  std::string testTable("asset_property");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0,
                    testTablePattern.data(), SQL_NTS, empty.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, nullptr, 0, empty.data(), SQL_NTS,
                    testTablePattern.data(), SQL_NTS, empty.data(), SQL_NTS);
  }

  int tableMatches = 0;

  // check all databases
  for (;;) {
    ret = SQLFetch(stmt);

    if (ret == SQL_NO_DATA) {
      break;
    } else if (!SQL_SUCCEEDED(ret)) {
      std::string sqlMessage = GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt);
      if (sqlMessage.empty()) {
        sqlMessage.append("SQLFetch returned: " + std::to_string(ret));
      }
    }

    SQLWCHAR buf[1024];
    SQLLEN bufLen = sizeof(buf);
    // columnIndex 1, 2, and 3 corresponds to CatalogName, SchemaName, and
    // TableName respectively.
    for (int i = 1; i <= 3; i++) {
      ret = SQLGetData(stmt, i, SQL_C_WCHAR, buf, sizeof(buf), &bufLen);

      if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }

      std::string actualValueStr = utility::SqlWcharToString(buf, bufLen);
      tableMatches++;
    }
  }

  // Check all tables that match the pattern are found
  int expectedTableMatches = 2;
  if (tableMatches < expectedTableMatches) {
    std::string sqlMessage =
        "Expected to find " + std::to_string(expectedTableMatches)
        + " tables (named \"" + testTable + "\"), but only found "
        + std::to_string(tableMatches) + +" tables";
    BOOST_FAIL(sqlMessage);
  }
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesIdentifierReturnsNone) {
  ConnectToSW();

  SQLRETURN ret = SQLSetConnectAttr(
      dbc, SQL_ATTR_METADATA_ID, reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > searchPattern = MakeSqlBuffer("%");

  // test with table passed as "%"
  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, nullptr, 0, searchPattern.data(), SQL_NTS,
                    searchPattern.data(), SQL_NTS, empty.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, searchPattern.data(), SQL_NTS, nullptr, 0,
                    searchPattern.data(), SQL_NTS, empty.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesIdentifierReturnsOne) {
  // Check that case-insensitive database/table identifiers return the correct
  // result
  ConnectToSW();

  SQLRETURN ret;

  // set SQL_ATTR_METADATA_ID to SQL_TRUE to test
  // parameters treated as case-sensitive identifiers
  ret = SQLSetConnectAttr(dbc, SQL_ATTR_METADATA_ID,
                          reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Provide mixed cases case-insensitive identifiers
  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > testDatabaseIdentifier =
      MakeSqlBuffer("sAmPlE");
  std::vector< SQLWCHAR > testTableIdentifier = MakeSqlBuffer("rAw_tImE_sErIeS");
  std::vector< SQLWCHAR > testTable = MakeSqlBuffer("raw_time_series");

  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, testDatabaseIdentifier.data(),
                    SQL_NTS, testTableIdentifier.data(), SQL_NTS, empty.data(),
                    SQL_NTS);
  } else {
    ret = SQLTables(stmt, testDatabaseIdentifier.data(), SQL_NTS, empty.data(),
                    SQL_NTS, testTableIdentifier.data(), SQL_NTS, empty.data(),
                    SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  CheckSingleRowResultSetWithGetData(
      stmt, 3, utility::SqlWcharToString(testTable.data()));
}

BOOST_AUTO_TEST_CASE(TestGetTablesPassNullTableMetadataIdTrue) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  SQLRETURN ret;

  // set SQL_ATTR_METADATA_ID to SQL_TRUE to test
  // parameters treated as case-sensitive identifiers
  ret = SQLSetConnectAttr(dbc, SQL_ATTR_METADATA_ID,
                          reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Case 1: provide database name only, and table name is nullptr
  std::vector< SQLWCHAR > testDatabase = MakeSqlBuffer("sample");

  if (DATABASE_AS_SCHEMA) {
    ExpectSQLTablesReject(
        empty.data(), SQL_NTS, testDatabase.data(), SQL_NTS, nullptr, 0,
        empty.data(), SQL_NTS, "HY009",
        "The SQL_ATTR_METADATA_ID statement attribute is set to SQL_TRUE, "
        "and TableName argument was a null pointer.");
  } else {
    ExpectSQLTablesReject(
        testDatabase.data(), SQL_NTS, empty.data(), SQL_NTS, nullptr, 0,
        empty.data(), SQL_NTS, "HY009",
        "The SQL_ATTR_METADATA_ID statement attribute is set to SQL_TRUE, "
        "and TableName argument was a null pointer.");
  }
}

BOOST_AUTO_TEST_CASE(TestGetTablesPassNullDatabaseMetadataIdTrue) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  SQLRETURN ret;

  // set SQL_ATTR_METADATA_ID to SQL_TRUE to test
  // parameters treated as case-sensitive identifiers
  ret = SQLSetConnectAttr(dbc, SQL_ATTR_METADATA_ID,
                          reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Case 2: provide table name only, and database name is nullptr
  std::vector< SQLWCHAR > testTable = MakeSqlBuffer("raw_time_series");

  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0, testTable.data(), 
                    SQL_NTS, empty.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, nullptr, 0, empty.data(), SQL_NTS, testTable.data(),
                    SQL_NTS, empty.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  CheckSingleRowResultSetWithGetData(
      stmt, 3, utility::SqlWcharToString(testTable.data()));
}

BOOST_AUTO_TEST_CASE(TestGetTablesPassNullToUnsupportedMetadataIdTrue) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  SQLRETURN ret;

  // set SQL_ATTR_METADATA_ID to SQL_TRUE to test
  // parameters treated as case-sensitive identifiers
  ret = SQLSetConnectAttr(dbc, SQL_ATTR_METADATA_ID,
                          reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  std::vector< SQLWCHAR > testDatabase = MakeSqlBuffer("sample");
  std::vector< SQLWCHAR > testTable = MakeSqlBuffer("raw_time_series");

  // Case 4: both database and table names are provided, and nullptr is passed
  // to unsupported functionality
  if (DATABASE_AS_SCHEMA) {
    // only schemas supported, driver should ignore catalogName being nullptr
    ret = SQLTables(stmt, nullptr, 0, testDatabase.data(), SQL_NTS,
                    testTable.data(), SQL_NTS, empty.data(), SQL_NTS);
  } else {
    // only catalogs supported, driver should ignore schemaName being nullptr
    ret = SQLTables(stmt, testDatabase.data(), SQL_NTS, nullptr, 0,
                    testTable.data(), SQL_NTS, empty.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  CheckSingleRowResultSetWithGetData(
      stmt, 3, utility::SqlWcharToString(testTable.data()));
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsOneWithTableTypes) {
  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("raw_time_series");
  std::vector< SQLWCHAR > tableTypes = MakeSqlBuffer(
      "TABLE,VIEW");  // Test that VIEW type is ignored by ODBC driver
  SQLRETURN ret;

  ConnectToSW();

  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0, table.data(),
                    SQL_NTS, tableTypes.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, nullptr, 0, empty.data(), SQL_NTS, table.data(),
                    SQL_NTS, tableTypes.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  CheckSingleRowResultSetWithGetData(stmt, 3,
                                     utility::SqlWcharToString(table.data()));
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsOneForQuotedTypes) {
  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("raw_time_series");
  std::vector< SQLWCHAR > tableTypes =
      MakeSqlBuffer("'TABLE' , 'VIEW'");  // Test that quoted values are handled
  SQLRETURN ret;

  ConnectToSW();

  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0, table.data(),
                    SQL_NTS, tableTypes.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, nullptr, 0, empty.data(), SQL_NTS, table.data(),
                    SQL_NTS, tableTypes.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  CheckSingleRowResultSetWithGetData(stmt, 3,
                                     utility::SqlWcharToString(table.data()));
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsNoneForUnsupportedTableType) {
  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("raw_time_series");
  std::vector< SQLWCHAR > tableTypes = MakeSqlBuffer("VIEW");

  ConnectToSW();

  SQLRETURN ret = SQLTables(stmt, nullptr, 0, nullptr, 0, table.data(), SQL_NTS,
                            tableTypes.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

// the SQL_ATTR_METADATA_ID statement attribute should have no effect upon the
// TableType argument
BOOST_DATA_TEST_CASE_F(MetaQueriesTestSuiteFixture,
                       TestGetTableTypesWithSQLTables,
                       data::make({false, true}), useIdentifier) {
  // tests special case: get a list of valid table types with SQLTables
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > tableType = MakeSqlBuffer(SQL_ALL_TABLE_TYPES);
  SQLRETURN ret;

  if (useIdentifier) {
    // SQL_ATTR_METADATA_ID defaults to SQL_FALSE, so set it to SQL_TRUE to test
    // parameters treated as identifiers
    ret = SQLSetConnectAttr(dbc, SQL_ATTR_METADATA_ID,
                            reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  }

  ret = SQLTables(stmt, empty.data(), SQL_NTS, empty.data(), SQL_NTS,
                  empty.data(), SQL_NTS, tableType.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::string expectedTableType = "TABLE";
  // check that columnIndex 4 (corresponds to TABLE_TYPE) is "TABLE"

  CheckSingleRowResultSetWithGetData(stmt, 4, expectedTableType, true);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsNone) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("nonexistent");

  SQLRETURN ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0,
                            table.data(), SQL_NTS, empty.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsMany) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("%");
  SQLRETURN ret;

  // test with table passed as "%"
  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0, table.data(),
                    SQL_NTS, empty.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, nullptr, 0, empty.data(), SQL_NTS, table.data(),
                    SQL_NTS, empty.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  int count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(count > 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);

  // test with table passed as nullptr
  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0, nullptr, 0,
                    empty.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, nullptr, 0, empty.data(), SQL_NTS, nullptr, 0,
                    empty.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(count > 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLColumnWithSQLBindCols) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("raw_time_series");
  std::vector< SQLWCHAR > column = MakeSqlBuffer("asset_id");

  SQLRETURN ret = SQL_SUCCESS;

  char table_cat[C_STR_LEN_DEFAULT]{};
  SQLLEN table_cat_len = sizeof(table_cat);
  char table_schem[C_STR_LEN_DEFAULT]{};
  SQLLEN table_schem_len = sizeof(table_schem);
  char table_name[C_STR_LEN_DEFAULT]{};
  SQLLEN table_name_len = sizeof(table_name);
  char column_name[C_STR_LEN_DEFAULT]{};
  SQLLEN column_name_len = sizeof(column_name);
  SQLSMALLINT data_type = 0;
  SQLLEN data_type_len = sizeof(data_type);
  char type_name[C_STR_LEN_DEFAULT]{};
  SQLLEN type_name_len = sizeof(type_name);
  SQLINTEGER column_size = 0;
  SQLLEN column_size_len = sizeof(column_size);
  SQLINTEGER buffer_length = 0;
  SQLLEN buffer_length_len = sizeof(buffer_length);
  SQLSMALLINT decimal_digits = 0;
  SQLLEN decimal_digits_len = sizeof(decimal_digits);
  SQLSMALLINT num_prec_radix = 0;
  SQLLEN num_prec_radix_len = sizeof(num_prec_radix);
  SQLSMALLINT nullable = 0;
  SQLLEN nullable_len = sizeof(nullable);
  char remarks[C_STR_LEN_DEFAULT]{};
  SQLLEN remarks_len = sizeof(remarks);
  char column_def[C_STR_LEN_DEFAULT]{};
  SQLLEN column_def_len = sizeof(column_def);
  SQLSMALLINT sql_data_type = 0;
  SQLLEN sql_data_type_len = sizeof(sql_data_type);
  SQLSMALLINT sql_datetime_sub = 0;
  SQLLEN sql_datetime_sub_len = sizeof(sql_datetime_sub);
  SQLINTEGER char_octet_length = 0;
  SQLLEN char_octet_length_len = sizeof(char_octet_length);
  SQLINTEGER ordinal_position = 0;
  SQLLEN ordinal_position_len = sizeof(ordinal_position);
  char is_nullable[C_STR_LEN_DEFAULT]{};
  SQLLEN is_nullable_len = sizeof(is_nullable);

  SQLColumnsBindColumns(
      stmt, table_cat, table_cat_len, table_schem, table_schem_len, table_name,
      table_name_len, column_name, column_name_len, data_type, data_type_len,
      type_name, type_name_len, column_size, column_size_len, buffer_length,
      buffer_length_len, decimal_digits, decimal_digits_len, num_prec_radix,
      num_prec_radix_len, nullable, nullable_len, remarks, remarks_len,
      column_def, column_def_len, sql_data_type, sql_data_type_len,
      sql_datetime_sub, sql_datetime_sub_len, char_octet_length,
      char_octet_length_len, ordinal_position, ordinal_position_len,
      is_nullable, is_nullable_len);

  ret = SQLColumns(stmt, nullptr, 0, nullptr, 0, table.data(), SQL_NTS,
                   column.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT numResultCols = 0;
  ret = SQLNumResultCols(stmt, &numResultCols);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
  BOOST_CHECK_EQUAL(18, numResultCols);

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  bool errorExpected = false;
  if (DATABASE_AS_SCHEMA) {
    BOOST_CHECK_EQUAL(true, WasNull(table_cat_len));
    BOOST_CHECK_EQUAL("", table_cat);  // TABLE_CAT
    BOOST_CHECK_EQUAL(true, WasNull(table_schem_len));
    BOOST_CHECK_EQUAL("", table_schem);  // TABLE_SCHEM
  } else {
    BOOST_CHECK_EQUAL(false, WasNull(table_cat_len));
    BOOST_CHECK_EQUAL("", table_cat);  // TABLE_CAT
    BOOST_CHECK_EQUAL(true, WasNull(table_schem_len));
    BOOST_CHECK_EQUAL("", table_schem);  // TABLE_SCHEM
  }
  BOOST_CHECK_EQUAL(false, WasNull(table_name_len));
  BOOST_CHECK_EQUAL("raw_time_series", table_name);  // TABLE_NAME
  BOOST_CHECK_EQUAL(false, WasNull(column_name_len));
  BOOST_CHECK_EQUAL("asset_id", column_name);  // COLUMN_NAME
  BOOST_CHECK_EQUAL(false, WasNull(data_type_len));
  BOOST_CHECK_EQUAL(SQL_VARCHAR, data_type);  // DATA_TYPE
  BOOST_CHECK_EQUAL(false, WasNull(type_name_len));
  BOOST_CHECK_EQUAL("VARCHAR", type_name);  // TYPE_NAME
  BOOST_CHECK_EQUAL(false, WasNull(column_size_len));
  BOOST_CHECK_EQUAL(IOTSITEWISE_SQL_MAX_LENGTH, column_size);  // COLUMN_SIZE
  BOOST_CHECK_EQUAL(false, WasNull(buffer_length_len));
  BOOST_CHECK_EQUAL(IOTSITEWISE_SQL_MAX_LENGTH, buffer_length);  // BUFFER_LENGTH
  BOOST_CHECK_EQUAL(true, WasNull(decimal_digits_len));
  BOOST_CHECK_EQUAL(0, decimal_digits);  // DECIMAL_DIGITS
  BOOST_CHECK_EQUAL(false, WasNull(num_prec_radix_len));
  BOOST_CHECK_EQUAL(0, num_prec_radix);  // NUM_PREC_RADIX
  BOOST_CHECK_EQUAL(false, WasNull(nullable_len));
  BOOST_CHECK_EQUAL(SQL_NO_NULLS, nullable);  // NULLABLE
  BOOST_CHECK_EQUAL(false, WasNull(remarks_len));
  BOOST_CHECK_EQUAL("raw_time_series", remarks);  // REMARKS
  BOOST_CHECK_EQUAL(true, WasNull(column_def_len));
  BOOST_CHECK_EQUAL("", column_def);  // COLUMN_DEF
  BOOST_CHECK_EQUAL(false, WasNull(sql_data_type_len));
  BOOST_CHECK_EQUAL(SQL_VARCHAR, sql_data_type);  // SQL_DATA_TYPE
  BOOST_CHECK_EQUAL(true, WasNull(sql_datetime_sub_len));
  BOOST_CHECK_EQUAL(0, sql_datetime_sub);  // SQL_DATETIME_SUB
  BOOST_CHECK_EQUAL(false, WasNull(char_octet_length_len));
  BOOST_CHECK_EQUAL(IOTSITEWISE_SQL_MAX_LENGTH,
                    char_octet_length);  // CHAR_OCTET_LENGTH
  BOOST_CHECK_EQUAL(false, WasNull(ordinal_position_len));
  BOOST_CHECK_EQUAL(1, ordinal_position);  // ORDINAL_POSITION
  BOOST_CHECK_EQUAL(false, WasNull(is_nullable_len));
  BOOST_CHECK_EQUAL("NO", is_nullable);  // IS_NULLABLE

  // Check that we can get an attribute on the columns metadata.
  SQLWCHAR attrColumnName[C_STR_LEN_DEFAULT];
  SQLSMALLINT attrColumnNameLen = 0;
  ret = SQLColAttribute(stmt, 2, SQL_DESC_NAME, attrColumnName,
                        sizeof(attrColumnName), &attrColumnNameLen,
                        nullptr);  // COLUMN_NAME, NOT NULL
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_ERROR(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
  BOOST_CHECK_EQUAL("TABLE_SCHEM", utility::SqlWcharToString(
                                       attrColumnName, attrColumnNameLen));

  // Test that the next fetch will have no data.
  ret = SQLFetch(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithSelectQuery) {
  ConnectToSW();

  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer("SELECT 515.0 FROM system.tables LIMIT 1");
  SQLRETURN ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

#ifdef __APPLE__
  CheckSingleRowResultSetWithGetData(stmt, 1, "", false, INVALID_CURSOR_STATE);
#else
  CheckSingleRowResultSetWithGetData(stmt);
#endif
}

BOOST_AUTO_TEST_CASE(TestGetInfoScrollOptions) {
  ConnectToSW();

  SQLUINTEGER val = 0;
  SQLRETURN ret = SQLGetInfo(dbc, SQL_SCROLL_OPTIONS, &val, 0, 0);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
  }

  BOOST_CHECK_NE(val, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLNumResultColsAfterSQLPrepare) {
  ConnectToSW();

  SQLRETURN ret = PrepareQuery("SELECT 515.0 FROM system.tables LIMIT 1");
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_CHECK_EQUAL(columnCount, 1);

  ret = SQLExecute(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_CHECK_EQUAL(columnCount, 1);
}

BOOST_AUTO_TEST_CASE(TestSQLDescribeColSQLTablesODBCVer3) {
  // Check SQLTable metadata when ODBC Ver is set to 3 (default).
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("%");

  // test with table passed as "%"
  SQLRETURN ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0,
                            table.data(), SQL_NTS, empty.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_CHECK_EQUAL(columnCount, 5);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "TABLE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "TABLE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "TABLE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "TABLE_TYPE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "REMARKS", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);
}

BOOST_AUTO_TEST_CASE(TestSQLDescribeColSQLTablesODBCVer2) {
  // Check SQLTable metadata when ODBC Ver is set to 2.
  ConnectToSW(SQL_OV_ODBC2);

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("%");

  // test with table passed as "%"
  SQLRETURN ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0,
                            table.data(), SQL_NTS, empty.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_CHECK_EQUAL(columnCount, 5);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "TABLE_QUALIFIER", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "TABLE_OWNER", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "TABLE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "TABLE_TYPE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "REMARKS", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);
}

// Test unsupported functions returnging empty results
BOOST_AUTO_TEST_CASE(TestSQLForeignKeys) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");

  SQLRETURN ret = SQLForeignKeys(stmt, NULL, 0,          /* Primary catalog */
                                 NULL, 0,                /* Primary schema */
                                 NULL, 0,                /* Primary table */
                                 NULL, 0,                /* Foreign catalog */
                                 NULL, 0,                /* Foreign schema */
                                 table.data(), SQL_NTS); /* Foreign table */

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 14);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "PKTABLE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "PKTABLE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "PKTABLE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "PKCOLUMN_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "FKTABLE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "FKTABLE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 7, "FKTABLE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 8, "FKCOLUMN_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 9, "KEY_SEQ", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 10, "UPDATE_RULE", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 11, "DELETE_RULE", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 12, "FK_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 13, "PK_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 14, "DEFERRABILITY", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  // Check SQL_NO_DATA is returned for SQLForeignKeys
  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLPrimaryKeys) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");

  SQLRETURN ret =
      SQLPrimaryKeys(stmt, nullptr, 0, nullptr, 0, table.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 6);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "TABLE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "TABLE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "TABLE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "COLUMN_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "KEY_SEQ", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "PK_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  // Check SQL_NO_DATA is returned for SQLPrimaryKeys
  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLSpecialColumns) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = MakeSqlBuffer("");
  std::vector< SQLWCHAR > database = MakeSqlBuffer("meta_queries_test_db");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLSpecialColumns(stmt, SQL_BEST_ROWID, empty.data(), SQL_NTS,
                            database.data(), SQL_NTS, table.data(), SQL_NTS,
                            SQL_SCOPE_CURROW, SQL_NO_NULLS);
  } else {
    ret = SQLSpecialColumns(stmt, SQL_BEST_ROWID, database.data(), SQL_NTS,
                            empty.data(), SQL_NTS, table.data(), SQL_NTS,
                            SQL_SCOPE_CURROW, SQL_NO_NULLS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_ERROR(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 8);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "SCOPE", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "COLUMN_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "DATA_TYPE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "TYPE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "COLUMN_SIZE", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "BUFFER_LENGTH", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 7, "DECIMAL_DIGITS", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 8, "PSEUDO_COLUMN", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  // Check SQL_NO_DATA is returned for SQLSpecialColumns
  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLStatisticsODBCVer3) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = MakeSqlBuffer("");
  std::vector< SQLWCHAR > database = MakeSqlBuffer("meta_queries_test_db");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLStatistics(stmt, empty.data(), SQL_NTS, database.data(), SQL_NTS,
                        table.data(), SQL_NTS, SQL_INDEX_UNIQUE, SQL_ENSURE);
  } else {
    ret = SQLStatistics(stmt, database.data(), SQL_NTS, empty.data(), SQL_NTS,
                        table.data(), SQL_NTS, SQL_INDEX_UNIQUE, SQL_ENSURE);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_ERROR(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 13);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "TABLE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "TABLE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "TABLE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "NON_UNIQUE", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "INDEX_QUALIFIER", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "INDEX_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 7, "TYPE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 8, "ORDINAL_POSITION", SQL_INTEGER,
                                    10, 0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 9, "COLUMN_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 10, "ASC_OR_DESC", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 11, "CARDINALITY", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 12, "PAGES", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 13, "FILTER_CONDITION", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  // Check SQL_NO_DATA is returned for SQLStatistics
  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLStatisticsODBCVer2) {
  // Check SQLStatistics metadata when ODBC Ver is set to 2.
  ConnectToSW(SQL_OV_ODBC2);

  std::vector< SQLWCHAR > empty = MakeSqlBuffer("");
  std::vector< SQLWCHAR > database = MakeSqlBuffer("meta_queries_test_db");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLStatistics(stmt, empty.data(), SQL_NTS, database.data(), SQL_NTS,
                        table.data(), SQL_NTS, SQL_INDEX_UNIQUE, SQL_ENSURE);
  } else {
    ret = SQLStatistics(stmt, database.data(), SQL_NTS, empty.data(), SQL_NTS,
                        table.data(), SQL_NTS, SQL_INDEX_UNIQUE, SQL_ENSURE);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_ERROR(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 13);

  // Only check items that are different in ODBC Ver 2.0
  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "TABLE_QUALIFIER", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "TABLE_OWNER", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 8, "SEQ_IN_INDEX", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 10, "COLLATION", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  // Check SQL_NO_DATA is returned for SQLStatistics
  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLProcedureColumns) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = MakeSqlBuffer("");
  std::vector< SQLWCHAR > any = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > database = MakeSqlBuffer("meta_queries_test_db");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret =
        SQLProcedureColumns(stmt, empty.data(), SQL_NTS, database.data(),
                            SQL_NTS, any.data(), SQL_NTS, any.data(), SQL_NTS);
  } else {
    ret =
        SQLProcedureColumns(stmt, database.data(), SQL_NTS, empty.data(),
                            SQL_NTS, any.data(), SQL_NTS, any.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_ERROR(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 19);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "PROCEDURE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "PROCEDURE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "PROCEDURE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "COLUMN_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "COLUMN_TYPE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "DATA_TYPE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 7, "TYPE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 8, "COLUMN_SIZE", SQL_INTEGER, 10, 0,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 9, "BUFFER_LENGTH", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 10, "DECIMAL_DIGITS", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 11, "NUM_PREC_RADIX", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 12, "NULLABLE", SQL_INTEGER, 10, 0,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 13, "REMARKS", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 14, "COLUMN_DEF", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 15, "SQL_DATA_TYPE", SQL_INTEGER, 10,
                                    0, SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 16, "SQL_DATETIME_SUB", SQL_INTEGER,
                                    10, 0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 17, "CHAR_OCTET_LENGTH", SQL_INTEGER,
                                    10, 0, SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 18, "ORDINAL_POSITION", SQL_INTEGER,
                                    10, 0, SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 19, "IS_NULLABLE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  // Check SQL_NO_DATA is returned for TestSQLProcedureColumns
  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLProcedures) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = MakeSqlBuffer("");
  std::vector< SQLWCHAR > any = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > database = MakeSqlBuffer("meta_queries_test_db");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLProcedures(stmt, empty.data(), SQL_NTS, database.data(), SQL_NTS,
                        any.data(), SQL_NTS);
  } else {
    ret = SQLProcedures(stmt, database.data(), SQL_NTS, empty.data(), SQL_NTS,
                        any.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_ERROR(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 8);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "PROCEDURE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "PROCEDURE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "PROCEDURE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "NUM_INPUT_PARAMS", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "NUM_OUTPUT_PARAMS", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "NUM_RESULT_SETS", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 7, "REMARKS", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 8, "PROCEDURE_TYPE", SQL_INTEGER, 10,
                                    0, SQL_NULLABLE);

  // Check SQL_NO_DATA is returned for SQLProcedures
  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLColumnPrivileges) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = MakeSqlBuffer("");
  std::vector< SQLWCHAR > any = MakeSqlBuffer("%");
  std::vector< SQLWCHAR > database = MakeSqlBuffer("meta_queries_test_db");
  std::vector< SQLWCHAR > table = MakeSqlBuffer("asset");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumnPrivileges(stmt, empty.data(), SQL_NTS, database.data(),
                              SQL_NTS, table.data(), SQL_NTS, any.data(),
                              SQL_NTS);
  } else {
    ret = SQLColumnPrivileges(stmt, database.data(), SQL_NTS, empty.data(),
                              SQL_NTS, table.data(), SQL_NTS, any.data(),
                              SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_ERROR(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 8);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "TABLE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "TABLE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "TABLE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "COLUMN_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "GRANTOR", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "GRANTEE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 7, "PRIVILEGE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 8, "IS_GRANTABLE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  // Check SQL_NO_DATA is returned for SQLColumnPrivileges
  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLTablePrivileges) {
  ConnectToSW();

  std::vector< SQLWCHAR > testDatabase = MakeSqlBuffer("sampleDB");
  std::vector< SQLWCHAR > testTable = MakeSqlBuffer("IoTMulti");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLTablePrivileges(stmt, nullptr, 0, testDatabase.data(), SQL_NTS,
                             testTable.data(), SQL_NTS);

  } else {
    ret = SQLTablePrivileges(stmt, testDatabase.data(), SQL_NTS, nullptr, 0,
                             testTable.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_REQUIRE_EQUAL(columnCount, 7);

  CheckColumnMetaWithSQLDescribeCol(stmt, 1, "TABLE_CAT", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 2, "TABLE_SCHEM", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 3, "TABLE_NAME", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 4, "GRANTOR", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  CheckColumnMetaWithSQLDescribeCol(stmt, 5, "GRANTEE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 6, "PRIVILEGE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NO_NULLS);

  CheckColumnMetaWithSQLDescribeCol(stmt, 7, "IS_GRANTABLE", SQL_VARCHAR,
                                    IOTSITEWISE_SQL_MAX_LENGTH, -1,
                                    SQL_NULLABLE);

  // Check SQL_NO_DATA is returned for SQLTablePrivileges
  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLCancelWithColumns) {
  ConnectToSW();

  std::string dbNameStr = "data_queries_test_db";
  std::vector< SQLWCHAR > table = MakeSqlBuffer("TestScalarTypes");
  std::vector< SQLWCHAR > databaseName = MakeSqlBuffer(dbNameStr);
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, nullptr, 0, databaseName.data(), SQL_NTS,
                     table.data(), SQL_NTS, nullptr, 0);
  } else {
    ret = SQLColumns(stmt, databaseName.data(), SQL_NTS, nullptr, 0,
                     table.data(), SQL_NTS, nullptr, 0);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  char column_name[C_STR_LEN_DEFAULT]{};
  SQLLEN column_name_len = sizeof(column_name);

  ret = SQLBindCol(stmt, 4, SQL_C_CHAR, column_name, sizeof(column_name),
                   &column_name_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));

  ret = SQLCancel(stmt);
  BOOST_CHECK(SQL_SUCCEEDED(ret));

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL(
      "HY010: [unixODBC][Driver Manager]Function sequence error",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#elif defined(__APPLE__)
  BOOST_REQUIRE_EQUAL("S1010: [iODBC][Driver Manager]Function sequence error",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL("HY010: Query was not executed.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLCancelWithTables) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > testTablePattern = MakeSqlBuffer("test_ableM%");
  std::vector< SQLWCHAR > testTable1 = MakeSqlBuffer("testTableMeta");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0,
                    testTablePattern.data(), SQL_NTS, empty.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, nullptr, 0, empty.data(), SQL_NTS,
                    testTablePattern.data(), SQL_NTS, empty.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLCancel(stmt);
  BOOST_CHECK(SQL_SUCCEEDED(ret));

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL(
      "HY010: [unixODBC][Driver Manager]Function sequence error",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#elif defined(__APPLE__)
  BOOST_REQUIRE_EQUAL("S1010: [iODBC][Driver Manager]Function sequence error",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL("HY010: Query was not executed.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLCloseCursorWithColumns) {
  ConnectToSW();

  std::string dbNameStr = "sample";
  std::vector< SQLWCHAR > table = MakeSqlBuffer("raw_time_series");
  std::vector< SQLWCHAR > databaseName = MakeSqlBuffer(dbNameStr);
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, nullptr, 0, databaseName.data(), SQL_NTS,
                     table.data(), SQL_NTS, nullptr, 0);
  } else {
    ret = SQLColumns(stmt, databaseName.data(), SQL_NTS, nullptr, 0,
                     table.data(), SQL_NTS, nullptr, 0);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  char column_name[C_STR_LEN_DEFAULT]{};
  SQLLEN column_name_len = sizeof(column_name);

  ret = SQLBindCol(stmt, 4, SQL_C_CHAR, column_name, sizeof(column_name),
                   &column_name_len);
  BOOST_CHECK(SQL_SUCCEEDED(ret));

  ret = SQLCloseCursor(stmt);
  BOOST_CHECK(SQL_SUCCEEDED(ret));

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

  ret = SQLCloseCursor(stmt);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL("24000: [unixODBC][Driver Manager]Invalid cursor state",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL("24000: No cursor was open",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLCloseCursorWithTables) {
  ConnectToSW();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > testTablePattern = MakeSqlBuffer("asset");
  std::vector< SQLWCHAR > testTable1 = MakeSqlBuffer("asset_property");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, nullptr, 0,
                    testTablePattern.data(), SQL_NTS, empty.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, nullptr, 0, empty.data(), SQL_NTS,
                    testTablePattern.data(), SQL_NTS, empty.data(), SQL_NTS);
  }

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLCloseCursor(stmt);
  BOOST_CHECK(SQL_SUCCEEDED(ret));

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

  ret = SQLCloseCursor(stmt);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL("24000: [unixODBC][Driver Manager]Invalid cursor state",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL("24000: No cursor was open",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

/**
 * Check that SQLDescribeCol return valid scale and precision for columns of
 * different type after Prepare.
 *
 * 1. Start node.
 * 2. Connect to node using ODBC.
 * 3. Create table with decimal and char columns with specified size and scale.
 * 4. Prepare statement.
 * 5. Check precision and scale of every column using SQLDescribeCol.
 */
BOOST_AUTO_TEST_CASE(TestSQLDescribeColPrecisionAndScaleAfterPrepare) {
  CheckSQLDescribeColPrecisionAndScale(&OdbcTestSuite::PrepareQuery);
}

/**
 * Check that SQLDescribeCol return valid scale and precision for columns of
 * different type after Execute.
 *
 * 1. Start node.
 * 2. Connect to node using ODBC.
 * 3. Create table with decimal and char columns with specified size and scale.
 * 4. Execute statement.
 * 5. Check precision and scale of every column using SQLDescribeCol. */
BOOST_AUTO_TEST_CASE(TestSQLDescribeColPrecisionAndScaleAfterExec) {
  CheckSQLDescribeColPrecisionAndScale(&OdbcTestSuite::ExecQuery);
}

/**
 * Check that SQLColAttribute return valid scale and precision for columns of
 * different type after Prepare.
 *
 * 1. Start node.
 * 2. Connect to node using ODBC.
 * 3. Create table with decimal and char columns with specified size and scale.
 * 4. Prepare statement.
 * 5. Check precision and scale of every column using SQLColAttribute.
 */
BOOST_AUTO_TEST_CASE(TestSQLColAttributePrecisionAndScaleAfterPrepare) {
  CheckSQLColAttributePrecisionAndScale(&OdbcTestSuite::PrepareQuery);
}

/**
 * Check that SQLColAttribute return valid scale and precision for columns of
 * different type after Execute.
 *
 * 1. Start node.
 * 2. Connect to node using ODBC.
 * 3. Create table with decimal and char columns with specified size and scale.
 * 4. Execute statement.
 * 5. Check precision and scale of every column using SQLColAttribute. */
BOOST_AUTO_TEST_CASE(TestSQLColAttributePrecisionAndScaleAfterExec) {
  CheckSQLColAttributePrecisionAndScale(&OdbcTestSuite::ExecQuery);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnAutoUniqueValue) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // only "NO" is returned for AUTO_INCREMENT field
  callSQLColAttributes(stmt, req, SQL_COLUMN_AUTO_INCREMENT, SQL_FALSE);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnCaseSensitive) {
  ConnectToSW(SQL_OV_ODBC2);

  // test that case sensitive returns true for string field.
  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttributes(stmt, req1, SQL_COLUMN_CASE_SENSITIVE, SQL_TRUE);

  // test that case sensitive returns false for int field.
  const SQLCHAR req2[] = "SELECT 1 FROM system.tables LIMIT 1";

  callSQLColAttributes(stmt, req2, SQL_COLUMN_CASE_SENSITIVE, SQL_FALSE);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnCount) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // count should be 1
  callSQLColAttributes(stmt, req, SQL_COLUMN_COUNT, 1);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnDisplaySize) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // SQL_VARCHAR should have display size IOTSITEWISE_SQL_MAX_LENGTH
  callSQLColAttributes(stmt, req, SQL_COLUMN_DISPLAY_SIZE,
                       IOTSITEWISE_SQL_MAX_LENGTH);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnLabel) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT true FROM system.tables LIMIT 1";

  callSQLColAttributes(stmt, req, SQL_COLUMN_LABEL, std::string("_1"));
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnLength) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // SQL_VARCHAR should have length IOTSITEWISE_SQL_MAX_LENGTH
  callSQLColAttributes(stmt, req, SQL_COLUMN_LENGTH,
                       IOTSITEWISE_SQL_MAX_LENGTH);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnFixedPrecScale) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT 1 FROM system.tables LIMIT 1";

  // only SQL_FALSE is returned
  callSQLColAttributes(stmt, req, SQL_COLUMN_MONEY, SQL_FALSE);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnName) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT 1 FROM system.tables LIMIT 1";

  callSQLColAttributes(stmt, req, SQL_COLUMN_NAME, std::string("_1"));
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnNullable) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttributes(stmt, req1, SQL_COLUMN_NULLABLE, SQL_NULLABLE_UNKNOWN);

  const SQLCHAR req2[] = "SELECT true FROM system.tables LIMIT 1";

  callSQLColAttributes(stmt, req2, SQL_COLUMN_NULLABLE, SQL_NULLABLE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnSchemaName) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // schema name is empty
  callSQLColAttributes(stmt, req, SQL_COLUMN_OWNER_NAME, std::string(""));
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnPrecision) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // SQL_VARCHAR should have precision IOTSITEWISE_SQL_MAX_LENGTH
  callSQLColAttributes(stmt, req, SQL_COLUMN_PRECISION,
                       IOTSITEWISE_SQL_MAX_LENGTH);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnQualifierName) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT 1 FROM system.tables LIMIT 1";

  // check that qualifier should be empty
  callSQLColAttributes(stmt, req, SQL_COLUMN_QUALIFIER_NAME, std::string(""));
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnScale) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT 1 FROM system.tables LIMIT 1";

  // default scale value is 0
  callSQLColAttributes(stmt, req, SQL_COLUMN_SCALE, 0);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnSearchable) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // only SQL_PRED_BASIC is returned
  callSQLColAttributes(stmt, req, SQL_COLUMN_SEARCHABLE, SQL_PRED_BASIC);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnTableName) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // table name is not set for a column
  callSQLColAttributes(stmt, req, SQL_COLUMN_TABLE_NAME, "");
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnType) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req1[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttributes(stmt, req1, SQL_COLUMN_TYPE, SQL_VARCHAR);

  const SQLCHAR req2[] = "SELECT 1 FROM system.tables LIMIT 1";

  callSQLColAttributes(stmt, req2, SQL_COLUMN_TYPE, SQL_INTEGER);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnUnsigned) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req1[] = "SELECT 1 FROM system.tables LIMIT 1";

  // numeric type should be signed
  callSQLColAttributes(stmt, req1, SQL_COLUMN_UNSIGNED, SQL_FALSE);

  const SQLCHAR req2[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // non-numeric types should be unsigned
  callSQLColAttributes(stmt, req2, SQL_COLUMN_UNSIGNED, SQL_TRUE);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnUpdatable) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  // only SQL_ATTR_READWRITE_UNKNOWN is returned
  callSQLColAttributes(stmt, req, SQL_COLUMN_UPDATABLE,
                       SQL_ATTR_READWRITE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestColAttributesODBC2ColumnTypeName) {
  ConnectToSW(SQL_OV_ODBC2);

  const SQLCHAR req[] = "SELECT table_name FROM system.tables WHERE table_name = 'asset'";

  callSQLColAttributes(stmt, req, SQL_COLUMN_TYPE_NAME,
                       std::string("VARCHAR"));
}

BOOST_AUTO_TEST_SUITE_END()
