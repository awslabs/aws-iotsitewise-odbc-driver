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
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include <chrono>

#include "iotsitewise/odbc/utility.h"
#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace iotsitewise;
using namespace iotsitewise_test;
using namespace iotsitewise::odbc;
using iotsitewise::odbc::utility::CheckEnvVarSetToTrue;

using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct QueriesTestSuiteFixture : odbc::OdbcTestSuite {
  template < typename T >
  void CheckTwoRowsInt(SQLSMALLINT type) {
    ConnectToSW();

    SQLRETURN ret;

    const SQLSMALLINT columnsCnt = 1;

    T columns[columnsCnt];

    std::memset(&columns, 0, sizeof(columns));

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       nullptr);

      if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }
    }

    std::vector< SQLWCHAR > request = MakeSqlBuffer(
        "SELECT 1 FROM system.tables LIMIT 2");

    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(columns[0], 1);

    SQLLEN columnLens[columnsCnt];

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       &columnLens[i]);

      if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }
    }

    ret = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(columns[0], 1);

    BOOST_CHECK_EQUAL(columnLens[0], static_cast< SQLLEN >(sizeof(T)));

    ret = SQLFetch(stmt);
    BOOST_CHECK(ret == SQL_NO_DATA);

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       nullptr);

      if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }
    }

    request = MakeSqlBuffer(
        "SELECT true FROM system.tables LIMIT 2");

    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(columns[0], 1);

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       &columnLens[i]);

      if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }
    }

    ret = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(columns[0], 1);

    BOOST_CHECK_EQUAL(columnLens[0], static_cast< SQLLEN >(sizeof(T)));

    ret = SQLFetch(stmt);
    BOOST_CHECK(ret == SQL_NO_DATA);

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       nullptr);

      if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }
    }

    request = MakeSqlBuffer(
        "SELECT 57.0 FROM system.tables LIMIT 2");

    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(columns[0], 57.0);

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       &columnLens[i]);

      if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }
    }

    ret = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_CHECK_EQUAL(columns[0], 57.0);

    BOOST_CHECK_EQUAL(columnLens[0], static_cast< SQLLEN >(sizeof(T)));

    ret = SQLFetch(stmt);
    BOOST_CHECK(ret == SQL_NO_DATA);
  }

  int CountRows(SQLHSTMT stmt) {
    int res = 0;

    SQLRETURN ret = SQL_SUCCESS;

    while (ret == SQL_SUCCESS) {
      ret = SQLFetch(stmt);

      if (ret == SQL_NO_DATA) {
        break;
      }

      if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }

      ++res;
    }

    return res;
  }

  std::string BigTablePaginationTestIsEnabled;
};

BOOST_FIXTURE_TEST_SUITE(QueriesTestSuite, QueriesTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestSingleResultUsingBindCol) {
  ConnectToSW();
  SQLRETURN ret;

  // Test String and Boolean Types
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT 'string', true FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR name[buf_size]{};
  SQLLEN name_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, name, sizeof(name), &name_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  bool fieldBoolean = false;
  SQLLEN fieldBoolean_len = 0;
  ret = SQLBindCol(stmt, 2, SQL_C_BIT, &fieldBoolean, sizeof(fieldBoolean),
                   &fieldBoolean_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("string",
                    utility::SqlWcharToString(name, name_len, true));
  BOOST_CHECK_EQUAL(true, fieldBoolean);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);

  // Test Double Type
  request = MakeSqlBuffer(
      "SELECT 515.0 FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  double fieldDouble = 0;
  SQLLEN fieldDouble_len = 0;
  ret = SQLBindCol(stmt, 1, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble),
                   &fieldDouble_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL(515.0, fieldDouble);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);

  // Test Integer Type
  request = MakeSqlBuffer(
      "SELECT 1 FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLBIGINT fieldLong;
  SQLLEN fieldLong_len = 0;
  ret = SQLBindCol(stmt, 1, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                   &fieldLong_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL(1, fieldLong);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestNoDataErrorMessage) {
  ConnectToSW();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT 0 FROM system.tables LIMIT 0");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLMoreResults(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
  BOOST_REQUIRE_EQUAL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt),
                      "Cannot find ODBC error message");
}

