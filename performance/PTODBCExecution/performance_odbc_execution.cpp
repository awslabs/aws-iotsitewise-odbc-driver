/*
 * Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// clang-format off
#ifdef _WIN32
#include <windows.h>
#endif //_WIN32

#include "chrono"
#include <string>
#include <vector>
#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include "performance_helper.h"
// clang-format on
#define ITERATION_COUNT 12
testString dsn_conn_string = CREATE_STRING("DSN=iotsitewise-iam");
const wchar_t* const query = L"SELECT * FROM ODBCTest.IoT limit 10000";

int Setup(SQLHENV* env, SQLHDBC* conn, SQLHSTMT* hstmt) {
  SQLTCHAR out_conn_string[1024];
  SQLSMALLINT out_conn_string_length;
  if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, env))
      && SQL_SUCCEEDED(
          SQLSetEnvAttr(*env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0))
      && SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, *env, conn))
      && SQL_SUCCEEDED(SQLDriverConnect(
          *conn, NULL, (SQLTCHAR*)dsn_conn_string.c_str(), SQL_NTS,
          out_conn_string, HELPER_SIZEOF(out_conn_string),
          &out_conn_string_length, SQL_DRIVER_COMPLETE))
      && SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, *conn, hstmt))) {
    return SQL_SUCCESS;
  }
  return SQL_ERROR;
}

void Teardown(SQLHDBC* conn, SQLHENV* env) {
  SQLDisconnect(*conn);
  SQLFreeHandle(SQL_HANDLE_ENV, *env);
}

int QueryExecutionTime() {
  SQLRETURN ret = SQL_ERROR;
  try {
    SQLHENV env = SQL_NULL_HENV;
    SQLHDBC conn = SQL_NULL_HDBC;
    SQLHSTMT hstmt = SQL_NULL_HSTMT;
    if (SQL_SUCCEEDED(Setup(&env, &conn, &hstmt))) {
      std::cout << "Time(ms) for query execution:" << std::endl;
      for (int i = 0; i < ITERATION_COUNT; i++) {
        // Calculate time(ms) for query execution
        auto start = std::chrono::steady_clock::now();
        ret = SQLExecDirect(hstmt, (SQLTCHAR*)query, SQL_NTS);
        auto end = std::chrono::steady_clock::now();
        std::cout << std::chrono::duration_cast< std::chrono::milliseconds >(
                         end - start)
                         .count()
                  << std::endl;
        SQLCloseCursor(hstmt);
      }
      Teardown(&conn, &env);
    }
  } catch (...) {
    std::cout << "Exception occurred" << std::endl;
  }
  return ret;
}

int main() {
  return QueryExecutionTime();
}
