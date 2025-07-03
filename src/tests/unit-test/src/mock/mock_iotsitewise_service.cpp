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

#include <aws/core/Aws.h>
#include <aws/iotsitewise/model/ExecuteQueryResult.h>
#include <aws/iotsitewise/IoTSiteWiseErrors.h>
#include <aws/core/utils/Outcome.h>
#include <aws/iotsitewise/model/Row.h>
#include <aws/iotsitewise/model/Datum.h>
#include <aws/iotsitewise/model/ColumnInfo.h>

#include <mock/mock_iotsitewise_service.h>

namespace iotsitewise {
namespace odbc {

std::mutex MockIoTSiteWiseService::mutex_;
MockIoTSiteWiseService* MockIoTSiteWiseService::instance_ = nullptr;
int MockIoTSiteWiseService::token = 0;
int MockIoTSiteWiseService::errorToken = 0;

void MockIoTSiteWiseService::CreateMockIoTSiteWiseService() {
  if (!instance_) {
    std::lock_guard< std::mutex > lock(mutex_);
    if (!instance_) {
      instance_ = new MockIoTSiteWiseService;
    }
  }
}

void MockIoTSiteWiseService::DestoryMockIoTSiteWiseService() {
  if (instance_) {
    std::lock_guard< std::mutex > lock(mutex_);
    if (instance_) {
      delete instance_;
      instance_ = nullptr;
    }
  }
}

MockIoTSiteWiseService ::~MockIoTSiteWiseService() {
  // No-op
}

void MockIoTSiteWiseService::AddCredential(const Aws::String& keyId,
                                          const Aws::String& secretKey) {
  credMap_[keyId] = secretKey;
}

void MockIoTSiteWiseService::RemoveCredential(const Aws::String& keyId) {
  credMap_.erase(keyId);
}

bool MockIoTSiteWiseService::Authenticate(const Aws::String& keyId,
                                         const Aws::String& secretKey) {
  auto itr = credMap_.find(keyId);
  if (itr == credMap_.end() || itr->second != secretKey) {
    return false;
  }
  return true;
}

// Setup ExecuteQueryResult for mockTables
void MockIoTSiteWiseService::SetupResultForMockTable(
    Aws::IoTSiteWise::Model::ExecuteQueryResult& result) {
  Aws::IoTSiteWise::Model::ColumnInfo firstColumn;
  firstColumn.SetName("measure");
  Aws::IoTSiteWise::Model::ColumnType stringType;
  stringType.SetScalarType(Aws::IoTSiteWise::Model::ScalarType::STRING);
  firstColumn.SetType(stringType);

  Aws::IoTSiteWise::Model::ColumnInfo secondColumn;
  secondColumn.SetName("time");
  Aws::IoTSiteWise::Model::ColumnType timeType;
  timeType.SetScalarType(Aws::IoTSiteWise::Model::ScalarType::TIMESTAMP);
  secondColumn.SetType(timeType);
  result.AddColumns(firstColumn);
  result.AddColumns(secondColumn);

  Aws::IoTSiteWise::Model::Datum measure;
  measure.SetScalarValue("cpu_usage");
  Aws::IoTSiteWise::Model::Datum time1;
  time1.SetScalarValue("2022-11-09 23:52:51.554000000");
  Aws::IoTSiteWise::Model::Datum time2;
  time2.SetScalarValue("2022-11-10 23:53:51.554000000");
  Aws::IoTSiteWise::Model::Datum time3;
  time3.SetScalarValue("2022-11-11 23:54:51.554000000");

  Aws::IoTSiteWise::Model::Row row1;
  row1.AddData(measure);
  row1.AddData(time1);

  Aws::IoTSiteWise::Model::Row row2;
  row2.AddData(measure);
  row2.AddData(time2);

  Aws::IoTSiteWise::Model::Row row3;
  row3.AddData(measure);
  row3.AddData(time3);

  result.AddRows(row1);
  result.AddRows(row2);
  result.AddRows(row3);
}

// This function simulates AWS IoT SiteWise service. It provides
// simple result without the need of parsing the query. Update
// this function if new query needs to be handled.
Aws::IoTSiteWise::Model::ExecuteQueryOutcome MockIoTSiteWiseService::HandleQueryReq(
    const Aws::IoTSiteWise::Model::ExecuteQueryRequest& request) {
  if (request.GetQueryStatement() == "SELECT table_name FROM system.tables") {
    // set up ExecuteQueryResult
    Aws::IoTSiteWise::Model::ExecuteQueryResult result;
    Aws::IoTSiteWise::Model::Datum datum;
    datum.SetScalarValue("1");

    Aws::IoTSiteWise::Model::Row row;
    row.AddData(datum);

    result.AddRows(row);
    return Aws::IoTSiteWise::Model::ExecuteQueryOutcome(result);
  } else if (request.GetQueryStatement()
             == "select measure, time from mockDB.mockTable") {
    Aws::IoTSiteWise::Model::ExecuteQueryResult result;
    SetupResultForMockTable(result);
    return Aws::IoTSiteWise::Model::ExecuteQueryOutcome(result);
  } else if (request.GetQueryStatement()
             == "select measure, time from mockDB.mockTable10000") {
    Aws::IoTSiteWise::Model::ExecuteQueryResult result;
    SetupResultForMockTable(result);

    // for pagination test
    result.SetNextToken(std::to_string(++token));
    return Aws::IoTSiteWise::Model::ExecuteQueryOutcome(result);
  } else if (request.GetQueryStatement()
             == "select measure, time from mockDB.mockTable10Error") {
    Aws::IoTSiteWise::Model::ExecuteQueryResult result;
    SetupResultForMockTable(result);

    // for pagination test
    if (errorToken < 3) {
      result.SetNextToken(std::to_string(++errorToken));
      return Aws::IoTSiteWise::Model::ExecuteQueryOutcome(result);
    } else {
      Aws::IoTSiteWise::IoTSiteWiseError error(
          Aws::Client::AWSError< Aws::Client::CoreErrors >(
              Aws::Client::CoreErrors::UNKNOWN, false));

      return Aws::IoTSiteWise::Model::ExecuteQueryOutcome(error);
    }
  } else {
    Aws::IoTSiteWise::IoTSiteWiseError error(
        Aws::Client::AWSError< Aws::Client::CoreErrors >(
            Aws::Client::CoreErrors::UNKNOWN, false));

    return Aws::IoTSiteWise::Model::ExecuteQueryOutcome(error);
  }
}
}  // namespace odbc
}  // namespace iotsitewise
