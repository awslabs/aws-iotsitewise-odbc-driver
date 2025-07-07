# Connection Proxy Setup Guide

IoT SiteWise ODBC driver supports connecting to Amazon IoT SiteWise through a proxy. To use this feauture the following environment variables should be configured based on your proxy setting.

|  Environment Variable       |   Comment                                                              |
|-----------------------------|------------------------------------------------------------------------|
|  SW_PROXY_HOST              |  Proxy host                                                            |
|  SW_PROXY_PORT              |  Proxy port                                                            |
|  SW_PROXY_SCHEME            |  Proxy scheme, `http` or `https`                                       |
|  SW_PROXY_USER              |  User name for proxy authentication                                    |
|  SW_PROXY_PASSWORD          |  User password for proxy authentication                                |
|  SW_PROXY_SSL_CERT_PATH     |  SSL Certificate file to use for connecting to an HTTPS proxy          |
|  SW_PROXY_SSL_CERT_TYPE     |  Type of proxy client SSL certificate                                  |
|  SW_PROXY_SSL_KEY_PATH      |  Private key file to use for connecting to an HTTPS proxy              |
|  SW_PROXY_SSL_KEY_TYPE      |  Type of private key file used to connect to an HTTPS proxy            |
|  SW_PROXY_SSL_KEY_PASSWORD  |  Passphrase to the private key file used to connect to an HTTPS proxy  |
