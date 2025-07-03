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

#include "iotsitewise/odbc/query/foreign_keys_query.h"

#include "iotsitewise/odbc/connection.h"
#include "iotsitewise/odbc/log.h"
#include "iotsitewise/odbc/type_traits.h"

namespace iotsitewise {
namespace odbc {
namespace query {
ForeignKeysQuery::ForeignKeysQuery(diagnostic::DiagnosableAdapter& diag)
    : Query(diag, QueryType::FOREIGN_KEYS) {
  using namespace iotsitewise::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(14);

  const std::string sch("");
  const std::string tbl("");

  columnsMeta.push_back(ColumnMeta(sch, tbl, "PKTABLE_CAT", ScalarType::STRING,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PKTABLE_SCHEM",
                                   ScalarType::STRING, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PKTABLE_NAME",
                                   ScalarType::STRING, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PKCOLUMN_NAME",
                                   ScalarType::STRING, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FKTABLE_CAT", ScalarType::STRING,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FKTABLE_SCHEM",
                                   ScalarType::STRING, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FKTABLE_NAME",
                                   ScalarType::STRING, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FKCOLUMN_NAME",
                                   ScalarType::STRING, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "KEY_SEQ", ScalarType::INT,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "UPDATE_RULE", ScalarType::INT,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DELETE_RULE", ScalarType::INT,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FK_NAME", ScalarType::STRING,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PK_NAME", ScalarType::STRING,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DEFERRABILITY",
                                   ScalarType::INT, Nullability::NULLABLE));
}

ForeignKeysQuery::~ForeignKeysQuery() {
  // No-op.
}

SqlResult::Type ForeignKeysQuery::Execute() {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLForeignKeys is not supported. Return empty result set.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_SUCCESS_WITH_INFO;
}

SqlResult::Type ForeignKeysQuery::Cancel() {
  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* ForeignKeysQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type ForeignKeysQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                       "SQLForeignKeys is not supported. No data is returned.",
                       LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ForeignKeysQuery::GetColumn(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                       "SQLForeignKeys is not supported. No data is returned.",
                       LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ForeignKeysQuery::Close() {
  return SqlResult::AI_SUCCESS;
}

bool ForeignKeysQuery::DataAvailable() const {
  return false;
}
int64_t ForeignKeysQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type ForeignKeysQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}
}  // namespace query
}  // namespace odbc
}  // namespace iotsitewise
