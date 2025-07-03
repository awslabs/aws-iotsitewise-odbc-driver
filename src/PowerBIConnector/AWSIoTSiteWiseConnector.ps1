# Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.

$ZipFile = '.\AWSIoTSiteWiseConnector.zip'
$MezFile = '.\AWSIoTSiteWiseConnector.mez'

if (Test-Path -Path $ZipFile) {
    Write-Host Removing previous AWSIoTSiteWiseConnector.zip

    Remove-Item $ZipFile
}

Write-Host Creating AWSIoTSiteWiseConnector.m file

Copy-Item '.\AWSIoTSiteWiseConnector\AWSIoTSiteWiseConnector.pq' '.\AWSIoTSiteWiseConnector\AWSIoTSiteWiseConnector.m'

Write-Host Creating AWSIoTSiteWiseConnector.zip file

compress-archive -path '.\AWSIoTSiteWiseConnector\*.png', '.\AWSIoTSiteWiseConnector\*.pqm', '.\AWSIoTSiteWiseConnector\*.resx', '.\AWSIoTSiteWiseConnector\AWSIoTSiteWiseConnector.m' -destinationpath $ZipFile -update -compressionlevel optimal

if (Test-Path -Path '.\AWSIoTSiteWiseConnector\AWSIoTSiteWiseConnector.m') {
    Write-Host Removing AWSIoTSiteWiseConnector.m

    Remove-Item '.\AWSIoTSiteWiseConnector\AWSIoTSiteWiseConnector.m'
}

if (Test-Path -Path $MezFile) {
    Write-Host Removing previous AWSIoTSiteWiseConnector.mez

    Remove-Item $MezFile
}

Write-Host Creating AWSIoTSiteWiseConnector.mez

Move-Item $ZipFile $MezFile

Write-Host done
