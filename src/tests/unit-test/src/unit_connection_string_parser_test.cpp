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

#include <string>

#include <odbc_unit_test_suite.h>
#include <iotsitewise/odbc/authentication/auth_type.h>
#include <iotsitewise/odbc/config/connection_string_parser.h>

using namespace iotsitewise::odbc;
using namespace iotsitewise::odbc::config;
using iotsitewise::odbc::OdbcUnitTestSuite;
using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct ConnectionStringParserUnitTestSuiteFixture : OdbcUnitTestSuite {
  ConnectionStringParserUnitTestSuiteFixture() : OdbcUnitTestSuite() {
  }

  /**
   * Destructor.
   */
  ~ConnectionStringParserUnitTestSuiteFixture() {
  }
};

const AuthType::Type testAuthType = AuthType::FromString("Aws_Profile");
const std::string profileName = "test-profile";

BOOST_FIXTURE_TEST_SUITE(ConnectionStringParserTestSuite,
                         ConnectionStringParserUnitTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestParsingCredentials) {
  iotsitewise::odbc::config::Configuration cfg;

  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  std::string connectionString =
            "driver={AWS IoT SiteWise ODBC Driver};"
            "auth=" + AuthType::ToString(testAuthType) + ";"
            "profileName=" + profileName + ";";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  if (diag.GetStatusRecordsNumber() != 0)
    BOOST_FAIL(diag.GetStatusRecord(1).GetMessageText());
}

BOOST_AUTO_TEST_CASE(TestParsingNoProvider) {
  iotsitewise::odbc::config::Configuration cfg;

  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  std::string connectionString =
      "driver={AWS IoT SiteWise ODBC Driver};"
      "profileName="
      + profileName + ";";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  if (diag.GetStatusRecordsNumber() != 0)
    BOOST_FAIL(diag.GetStatusRecord(1).GetMessageText());
}

BOOST_AUTO_TEST_CASE(TestParsingEmptyCredentials) {
  iotsitewise::odbc::config::Configuration cfg;

  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  std::string connectionString =
            "driver={AWS IoT SiteWise ODBC Driver};"
            "auth=" + AuthType::ToString(testAuthType) + ";"
            "profileName=" + profileName + ";";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  if (diag.GetStatusRecordsNumber() != 0)
    BOOST_FAIL(diag.GetStatusRecord(1).GetMessageText());
}

BOOST_AUTO_TEST_CASE(TestParsingRequestTimeout) {
  iotsitewise::odbc::config::Configuration cfg;

  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  std::string connectionString =
      "driver={AWS IoT SiteWise ODBC Driver};"
      "RequestTimeout=0;";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  BOOST_CHECK(diag.GetStatusRecordsNumber() == 1);
  BOOST_CHECK_EQUAL(diag.GetStatusRecord(1).GetMessageText(),
                    "Request Timeout attribute value is out of range. Using "
                    "default value. [key='RequestTimeout', value='0']");

  connectionString =
      "driver={AWS IoT SiteWise ODBC Driver};"
      "RequestTimeout=-1000;";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  BOOST_CHECK(diag.GetStatusRecordsNumber() == 2);
  BOOST_CHECK_EQUAL(
      diag.GetStatusRecord(2).GetMessageText(),
      "Request Timeout attribute value contains unexpected characters. Using "
      "default value. [key='RequestTimeout', value='-1000']");
}

BOOST_AUTO_TEST_CASE(TestParsingConnectionTimeout) {
  iotsitewise::odbc::config::Configuration cfg;

  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  std::string connectionString =
      "driver={AWS IoT SiteWise ODBC Driver};"
      "ConnectionTimeout=0;";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  BOOST_CHECK(diag.GetStatusRecordsNumber() == 1);
  BOOST_CHECK_EQUAL(diag.GetStatusRecord(1).GetMessageText(),
                    "Connection Timeout attribute value is out of range. Using "
                    "default value. [key='ConnectionTimeout', value='0']");

  connectionString =
      "driver={AWS IoT SiteWise ODBC Driver};"
      "ConnectionTimeout=-1000;";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  BOOST_CHECK(diag.GetStatusRecordsNumber() == 2);
  BOOST_CHECK_EQUAL(
      diag.GetStatusRecord(2).GetMessageText(),
      "Connection Timeout attribute value contains unexpected characters. "
      "Using default value. [key='ConnectionTimeout', value='-1000']");
}

BOOST_AUTO_TEST_CASE(TestParsingMaxRetryCountClient) {
  iotsitewise::odbc::config::Configuration cfg;

  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  std::string connectionString =
      "driver={AWS IoT SiteWise ODBC Driver};"
      "MaxRetryCountClient=-1000;";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  BOOST_CHECK(diag.GetStatusRecordsNumber() == 1);
  BOOST_CHECK_EQUAL(
      diag.GetStatusRecord(1).GetMessageText(),
      "Max Retry Count Client attribute value contains unexpected characters. "
      "Using default value. [key='MaxRetryCountClient', value='-1000']");
}

BOOST_AUTO_TEST_CASE(TestParsingMaxConnections) {
  iotsitewise::odbc::config::Configuration cfg;

  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  std::string connectionString =
      "driver={AWS IoT SiteWise ODBC Driver};"
      "MaxConnections=0;";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  BOOST_CHECK(diag.GetStatusRecordsNumber() == 1);
  BOOST_CHECK_EQUAL(diag.GetStatusRecord(1).GetMessageText(),
                    "Max Connections attribute value is out of range. Using "
                    "default value. [key='MaxConnections', value='0']");

  connectionString =
      "driver={AWS IoT SiteWise ODBC Driver};"
      "MaxConnections=-1000;";

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectionString, &diag));

  BOOST_CHECK(diag.GetStatusRecordsNumber() == 2);
  BOOST_CHECK_EQUAL(
      diag.GetStatusRecord(2).GetMessageText(),
      "Max Connections attribute value contains unexpected characters. Using "
      "default value. [key='MaxConnections', value='-1000']");
}

BOOST_AUTO_TEST_SUITE_END()
