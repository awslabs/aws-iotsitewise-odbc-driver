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

#include "iotsitewise/odbc/query/table_metadata_query.h"

#include <aws/iotsitewise/model/ScalarType.h>

#include <algorithm>
#include <vector>

#include "iotsitewise/odbc/connection.h"
#include "iotsitewise/odbc/log.h"
#include "iotsitewise/odbc/type_traits.h"

using Aws::IoTSiteWise::Model::ScalarType;

namespace iotsitewise {
namespace odbc {
namespace query {
using iotsitewise::odbc::type_traits::OdbcNativeType;

TableMetadataQuery::TableMetadataQuery(
    diagnostic::DiagnosableAdapter& diag, Connection& connection,
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema,
    const boost::optional< std::string >& table,
    const boost::optional< std::string >& tableType)
    : Query(diag, iotsitewise::odbc::query::QueryType::TABLE_METADATA),
      connection(connection),
      catalog(catalog),
      schema(schema),
      table(table),
      tableType(tableType),
      executed(false),
      fetched(false),
      all_schemas(false),
      all_catalogs(false),
      all_table_types(false),
      meta(),
      columnsMeta() {
  LOG_DEBUG_MSG("TableMetadataQuery constructor is called");
  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(5);

  const std::string sch("");
  const std::string tbl("");

  if (!connection.GetMetadataID()) {
    all_catalogs = catalog && *catalog == SQL_ALL_CATALOGS && schema
                   && schema->empty() && table && table->empty();

    all_schemas = schema && *schema == SQL_ALL_SCHEMAS && catalog
                  && catalog->empty() && table && table->empty();
  }

  // TableType is a value list argument, regardless of the setting of
  // SQL_ATTR_METADATA_ID.
  all_table_types = tableType && *tableType == SQL_ALL_TABLE_TYPES && catalog
                    && catalog->empty() && schema && schema->empty() && table
                    && table->empty();

  int32_t odbcVer = connection.GetEnvODBCVer();

  // driver needs to have have 2.0 column names for applications (e.g., Excel on
  // macOS) that expect ODBC driver ver 2.0.
  std::string catalog_meta_name = "TABLE_CAT";
  std::string schema_meta_name = "TABLE_SCHEM";
  if (odbcVer == SQL_OV_ODBC2) {
    // For backwards compatibility with ODBC 2.0
    catalog_meta_name = "TABLE_QUALIFIER";
    schema_meta_name = "TABLE_OWNER";
  }
  LOG_DEBUG_MSG("all_catalogs is " << all_catalogs << ", all_schemas is "
                                   << all_schemas << ", all_table_types is "
                                   << all_table_types << ", odbcVer is "
                                   << odbcVer);

  if (all_catalogs) {
    /**
     * If CatalogName equals SQL_ALL_CATALOGS, and SchemaName and TableName are
     * empty strings, the result set should contain a list of valid catalogs for
     * the data source. (All columns except the TABLE_CAT column contain NULLs.)
     * If DATABASE_AS_SCHEMA is set to TRUE, an empty result set is returned
     * since driver does not support catalogs, otherwise, a list of databases is
     * returned.
     */
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, catalog_meta_name, ScalarType::STRING,
        DATABASE_AS_SCHEMA ? Nullability::NULLABLE : Nullability::NO_NULL));
    columnsMeta.push_back(ColumnMeta(sch, tbl, schema_meta_name,
                                     ScalarType::STRING,
                                     Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, "TABLE_NAME", ScalarType::STRING, Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, "TABLE_TYPE", ScalarType::STRING, Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(sch, tbl, "REMARKS", ScalarType::STRING,
                                     Nullability::NULLABLE));
  } else if (all_schemas) {
    /**
     * If SchemaName equals SQL_ALL_SCHEMAS, and CatalogName and TableName are
     * empty strings, the result set should contain a list of valid schemas for
     * the data source. (All columns except the TABLE_SCHEM column contain
     * NULLs.)
     * If DATABASE_AS_SCHEMA is set to TRUE, a list of databases is returned,
     * otherwise, an empty result set is returned since driver does not
     * support schemas.
     */
    columnsMeta.push_back(ColumnMeta(sch, tbl, catalog_meta_name,
                                     ScalarType::STRING,
                                     Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, schema_meta_name, ScalarType::STRING,
        DATABASE_AS_SCHEMA ? Nullability::NO_NULL : Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, "TABLE_NAME", ScalarType::STRING, Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, "TABLE_TYPE", ScalarType::STRING, Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(sch, tbl, "REMARKS", ScalarType::STRING,
                                     Nullability::NULLABLE));
  } else if (all_table_types) {
    /**
     * If TableType equals SQL_ALL_TABLE_TYPES and CatalogName, SchemaName, and
     * TableName are empty strings, the result set contains a list of valid
     * table types for the data source. (All columns except the TABLE_TYPE
     * column contain NULLs.) "TABLE_TYPE" is set to "TABLE" for IoT SiteWise.
     */
    columnsMeta.push_back(ColumnMeta(sch, tbl, catalog_meta_name,
                                     ScalarType::STRING,
                                     Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(sch, tbl, schema_meta_name,
                                     ScalarType::STRING,
                                     Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, "TABLE_NAME", ScalarType::STRING, Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, "TABLE_TYPE", ScalarType::STRING, Nullability::NO_NULL));
    columnsMeta.push_back(ColumnMeta(sch, tbl, "REMARKS", ScalarType::STRING,
                                     Nullability::NULLABLE));
  } else {
    columnsMeta.push_back(ColumnMeta(sch, tbl, catalog_meta_name,
                                     ScalarType::STRING,
                                     Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(sch, tbl, schema_meta_name,
                                     ScalarType::STRING,
                                     Nullability::NULLABLE));
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, "TABLE_NAME", ScalarType::STRING, Nullability::NO_NULL));
    columnsMeta.push_back(ColumnMeta(
        sch, tbl, "TABLE_TYPE", ScalarType::STRING, Nullability::NO_NULL));
    columnsMeta.push_back(ColumnMeta(sch, tbl, "REMARKS", ScalarType::STRING,
                                     Nullability::NULLABLE));
  }
}

TableMetadataQuery::~TableMetadataQuery() {
  // No-op.
}

SqlResult::Type TableMetadataQuery::Execute() {
  LOG_DEBUG_MSG("Execute: catalog='" << (catalog ? *catalog : "NULL")
                << "', schema='" << (schema ? *schema : "NULL") 
                << "', table='" << (table ? *table : "NULL")
                << "', tableType='" << (tableType ? *tableType : "NULL") << "'");
  
  if (executed) {
    Close();
  }

  SqlResult::Type result = MakeRequestGetTablesMeta();

  if (result == SqlResult::AI_SUCCESS
      || result == SqlResult::AI_SUCCESS_WITH_INFO) {
    executed = true;
    fetched = false;

    if (!meta.empty()) {
      cursor = meta.begin();
      // Log all table names for debugging
      for (size_t i = 0; i < meta.size(); ++i) {
        LOG_DEBUG_MSG("Execute: Table[" << i << "] = " << meta[i].GetTableName().get_value_or("NULL"));
      }
    } else {
      LOG_DEBUG_MSG("Execute: meta is empty");
    }
  }

  return result;
}

SqlResult::Type TableMetadataQuery::Cancel() {
  LOG_DEBUG_MSG("Cancel is called");

  if (dataQuery_) {
    dataQuery_->Cancel();
  }

  Close();

  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* TableMetadataQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type TableMetadataQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  LOG_DEBUG_MSG("FetchNextRow is called");
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");
    return SqlResult::AI_ERROR;
  } else if (meta.empty()) {
    LOG_DEBUG_MSG("FetchNextRow: Exit due to meta vector is empty");
    return SqlResult::AI_NO_DATA;
  }

