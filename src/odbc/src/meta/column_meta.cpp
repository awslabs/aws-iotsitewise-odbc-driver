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

#include "iotsitewise/odbc/meta/column_meta.h"

#include "iotsitewise/odbc/utils.h"
#include "iotsitewise/odbc/common_types.h"
#include "iotsitewise/odbc/log.h"
#include "iotsitewise/odbc/system/odbc_constants.h"
#include "iotsitewise/odbc/type_traits.h"

#include <aws/iotsitewise/model/ColumnType.h>
#include <sqltypes.h>

namespace iotsitewise {
namespace odbc {
namespace meta {

#define DBG_STR_CASE(x) \
  case x:               \
    return #x

const char* ColumnMeta::AttrIdToString(uint16_t id) {
  switch (id) {
    DBG_STR_CASE(SQL_DESC_LABEL);
    DBG_STR_CASE(SQL_DESC_BASE_COLUMN_NAME);
    DBG_STR_CASE(SQL_DESC_NAME);
    DBG_STR_CASE(SQL_DESC_TABLE_NAME);
    DBG_STR_CASE(SQL_DESC_BASE_TABLE_NAME);
    DBG_STR_CASE(SQL_DESC_SCHEMA_NAME);
    DBG_STR_CASE(SQL_DESC_CATALOG_NAME);
    DBG_STR_CASE(SQL_DESC_LITERAL_PREFIX);
    DBG_STR_CASE(SQL_DESC_LITERAL_SUFFIX);
    DBG_STR_CASE(SQL_DESC_TYPE_NAME);
    DBG_STR_CASE(SQL_DESC_LOCAL_TYPE_NAME);
    DBG_STR_CASE(SQL_DESC_FIXED_PREC_SCALE);
    DBG_STR_CASE(SQL_DESC_AUTO_UNIQUE_VALUE);
    DBG_STR_CASE(SQL_DESC_CASE_SENSITIVE);
    DBG_STR_CASE(SQL_DESC_CONCISE_TYPE);
    DBG_STR_CASE(SQL_DESC_TYPE);
    DBG_STR_CASE(SQL_DESC_DISPLAY_SIZE);
    DBG_STR_CASE(SQL_DESC_LENGTH);
    DBG_STR_CASE(SQL_DESC_OCTET_LENGTH);
    DBG_STR_CASE(SQL_DESC_NULLABLE);
    DBG_STR_CASE(SQL_DESC_NUM_PREC_RADIX);
    DBG_STR_CASE(SQL_DESC_PRECISION);
    DBG_STR_CASE(SQL_DESC_SCALE);
    DBG_STR_CASE(SQL_DESC_SEARCHABLE);
    DBG_STR_CASE(SQL_DESC_UNNAMED);
    DBG_STR_CASE(SQL_DESC_UNSIGNED);
    DBG_STR_CASE(SQL_DESC_UPDATABLE);
    DBG_STR_CASE(SQL_COLUMN_LENGTH);
    DBG_STR_CASE(SQL_COLUMN_PRECISION);
    DBG_STR_CASE(SQL_COLUMN_SCALE);
    default:
      break;
  }
  return "<< UNKNOWN ID >>";
}

#undef DBG_STR_CASE

SqlLen Nullability::ToSql(boost::optional< int32_t > nullability) {
  if (!nullability) {
    LOG_WARNING_MSG(
        "nullability is not defined. Returning SQL_NULLABLE_UNKNOWN by "
        "default");
    return SQL_NULLABLE_UNKNOWN;
  }
  switch (*nullability) {
    case Nullability::NO_NULL:
      return SQL_NO_NULLS;

    case Nullability::NULLABLE:
      return SQL_NULLABLE;

    case Nullability::NULLABILITY_UNKNOWN:
      return SQL_NULLABLE_UNKNOWN;

    default:
      break;
  }

  assert(false);
  return SQL_NULLABLE_UNKNOWN;
}

const std::string TABLE_CAT = "TABLE_CAT";
const std::string TABLE_SCHEM = "TABLE_SCHEM";
const std::string TABLE_NAME = "TABLE_NAME";
const std::string COLUMN_NAME = "COLUMN_NAME";
const std::string DATA_TYPE = "DATA_TYPE";
const std::string DECIMAL_DIGITS = "DECIMAL_DIGITS";
const std::string REMARKS = "REMARKS";
const std::string COLUMN_DEF = "COLUMN_DEF";
const std::string NULLABLE = "NULLABLE";
const std::string ORDINAL_POSITION = "ORDINAL_POSITION";
const std::string IS_AUTOINCREMENT = "IS_AUTOINCREMENT";

Aws::IoTSiteWise::Model::ScalarType ColumnMeta::GetScalarDataType(
    const std::string& dataType) {
  LOG_DEBUG_MSG("GetScalarDataType is called with dataType " << dataType);
  if (dataType == "STRING") {
    return Aws::IoTSiteWise::Model::ScalarType::STRING;
  } else if (dataType == "DOUBLE") {
    return Aws::IoTSiteWise::Model::ScalarType::DOUBLE;
  } else if (dataType == "BOOLEAN") {
    return Aws::IoTSiteWise::Model::ScalarType::BOOLEAN;
  } else if (dataType == "TIMESTAMP") {
    return Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP;
  } else if (dataType == "INTEGER") {
    return Aws::IoTSiteWise::Model::ScalarType::INT;
  } else {
    return Aws::IoTSiteWise::Model::ScalarType::NOT_SET;
  }
}

void ColumnMeta::ReadColumnMetadata(app::ColumnBindingMap& columnBindings, int32_t position) {
  LOG_DEBUG_MSG("ReadColumnMetadata is called");
  
  auto itr = columnBindings.find(1);
  if (itr == columnBindings.end()) {
    LOG_ERROR_MSG("Could not find the first column");
    return;
  }
  columnName = itr->second.GetString(STRING_BUFFER_SIZE);

  itr = columnBindings.find(2);
  if (itr == columnBindings.end()) {
    LOG_ERROR_MSG("Could not find the second column");
    return;
  }

  dataType = static_cast< int16_t >(
      GetScalarDataType(itr->second.GetString(STRING_BUFFER_SIZE)));

  itr = columnBindings.find(3);
  if (itr == columnBindings.end()) {
    LOG_ERROR_MSG("Could not find the third column");
    return;
  }
  
  // Column 3 is table_name from the database - we can validate it matches what we expect
  std::string tableNameFromDB = itr->second.GetString(STRING_BUFFER_SIZE);
  LOG_DEBUG_MSG("Table name from DB: " << tableNameFromDB << ", Expected table name: " << (tableName ? *tableName : "NULL"));
  
  // Note: We don't overwrite the tableName that was already set correctly in the constructor
  // The tableName field should already be set to the correct value from the function parameter
  
  if (columnName.value() == "int_value" || columnName.value() == "boolean_value" || 
      columnName.value() == "double_value" || columnName.value() == "string_value") {
    // These are measure values which could be nullable.
    nullability = Nullability::NULLABLE;
  } else {
    nullability = Nullability::NO_NULL;
  }
  ordinalPosition = position;
}

void ColumnMeta::Read(app::ColumnBindingMap& columnBindings, int32_t position) {
  LOG_DEBUG_MSG("Read is called");
  
  auto itr = columnBindings.find(1);
  if (itr == columnBindings.end()) {
    LOG_ERROR_MSG("Could not find the first column");
    return;
  }
  columnName = itr->second.GetString(STRING_BUFFER_SIZE);

  itr = columnBindings.find(2);
  if (itr == columnBindings.end()) {
    LOG_ERROR_MSG("Could not find the second column");
    return;
  }

  dataType = static_cast< int16_t >(
      GetScalarDataType(itr->second.GetString(STRING_BUFFER_SIZE)));

  itr = columnBindings.find(3);
  if (itr == columnBindings.end()) {
    LOG_ERROR_MSG("Could not find the third column");
    return;
  }
  
  // Check if remarks field is NULL in the database
  if (itr->second.GetInputSize() == SQL_NULL_DATA) {
    SetRemarksNull();
  } else {
    SetRemarks(itr->second.GetString(STRING_BUFFER_SIZE));
  }
  
  if (columnName.value() == "int_value" || columnName.value() == "boolean_value" || 
      columnName.value() == "double_value" || columnName.value() == "string_value") {
    // These are measure values which could be nullable.
    nullability = Nullability::NULLABLE;
  } else {
    nullability = Nullability::NO_NULL;
  }
  ordinalPosition = position;
}

void ColumnMeta::ReadMetadata(const ColumnInfo& swMetadata) {
  LOG_DEBUG_MSG("ReadMetadata is called");
  using Aws::IoTSiteWise::Model::ColumnType;

  columnInfo = swMetadata;

  ColumnType columnType = swMetadata.GetType();

  // columnName and scalarType are the only 2 piece of info from Type object
  columnName = swMetadata.GetName();
  LOG_DEBUG_MSG("columnName is " << columnName);
  if (columnType.ScalarTypeHasBeenSet()) {
    dataType = static_cast< int16_t >(columnType.GetScalarType());
  } else {
    dataType = static_cast< int16_t >(
        Aws::IoTSiteWise::Model::ScalarType::STRING);
  }
}

bool ColumnMeta::GetAttribute(uint16_t fieldId, std::string& value) const {
  LOG_DEBUG_MSG("GetAttribute is called with fieldId " << fieldId);

  // an empty string is returned if the column does not have the requested field
  value = "";
  bool retval = true;
  switch (fieldId) {
    case SQL_DESC_LABEL:
    case SQL_DESC_BASE_COLUMN_NAME:
    case SQL_DESC_NAME: {
      if (columnName) {
        value = *columnName;
      }

      break;
    }

    case SQL_DESC_TABLE_NAME:
    case SQL_DESC_BASE_TABLE_NAME: {
      if (tableName) {
        value = *tableName;
      }

      break;
    }

    case SQL_DESC_SCHEMA_NAME: {
      if (schemaName) {
        value = *schemaName;
      }

      break;
    }

    case SQL_DESC_CATALOG_NAME: {
      if (catalogName) {
        value = *catalogName;
      }

      break;
    }

    case SQL_DESC_LITERAL_PREFIX:
      // value should be "'" if data type is VARCHAR,
      // and "0x" if data type is binary, but IoT SiteWise does not have
      // binary data type, so SQL_DESC_LITERAL_PREFIX would have same
      // result as SQL_DESC_LITERAL_SUFFIX
    case SQL_DESC_LITERAL_SUFFIX: {
      if (GetScalarType() == ScalarType::STRING) {
        value = "'";
      } else {
        value.clear();
      }

      break;
    }

    case SQL_DESC_TYPE_NAME:
    case SQL_DESC_LOCAL_TYPE_NAME: {
      if (boost::optional< std::string > val =
              type_traits::BinaryTypeToSqlTypeName(dataType)) {
        value = *val;
      } else {
        value.clear();
      }

      break;
    }

    case SQL_DESC_PRECISION:
    case SQL_COLUMN_LENGTH:
    case SQL_COLUMN_PRECISION: {
      if (!precision || *precision == -1) {
        retval = false;
      } else {
        value =
            iotsitewise::odbc::common::LexicalCast< std::string >(*precision);
      }

      break;
    }

    case SQL_DESC_SCALE:
    case SQL_COLUMN_SCALE: {
      if (!scale || *scale == -1) {
        retval = false;
      } else {
        value = iotsitewise::odbc::common::LexicalCast< std::string >(*scale);
      }

      break;
    }

    default:
      return false;
  }

  LOG_DEBUG_MSG("value: " << value);
  return retval;
}

bool ColumnMeta::GetAttribute(uint16_t fieldId, SqlLen& value) const {
  LOG_DEBUG_MSG("GetAttribute is called with fieldId " << fieldId);

  // value equals -1 by default.
  value = -1;
  switch (fieldId) {
    case SQL_DESC_FIXED_PREC_SCALE: {
      if ((!precision || *precision == -1)
          || (!scale || *scale == -1 || *scale == 0)) {
        value = SQL_FALSE;
      } else {
        value = SQL_TRUE;
      }

      break;
    }

    case SQL_DESC_AUTO_UNIQUE_VALUE: {
      if (isAutoIncrement == "YES") {
        value = SQL_TRUE;
      } else {
        value = SQL_FALSE;
      }

      break;
    }

    case SQL_DESC_CASE_SENSITIVE: {
      if (GetScalarType() == ScalarType::STRING) {
        value = SQL_TRUE;
      } else {
        value = SQL_FALSE;
      }

      break;
    }

    case SQL_DESC_CONCISE_TYPE:
    case SQL_DESC_TYPE: {
      if (boost::optional< int16_t > val =
              type_traits::BinaryToSqlType(dataType))
        value = *val;

      break;
    }

    case SQL_DESC_LENGTH:
      // SQL_DESC_LENGTH is either the maximum or actual character length of a
      // character string or binary data type
    case SQL_COLUMN_LENGTH:
    case SQL_DESC_DISPLAY_SIZE: {
      if (boost::optional< int > val =
              type_traits::BinaryTypeDisplaySize(dataType))
        value = *val;

      break;
    }

    case SQL_DESC_OCTET_LENGTH: {
      if (boost::optional< int > val =
              type_traits::BinaryTypeTransferLength(dataType)) {
        value = *val;
      }

      break;
    }

    case SQL_DESC_NULLABLE: {
      value = Nullability::ToSql(nullability);

      break;
    }

    case SQL_DESC_NUM_PREC_RADIX: {
      if (boost::optional< int > val =
              type_traits::BinaryTypeNumPrecRadix(dataType))
        value = *val;

      break;
    }

    case SQL_DESC_PRECISION:
    case SQL_COLUMN_PRECISION: {
      if (dataType && (!precision || *precision == -1)) {
        if (boost::optional< int > val =
                type_traits::BinaryTypeColumnSize(dataType))
          value = *val;
      } else if (precision) {
        value = *precision;
      }
      break;
    }

    case SQL_DESC_SCALE:
    case SQL_COLUMN_SCALE: {
      if (dataType && (!scale || *scale == -1)) {
        if (boost::optional< int16_t > val =
                type_traits::BinaryTypeDecimalDigits(dataType))
          value = *val;
      } else if (scale) {
        value = *scale;
      }
      break;
    }

    case SQL_DESC_SEARCHABLE: {
      value = SQL_PRED_BASIC;

      break;
    }

    case SQL_DESC_UNNAMED: {
      value = (columnName && *columnName != "") ? SQL_NAMED : SQL_UNNAMED;

      break;
    }

    case SQL_DESC_UNSIGNED: {
      value = type_traits::BinaryTypeUnsigned(dataType) ? SQL_TRUE : SQL_FALSE;

      break;
    }

    case SQL_DESC_UPDATABLE: {
      value = SQL_ATTR_READWRITE_UNKNOWN;

      break;
    }

    default:
      return false;
  }

  LOG_DEBUG_MSG("value: " << value);

  return true;
}
}  // namespace meta
}  // namespace odbc
}  // namespace iotsitewise
