# Development Environment

For documentation regarding Power BI connector, please refer
to [build Power BI connector](./powerbi-connector-setup/build_powerbi_connector.md).
Note that we only officially support Windows platform build at this time.

## Pre-requisites

### C/C++ Formatting

- This project uses [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.html) as a basis for
  C/C++ usage and formatting.
- Some formatting is set using the .clang-format file at the base of repository. Other options for Visual Studio can be
  imported from the
  `VS-C++-Settings-Export.vssettings` file also found at root of repository.

### Environment Variables for Testing Accounts/Secrets

- To use IAM accessKeyId and secretKey to access AWS IoT SiteWise, you need to specify the following environment
  variables.

  | Enviornment Variable | Description |
     |----------------------|-------------|
  | AWS_ACCESS_KEY_ID | `<AWS Access Key ID>` |
  | AWS_SECRET_ACCESS_KEY | `<AWS Secret Access Key>` |
  | AWS_SESSION_TOKEN | `<Session Token>`, if your account is enabled with MFA

- [ Optional ] Log configurations

  Set these 2 variables only if you would like to set a custom log path or log level for connection tests; it is
  completely optional.
    1. `IOTSITEWISE_LOG_PATH`=`<path_to_log_file>`(e.g.:`"C:\\Users\\BitQuillUser\\Desktop\\IoT SiteWise ODBC Driver"`)

  The user needs to ensure that the directory mentioned in the log file path exists or the driver will ignore the user's
  passed-in value and create the log file in the default log path. Do **not** include a slash at the end of the log
  path.

  The log path indicates the path to store the log file. The log file name is formatted as
  `iotsitewise_odbc_YYYYMMDD.log`, where `YYYYMMDD` (e.g., 20220225 <= Feb 25th, 2022) is the date at the first log
  message.

    2. `IOTSITEWISE_LOG_LEVEL`=`<log_level>`. The default is `2` (means WARNING Level). Possible values:
        - 0: OFF
        - 1: ERROR
        - 2: WARNING
        - 3: INFO
        - 4: DEBUG

  More details about logging in [`support\troubleshooting-guide.md`](../support/troubleshooting-guide.md).

- AWS Logs

  This ODBC driver uses AWS logs beside its own logging. Please see how AWS Logs work in
  their [official document](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/logging.html). The logs will be
  stored inthe executable directory following the default naming pattern of `aws_sdk_<date>.log`.

### Test Data

