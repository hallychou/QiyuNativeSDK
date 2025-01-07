/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/
#include "QYShader.h"
#include "QYUtil.h"


const char shader_handle_vs[] =
"#version 300 es\n"
"precision mediump float;\n"
"layout (location = 0) in vec3 Position;\n"
"layout (location = 1) in vec3 Normal;\n"
"layout (location = 2) in vec2 TexCoord;\n"
"uniform mat4 u_matModel;\n"
"uniform mat4 u_matView;\n"
"uniform mat4 u_matProj;\n"
"uniform vec3 u_lightPos;\n"
"uniform vec3 u_clrLight;\n"
"uniform vec3 u_viewPos;\n"
"out vec2 vTexCoord;\n"
"out vec3 vAmbient;\n"
"out vec3 vDiffuse;\n"
"out vec3 vSpecular;\n"
"void main()\n"
"{\n"
"    vTexCoord = TexCoord;\n"
"    vec4 pos = vec4(Position, 1.0);\n"
"    gl_Position = u_matProj * (u_matView * (u_matModel * pos));\n"
"    vec3 posWorld = vec3(u_matModel * pos);\n"
"\n"
"    float ambientStrength = 1.0;\n"
"    vAmbient = ambientStrength * u_clrLight;\n"
"\n"
"    float diffuseStrength = 1.0;\n"
"    vec3 unitNormal = normalize(vec3(u_matModel * vec4(Normal, 1.0)));\n"
"    vec3 lightDir = normalize(u_lightPos - posWorld);\n"
"    float diff = max(dot(unitNormal, lightDir), 0.0);\n"
"    vDiffuse = diffuseStrength * diff * u_clrLight;\n"
"\n"
"    float specularStrength = 2.3;\n"
"    vec3 viewDir = normalize(u_viewPos - posWorld);\n"
"    vec3 reflectDir = reflect(-lightDir, unitNormal);\n"
"    float spec = pow(max(dot(unitNormal, reflectDir), 0.0), 16.0);\n"
"    vSpecular = specularStrength * spec * u_clrLight;\n"
"}";
const char shader_handleTex_fs[] =
"#version 300 es\n"
"precision mediump float;\n"
"in vec2 vTexCoord;\n"
"in vec3 vAmbient;\n"
"in vec3 vDiffuse;\n"
"in vec3 vSpecular;\n"
"uniform sampler2D u_texDiffuse1;\n"
"uniform vec3 u_clrPressBtn;\n"
"out vec4 Color;\n"
"void main()\n"
"{\n"
"    vec4 objectColor = texture(u_texDiffuse1, vTexCoord);\n"
"    vec3 finalColor = (vAmbient*u_clrPressBtn + vDiffuse + vSpecular) * vec3(objectColor);\n"
"    Color = vec4(finalColor, 1.0);\n"
"}";
const char shader_handle_fs[] =
"#version 300 es\n"
"precision highp float;\n"
"in vec3 vAmbient;\n"
"in vec3 vDiffuse;\n"
"in vec3 vSpecular;\n"
"out vec4 Color;\n"
"void main()\n"
"{\n"
"    vec4 objectColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    vec3 finalColor = (vAmbient + vDiffuse + vSpecular) * vec3(objectColor);\n"
"    Color = vec4(finalColor, 1.0);\n"
"}";

const char shader_ray_vs[] =
"#version 300 es\n"
"layout(location = 0) in vec3 Vertex;\n"
"uniform mat4 u_matModel_ray;\n"
"uniform mat4 u_matView_ray;\n"
"uniform mat4 u_matProj_ray;\n"
"void main()\n"
"{\n"
"    gl_Position = u_matProj_ray * (u_matView_ray * (u_matModel_ray * vec4(Vertex, 1.0)));\n"
"}";
const char shader_ray_fs[] =
"#version 300 es\n"
"uniform vec4 u_clrRay;\n"
"out highp vec4 Color;\n"
"void main()\n"
"{\n"
"    Color = vec4(u_clrRay);\n"
"}";

////////////////////////////////////////////////////////////////////////////////

