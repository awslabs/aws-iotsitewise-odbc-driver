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

#ifndef _MOCK_IOTSITEWISE_CLIENT
#define _MOCK_IOTSITEWISE_CLIENT

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/iotsitewise/IoTSiteWiseClient.h>
#include <aws/iotsitewise/model/ExecuteQueryRequest.h>

namespace iotsitewise {
namespace odbc {
/**
 * Mock IoT SiteWiseQueryClient so its behavior could be controlled by us.
 * All interfaces should be kept same as IoT SiteWiseQueryClient.
 */
class MockIoTSiteWiseClient
    : public Aws::IoTSiteWise::IoTSiteWiseClient {
 public:
  /**
   * Constructor.
   */
  MockIoTSiteWiseClient(
      const Aws::Auth::AWSCredentials &credentials,
      const Aws::Client::ClientConfiguration &clientConfiguration =
          Aws::Client::ClientConfiguration())
      : Aws::IoTSiteWise::IoTSiteWiseClient(credentials,
                                                    clientConfiguration),
        credentials_(credentials),
        clientConfiguration_(clientConfiguration) {
  }

  /**
   * Destructor.
   */
  ~MockIoTSiteWiseClient() {
  }

  /**
   * Run a query.
   *
   * @param request Aws ExecuteQueryResult .
   * @return Operation outcome.
   */
  virtual Aws::IoTSiteWise::Model::ExecuteQueryOutcome ExecuteQuery(
      const Aws::IoTSiteWise::Model::ExecuteQueryRequest &request) const;

 private:
  Aws::Auth::AWSCredentials credentials_;
  Aws::Client::ClientConfiguration clientConfiguration_;
};
}  // namespace odbc
}  // namespace iotsitewise

#endif
