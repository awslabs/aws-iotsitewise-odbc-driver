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

#ifndef _IOTSITEWISE_ODBC_META_TABLE_META
#define _IOTSITEWISE_ODBC_META_TABLE_META

#include <string>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

#include "iotsitewise/odbc/utility.h"

namespace iotsitewise {
namespace odbc {
namespace meta {
/**
 * Table metadata.
 */
class TableMeta {
 public:
  /**
   * Default constructor.
   */
  TableMeta() {
    // No-op.
  }

  /**
   * Constructor.
   *
   * @param catalogName Catalog name.
   * @param schemaName Schema name.
   * @param tableName Table name.
   * @param tableType Table type.
   */
  TableMeta(const std::string& catalogName, const std::string& schemaName,
            const std::string& tableName, const std::string& tableType)
      : catalogName(catalogName),
        schemaName(schemaName),
        tableName(tableName),
        tableType(tableType) {
    // No-op.
  }

  /**
   * Destructor.
   */
  ~TableMeta() {
    // No-op.
  }

  /**
   * Copy constructor.
   */
  TableMeta(const TableMeta& other)
      : catalogName(other.catalogName),
        schemaName(other.schemaName),
        tableName(other.tableName),
        tableType(other.tableType),
        remarks(other.remarks) {
    // No-op.
  }

  /**
   * Copy operator.
   */
  TableMeta& operator=(const TableMeta& other) {
    catalogName = other.catalogName;
    schemaName = other.schemaName;
    tableName = other.tableName;
    tableType = other.tableType;
    remarks = other.remarks;

    return *this;
  }

  /**
   * Read table type
   * @param tbType string for table type.
   */
  void Read(std::string& tbType);

  /**
   * Set table name.
   * @param tableName Table name.
   */
  void SetTableName(const std::string& tableName) {
    this->tableName = tableName;
  }

  /**
   * Set table type.
   * @param tableType Table type.
   */
  void SetTableType(const std::string& tableType) {
    this->tableType = tableType;
  }

  /**
   * Set catalog name.
   * @param catalogName Catalog name.
   */
  void SetCatalogName(const std::string& catalogName) {
    this->catalogName = catalogName;
  }

  /**
   * Set catalog name to NULL.
   */
  void SetCatalogNameNull() {
    this->catalogName = boost::none;
  }

  /**
   * Set schema name.
   * @param schemaName Schema name.
   */
  void SetSchemaName(const std::string& schemaName) {
    this->schemaName = schemaName;
  }

  /**
   * Set schema name to NULL.
   */
  void SetSchemaNameNull() {
    this->schemaName = boost::none;
  }

  /**
   * Set remarks.
   * @param remarks Remarks.
   */
  void SetRemarks(const std::string& remarks) {
    this->remarks = remarks;
  }

  /**
   * Set remarks to NULL.
   */
  void SetRemarksNull() {
    this->remarks = boost::none;
  }

  /**
   * Get catalog name.
   * @return Catalog name.
   */
  const boost::optional< std::string >& GetCatalogName() const {
    return catalogName;
  }

  /**
   * Get schema name.
   * @return Schema name.
   */
  const boost::optional< std::string >& GetSchemaName() const {
    return schemaName;
  }

  /**
   * Get table name.
   * @return Table name.
   */
  const boost::optional< std::string >& GetTableName() const {
    return tableName;
  }

  /**
   * Get table type.
   * @return Table type.
   */
  const boost::optional< std::string >& GetTableType() const {
    return tableType;
  }

  /**
   * Get the remarks.
   * @return Remarks.
   */
  const boost::optional< std::string >& GetRemarks() const {
    return remarks;
  }

 private:
  /** Catalog name. */
  boost::optional< std::string > catalogName;

  /** Schema name. */
  boost::optional< std::string > schemaName;

  /** Table name. */
  boost::optional< std::string > tableName;

  /** Table type. */
  boost::optional< std::string > tableType;

  /** Remarks */
  boost::optional< std::string > remarks;
};

/** Table metadata vector alias. */
typedef std::vector< TableMeta > TableMetaVector;
}  // namespace meta
}  // namespace odbc
}  // namespace iotsitewise

#endif  //_IOTSITEWISE_ODBC_META_TABLE_META