const char shader_handle_vs_MultiView[] =
"#version 300 es\n"
"#extension GL_OVR_multiview : enable\n"
"#extension GL_OVR_multiview2 : enable\n"
"#extension GL_OVR_multiview_multisampled_render_to_texture : enable\n"
"layout(num_views = 2) in;\n"
"\n"
"precision mediump float;\n"
"\n"
"layout (location = 0) in vec3 Position;\n"
"layout (location = 1) in vec3 Normal;\n"
"layout (location = 2) in vec2 TexCoord;\n"
"uniform mat4 u_matModel;\n"
//"uniform mat4 u_matView;\n"
"uniform mat4 u_matView[2];\n"
"uniform mat4 u_matProj;\n"
"uniform vec3 u_lightPos;\n"
"uniform vec3 u_clrLight;\n"
"uniform vec3 u_viewPos;\n"
"out vec2 vTexCoord;\n"
"out vec3 vAmbient;\n"
"out vec3 vDiffuse;\n"
"out vec3 vSpecular;\n"
"out vec3 clrTestMultiView;\n"
"void main()\n"
"{\n"
"    vTexCoord = TexCoord;\n"
"    vec4 pos = vec4(Position, 1.0);\n"
//"    gl_Position = u_matProj * (u_matView * (u_matModel * pos));\n"
"    gl_Position = u_matProj * (u_matView[gl_ViewID_OVR] * (u_matModel * pos));\n"
"    clrTestMultiView = vec3(1.0 - float(gl_ViewID_OVR), float(gl_ViewID_OVR), 0.0);\n"
"    vec3 posWorld = vec3(u_matModel * pos);\n"
"\n"
"    float ambientStrength = 1.0;\n"
"    vAmbient = ambientStrength * u_clrLight;\n"
"\n"
"    float diffuseStrength = 1.0;\n"
"    vec3 unitNormal = normalize(vec3(u_matModel * vec4(Normal, 1.0)));\n"
"    vec3 lightDir = normalize(u_lightPos - posWorld);\n"
"    float diff = max(dot(unitNormal, lightDir), 0.0);\n"
"    vDiffuse = diffuseStrength * diff * u_clrLight;\n"
"\n"
"    float specularStrength = 2.3;\n"
"    vec3 viewDir = normalize(u_viewPos - posWorld);\n"
"    vec3 reflectDir = reflect(-lightDir, unitNormal);\n"
"    float spec = pow(max(dot(unitNormal, reflectDir), 0.0), 16.0);\n"
"    vSpecular = specularStrength * spec * u_clrLight;\n"
"}";
const char shader_handleTex_fs_MultiView[] =
"#version 300 es\n"
"precision mediump float;\n"
"in vec2 vTexCoord;\n"
"in vec3 vAmbient;\n"
"in vec3 vDiffuse;\n"
"in vec3 vSpecular;\n"
"in vec3 clrTestMultiView;\n"
"uniform sampler2D u_texDiffuse1;\n"
"uniform vec3 u_clrPressBtn;\n"
"out vec4 Color;\n"
"void main()\n"
"{\n"
"    vec4 objectColor = texture(u_texDiffuse1, vTexCoord);\n"
"    vec3 finalColor = (vAmbient*u_clrPressBtn + vDiffuse + vSpecular) * vec3(objectColor);\n"
//"    Color = vec4(finalColor, 1.0);\n"
//"    Color = vec4(finalColor.xyz * clrTestMultiView, 1.0);\n"
"    Color = vec4(clrTestMultiView, 1.0);\n"
"}";
const char shader_handle_fs_MultiView[] =
"#version 300 es\n"
"precision highp float;\n"
"in vec3 vAmbient;\n"
"in vec3 vDiffuse;\n"
"in vec3 vSpecular;\n"
"in vec3 clrTestMultiView;\n"
"out vec4 Color;\n"
"void main()\n"
"{\n"
"    vec4 objectColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    vec3 finalColor = (vAmbient + vDiffuse + vSpecular) * vec3(objectColor);\n"
//"    Color = vec4(finalColor, 1.0);\n"
//"    Color = vec4(finalColor.xyz * clrTestMultiView, 1.0);\n"
"    Color = vec4(clrTestMultiView, 1.0);\n"
"}";

const char shader_ray_vs_MultiView[] =
"#version 300 es\n"
"\n"
"#extension GL_OVR_multiview : enable\n"
"#extension GL_OVR_multiview2 : enable\n"
"#extension GL_OVR_multiview_multisampled_render_to_texture : enable\n"
"layout(num_views = 2) in;\n"
"\n"
"layout(location = 0) in vec3 Vertex;\n"
"uniform mat4 u_matModel_ray;\n"
//"uniform mat4 u_matView_ray;\n"
"uniform mat4 u_matView_ray[2];\n"
"uniform mat4 u_matProj_ray;\n"
"out vec3 clrTestMultiView;\n"
"void main()\n"
"{\n"
//"    gl_Position = u_matProj_ray * (u_matView_ray * (u_matModel_ray * vec4(Vertex, 1.0)));\n"
"    gl_Position = u_matProj_ray * (u_matView_ray[gl_ViewID_OVR] * (u_matModel_ray * vec4(Vertex, 1.0)));\n"
"    clrTestMultiView = vec3(1.0 - float(gl_ViewID_OVR), float(gl_ViewID_OVR), 0.0);\n"
"}";
const char shader_ray_fs_MultiView[] =
"#version 300 es\n"
"in vec3 clrTestMultiView;\n"
"uniform vec4 u_clrRay;\n"
"out highp vec4 Color;\n"
"void main()\n"
"{\n"
//"    Color = u_clrRay;\n"
//"    Color = vec4(u_clrRay.xyz * clrTestMultiView, 1.0);\n"
"    Color = vec4(clrTestMultiView, 1.0);\n"
"}";


