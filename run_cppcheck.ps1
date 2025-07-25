# 
#    Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
# 
#    Licensed under the Apache License, Version 2.0 (the "License").
#    You may not use this file except in compliance with the License.
#    A copy of the License is located at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#    or in the "license" file accompanying this file. This file is distributed
#    on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#    express or implied. See the License for the specific language governing
#    permissions and limitations under the License.
# 

#  --force: force checks all define combinations (default max is 12)
#  --suppress=objectIndex: seemingly false-positive

cppcheck --check-level=exhaustive --force --library=boost --library=iotsitewise .\src\ 2>  .\cppcheck-results.log
If ((Get-Content "cppcheck-results.log") -eq $Null) {
  exit 0
}
exit 1
