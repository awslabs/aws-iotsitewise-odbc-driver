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

#include <mock/mock_statement.h>

namespace iotsitewise {
namespace odbc {
MockStatement::MockStatement(Connection& parent) : Statement(parent) {
  // No-op.
}

MockStatement::~MockStatement() {
  // No-op.
}
}  // namespace odbc
}  // namespace iotsitewise
