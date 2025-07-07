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

#include <iostream>

#include <aws/iotsitewise/model/ScalarType.h>

#include "iotsitewise_writer.h"

bool verifyParameters(std::string& accessKeyId, std::string& secretKey,
                      std::string& sessionToken, std::string& region,
                      int assetsNum, int propertiesNum,
                      int datapointsNum, std::string usage) {
  if (accessKeyId.empty() || secretKey.empty() ||  region.empty()
      || assetsNum == 0 || propertiesNum == 0 || datapointsNum == 0) {
    std::cerr << "Invalid parameters, please check ";
    std::cerr << usage << std::endl;
    return false;
  }

  return true;
}

int main(int argc, char* argv[]) {
  std::string usage("Usage: ");
  usage += argv[0];
  usage +=
      " [-u access_key_id -p secret_access_key -s session_token -r region] -a num_assets -n num_properties "
      "-d num_datapoints";

  if (argc < 7) {
    std::cerr << "Invalid parameters" << std::endl;
    std::cerr << usage << std::endl;
    return -1;
  }

  std::string accessKeyId;
  std::string secretKey;
  std::string sessionToken;
  std::string region;
  int assetsNum = 0;
  int propertiesNum = 0;
  int datapointsNums = 0;

  // read options
  for (int i = 1; i < argc; i++) {
    if ((i < argc - 1) && !strcmp(argv[i], "-u"))
      accessKeyId = argv[++i];
    else if ((i < argc - 1) && !strcmp(argv[i], "-p"))
      secretKey = argv[++i];
    else if ((i < argc - 1) && !strcmp(argv[i], "-s"))
      sessionToken = argv[++i];
    else if ((i < argc - 1) && !strcmp(argv[i], "-r"))
      region = argv[++i];
    else if ((i < argc - 1) && !strcmp(argv[i], "-a"))
      assetsNum = atoi(argv[++i]);
    else if ((i < argc - 1) && !strcmp(argv[i], "-n"))
      propertiesNum = atoi(argv[++i]);
    else if ((i < argc - 1) && !strcmp(argv[i], "-d"))
      datapointsNums = atoi(argv[++i]);
    else {
      std::cerr << "Unsupported parameters " << argv[i] << std::endl;
      std::cerr << usage << std::endl;
      return -1;
    }
  }

  // use environment variables if accessKeyId, secretKey, and sessionToken are not set
  if (accessKeyId.empty())
    accessKeyId = getenv("AWS_ACCESS_KEY_ID");
  if (secretKey.empty())
    secretKey = getenv("AWS_SECRET_ACCESS_KEY");
  if (sessionToken.empty())
    sessionToken = getenv("AWS_SESSION_TOKEN");
  if (region.empty())
    region = getenv("AWS_DEFAULT_REGION");

  // verify parameter values
  if (!verifyParameters(accessKeyId, secretKey, sessionToken, region,
                        assetsNum, propertiesNum, datapointsNums, usage)) {
    return -1;
  }

  Aws::SDKOptions options;
  options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Warn;
  Aws::InitAPI(options);

  Aws::Auth::AWSCredentials credentials;
  credentials.SetAWSAccessKeyId(accessKeyId);
  credentials.SetAWSSecretKey(secretKey);
  credentials.SetSessionToken(sessionToken);

  Aws::Client::ClientConfiguration clientCfg;

  clientCfg.region = region;
  clientCfg.enableEndpointDiscovery = true;

  iotsitewise::odbc::IoTSiteWiseWriter writer(credentials, clientCfg);

  // initialize random seed
  srand(time(NULL));

  bool result = false;
  Aws::String assetModelId;
  Aws::Vector<Aws::String> propertyIds;
  Aws::Vector<Aws::String> assetIds;

  Aws::String assetModelName = "AWS IoT SiteWise Query Language ODBC Driver Integration Test";

  std::cout << "Writing Asset Model to IoT SiteWise ..." << std::endl;
  result = writer.WriteAssetModel(assetModelName, propertiesNum, &assetModelId, propertyIds);
  if (result) 
    std::cout << "Wrote Asset Model to IoT SiteWise successfully" << std::endl;
  else {
    std::cerr << "Failed to write Asset Model to IoT SiteWise" << std::endl;
    goto Shutdown;
  }

  std::cout << "Writing Assets to IoT SiteWise ..." << std::endl;
  for (int i = 0; i < assetsNum; i++) {
    Aws::String assetName = assetModelName + " Asset " + std::to_string(i);
    Aws::String assetId;
    result = writer.WriteAsset(assetModelId, assetName, &assetId);
    if (!result) {
      std::cerr << "Failed to write Asset to IoT SiteWise" << std::endl;
      goto Shutdown;
    } 
    assetIds.push_back(assetId);
  }

  std::cout << "Wrote " << assetsNum << " Assets to IoT SiteWise successfully" << std::endl;

  std::cout << "Writing Property Datapoints to IoT SiteWise ..." << std::endl;
  for (int i = 0; i < assetIds.size(); i++) {
    Aws::String assetId = assetIds.at(i);
    for (int j = 0; j < propertyIds.size(); j++) {
      Aws::Vector<double> datapoints;
      for (int k = 0; k < datapointsNums; k++) {
        datapoints.push_back((rand()%1000000)/1000);
      }
      result = writer.WriteDoubleValues(assetId, propertyIds.at(j), datapoints);
        if (!result) {
          std::cerr << "Failed to write Asset Property Value to IoT SiteWise" << std::endl;
          goto Shutdown;
        } 
    }
  }

  std::cout << "Wrote " << datapointsNums * propertiesNum * assetsNum 
    << " datapoints to IoT SiteWise successfully " << std::endl;

Shutdown:
  Aws::ShutdownAPI(options);
  return result ? 0 : -1;
}

