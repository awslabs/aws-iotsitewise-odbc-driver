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

$ZipFile = '.\AWSIoTSiteWiseAADConnector.zip'
$MezFile = '.\AWSIoTSiteWiseAADConnector.mez'

if (Test-Path -Path $ZipFile) {
    Write-Host Removing previous AWSIoTSiteWiseAADConnector.zip

    Remove-Item $ZipFile
}

Write-Host Creating AWSIoTSiteWiseAADConnector.m file

Copy-Item '.\AWSIoTSiteWiseAADConnector\AWSIoTSiteWiseAADConnector.pq' '.\AWSIoTSiteWiseAADConnector\AWSIoTSiteWiseAADConnector.m'

Write-Host Creating AWSIoTSiteWiseAADConnector.zip file

compress-archive -path '.\AWSIoTSiteWiseAADConnector\*.png', '.\AWSIoTSiteWiseAADConnector\*.pqm', '.\AWSIoTSiteWiseAADConnector\*.resx', '.\AWSIoTSiteWiseAADConnector\AWSIoTSiteWiseAADConnector.m' -destinationpath $ZipFile -update -compressionlevel optimal

if (Test-Path -Path '.\AWSIoTSiteWiseAADConnector\AWSIoTSiteWiseAADConnector.m') {
    Write-Host Removing AWSIoTSiteWiseAADConnector.m

    Remove-Item '.\AWSIoTSiteWiseAADConnector\AWSIoTSiteWiseAADConnector.m'
}

if (Test-Path -Path $MezFile) {
    Write-Host Removing previous AWSIoTSiteWiseAADConnector.mez

    Remove-Item $MezFile
}

Write-Host Creating AWSIoTSiteWiseAADConnector.mez

Move-Item $ZipFile $MezFile

Write-Host done