  if (!fetched) {
    fetched = true;
    LOG_DEBUG_MSG("FetchNextRow: First fetch, cursor at position 0, meta.size() = " << meta.size());
  } else if (cursor != meta.end()) {
    ++cursor;
    size_t position = cursor - meta.begin();
    LOG_DEBUG_MSG("FetchNextRow: Advanced cursor to position " << position);
  }
  
  if (cursor == meta.end()) {
    LOG_DEBUG_MSG("FetchNextRow: Exit due to cursor reaches the end of meta");
    return SqlResult::AI_NO_DATA;
  }

  size_t currentPosition = cursor - meta.begin();
  LOG_DEBUG_MSG("FetchNextRow: Processing table at position " << currentPosition 
                << " = " << cursor->GetTableName().get_value_or("NULL"));

  app::ColumnBindingMap::iterator it;

  for (it = columnBindings.begin(); it != columnBindings.end(); ++it)
    GetColumn(it->first, it->second);

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type TableMetadataQuery::GetColumn(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  LOG_DEBUG_MSG("GetColumn is called");
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");
    return SqlResult::AI_ERROR;
  } else if (meta.empty()) {
    LOG_DEBUG_MSG("Exit due to meta vector is empty");
    return SqlResult::AI_NO_DATA;
  }

  if (cursor == meta.end()) {
    std::string errMsg = "Cursor has reached end of the result set.";
    diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE, errMsg);
    return SqlResult::AI_ERROR;
  }

  const meta::TableMeta& currentColumn = *cursor;

  LOG_DEBUG_MSG("columnIdx: " << columnIdx);

