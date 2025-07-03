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

#include "iotsitewise_writer.h"

#include <aws/iotsitewise/model/AssetModelProperty.h>
#include <aws/iotsitewise/model/AssetModelPropertyDefinition.h>
#include <aws/iotsitewise/model/AssetPropertyValue.h>
#include <aws/iotsitewise/model/BatchPutAssetPropertyError.h>
#include <aws/iotsitewise/model/BatchPutAssetPropertyErrorEntry.h>
#include <aws/iotsitewise/model/BatchPutAssetPropertyValueRequest.h>
#include <aws/iotsitewise/model/CreateAssetModelRequest.h>
#include <aws/iotsitewise/model/CreateAssetRequest.h>
#include <aws/iotsitewise/model/DescribeAssetModelRequest.h>
#include <aws/iotsitewise/model/DescribeAssetRequest.h>
#include <aws/iotsitewise/model/Measurement.h>
#include <aws/iotsitewise/model/PropertyDataType.h>
#include <aws/iotsitewise/model/PropertyType.h>
#include <aws/iotsitewise/model/PutAssetPropertyValueEntry.h>
#include <aws/iotsitewise/model/TimeInNanos.h>
#include <aws/iotsitewise/model/Variant.h>

#include <time.h>

#if defined(_WIN32)
#include <winsock.h>
#else
#include <sys/time.h>
#endif
#include <chrono>
#include <iostream>
#include <exception>
#include <thread>
#include <typeinfo>
#include <stdexcept>

using Aws::IoTSiteWise::Model::AssetModelProperty;
using Aws::IoTSiteWise::Model::AssetModelPropertyDefinition;
using Aws::IoTSiteWise::Model::AssetPropertyValue;
using Aws::IoTSiteWise::Model::Attribute;
using Aws::IoTSiteWise::Model::BatchPutAssetPropertyError;
using Aws::IoTSiteWise::Model::BatchPutAssetPropertyErrorEntry;
using Aws::IoTSiteWise::Model::BatchPutAssetPropertyValueRequest;
using Aws::IoTSiteWise::Model::CreateAssetModelRequest;
using Aws::IoTSiteWise::Model::CreateAssetRequest;
using Aws::IoTSiteWise::Model::DescribeAssetModelRequest;
using Aws::IoTSiteWise::Model::DescribeAssetRequest;
using Aws::IoTSiteWise::Model::Measurement;
using Aws::IoTSiteWise::Model::PropertyDataType;
using Aws::IoTSiteWise::Model::PropertyType;
using Aws::IoTSiteWise::Model::PutAssetPropertyValueEntry;
using Aws::IoTSiteWise::Model::TimeInNanos;
using Aws::IoTSiteWise::Model::Variant;

// needed to avoid misreplace
#ifdef GetMessage
#undef GetMessage
#endif

