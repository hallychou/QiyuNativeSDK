/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/
#ifndef _QYMESH_H_
#define _QYMESH_H_

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "assimp/matrix4x4.inl"
#include "QiyuApi.h"


//L_handle & R_handle
static const char* meshName_MainBody = "MainBody";
static const char* meshName_Trigger = "Trigger";
static const char* meshName_Grip = "Grip";
static const char* meshName_Home = "Home";
static const char* meshName_AX = "AX";
static const char* meshName_BY = "BY";
static const char* meshName_JoyStick = "JoyStick";

struct QYVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct QYTexture
{
	unsigned int 	id;
	std::string 	type;
	std::string 	path;
};

class QYShader;
class QYMesh
{
public:
	QYMesh(const aiMatrix4x4& matParents_, const char* szName, std::vector<QYVertex> vecVert, std::vector<unsigned int> vecIndex, std::vector<QYTexture> vecTex);
	~QYMesh();
	void Release();

	void DrawMesh(QYShader* pShader);

public:
	aiMatrix4x4 	m_matParents_;//FIXME! temp
	qiyu_ButtonType m_eButtonType;

private:
	void setupMesh();

private:
	unsigned int m_VBO;
	unsigned int m_EBO;
	unsigned int m_VAO;

	std::vector<QYVertex> 		m_vecVert;
	std::vector<unsigned int> 	m_vecIndex;
	std::vector<QYTexture> 		m_vecTex;

	std::string 				m_strName;
};


#endif//_QYMESH_H_