/*  ProgramGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
 *  Copyright 2011-2022 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#import <string>
#import "ProgramGLES.h"
#import "Lighting.h"
#import "UtilsGLES.h"
#import "SceneRendererGLES.h"
#import "TextureGLES.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{
    
ProgramGLES::ProgramGLES()
    : lightsLastUpdated(0.0)
    , program(0)
    , vertShader(0)
    , fragShader(0)
    , attrs(10)
    , uniforms(10)
{
}

ProgramGLES::~ProgramGLES()
{
    if (program)
    {
        wkLogLevel(Warn, "ProgramGLES destroyed without being cleaned up");
    }
    // Clean up anyway, may fail due to thread context
    cleanUp();
}

bool ProgramGLES::setUniform(StringIdentity nameID,float val)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT)
        return false;
    
    if (uni->isSet && uni->val.fVals[0] == val)
        return true;
    
    glUniform1f(uni->index,val);
    CheckGLError("ProgramGLES::setUniform() glUniform1f");
    uni->isSet = true;
    uni->val.fVals[0] = val;
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,float val,int index)
{
    std::string name = StringIndexer::getString(nameID) + "[0]";
    OpenGLESUniform *uni = findUniform(StringIndexer::getStringID(name));
    if (!uni)
        return false;

    if (uni->type != GL_FLOAT)
        return false;
    
    glUniform1f(uni->index+index,val);
    CheckGLError("ProgramGLES::setUniform() glUniform1f");
    uni->isSet = true;
    uni->val.fVals[0] = val;
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,int val)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_INT && uni->type != GL_SAMPLER_2D && uni->type != GL_UNSIGNED_INT && uni->type != GL_BOOL)
        return false;
    
    if (uni->isSet && uni->val.iVals[0] == val)
        return true;
    
    glUniform1i(uni->index,val);
    CheckGLError("ProgramGLES::setUniform() glUniform1i");
    uni->isSet = true;
    uni->val.iVals[0] = val;
    
    return true;
}
    
bool ProgramGLES::setTexture(StringIdentity nameID,TextureBase *inTex,int textureSlot)
{
    const auto tex = dynamic_cast<TextureBaseGLES *>(inTex);
    if (!tex)
        return false;
    
    const GLuint val = tex->getGLId();
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_INT && uni->type != GL_SAMPLER_2D && uni->type != GL_UNSIGNED_INT && uni->type != GL_BOOL)
        return false;
    
    uni->isTexture = true;
    uni->isSet = true;
    uni->val.iVals[0] = val;
    
    return true;
}


void ProgramGLES::clearTexture(SimpleIdentity texID)
{
    // Note: Doesn't do anything
}

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector2f &vec)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC2)
        return false;
    
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y())
        return true;
    
    glUniform2f(uni->index, vec.x(), vec.y());
    CheckGLError("ProgramGLES::setUniform() glUniform2f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector3f &vec)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC3)
        return false;
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() && uni->val.fVals[2] == vec.z())
        return true;
    
    glUniform3f(uni->index, vec.x(), vec.y(), vec.z());
    CheckGLError("ProgramGLES::setUniform() glUniform3f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z();
    
    return true;
}
    

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector4f &vec)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC4)
        return false;
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() &&
        uni->val.fVals[2] == vec.z() && uni->val.fVals[3] == vec.w())
        return true;
    
    glUniform4f(uni->index, vec.x(), vec.y(), vec.z(), vec.w());
    CheckGLError("ProgramGLES::setUniform() glUniform4f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z(); uni->val.fVals[3] = vec.w();
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector4f &vec,int index)
{
    std::string name = StringIndexer::getString(nameID) + "[0]";
    OpenGLESUniform *uni = findUniform(StringIndexer::getStringID(name));
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC4)
        return false;
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() &&
        uni->val.fVals[2] == vec.z() && uni->val.fVals[3] == vec.w())
        return true;
    
    glUniform4f(uni->index+index, vec.x(), vec.y(), vec.z(), vec.w());
    CheckGLError("ProgramGLES::setUniform() glUniform4f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z(); uni->val.fVals[3] = vec.w();
    
    return true;
}
    


bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Matrix4f &mat)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_MAT4)
        return false;
    
    if (uni->isSet)
    {
        bool equal = true;
        for (unsigned int ii=0;ii<16;ii++)
            if (mat.data()[ii] != uni->val.mat[ii])
            {
                equal = false;
                break;
            }
        if (equal)
            return true;
    }
    
    glUniformMatrix4fv(uni->index, 1, GL_FALSE, (GLfloat *)mat.data());
    CheckGLError("ProgramGLES::setUniform() glUniformMatrix4fv");
    uni->isSet = true;
    for (unsigned int ii=0;ii<16;ii++)
        uni->val.mat[ii] = mat.data()[ii];
    
    return true;
}
    
bool ProgramGLES::setUniform(const SingleVertexAttribute &attr)
{
    bool ret = false;
    
    switch (attr.type)
    {
        case BDFloat4Type:
            ret = setUniform(attr.nameID, Vector4f(attr.data.vec4[0],attr.data.vec4[1],attr.data.vec4[2],attr.data.vec4[3]));
            break;
        case BDFloat3Type:
            ret = setUniform(attr.nameID, Vector3f(attr.data.vec3[0],attr.data.vec3[1],attr.data.vec3[2]));
            break;
        case BDChar4Type:
            ret = setUniform(attr.nameID, Vector4f(attr.data.color[0],attr.data.color[1],attr.data.color[2],attr.data.color[3]));
            break;
        case BDFloat2Type:
            ret = setUniform(attr.nameID, Vector2f(attr.data.vec2[0],attr.data.vec2[1]));
            break;
        case BDFloatType:
            ret = setUniform(attr.nameID, attr.data.floatVal);
            break;
        case BDIntType:
            ret = setUniform(attr.nameID, attr.data.intVal);
            break;
        default:
            break;
    }
    
    return ret;
}

// Helper routine to compile a shader and check return
bool compileShader(const std::string &name,const char *shaderTypeStr,GLuint *shaderId,GLenum shaderType,const std::string &shaderStr)
{
    *shaderId = glCreateShader(shaderType);
    if (*shaderId == 0) {
        wkLogLevel(Error,"Failed to create GL shader (%d)", shaderType);
        return false;
    }

    const GLchar *sourceCStr = shaderStr.c_str();
    glShaderSource(*shaderId, 1, &sourceCStr, nullptr);
    glCompileShader(*shaderId);
    
    GLint status = GL_FALSE;
    glGetShaderiv(*shaderId, GL_COMPILE_STATUS, &status);
    
    if (status != GL_TRUE)
    {
        GLint len = 0;
        glGetShaderiv(*shaderId, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            std::vector<char> logStr(len+1);
            glGetShaderInfoLog(*shaderId, len, &len, &logStr[0]);
            wkLogLevel(Error,"Compile error for %s shader %s:\n%s",shaderTypeStr,name.c_str(),&logStr[0]);
        }
        
        glDeleteShader(*shaderId);
        *shaderId = 0;
    }
    
    return status == GL_TRUE;
}

#define DUMP_UNIFORMS 0

// Construct the program, compile and link
ProgramGLES::ProgramGLES(const std::string &inName,const std::string &vShaderString,const std::string &fShaderString,const std::vector<std::string> *varying)
    : ProgramGLES()
{
    name = inName;
    program = glCreateProgram();
    if (!CheckGLError("ProgramGLES glCreateProgram"))
    {
        return;
    }
    if (!program)
    {
        // glCreateProgram sometimes produces zero without setting any error.
        // This seems to be related to being called without a current context.
#if defined(EGL_VERSION_1_4)
        wkLogLevel(Warn, "glCreateProgram Failed (%x,%x)", glGetError(), eglGetCurrentContext());
#else
        wkLogLevel(Warn, "glCreateProgram Failed (%x)", glGetError());
#endif
        return;
    }
    
    if (!compileShader(name,"vertex",&vertShader,GL_VERTEX_SHADER,vShaderString))
    {
        cleanUp();
        return;
    }
    CheckGLError("ProgramGLES: compileShader() vertex");
    if (!compileShader(name,"fragment",&fragShader,GL_FRAGMENT_SHADER,fShaderString))
    {
        cleanUp();
        return;
    }
    CheckGLError("ProgramGLES: compileShader() fragment");

    glAttachShader(program, vertShader);
    CheckGLError("ProgramGLES: glAttachShader() vertex");
    glAttachShader(program, fragShader);
    CheckGLError("ProgramGLES: glAttachShader() fragment");

    // Designate the varyings that we want out of the shader
    if (varying) {
        auto **names = (GLchar **)malloc(sizeof(GLchar *)*varying->size());
        if (names) {
            for (unsigned int ii = 0; ii < varying->size(); ii++) {
                const std::string &name = (*varying)[ii];
                names[ii] = (GLchar *) malloc(sizeof(GLchar) * (name.size() + 1));
                if (names[ii]) {
                    strcpy(names[ii], name.c_str());
                }
            }
            glTransformFeedbackVaryings(program, varying->size(), names, GL_SEPARATE_ATTRIBS);

            CheckGLError("ProgramGLES: Error setting up varyings in");

            for (unsigned int ii = 0; ii < varying->size(); ii++) {
                if (names[ii]) {
                    free(names[ii]);
                }
            }
            free(names);
        }
    }
    
    // Now link it
    GLint status;
    glLinkProgram(program);
    CheckGLError("ProgramGLES: glLinkProgram");

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            std::vector<char> logStr(len+1);
            glGetProgramInfoLog(program, len, &len, &logStr[0]);
            wkLogLevel(Error,"Link error for shader program %s:\n%s",name.c_str(),&logStr[0]);
        }
        cleanUp();
        return;
    }

    if (vertShader)
    {
        glDeleteShader(vertShader);
        vertShader = 0;
    }
    if (fragShader)
    {
        glDeleteShader(fragShader);
        fragShader = 0;
    }
    
    // Convert the uniforms into a more friendly form
    GLint numUniform;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniform);
    char thingName[1024];
    for (unsigned int ii=0;ii<numUniform;ii++)
    {
        auto uni = std::make_shared<OpenGLESUniform>();
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveUniform(program, ii, sizeof(thingName)-1, &bufLen, &uni->size, &uni->type, thingName);
        uni->nameID = StringIndexer::getStringID(thingName);
        uni->index = glGetUniformLocation(program, thingName);
        uniforms[uni->nameID] = uni;
#if DUMP_UNIFORMS
        wkLog("%s Uniform %d/%d, name=%d, idx=%d, %s", inName.c_str(), ii, numUniform, uni->nameID, uni->index, thingName);
#endif
    }
    CheckGLError("ProgramGLES: glGetActiveUniform");

    // Convert the attributes into a more useful form
    GLint numAttr;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttr);
    for (unsigned int ii=0;ii<numAttr;ii++)
    {
        auto attr = std::make_shared<OpenGLESAttribute>();
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveAttrib(program, ii, sizeof(thingName)-1, &bufLen, &attr->size, &attr->type, thingName);
        attr->index = glGetAttribLocation(program, thingName);
        attr->nameID = StringIndexer::getStringID(thingName);
        attrs[attr->nameID] = attr;
#if DUMP_UNIFORMS
        wkLog("%s Attribute %d/%d, name=%d, idx=%d, %s", inName.c_str(), ii, numAttr, attr->nameID, attr->index, thingName);
#endif
    }
    CheckGLError("ProgramGLES: glGetActiveAttrib");
}
    
void ProgramGLES::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene,RenderTeardownInfoRef teardown)
{
    cleanUp();
}
    
// Clean up outstanding OpenGL resources
void ProgramGLES::cleanUp()
{
    if (program)
    {
        glDeleteProgram(program);
        CheckGLError("ProgramGLES::cleanup() glDeleteProgram");
        program = 0;
    }
    if (vertShader)
    {
        glDeleteShader(vertShader);
        CheckGLError("ProgramGLES::cleanup() glDeleteShader vertShader");
        vertShader = 0;
    }
    if (fragShader)
    {
        glDeleteShader(fragShader);
        CheckGLError("ProgramGLES::cleanup() glDeleteShader fragShader");
        fragShader = 0;
    }
    
    uniforms.clear();
    attrs.clear();
}
    
bool ProgramGLES::isValid() const
{
    return (program != 0);
}
    

OpenGLESUniform *ProgramGLES::findUniform(StringIdentity nameID) const
{
    auto it = uniforms.find(nameID);
    if (it == uniforms.end())
        return nullptr;
    return it->second.get();
}

const OpenGLESAttribute *ProgramGLES::findAttribute(StringIdentity nameID) const
{
    auto it = attrs.find(nameID);
    if (it == attrs.end())
        return nullptr;
    return it->second.get();
}
    
bool ProgramGLES::hasLights() const
{
    OpenGLESUniform *lightAttr = findUniform(u_numLightsNameID);
    return lightAttr != nullptr;
}
    
bool ProgramGLES::setLights(const std::vector<DirectionalLight> &lights, TimeInterval lastUpdated, const Material *mat, const Eigen::Matrix4f &modelMat)
{
    if (lightsLastUpdated >= lastUpdated)
        return false;
    lightsLastUpdated = lastUpdated;
    
    int numLights = (int)lights.size();
    numLights = std::min(numLights,8);
    bool lightsSet = true;
    for (unsigned int ii=0;ii<numLights;ii++)
    {
        const DirectionalLight &light = lights[ii];
        const Eigen::Vector3f dir = light.pos.normalized();
        const Eigen::Vector3f halfPlane = (dir + Eigen::Vector3f(0,0,1)).normalized();

        setUniform(lightViewDependNameIDs[ii], (light.viewDependent ? 0.0f : 1.0f));
        setUniform(lightDirectionNameIDs[ii], dir);
        setUniform(lightHalfplaneNameIDs[ii], halfPlane);
        setUniform(lightAmbientNameIDs[ii], light.ambient);
        setUniform(lightDiffuseNameIDs[ii], light.diffuse);
        setUniform(lightSpecularNameIDs[ii], light.specular);
    }
    OpenGLESUniform *lightAttr = findUniform(u_numLightsNameID);
    if (lightAttr)
        glUniform1i(lightAttr->index, numLights);
    else
        return false;
    
    // Bind the material
    if (mat)
    {
        setUniform(materialAmbientNameID, mat->ambient);
        setUniform(materialDiffuseNameID, mat->diffuse);
        setUniform(materialSpecularNameID, mat->specular);
        setUniform(materialSpecularExponentNameID, mat->specularExponent);
    }

    return lightsSet;
}

int ProgramGLES::bindTextures()
{
    int numTextures = 0;
    
    for (const auto &uni : uniforms)
    {
        if (uni.second->isTexture)
        {
            glActiveTexture(GL_TEXTURE0+numTextures);
            glBindTexture(GL_TEXTURE_2D, uni.second->val.iVals[0]);
            glUniform1i(uni.second->index,numTextures);
            numTextures++;
        }
    }
    
    return numTextures;
}

}