namespace iotsitewise {
namespace odbc {

#if defined(_WIN32)
int gettimeofday(struct timeval* tp, struct timezone* tzp) {
  namespace sc = std::chrono;
  sc::system_clock::duration d = sc::system_clock::now().time_since_epoch();
  sc::seconds s = sc::duration_cast< sc::seconds >(d);
  tp->tv_sec = s.count();
  tp->tv_usec = sc::duration_cast< sc::microseconds >(d - s).count();

  return 0;
}
#endif  // _WIN32


const int MAX_ATTEMPTS = 5;
const long long BACKOFF_SECONDS = 5;

bool IoTSiteWiseWriter::WriteAssetModel(const Aws::String& assetModelName,
                     int propertiesNum, Aws::String* assetModelId,
                     Aws::Vector<Aws::String>& propertyIds) {

  Measurement measurement;

  PropertyType propertyType;
  propertyType.SetMeasurement(measurement);

  Aws::Vector<AssetModelPropertyDefinition> assetModelPropertyDefinitions;

  for (int i = 0; i < propertiesNum; i++) {
    AssetModelPropertyDefinition assetModelPropertyDefinition;
    assetModelPropertyDefinition.SetName("Property " + std::to_string(i));
    assetModelPropertyDefinition.SetDataType(PropertyDataType::DOUBLE);
    assetModelPropertyDefinition.SetType(propertyType);
    assetModelPropertyDefinitions.push_back(assetModelPropertyDefinition);
  }

  Attribute integerAttribute;
  integerAttribute.SetDefaultValue("1");

  PropertyType integerAttributePropertyType;
  integerAttributePropertyType.SetAttribute(integerAttribute);

  AssetModelPropertyDefinition integerAssetModelPropertyDefinition;
  integerAssetModelPropertyDefinition.SetName("Static Integer");
  integerAssetModelPropertyDefinition.SetDataType(PropertyDataType::INTEGER);
  integerAssetModelPropertyDefinition.SetType(integerAttributePropertyType);
  assetModelPropertyDefinitions.push_back(integerAssetModelPropertyDefinition);

  Attribute booleanAttribute;
  booleanAttribute.SetDefaultValue("true");

  PropertyType booleanAttributePropertyType;
  booleanAttributePropertyType.SetAttribute(booleanAttribute);

  AssetModelPropertyDefinition booleanAssetModelPropertyDefinition;
  booleanAssetModelPropertyDefinition.SetName("Static Boolean");
  booleanAssetModelPropertyDefinition.SetDataType(PropertyDataType::BOOLEAN);
  booleanAssetModelPropertyDefinition.SetType(booleanAttributePropertyType);
  assetModelPropertyDefinitions.push_back(booleanAssetModelPropertyDefinition);

  CreateAssetModelRequest createAssetModelRequest;
  createAssetModelRequest.SetAssetModelName(assetModelName);
  createAssetModelRequest.SetAssetModelProperties(assetModelPropertyDefinitions);

  try {
    Aws::IoTSiteWise::Model::CreateAssetModelOutcome createAssetModelOutcome 
      = client_->CreateAssetModel(createAssetModelRequest);
    if (!createAssetModelOutcome.IsSuccess()) {
      std::cout << "Error msg is " << createAssetModelOutcome.GetError().GetMessage() << std::endl;
      return false;
    }
    *assetModelId = createAssetModelOutcome.GetResult().GetAssetModelId();

    DescribeAssetModelRequest describeAssetModelRequest;
    describeAssetModelRequest.SetAssetModelId(*assetModelId);

    int attempts = 0;
    Aws::IoTSiteWise::Model::DescribeAssetModelOutcome describeAssetModelOutcome;

    do {
      std::this_thread::sleep_for(std::chrono::seconds(BACKOFF_SECONDS));
      describeAssetModelOutcome = client_->DescribeAssetModel(describeAssetModelRequest);
      attempts++;
    } while (attempts < MAX_ATTEMPTS && describeAssetModelOutcome.IsSuccess() 
      && describeAssetModelOutcome.GetResult().GetAssetModelStatus().GetState() 
        != Aws::IoTSiteWise::Model::AssetModelState::ACTIVE);

    if (!describeAssetModelOutcome.IsSuccess()) {
      std::cout << "Error msg is " << describeAssetModelOutcome.GetError().GetMessage() << std::endl;
      return false;
    }

    if (describeAssetModelOutcome.GetResult().GetAssetModelStatus().GetState() 
        != Aws::IoTSiteWise::Model::AssetModelState::ACTIVE) {
      std::cout << "Timeout waiting for Asset Model Activation" << std::endl;
      return false; 
    }

    Aws::Vector<AssetModelProperty> properties 
      = describeAssetModelOutcome.GetResult().GetAssetModelProperties();

    for(int i = 0; i < properties.size(); i++) {
      if (properties.at(i).GetDataType() == PropertyDataType::DOUBLE) {
        Aws::String propertyId = properties.at(i).GetId();
        propertyIds.push_back(propertyId);
      }
    }
  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    return false;
  }
  return true;
}

bool IoTSiteWiseWriter::WriteAsset(const Aws::String& assetModelId,
                const Aws::String& assetName, 
                Aws::String* assetId) {
  CreateAssetRequest createAssetRequest;
  createAssetRequest.SetAssetModelId(assetModelId);
  createAssetRequest.SetAssetName(assetName);

  try {
    Aws::IoTSiteWise::Model::CreateAssetOutcome outcome = client_->CreateAsset(createAssetRequest);
    if (!outcome.IsSuccess()) {
      std::cout << "Error msg is " << outcome.GetError().GetMessage() << std::endl;
      return false;
    }
    *assetId = outcome.GetResult().GetAssetId();

    DescribeAssetRequest describeAssetRequest;
    describeAssetRequest.SetAssetId(*assetId);

    int attempts = 0;
    Aws::IoTSiteWise::Model::DescribeAssetOutcome describeAssetOutcome;

    do {
      std::this_thread::sleep_for(std::chrono::seconds(BACKOFF_SECONDS));
      describeAssetOutcome = client_->DescribeAsset(describeAssetRequest);
      attempts++;
    } while (attempts < MAX_ATTEMPTS && describeAssetOutcome.IsSuccess() 
      && describeAssetOutcome.GetResult().GetAssetStatus().GetState() 
        != Aws::IoTSiteWise::Model::AssetState::ACTIVE);

    if (!describeAssetOutcome.IsSuccess()) {
      std::cout << "Error msg is " << describeAssetOutcome.GetError().GetMessage() << std::endl;
      return false;
    }
  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    return false;
  }
  return true;
};

bool IoTSiteWiseWriter::WriteDoubleValues(const Aws::String& assetId, 
                        const Aws::String& propertyId, 
                        const Aws::Vector<double> values) {
  struct timeval tp;
  gettimeofday(&tp, NULL);

  for (int i = 0; i < values.size()/10 + (values.size()%10 ? 1 : 0); i++) {
    Aws::Vector<AssetPropertyValue> assetPropertyValues;

    for (int j = 0; j < values.size(); j++) {
      TimeInNanos timeInNanos;
      timeInNanos.SetTimeInSeconds(tp.tv_sec + j);
      timeInNanos.SetOffsetInNanos(tp.tv_usec * 1000);

      Variant variant;
      variant.SetDoubleValue(values.at(j));

      AssetPropertyValue assetPropertyValue;
      assetPropertyValue.SetTimestamp(timeInNanos);
      assetPropertyValue.SetValue(variant);
      assetPropertyValue.SetQuality(Aws::IoTSiteWise::Model::Quality::GOOD);

      assetPropertyValues.push_back(assetPropertyValue);
    }

    PutAssetPropertyValueEntry putAssetPropertyValueEntry;
    putAssetPropertyValueEntry.SetAssetId(assetId);
    putAssetPropertyValueEntry.SetEntryId("1");
    putAssetPropertyValueEntry.SetPropertyId(propertyId);
    putAssetPropertyValueEntry.SetPropertyValues(assetPropertyValues);

    Aws::Vector<PutAssetPropertyValueEntry> putAssetPropertyValueEntries;
    putAssetPropertyValueEntries.push_back(putAssetPropertyValueEntry);

    BatchPutAssetPropertyValueRequest batchPutAssetPropertyValueRequest;

    batchPutAssetPropertyValueRequest.SetEntries(putAssetPropertyValueEntries);

    Aws::IoTSiteWise::Model::BatchPutAssetPropertyValueOutcome outcome = 
      client_->BatchPutAssetPropertyValue(batchPutAssetPropertyValueRequest);

    if (!outcome.IsSuccess()) {
      std::cout << "Error msg is " << outcome.GetError().GetMessage() << std::endl;
      return false;
    } else if (outcome.GetResult().GetErrorEntries().size()) {
      Aws::Vector<BatchPutAssetPropertyErrorEntry> entries = outcome.GetResult().GetErrorEntries();
      for (int i = 0; i < entries.size(); i++) {
          Aws::Vector<BatchPutAssetPropertyError> errors = entries.at(i).GetErrors();
          for (int j = 0; j < errors.size(); j++) {
            std::cout << "Error Entry: " << errors.at(j).GetErrorMessage() << std::endl;
          }
      }
      return false;
    }
  }

  return true;
}

}  // namespace odbc
}  // namespace iotsitewise

