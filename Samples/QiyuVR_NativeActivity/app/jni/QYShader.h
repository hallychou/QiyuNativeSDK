/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/
#ifndef _QYSHADER_H_
#define _QYSHADER_H_

#include "QYRefCount.h"
#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <string>

extern const char shader_handle_vs[];
extern const char shader_handleTex_fs[];
extern const char shader_handle_fs[];
extern const char shader_ray_vs[];
extern const char shader_ray_fs[];
//
extern const char shader_handle_vs_MultiView[];
extern const char shader_handleTex_fs_MultiView[];
extern const char shader_handle_fs_MultiView[];
extern const char shader_ray_vs_MultiView[];
extern const char shader_ray_fs_MultiView[];

class QYShader : public QYRefCount
{
public:
	QYShader(const char* szVS, const char* szFS);
	~QYShader();

	void Release();

	bool Bind();
	bool UnBind();

	void SetVector3(const std::string& name, const glm::vec3& value) const;
	void SetVector3(const std::string& name, float x, float y, float z) const;

	void SetVector4(const std::string& name, const glm::vec4& value) const;
	void SetVector4(const std::string& name, float x, float y, float z, float w);

	void SetMatrix4(const std::string& name, const glm::mat4& mat) const;
	void SetMat4fv(const std::string& name, unsigned int count, const float *pData);

	unsigned int GetID() const { return m_uID; }
protected:
	unsigned int m_uID;
};


#endif//_QYSHADER_H_