BOOST_AUTO_TEST_CASE(TestSingleResultUsingGetData) {
  ConnectToSW();
  SQLRETURN ret;

  // Test String and Boolean Types
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT 'string', true FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  const int32_t buf_size = 1024;
  SQLWCHAR name[buf_size]{};
  SQLLEN name_len = 0;

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, name, sizeof(name), &name_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  bool fieldBoolean = false;
  SQLLEN fieldBoolean_len = 0;
  ret = SQLGetData(stmt, 2, SQL_C_BIT, &fieldBoolean, sizeof(fieldBoolean),
                   &fieldBoolean_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("string",
                    utility::SqlWcharToString(name, name_len, true));
  BOOST_CHECK_EQUAL(true, fieldBoolean);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);

  // Test Double Type
  request = MakeSqlBuffer(
      "SELECT 515.0 FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  double fieldDouble = 0;
  SQLLEN fieldDouble_len = 0;
  ret = SQLGetData(stmt, 1, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble),
                   &fieldDouble_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL(515.0, fieldDouble);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);

  // Test Integer Type
  request = MakeSqlBuffer(
      "SELECT 1 FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLBIGINT fieldLong;
  SQLLEN fieldLong_len = 0;
  ret = SQLGetData(stmt, 1, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                   &fieldLong_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL(1, fieldLong);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestMultiLineResultUsingGetData) {
  ConnectToSW();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT 'string' FROM system.tables LIMIT 2");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  const int32_t buf_size = 1024;
  SQLWCHAR name[buf_size]{};
  SQLLEN name_len = 0;

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, name, sizeof(name), &name_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("string",
                    utility::SqlWcharToString(name, name_len, true));

  // Fetch 2nd row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, name, sizeof(name), &name_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("string",
                    utility::SqlWcharToString(name, name_len, true));

  // Fetch 3th row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestRowSingleResultUsingBindCol) {
  ConnectToSW();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
    "SELECT 'column1', 'column2' FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR rowChar[buf_size]{};
  SQLLEN rowChar_len = 0;

  // fetch result as a string
  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, rowChar, sizeof(rowChar), &rowChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR rowWchar[buf_size]{};
  SQLLEN rowWchar_len = 0;

  // fetch result as a unicode string
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, rowWchar, sizeof(rowWchar),
                   &rowWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("column1",
                    utility::SqlCharToString(rowChar, rowChar_len));
  BOOST_CHECK_EQUAL("column2",
                    utility::SqlWcharToString(rowWchar, rowWchar_len, true));
}

BOOST_AUTO_TEST_CASE(TestNullSingleResultUsingBindCol) {
  ConnectToSW();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT NULL, NULL FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
  const int32_t buf_size = 1024;
  SQLCHAR nullChar[buf_size]{};
  SQLLEN nullChar_len = 0;

  // fetch result as a string
  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, nullChar, sizeof(nullChar),
    &nullChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR nullWchar[buf_size]{};
  SQLLEN nullWchar_len = 0;

  // fetch result as a unicode string
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, nullWchar, sizeof(nullWchar),
    &nullWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL(nullChar_len, SQL_NULL_DATA);
  BOOST_CHECK_EQUAL(nullWchar_len, SQL_NULL_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLCancel) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT table_name FROM system.tables");
  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLCancel(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
#if defined(__linux__) || defined(__APPLE__)
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);
#else
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLCloseCursor) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT table_name FROM system.tables");
  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLCloseCursor(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  ret = SQLCloseCursor(stmt);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL("24000: [unixODBC][Driver Manager]Invalid cursor state",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL("24000: No cursor was open",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(TestSmallResultPagination) {
  // This test runs a query which returns 3 rows. It sets each page
  // contains 1 row. It verifies the results are correct.
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT 'string' FROM system.tables LIMIT 3");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Fetch 1st row
  // These time_points could be reopened in case there is
  // a performance check need.
  // std::chrono::steady_clock::time_point time_exec_start =
  //    std::chrono::steady_clock::now();
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  /*
  std::chrono::steady_clock::time_point time_exec_end =
      std::chrono::steady_clock::now();

  std::chrono::steady_clock::duration time_span =
      time_exec_end - time_exec_start;

  double nseconds = double(time_span.count())
                    * std::chrono::steady_clock::period::num
                    / std::chrono::steady_clock::period::den;
  std::cout << "Fetching 1st row took " << nseconds << " seconds" << std::endl;
  */

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("string", utility::SqlWcharToString(id, id_len, true));

  // Fetch 2nd row
  // time_exec_start = std::chrono::steady_clock::now();
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  /*
  time_exec_end = std::chrono::steady_clock::now();

  time_span = time_exec_end - time_exec_start;

  nseconds = double(time_span.count()) * std::chrono::steady_clock::period::num
             / std::chrono::steady_clock::period::den;
  std::cout << "Fetching 2nd row took " << nseconds << " seconds" << std::endl;
  */

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("string", utility::SqlWcharToString(id, id_len, true));


  // Fetch 3rd row
  // time_exec_start = std::chrono::steady_clock::now();
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  /*
  time_exec_end = std::chrono::steady_clock::now();

  time_span = time_exec_end - time_exec_start;

  nseconds = double(time_span.count()) * std::chrono::steady_clock::period::num
             / std::chrono::steady_clock::period::den;
  std::cout << "Fetching 3rd row took " << nseconds << " seconds" << std::endl;
  */

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("string", utility::SqlWcharToString(id, id_len, true));

  // Fetch 4th row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSmallResultPaginationTermination) {
  // This test runs a query which returns 3 rows. It sets each page
  // contains 1 row. It only fetches the first two rows. It could verify
  // the 3rd row asynchronous thread be terminated without a problem.
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
    "SELECT 'string' FROM system.tables LIMIT 3");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("string", utility::SqlWcharToString(id, id_len, true));

  // Fetch 2nd row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("string", utility::SqlWcharToString(id, id_len, true));
}

BOOST_AUTO_TEST_CASE(TestSmallResultPaginationNoFetch) {
  // This test runs a query which returns 3 rows. It sets each page
  // to contain 1 row. It does not fetch any data. It verifies that
  // the asynchronous thread could be terminated without a problem.
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
    "SELECT 'string' FROM system.tables LIMIT 3");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestSQLFetchPaginationEmptyTable) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT table_name FROM system.tables WHERE table_name='nonexistent'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  BOOST_REQUIRE_NE(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt)
                       .find("01000: Query result is empty"),
                   std::string::npos);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSQLRowCountWithNoResults) {
  ConnectToSW();
  SQLRETURN ret;

  std::vector< SQLWCHAR > sql = MakeSqlBuffer(
      "SELECT table_name FROM system.tables WHERE table_name='asset'");

  ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLLEN rows = 0;

  ret = SQLRowCount(stmt, &rows);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // SQLRowCount should set rows to -1 as no rows were changed
  BOOST_CHECK_EQUAL(-1, rows);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt8) {
  CheckTwoRowsInt< signed char >(SQL_C_STINYINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint8) {
  CheckTwoRowsInt< unsigned char >(SQL_C_UTINYINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt16) {
  CheckTwoRowsInt< signed short >(SQL_C_SSHORT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint16) {
  CheckTwoRowsInt< unsigned short >(SQL_C_USHORT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt32) {
  CheckTwoRowsInt< SQLINTEGER >(SQL_C_SLONG);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint32) {
  CheckTwoRowsInt< SQLUINTEGER >(SQL_C_ULONG);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt64) {
  CheckTwoRowsInt< int64_t >(SQL_C_SBIGINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint64) {
  CheckTwoRowsInt< uint64_t >(SQL_C_UBIGINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsString) {
  ConnectToSW();

  SQLRETURN ret;

  const SQLSMALLINT columnsCnt = 1;

  SQLWCHAR columns[columnsCnt][ODBC_BUFFER_SIZE];
  SQLLEN reslen;

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i],
                     ODBC_BUFFER_SIZE * sizeof(SQLWCHAR), &reslen);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
  }

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
        "SELECT 1 FROM system.tables LIMIT 2");
  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true), "1");

  SQLLEN columnLens[columnsCnt];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i],
                     ODBC_BUFFER_SIZE * sizeof(SQLWCHAR), &columnLens[i]);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true), "1");