  switch (columnIdx) {
    case ResultColumn::TABLE_CAT: {
      buffer.PutString(currentColumn.GetCatalogName());
      break;
    }

    case ResultColumn::TABLE_SCHEM: {
      buffer.PutString(currentColumn.GetSchemaName());
      break;
    }

    case ResultColumn::TABLE_NAME: {
      buffer.PutString(currentColumn.GetTableName());
      break;
    }

    case ResultColumn::TABLE_TYPE: {
      buffer.PutString(currentColumn.GetTableType());
      break;
    }

    case ResultColumn::REMARKS: {
      buffer.PutString(currentColumn.GetRemarks());
      break;
    }

    default:
      break;
  }

  LOG_INFO_MSG("buffer: " << buffer.GetString(SQL_NTS));

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type TableMetadataQuery::Close() {
  LOG_DEBUG_MSG("Close is called");
  meta.clear();
  cursor = meta.end();

  executed = false;

  return SqlResult::AI_SUCCESS;
}

bool TableMetadataQuery::DataAvailable() const {
  return executed && !meta.empty() && cursor != meta.end();
}

int64_t TableMetadataQuery::AffectedRows() const {
  return 0;
}

int64_t TableMetadataQuery::RowNumber() const {
  if (!executed || cursor == meta.end()) {
    diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                         "Cursor does not point to any data.",
                         iotsitewise::odbc::LogLevel::Type::WARNING_LEVEL);

    LOG_DEBUG_MSG("Row number returned is 0.");

    return 0;
  }

  int64_t rowNumber = cursor - meta.begin() + 1;
  LOG_DEBUG_MSG("Row number returned: " << rowNumber);

  return rowNumber;
}

SqlResult::Type TableMetadataQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}

SqlResult::Type TableMetadataQuery::MakeRequestGetTablesMeta() {
  LOG_DEBUG_MSG("MakeRequestGetTablesMeta is called");
  // clear meta object at beginning of function
  meta.clear();

  if (all_table_types) {
    // case for SQL_ALL_TABLE_TYPES is the same whether databases are reported
    // as schemas or catalogs
    using meta::TableMeta;
    std::string tableType = "TABLE";
    meta.emplace_back(TableMeta());
    meta.back().Read(tableType);

    return SqlResult::AI_SUCCESS;
  }

  if (tableType) {
    // Parse provided table types
    bool validTableType = false;
    if (tableType->empty() || *tableType == SQL_ALL_TABLE_TYPES) {
      // Table type not specified. "TABLE" table type is accepted
      validTableType = true;
    } else {
      std::stringstream ss(*tableType);
      std::string singleTableType;
      while (std::getline(ss, singleTableType, ',')) {
        if (dequote(utility::Trim(singleTableType)) == "TABLE") {
          validTableType = true;
          break;
        }
      }
    }

    if (!validTableType) {
      // table type(s) provided is not valid for IoT SiteWise.
      std::string warnMsg =
          "Empty result set is returned as tableType is set to \"" + *tableType
          + "\" and IoT SiteWise only supports \"TABLE\" table type";
      diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING, warnMsg,
                           iotsitewise::odbc::LogLevel::Type::WARNING_LEVEL);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }
  }

  // Check for corner cases and handle database search patterns / identifiers in
  // tables meta retrieval
  SqlResult::Type retval = getTables();
  LOG_DEBUG_MSG("retval is " << retval);
  return retval;
}

