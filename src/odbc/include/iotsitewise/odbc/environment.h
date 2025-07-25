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

#ifndef _IOTSITEWISE_ODBC_ENVIRONMENT
#define _IOTSITEWISE_ODBC_ENVIRONMENT

#include <set>

#include "iotsitewise/odbc/diagnostic/diagnosable_adapter.h"

namespace iotsitewise {
namespace odbc {
class Connection;

/**
 * ODBC environment.
 */
class IGNITE_IMPORT_EXPORT Environment : public diagnostic::DiagnosableAdapter {
 public:
  /** Connection set type. */
  typedef std::set< Connection* > ConnectionSet;

  /**
   * Constructor.
   */
  Environment();

  /**
   * Destructor.
   */
  ~Environment();

  /**
   * Create connection associated with the environment.
   *
   * @return Pointer to valid instance on success or NULL on failure.
   */
  Connection* CreateConnection();

  /**
   * Deregister connection.
   *
   * @param conn Connection to deregister.
   */
  void DeregisterConnection(Connection* conn);

  /**
   * Set attribute.
   *
   * @param attr Attribute to set.
   * @param value Value.
   * @param len Value length if the attribute is of string type.
   */
  void SetAttribute(int32_t attr, void* value, int32_t len);

  /**
   * Get attribute.
   *
   * @param attr Attribute to set.
   * @param buffer Buffer to put value to.
   */
  void GetAttribute(int32_t attr, app::ApplicationDataBuffer& buffer);

 protected:
  /**
   * Create connection associated with the environment.
   * Internal call.
   *
   * @return Pointer to valid instance on success or NULL on failure.
   * @return Operation result.
   */
  virtual SqlResult::Type InternalCreateConnection(Connection*& connection);

  /** Assotiated connections. */
  ConnectionSet connections;

 private:
  IGNITE_NO_COPY_ASSIGNMENT(Environment);

  /**
   * Set attribute.
   * Internal call.
   *
   * @param attr Attribute to set.
   * @param value Value.
   * @param len Value length if the attribute is of string type.
   * @return Operation result.
   */
  SqlResult::Type InternalSetAttribute(int32_t attr, void* value, int32_t len);

  /**
   * Get attribute.
   * Internal call.
   *
   * @param attr Attribute to set.
   * @param buffer Buffer to put value to.
   * @return Operation result.
   */
  SqlResult::Type InternalGetAttribute(int32_t attr,
                                       app::ApplicationDataBuffer& buffer);

  /** ODBC version. */
  int32_t odbcVersion;

  /** ODBC null-termintaion of string behaviour. */
  int32_t odbcNts;
};
}  // namespace odbc
}  // namespace iotsitewise

#endif  //_IOTSITEWISE_ODBC_ENVIRONMENT