#ifdef __APPLE__
  SQLLEN expectedLen = 4;
#else
  SQLLEN expectedLen = 2;
#endif

  BOOST_CHECK_EQUAL(columnLens[0], expectedLen);


  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);

  request = MakeSqlBuffer(
        "SELECT true FROM system.tables LIMIT 2");
  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true), "1");

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i],
                     ODBC_BUFFER_SIZE * sizeof(SQLWCHAR), &columnLens[i]);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true), "1");


  BOOST_CHECK_EQUAL(columnLens[0], expectedLen);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);

  request = MakeSqlBuffer(
        "SELECT 515.0 FROM system.tables LIMIT 2");
  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true), "515");

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i],
                     ODBC_BUFFER_SIZE * sizeof(SQLWCHAR), &columnLens[i]);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true), "515");

#ifdef __APPLE__
  BOOST_CHECK_EQUAL(columnLens[0], 10);
#else
  BOOST_CHECK_EQUAL(columnLens[0], 6);
#endif

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestDefaultValues) {
  ConnectToSW();

  SQLRETURN ret;

  const SQLSMALLINT columnsCnt = 4;

  SQLLEN columnLens[columnsCnt];

  char strColumn[ODBC_BUFFER_SIZE];
  double defaultDoubleColumn = 1.0;
  int64_t defaultBigintColumn = 100;
  bool defaultBoolColumn = true;

  // Binding columns.
  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, &strColumn, ODBC_BUFFER_SIZE,
                   &columnLens[0]);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLBindCol(stmt, 2, SQL_C_BIT, &defaultBoolColumn, 0, &columnLens[1]);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLBindCol(stmt, 3, SQL_C_DOUBLE, &defaultDoubleColumn, 0,
                   &columnLens[2]);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLBindCol(stmt, 4, SQL_C_SBIGINT, &defaultBigintColumn, 0,
                   &columnLens[3]);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT 'string', true, 515.0, 1 FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Fetching the first non-null row.
  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Checking that the first three columns are not null.
  // flag, rebuffering_ratio, and video_startup_time will be null.
  
  BOOST_CHECK_EQUAL(columnLens[1], 1);
  BOOST_CHECK_EQUAL(columnLens[3], 8);

  BOOST_CHECK_EQUAL(columnLens[0], 6);
  BOOST_CHECK_EQUAL(columnLens[2], 8);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLMoreResults) {
  ConnectToSW();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT table_name FROM system.tables");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLMoreResults(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestExecuteAfterCursorClose) {
  ConnectToSW();

  double doubleField = 0.0;
  SQLWCHAR strField[1024];
  SQLLEN strFieldLen = 0;

  // Binding columns.
  SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, &strField, sizeof(strField),
                             &strFieldLen);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Binding columns.
  ret = SQLBindCol(stmt, 2, SQL_C_DOUBLE, &doubleField, 0, 0);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer(
      "SELECT 'string', 515.0 FROM system.tables LIMIT 1");

  ret = SQLPrepare(stmt, selectReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLExecute(stmt);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFreeStmt(stmt, SQL_CLOSE);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLExecute(stmt);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(std::abs(doubleField - 515.0) < 0.1, true);

  BOOST_CHECK_EQUAL("string",
                    utility::SqlWcharToString(strField, strFieldLen, true));

  ret = SQLFetch(stmt);

  BOOST_CHECK_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestCloseNonFullFetch) {
  ConnectToSW();

  double doubleField = 0.0;
  SQLWCHAR strField[1024];
  SQLLEN strFieldLen = 0;

  // Binding columns.
  SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, &strField, sizeof(strField),
                             &strFieldLen);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Binding columns.
  ret = SQLBindCol(stmt, 2, SQL_C_DOUBLE, &doubleField, 0, 0);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer(
    "SELECT 'string', 515.0 FROM system.tables LIMIT 1");

  ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  BOOST_CHECK_EQUAL(std::abs(doubleField - 515.0) < 0.1, true);

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(strField, strFieldLen, true),
                    "string");

  ret = SQLFreeStmt(stmt, SQL_CLOSE);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestErrorMessage) {
  ConnectToSW();

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer("SELECT A FROM B");

  SQLRETURN ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  std::string error = GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt);
  std::string pattern = "Failed to execute query";

  if (error.find(pattern) == std::string::npos) {
    BOOST_FAIL("'" + error + "' does not match '" + pattern + "'");
  }
}