Test data are needed for tests to run successfully and they only need to be loaded once for each AWS account.
Check [here](#data-population-for-testing) for instructions on loading the test data.

## Windows

1. Microsoft Visual Studio (Community 2019 Verified)
    1. Desktop Development for C++
    2. Visual Studio core editor
    3. C++ ATL for latest v142 build tools (x86 & x64)
    4. C++ MFC for latest v142 build tools (x86 & x64)
    5. [WiX Toolset v3 Schemas for Visual Studio](https://wixtoolset.org/docs/wix3/)
       If encountering "wix toolset requires .net 3.5.1 to be enabled", follow the steps below.
        1. Open Settings -> Apps -> Optional Features -> Under "Related Settings", click on "More Windows features", and
           select ".Net Framework 3.5".
           ![Alt text](../images/dotNet_screenshot.PNG "Example")
    6. [WiX Toolset Visual Studio 2019 Extension](https://marketplace.visualstudio.com/items?itemName=WixToolset.WiXToolset)
2. [WiX Installer (3.11)](https://wixtoolset.org/releases/)
    1. Ensure to add path to WiX executables (e.g. `C:\Program Files (x86)\WiX Toolset v3.11\bin`)
3. Boost Test Framework
    1. Install [VCPKG](https://vcpkg.io/en/getting-started.html)
    2. `cd vcpkg`
    3. Install Boost and AWS SDK

       `.\vcpkg install boost-test:x64-windows boost-asio:x64-windows boost-chrono:x64-windows boost-interprocess:x64-windows boost-regex:x64-windows boost-system:x64-windows boost-thread:x64-windows --recurse`
5. Run `.\vcpkg integrate install` to implicitly add Include Directories, Link Directories, and Link Libraries for all
   packages installed with Vcpkg to all VS2015, VS2017 and VS2019 MSBuild projects
6. On the Developer PowerShell, run one of the build scripts to create an initial compilation.
    1. E.g.: `.\build_win_debug64.ps1`
    2. Navigate to the `build\odbc\cmake` folder to use the generated solution file, `IoTSiteWise-ODBC.sln` to work on
       source code development and testing.
8. Open a **64-bit** command shell or **64-bit** PowerShell window, **as Administrator**, run the command below
   ```
   .\<repo-folder>\src\odbc\install\install_amd64.cmd <repo-folder>\build\odbc\cmake\Debug\iotsitewise.odbc.dll
   ```
   Ensure that backslashes are used in your command.
9. Now you're ready to begin [configuration for integration and unit testing](#integration-tests).
10. Once configured, run the tests:
    - Run integration tests: `.\build\odbc\bin\<Release or Debug>\iotsitewise-odbc-integration-tests.exe`.
    - Run unit tests: `.\build\odbc\bin\<Release or Debug>\iotsitewise-odbc-unit-tests.exe`.

### Known issues

Driver installer detects existing driver and isn't allowing installation

1. Go to `Apps & Features` settings page and search for `IoT SiteWise ODBC Driver`. If there is already a IoT SiteWise
   ODBC driver installed, manually uninstall the driver (optional to uninstall C++ Redistributable, for details
   see [Windows installation guide](windows-installation-guide#microsoft-visual-c-redistributable)), and then attempt
   installing again.
2. If the driver is not showing under `Apps & Features`, remove driver registry
   `Computer\HKEY_LOCAL_MACHINE\SOFTWARE\ODBC\ODBCINST.INI\Amazon IoT SiteWise ODBC Driver`
   using [Registry Editor](https://support.microsoft.com/en-us/windows/how-to-open-registry-editor-in-windows-10-deab38e6-91d6-e0aa-4b7c-8878d9e07b11),
   and then attempt installing again.

- If you wish to re-register the driver, follow step 6 under [Windows developer setup guide](#windows)

## MacOS

1. Install dependencies
    1. `brew install cmake`
    2. `brew install libiodbc`
        - You may need to unlink `unixodbc` if you already have this installed. Use `brew unlink unixodbc`.
        - You may need to run `brew link --overwrite --force libiodbc`.
    3. `brew install boost`
    4. If creating a debug build (`./build_mac_debug64.sh`), LLVM is required.
        - If you only have XCode Command Line Tools, use the LLVM included with XCode by modifying the PATH with
          `export PATH=/Library/Developer/CommandLineTools/usr/bin/:$PATH`. Ensure this XCode path comes first in $PATH.
          If error occurs, check that clang and llvm are under folder Library/Developer/CommandLineTools/usr/bin.
        - If you have XCode application, to ensure LLVM and CMake are compatible, use the LLVM included with XCode by
          modifying the PATH with
          `export PATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/:$PATH`.
2. Run one of the build scripts to create an initial compilation.
    1. E.g.: from the root of the IoT SiteWise ODBC repository, run `./build_mac_release64.sh`
    2. The generated driver files will be placed in the `build/odbc/lib` folder.
3. Set the environment variable `ODBCINSTINI`. On a developer's machine, set it to
   `<repo-folder>/build/odbc/lib/iotsitewise-odbc-install.ini`.
4. Set the environment variable `DYLD_LIBRARY_PATH`. On a developer's machine, set it to
   `<repo-folder>/build/odbc/lib:$DYLD_LIBRARY_PATH`.
5. Run the following command to register the ODBC driver.
   `./scripts/register_driver_unix.sh`.
6. Now you're ready to begin [configuration for integration and unit testing](#integration-tests).
7. Once configured, run the tests:
    - Run integration tests: `./build/odbc/bin/iotsitewise-odbc-integration-tests --catch_system_errors=false`.
    - Run unit tests: `./build/odbc/bin/iotsitewise-odbc-unit-tests  --catch_system_errors=false`.

You should have all the following environment variables set. If you encounter any issues, check that these variables
have all been set correctly:

- `AWS_ACCESS_KEY_ID` (from prerequisites)
- `AWS_SECRET_ACCESS_KEY` (from prerequisites)
- `ODBCINSTINI`
- `DYLD_LIBRARY`

## Linux

### Using Ubuntu 64bit

1. Install all dependencies
    1. Ubuntu dev dependencies
       E.g.

```
           apt-get -y update \
           && apt-get -y install wget \
                                 curl \
                                 libcurl4-openssl-dev \
                                 libssl-dev \
                                 uuid-dev \
                                 zlib1g-dev \
                                 libpulse-dev \
                                 gcc \
                                 gcc-multilib  \
                                 g++ \
                                 g++-multilib \
                                 build-essential \
                                 valgrind \
                                 libboost-all-dev \
                                 libsasl2-dev \
                                 lcov \
                                 git \
                                 unixodbc-dev \
                                 valgrind \
                                 zip \
                                 unzip \
                                 tar \
                                 rpm                         
```

2. Run one of the build scripts to create an initial compilation. E.g. `./build_linux_release64_deb.sh`
3. Set all necessary environment variables and run the following command to register the ODBC driver.

   `./scripts/register_driver_unix.sh`
4. Set environment variables for testing and double-check if all dev environmnet variables are set running
   `scripts/env_variables_check.sh`.
5. Now you're ready to begin [configuration for integration and unit testing](#integration-tests).
6. Once configured, run the tests under the repository root folder:
    - Run integration tests: `./build/odbc/bin/iotsitewise-odbc-integration-tests --catch_system_errors=false`.
    - Run unit tests: `./build/odbc/bin/iotsitewise-odbc-unit-tests --catch_system_errors=false`.

### Using openSUSE 64bit

1. Install all dependencies
    1. openSUSE dev dependencies
       E.g.

```
           zypper refresh \
           && zypper install wget \
                                 curl \
                                 gcc \
                                 gcc-c++ \
                                 valgrind \
                                 lcov \
                                 git \
                                 valgrind \
                                 zip \
                                 unzip \
                                 tar \
                                 rpm    \
                                 libopenssl-3-devel \
                                 openssl \
                                 cmake \
                                 libcurl-devel \
                                 unixODBC \
                                 unixODBC-devel \
                                 rpmbuild \
                                 libboost_regex-devel \
                                 libboost_system-devel \
                                 libboost_thread-devel \
                                 libboost_chrono-devel \
                                 libboost_test-devel \
                                 boost-devel 
        
```

2. Run one of the build scripts to create an initial compilation. E.g. `./build_linux_release64_deb.sh`
3. Set all necessary environment variables and run the following command to register the ODBC driver.

   `sudo ./scripts/register_driver_unix.sh`
4. Set environment variables for testing and double-check if all dev environmnet variables are set running
   `scripts/env_variables_check.sh`.
5. Now you're ready to begin [configuration for integration and unit testing](#integration-tests).
6. Once configured, run the tests under repository root folder:
    - Run integration tests: `./build/odbc/bin/iotsitewise-odbc-integration-tests --catch_system_errors=false`.
    - Run unit tests: `./build/odbc/bin/iotsitewise-odbc-unit-tests --catch_system_errors=false`.

### Using Ubuntu 32bit

1. Install all dependencies
    1. Ubuntu dev dependencies
       E.g.

```
           apt-get -y update \
           && apt-get -y install wget \
                                 curl \
                                 libcurl4-openssl-dev \
                                 libssl-dev \
                                 uuid-dev \
                                 zlib1g-dev \
                                 libpulse-dev \
                                 gcc \
                                 gcc-multilib  \
                                 g++ \
                                 g++-multilib \
                                 build-essential \
                                 valgrind \
                                 libboost-all-dev \
                                 libsasl2-dev \
                                 lcov \
                                 git \
                                 unixodbc-dev \
                                 valgrind \
                                 zip \
                                 unzip \
                                 tar \
                                 rpm                           
```

2. Install cmake
   `apt-get install cmake`

3. The version of cmake installed is lower than 3.20 which is the minimal required version. Follow below steps to build
   cmake 3.20 (or above) from source.

    1. Download cmake 3.20 or above from https://github.com/Kitware/CMake/releases/

    2. Under cmake source directory create a build directory

       `mkdir build`

    3. Run `cmake` under source directory
    4. `cd build` and run `make`
    5. Install the new cmake

       `sudo make install`

    6. Add `/usr/local/bin` to PATH and make sure it is ahead of lower version cmake path
       `export PATH=/usr/local/bin:$PATH`
4. Run one of the build scripts to create an initial compilation. E.g. `./build_linux_release32_deb.sh`
5. Set all necessary environment variables and run the following command to register the ODBC driver.

   `./scripts/register_driver_unix.sh`
6. Set environment variables for testing and double-check if all dev environmnet variables are set running
   `scripts/env_variables_check.sh`.
7. Now you're ready to begin [configuration for integration and unit testing](#integration-tests).
8. Once configured, run the tests under repository root folder:
    - Run integration tests: `./build/odbc/bin/iotsitewise-odbc-integration-tests --catch_system_errors=false`.
    - Run unit tests: `./build/odbc/bin/iotsitewise-odbc-unit-tests --catch_system_errors=false`.

### Known issues

When running integration tests you may encounter the error

```
message: 01000: [unixODBC][Driver Manager]Can't open lib 'Amazon IoT SiteWise ODBC Driver' : file not found
```

Running `./scripts/register_driver_unix.sh` and copying `/etc/odbcinst.ini` to `$HOME/.odbcinist.ini` and
`/etc/odbc.ini` to `$HOME/.odbc.ini` may help the Driver and DNS be discovered.

## Code Coverage

### MacOS/Linux

To generate code coverage reports you to need to use the debug builds for macOS/Linux, run the tests and use `gcovr` to
compile the report. For an installation guide and how to use it look at the official (
documentation)[https://gcovr.com/en/stable/index.html].
If you want to check a detailed report generate the with `--html-details` option.

### Windows

OpenCppCoverage is used to generate code coverage for windows, for more information check it in the official (
documentation)[https://github.com/OpenCppCoverage/OpenCppCoverage]

## Versioning

1. To set the version of the ODBC driver, update the `src/ODBC_DRIVER_VERSION.txt` file with the appropriate version.

## Database Reporting

By default, the IoT SiteWise ODBC driver reports databases as schemas. However, Excel on macOS does not show tables with
duplicate names when databases are reported as schemas.
Driver will report databases as catalogs when the user exports environment variable `DATABASE_AS_SCHEMA` to any value
other than `TRUE`.

| Value of `DATABASE_AS_SCHEMA` | Behavior of driver                                                           |
|-------------------------------|------------------------------------------------------------------------------|
| any value / unset             | Databases are reported as schemas. Catalogs will not be supported by driver. |
| `FALSE`                       | Databases are reported as catalogs. Schemas will not be supported by driver. |

3. Load large scale data with data writer.

   i. Run the driver build script, it will build the data writer executable.

   ii. Run below `timestream-populate-data` command to insert test data for big table tests. For help with this command,
   see [data-population guide](data-population-guide.md). Note that this command will write data in region `us-east-1`,
   which is expected by our tests.
   ```
   iotsitewise-populate-data -u <access_key_id> -p <secret_access_key> -d data_queries_test_db -t TestMultiMeasureBigTable -ty computer -l 20000
   ```
   Since this command inserts randomly generated 20000 rows of data, it is estimated to take 15 - 20 minutes for the
   command to run.

4. (Optional) Now all test data has been loaded. Double check that all table dependencies have been created:
   ```
   sampleDB
   └-- IoTMulti
   meta_queries_test_db
   └-- IoTMulti
   └-- DevOpsMulti
   └-- TestColumnsMetadata1
   └-- TestColumnsMetadata2
   └-- testTableMeta (empty table)
   data_queries_test_db
   └-- TestComplexTypes
   └-- TestScalarTypes
   └-- EmptyTable (empty table)
   └-- TestMultiMeasureBigTable
   ```

### Known Issues with test data loading

#### AWS CLI parsing errors on PowerShell

AWS CLI commands may have parsing errors with PowerShell. The workaround is to wrap the json inputs with single quotes.
For example, before wrapping json data:
`aws timestream-write write-records --database-name <database name>  --table-name <table name> --common-attributes "{<json data>}" --records "{<json data>}"`

After wrapping json data:
`aws timestream-write write-records --database-name <database name>  --table-name <table name> --common-attributes '"{<json data>}"' --records '"{<json data>}"'`

#### Lacking permissions to read/change/write table

Reading and writing data on IoT SiteWise requires corresponding permissions. For read permissions, it is suggested to
add Amazon-managed policy `AWSIoTSiteWiseReadOnlyAccess`. For write permissions, see below for example policy.

```
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Sid": "VisualEditor0",
            "Effect": "Allow",
            "Action": [
                "iotsitewise:BatchPutAssetPropertyValue"
            ],
            "Resource": "*"
        }
    ]
}
```

#### `iotsitewise-populate-data` command not found on PowerShell

`iotsite-populate-data` is a data population tool provided together with IoT SiteWise ODBC driver. It is under
`<your_iotsitewise_repository>\build\odbc\bin\<Debug or Release>`, add that path to your environment variable PATH to
use it without path.

## Integration Tests

### IAM Profile Tests

1. The IAM profile tests are disabled by default because they require valid IAM profiles. They can be enabled by
   exporting environment variable `ENABLE_PROFILE_TEST` to `true`. Follow below instructions for setting up valid IAM
   profiles and required environmenr variables.

#### Windows

1. `cd` to repository root. Set environment variable REPOSITORY_ROOT to your repository root.
2. Run `.\src\tests\input\create_credentials_file.ps1` to create credential files for testing. Note that this script
   will write AWS IAM credentials file `src\tests\input\credentials`.
3. Set environment variable AWS_SHARED_CREDENTIALS_FILE to the newly created credentials file.

#### Linux or macOS

1. `cd` to repository root. Set environment variable REPOSITORY_ROOT to your repository root

   `export REPOSITORY_ROOT=<your repository root>`
2. Run `./src/tests/input/create_credentials_file.sh` from the respository root to create credential files for testing.
   Note that this script will write AWS IAM credentials file `src/tests/input/credentials`.
3. Set environment variable AWS_SHARED_CREDENTIALS_FILE to the newly created credentials file.

   `export AWS_SHARED_CREDENTIALS_FILE=$REPOSITORY_ROOT/src/tests/input/credentials`

### Azure AD Authentication Tests

1. The Azure AD authentication tests are disabled by default because they require valid Azure AD test account. They can
   be enabled by exporting environment variable `ENABLE_AAD_TEST` to `true`.

2. For AAD integration tests to be run successfully, you need to specify the following environment variables. For
   instructions on how to set-up and find the AAD connection property values, go to
   the [SAML 2.0 Azure AD set up guide](/docs/markdown/setup/aad-saml-setup.md#iotsitewise-odbc-dsn-configuration).

   |     Variable Name    | Corresponding Connection String Option |
      |----------------------|----------------------------------------|
   | `AAD_APP_ID`         | AADApplicationID                       |
   | `AAD_ROLE_ARN`       | RoleARN                                |
   | `AAD_IDP_ARN`        | IdPARN                                 |
   | `AAD_TENANT`         | AADTenant                              |
   | `AAD_USER`           | UID or IdPUserName                     |
   | `AAD_USER_PWD`       | PWD or IdPPassword                     |
   | `AAD_CLIENT_SECRET`  | AADClientSecret                        |

### Okta Authentication Tests

1. The Okta authentication tests are disabled by default because they require a valid Okta test account. They can be
   enabled by exporting environment variable `ENABLE_OKTA_TEST` to `true`.

2. To run Okta authentication test, the environment variables in the following table need to be configured with correct
   values. Refer to [Okta Authentication Setup Guide](Okta-setup.md) for instructions on setting up an Okta
   authentication.

   | Variable Name |  Corresponding Connection String Option   |
      |---------------|-------------------------------------------|
   |`OKTA_HOST`    |   IdPHost                                 |
   |`OKTA_USER`    |   UID or IdPUserName                      |
   |`OKTA_USER_PWD`|   PWD or IdPPassword                      |
   |`OKTA_APP_ID`  |   OktaApplicationID                       |
   |`OKTA_ROLE_ARN`|   RoleARN                                 |
   |`OKTA_IDP_ARN` |   IdPARN                                  |

Ensure `OKTA_HOST` does not include `https://` or `http://`.

### Big Table Pagination Tests

Big table pagination tests are time-consuming. To save time for integration test， they are disabled by default. They
could be enabled by export environment variable `BIG_TABLE_PAGINATION_TEST_ENABLE` to `true`.

### Proxy Manual Test on Windows

For setting up connection proxy properties, see [connection proxy guide.](connection-proxy-guide.md).

1. Download proxy-test-server from https://github.com/andris9/proxy-test-server
   `git clone git@github.com:andris9/proxy-test-server.git`
2. Install proxy-test-server
   `npm install proxy-test-server`
3. Run proxy server at port 9999
   `cd proxy-test-server/examples`
   `node proxy.js`
4. Set environment variable SW_PROXY_HOST, SW_PROXY_PORT and SW_PROXY_SCHEME.
5. Start DSN window and create a connection to IoT SiteWise. Click 'Test' button to verify.

## Test Results

Unit test results can be viewed in `odbc_unit_test_result.xml` and integration test results can be viewed in
`odbc_test_result.xml`.
