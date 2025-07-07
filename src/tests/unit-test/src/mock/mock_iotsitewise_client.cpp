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
#include <aws/core/utils/Outcome.h>
#include <aws/iotsitewise/model/ExecuteQueryResult.h>
#include <aws/iotsitewise/model/Row.h>
#include <aws/iotsitewise/model/Datum.h>
#include <aws/iotsitewise/IoTSiteWiseErrors.h>

#include <mock/mock_iotsitewise_client.h>
#include <mock/mock_iotsitewise_service.h>

namespace iotsitewise {
namespace odbc {
// All the working logic is done by the singleton MockIoTSiteWiseService object.
Aws::IoTSiteWise::Model::ExecuteQueryOutcome MockIoTSiteWiseClient::ExecuteQuery(
    const Aws::IoTSiteWise::Model::ExecuteQueryRequest &request) const {
  // authenticate first
  if (!MockIoTSiteWiseService::GetInstance()->Authenticate(
          credentials_.GetAWSAccessKeyId(), credentials_.GetAWSSecretKey())) {
    Aws::IoTSiteWise::IoTSiteWiseError error(
        Aws::Client::AWSError< Aws::Client::CoreErrors >(
            Aws::Client::CoreErrors::INVALID_ACCESS_KEY_ID, false));

    return Aws::IoTSiteWise::Model::ExecuteQueryOutcome(error);
  }

  return MockIoTSiteWiseService::GetInstance()->HandleQueryReq(request);
}

}  // namespace odbc
}  // namespace iotsitewise