BOOST_AUTO_TEST_CASE(TestManyCursors) {
  ConnectToSW();

  for (int32_t i = 0; i < 1; ++i) {
    std::vector< SQLWCHAR > req =
        MakeSqlBuffer("SELECT table_name FROM system.tables");

    SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    ret = SQLFreeStmt(stmt, SQL_CLOSE);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
  }
}

BOOST_AUTO_TEST_CASE(TestManyCursors2) {
  ConnectToSW();

  SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  for (int32_t i = 0; i < 1; ++i) {
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    std::vector< SQLWCHAR > req = MakeSqlBuffer(
      "SELECT 1 FROM system.tables LIMIT 1");

    ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    int32_t res = 0;
    SQLLEN resLen = 0;
    ret = SQLBindCol(stmt, 1, SQL_INTEGER, &res, 0, &resLen);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    BOOST_REQUIRE_EQUAL(res, 1);

    ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    stmt = NULL;
  }
}

BOOST_AUTO_TEST_CASE(TestSingleResultUsingGetDataWideChar) {
  ConnectToSW();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT table_name FROM system.tables WHERE table_name = 'asset'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLWCHAR fieldString[1024]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(u8"asset", utility::SqlWcharToString(
                                    fieldString, fieldString_len, true));

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultSelectWideCharUsingGetDataWideChar) {
  ConnectToSW();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      u8"SELECT table_name FROM system.tables WHERE table_name = 'asset'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR fieldString[buf_size]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(u8"asset", utility::SqlWcharToString(
                                    fieldString, fieldString_len, true));

  // Fetch 2nd row - does not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultSelectWideCharUsingGetDataNarrowChar) {
  ConnectToSW();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      u8"SELECT table_name FROM system.tables WHERE table_name = 'asset'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR fieldString[buf_size]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_CHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  if (ANSI_STRING_ONLY) {
    BOOST_CHECK_EQUAL(5, fieldString_len);
    BOOST_CHECK_EQUAL(std::string("asset"), std::string((char*)fieldString));
  } else {
    BOOST_CHECK_EQUAL(5, fieldString_len);
    BOOST_CHECK_EQUAL(std::string("asset"), std::string((char*)fieldString));
  }

  // Fetch 2nd row - does not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_SUITE_END()
