//
// Copyright (C) 2018 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//  ShaderPlain.vsh
//

attribute highp vec3    myVertex;
attribute highp vec3    myNormal;
attribute mediump vec2  myUV;
attribute mediump vec4  myBone;

varying lowp    vec4    colorDiffuse;

varying mediump vec3 position;
varying mediump vec3 normal;

uniform highp mat4      uMVMatrix;
uniform highp mat4      uPMatrix;

uniform highp vec3      vLight0;

uniform lowp vec4       vMaterialDiffuse;
uniform lowp vec3       vMaterialAmbient;
uniform lowp vec4       vMaterialSpecular;

void main(void)
{
    highp vec4 p = vec4(myVertex,1);
    gl_Position = uPMatrix * p;

    highp vec3 worldNormal = vec3(mat3(uMVMatrix[0].xyz, uMVMatrix[1].xyz, uMVMatrix[2].xyz) * myNormal);
    highp vec3 ecPosition = p.xyz;

    // brighten the teapot color by 50%
    colorDiffuse = clamp(vMaterialDiffuse * 1.5,
                         vec4(0.0, 0.0, 0.0, 0.0),
                         vec4(1.0, 1.0, 1.0, 1.0));

    normal = worldNormal;
    position = ecPosition;
}