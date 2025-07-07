# Amazon IoT SiteWise ODBC Driver Setup on MacOS

## Supported versions
macOS 11 (Big Sur) and above on 64-bit Intel processor. Apple M1 is not currently supported.

## Prerequisites
In order to use the IoT SiteWise ODBC Driver, [libiodbc](https://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/WelcomeVisitors) must be installed.
```
brew install libiodbc
```

## Install IoT SiteWise ODBC Driver
1. Download the IoT SiteWise ODBC driver installer AmazonIoTSiteWiseODBC-[version].pkg.
2. Double-click the installer to install by GUI or use the following command line to install it directly.
```
sudo installer -pkg AmazonIoTSiteWiseODBC-[version].pkg -target /
```
3. Follow the instructions and finish the installation if you install by GUI.

## Next Steps

- [Set up the DSN](macOS-dsn-configuration.md)

## Uninstall IoT SiteWise ODBC Driver
There is no automatic way to uninstall it. It has to be done manually.
1. Remove "Amazon IoT SiteWise ODBC Driver" entry from /Library/ODBC/odbcinst.ini
2. Remove the driver residence directory /Library/ODBC/iotsitewise-odbc
```
sudo rm -rf /Library/ODBC/iotsitewise-odbc
```