SqlResult::Type TableMetadataQuery::getTables() {
  LOG_DEBUG_MSG("getTables is called");
  SqlResult::Type retval;

  if(connection.GetMetadataID()) {
    if (!table) {
      diag.AddStatusRecord(SqlState::SHY009_INVALID_USE_OF_NULL_POINTER,
                           "The SQL_ATTR_METADATA_ID statement attribute "
                           "is set to SQL_TRUE, "
                           "and TableName argument was a null pointer.");
      return SqlResult::AI_ERROR;
    }
  }

  std::vector< std::string > tableNames;
  std::string tablePattern = table.get_value_or("%");
  LOG_DEBUG_MSG("getTables: tablePattern = " << tablePattern);
  
  SqlResult::Type res = getMatchedTables(tablePattern.empty() ? "%" : tablePattern, tableNames);

  if (res != SqlResult::AI_SUCCESS) {
    retval = res;
  } else {
    int numTables = tableNames.size();
    LOG_DEBUG_MSG("getTables: numTables from getMatchedTables = " << numTables);
    
    // Clear any existing metadata to prevent duplicates
    meta.clear();

    bool match = false;
    for (int j = 0; j < numTables; j++) {
      using meta::TableMeta;

      // For Excel compatibility, ensure we only add each table once
      // Check if this table name already exists in meta
      bool alreadyExists = false;
      for (const auto& existingMeta : meta) {
        if (existingMeta.GetTableName() &&
            existingMeta.GetTableName().get() == tableNames.at(j)) {
          alreadyExists = true;
          break;
        }
      }

      if (connection.GetMetadataID() && !alreadyExists) {
        // Check exact match for table name case-insensitive identifier
        std::string foundTableName = tableNames.at(j);
        Aws::String tableUpper =
            Aws::Utils::StringUtils::ToUpper(table.get().data());
        Aws::String tbNameUpper =
            Aws::Utils::StringUtils::ToUpper(foundTableName.data());
        match = (tableUpper == tbNameUpper);

        if (match) {
          meta.emplace_back(TableMeta());
          meta.back().SetTableName(foundTableName);
          meta.back().SetTableType("TABLE");
          // Explicitly set catalog, schema, and remarks to NULL for Excel compatibility
          meta.back().SetCatalogNameNull();
          meta.back().SetSchemaNameNull();
          meta.back().SetRemarksNull();
          LOG_DEBUG_MSG("getTables: Found matched table for " << table.get());
        }
      } else if (!alreadyExists) {
          meta.emplace_back(TableMeta());
          meta.back().SetTableName(tableNames.at(j));
          meta.back().SetTableType("TABLE");
          // Explicitly set catalog, schema, and remarks to NULL for Excel compatibility
          meta.back().SetCatalogNameNull();
          meta.back().SetSchemaNameNull();
          meta.back().SetRemarksNull();
          LOG_DEBUG_MSG("getTables: Added table " << tableNames.at(j) << " to meta");
      } else {
          LOG_DEBUG_MSG("getTables: Skipped duplicate table " << tableNames.at(j));
      }
    }

    LOG_DEBUG_MSG("getTables: final meta.size() = " << meta.size());

    if (meta.empty()) {
      std::string warnMsg =
        "Empty result set is returned as we could not find tables with "
        + table.get();
      diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING, warnMsg,
                          iotsitewise::odbc::LogLevel::Type::WARNING_LEVEL);
      retval = SqlResult::AI_SUCCESS_WITH_INFO;
    }

    retval = SqlResult::AI_SUCCESS;
  }

  LOG_DEBUG_MSG("getTables: retval = " << retval);
  return retval;
}

SqlResult::Type TableMetadataQuery::getMatchedTables(
    const std::string& tablePattern, std::vector< std::string >& tableNames) {
  LOG_DEBUG_MSG("getMatchedTables is called");
  std::string sql;

  // If pattern is "%" (get all tables), use a simpler query
  if (tablePattern == "%" || tablePattern.empty()) {
    sql = "SELECT table_name FROM system.tables";
  } else {
    std::string tablePatternLowercase;
    tablePatternLowercase.resize(tablePattern.size());
    std::transform(tablePattern.begin(), tablePattern.end(),
      tablePatternLowercase.begin(), ::tolower);
    sql = "SELECT table_name FROM system.tables WHERE table_name LIKE \'" + tablePatternLowercase + "\'";
  }
  LOG_DEBUG_MSG("sql is " << sql);

  dataQuery_ = std::make_shared< DataQuery >(diag, connection, sql);
  SqlResult::Type result = dataQuery_->Execute();

  if (result == SqlResult::AI_NO_DATA) {
    std::string warnMsg = "No table is found with pattern \'" + tablePattern + "\'";
    diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING, warnMsg,
                         iotsitewise::odbc::LogLevel::Type::WARNING_LEVEL);
    return SqlResult::AI_SUCCESS_WITH_INFO;
    // DataQuery::Execute() does not return SUCCESS_WITH_INFO
  } else if (result != SqlResult::AI_SUCCESS) {
    LOG_ERROR_MSG("Failed to execute sql:" << sql);
    return result;
  }

  app::ColumnBindingMap columnBindings;
  SqlLen buflen = STRING_BUFFER_SIZE;
  // According to IoT SiteWise, table name could only contain
  // letters, digits, dashes, periods or underscores. It could
  // not be a unicode string.
  char tableName[STRING_BUFFER_SIZE]{};
  ApplicationDataBuffer buf(OdbcNativeType::Type::AI_CHAR, &tableName, buflen,
                            nullptr);
  columnBindings[1] = buf;

  while (dataQuery_->FetchNextRow(columnBindings) == SqlResult::AI_SUCCESS) {
    tableNames.emplace_back(tableName);
    LOG_DEBUG_MSG("tableName: " << tableName);
  }

  return SqlResult::AI_SUCCESS;
}

std::string TableMetadataQuery::dequote(const std::string& s) {
  if (s.size() >= 2
      && ((s.front() == '\'' && s.back() == '\'')
          || (s.front() == '"' && s.back() == '"'))) {
    return s.substr(1, s.size() - 2);
  }
  return s;
}
}  // namespace query
}  // namespace odbc
}  // namespace iotsitewise