GLuint LoadShader_(GLenum shaderType, const char* pSource)
{
	GLuint shader = 0;
	shader = glCreateShader(shaderType);
	if (shader)
	{
		glShaderSource(shader, 1, &pSource, NULL);
		glCompileShader(shader);
		GLint compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen)
			{
				char* buf = (char*)malloc((size_t)infoLen);
				if (buf)
				{
					glGetShaderInfoLog(shader, infoLen, NULL, buf);
					free(buf);
				}
				glDeleteShader(shader);
				shader = 0;
			}
		}
	}
	return shader;
}

void DeleteProgram_(GLuint& program)
{
	if (program)
	{
		glUseProgram(0);
		glDeleteProgram(program);
		program = 0;
	}
}

static GLuint CreateProgram_(const char* pVertexShaderSource, const char* pFragShaderSource, GLuint& vertexShaderHandle, GLuint& fragShaderHandle)
{
	GLuint program = 0;
	vertexShaderHandle = LoadShader_(GL_VERTEX_SHADER, pVertexShaderSource);
	if (!vertexShaderHandle) return program;
	fragShaderHandle = LoadShader_(GL_FRAGMENT_SHADER, pFragShaderSource);
	if (!fragShaderHandle) return program;

	program = glCreateProgram();
	if (program)
	{
		glAttachShader(program, vertexShaderHandle);
		glAttachShader(program, fragShaderHandle);
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

		glDetachShader(program, vertexShaderHandle);
		glDeleteShader(vertexShaderHandle);
		vertexShaderHandle = 0;
		glDetachShader(program, fragShaderHandle);
		glDeleteShader(fragShaderHandle);
		fragShaderHandle = 0;
		if (linkStatus != GL_TRUE)
		{
			GLint bufLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
			if (bufLength)
			{
				char* buf = (char*)malloc((size_t)bufLength);
				if (buf)
				{
					glGetProgramInfoLog(program, bufLength, NULL, buf);
					free(buf);
				}
			}
			glDeleteProgram(program);
			program = 0;
		}
	}
	return program;
}


QYShader::QYShader(const char* szVS, const char* szFS)
{
	GLuint hVS, hFS;
	m_uID = CreateProgram_(szVS, szFS, hVS, hFS);
	assert(m_uID != 0);
}

QYShader::~QYShader()
{
	Release();
}

void QYShader::Release()
{
	DeleteProgram_(m_uID);
}

bool QYShader::Bind()
{
	if (!Ref())
	{
		LOGE_("@@QYShader::Bind, Shader(ID = %d) is being bound without being unbound. Bind count = %d", m_uID, m_uRefCount);
		return false;
	}
	QY_GL(glUseProgram(m_uID));
	return true;
}
bool QYShader::UnBind()
{
	if (!UnRef())
	{
		LOGE_("@@QYShader::UnBind, Shader(ID = %d) is being unbound without being bound.", m_uID);
		return false;
	}
	QY_GL(glUseProgram(0));
	return true;
}

void QYShader::SetVector3(const std::string& name, const glm::vec3& value) const
{
	QY_GL(glUniform3fv(glGetUniformLocation(m_uID, name.c_str()), 1, &value[0]));
}
void QYShader::SetVector3(const std::string& name, float x, float y, float z) const
{
	QY_GL(glUniform3f(glGetUniformLocation(m_uID, name.c_str()), x, y, z));
}

void QYShader::SetVector4(const std::string& name, const glm::vec4& value) const
{
	QY_GL(glUniform4fv(glGetUniformLocation(m_uID, name.c_str()), 1, &value[0]));
}
void QYShader::SetVector4(const std::string& name, float x, float y, float z, float w)
{
	QY_GL(glUniform4f(glGetUniformLocation(m_uID, name.c_str()), x, y, z, w));
}

void QYShader::SetMatrix4(const std::string& name, const glm::mat4& mat) const
{
	QY_GL(glUniformMatrix4fv(glGetUniformLocation(m_uID, name.c_str()), 1, GL_FALSE, &mat[0][0]));
}

void QYShader::SetMat4fv(const std::string& name, unsigned int count, const float *pData)
{
	QY_GL(glUniformMatrix4fv(glGetUniformLocation(m_uID, name.c_str()), count, GL_FALSE, pData));
}