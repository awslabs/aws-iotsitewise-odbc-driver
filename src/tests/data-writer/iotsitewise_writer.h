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

#ifndef _IGNITE_ODBC_IOTSITEWISE_WRITER
#define _IGNITE_ODBC_IOTSITEWISE_WRITER

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/iotsitewise/IoTSiteWiseClient.h>
#include <aws/iotsitewise/model/AssetModelPropertyDefinition.h>

namespace iotsitewise {
namespace odbc {

class IoTSiteWiseWriter {
 public:
  IoTSiteWiseWriter(Aws::Auth::AWSCredentials& credentials,
                   Aws::Client::ClientConfiguration& clientCfg) {
    client_ = std::make_shared< Aws::IoTSiteWise::IoTSiteWiseClient >(
        credentials, clientCfg);
  }

  ~IoTSiteWiseWriter() = default;

  bool WriteAssetModel(const Aws::String& assetModelName,
                     int propertiesNum, Aws::String* assetModelId,
                     Aws::Vector<Aws::String>& propertyIds);

  bool WriteAsset(const Aws::String& assetModelId,
                         const Aws::String& assetName, 
                        Aws::String* assetId);

  bool WriteDoubleValues(const Aws::String& assetId, 
                        const Aws::String& propertyId, 
                        const Aws::Vector<double> values);

 private:
  std::shared_ptr< Aws::IoTSiteWise::IoTSiteWiseClient > client_;
};
}  // namespace odbc
}  // namespace iotsitewise

#endif  //_IGNITE_ODBC_IOTSITEWISE_WRITER

