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

#ifndef _IOTSITEWISE_ODBC_QUERY_QUERY
#define _IOTSITEWISE_ODBC_QUERY_QUERY

#include <stdint.h>

#include <map>

#include "iotsitewise/odbc/common_types.h"
#include "iotsitewise/odbc/diagnostic/diagnosable_adapter.h"
#include "iotsitewise/odbc/meta/column_meta.h"

using iotsitewise::odbc::SqlResult;

namespace iotsitewise {
namespace odbc {
namespace query {
/** Query type. */
struct QueryType {
  enum Type {
    /** Column metadata query type. */
    COLUMN_METADATA,

    /** Data query type. */
    DATA,

    /** Foreign keys query type. */
    FOREIGN_KEYS,

    /** Primary keys query type. */
    PRIMARY_KEYS,

    /** Special columns query type. */
    SPECIAL_COLUMNS,

    /** Table metadata query type. */
    TABLE_METADATA,

    /** Type info query type. */
    TYPE_INFO,

    /** Statistics query type. */
    STATISTICS,

    /** Procedures query type. */
    PROCEDURES,

    /** Procedure columns query type. */
    PROCEDURE_COLUMNS,

    /** Column privileges query type. */
    COLUMN_PRIVILEGES,

    /** Table privileges query type. */
    TABLE_PRIVILEGES,

    /** Internal query, that should be parsed by a driver itself. */
    INTERNAL
  };
};

/**
 * Query.
 */
class Query {
 public:
  /**
   * Destructor.
   */
  virtual ~Query() {
    // No-op.
  }

  /**
   * Execute query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Execute() = 0;

  /**
   * Cancel query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Cancel() = 0;

  /**
   * Fetch next result row to application buffers.
   *
   * @param columnBindings Application buffers to put data to.
   * @return Operation result.
   */
  virtual SqlResult::Type FetchNextRow(
      iotsitewise::odbc::app::ColumnBindingMap& columnBindings) = 0;

  /**
   * Get data of the specified column in the result set.
   *
   * @param columnIdx Column index.
   * @param buffer Buffer to put column data to.
   * @return Operation result.
   */
  virtual SqlResult::Type GetColumn(
      uint16_t columnIdx,
      iotsitewise::odbc::app::ApplicationDataBuffer& buffer) = 0;

  /**
   * Close query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Close() = 0;

  /**
   * Get column metadata.
   *
   * @return Column metadata.
   */
  virtual const iotsitewise::odbc::meta::ColumnMetaVector* GetMeta() = 0;

  /**
   * Check if data is available.
   *
   * @return True if data is available.
   */
  virtual bool DataAvailable() const = 0;

  /**
   * Get number of rows affected by the statement.
   *
   * @return Number of rows affected by the statement.
   */
  virtual int64_t AffectedRows() const = 0;

  /**
   * Get row number of the row that the cursor points at.
   * Row number starts at 1.
   * 0 is invalid row number and is the default return value for unsupported
   * queries.
   *
   * @return Row number of the row that the cursor points at.
   */
  virtual int64_t RowNumber() const {
    return 0;
  }

  /**
   * Move to the next result set.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type NextResultSet() = 0;

  /**
   * Get query type.
   *
   * @return Query type.
   */
  QueryType::Type GetType() const {
    return type;
  }

 protected:
  /**
   * Constructor.
   */
  Query(iotsitewise::odbc::diagnostic::DiagnosableAdapter& diag,
        QueryType::Type type)
      : diag(diag), type(type) {
    // No-op.
  }

  /** Diagnostics collector. */
  iotsitewise::odbc::diagnostic::DiagnosableAdapter& diag;

  /** Query type. */
  QueryType::Type type;
};
}  // namespace query
}  // namespace odbc
}  // namespace iotsitewise

#endif  //_IOTSITEWISE_ODBC_QUERY_QUERY
