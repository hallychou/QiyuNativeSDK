/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/
#include <GLES3/gl3.h>
#include "QYMesh.h"
#include "QYUtil.h"
#include "QYShader.h"

QYMesh::QYMesh(const aiMatrix4x4& matParents_, const char* szName, std::vector<QYVertex> vecVert, std::vector<unsigned int> vecIndex, std::vector<QYTexture> vecTex)
{
	m_matParents_ = matParents_;

	m_vecVert = vecVert;
	m_vecIndex = vecIndex;
	m_vecTex = vecTex;

	m_strName = szName;
	m_eButtonType = BT_None;
	if (m_strName.compare(meshName_Trigger) == 0)
		m_eButtonType = BT_Trigger;
	else if (m_strName.compare(meshName_Grip) == 0)
		m_eButtonType = BT_Grip;
	else if (m_strName.compare(meshName_Home) == 0)
		m_eButtonType = BT_Home_Menu;
	else if (m_strName.compare(meshName_AX) == 0)
		m_eButtonType = BT_A_X;
	else if (m_strName.compare(meshName_BY) == 0)
		m_eButtonType = BT_B_Y;
	else if (m_strName.compare(meshName_JoyStick) == 0)
		m_eButtonType = BT_JoyStick;
	else if (m_strName.compare(meshName_MainBody) == 0)
	{
	}
	else
	{
		m_eButtonType = BT_None;
	}

	setupMesh();
}

QYMesh::~QYMesh()
{
	//Release();
}

void QYMesh::Release()
{
	for (int i = 0; i < m_vecTex.size(); ++i)
	{
		QY_GL(glDeleteTextures(1, &m_vecTex[i].id));
	}
	QY_GL(glDeleteBuffers(1, &m_EBO));
	QY_GL(glDeleteBuffers(1, &m_VBO));
	QY_GL(glDeleteVertexArrays(1, &m_VAO));
	m_VAO = GL_NONE;
	m_EBO = GL_NONE;
	m_VBO = GL_NONE;
}

void QYMesh::setupMesh()
{
	QY_GL(glGenVertexArrays(1, &m_VAO));
	QY_GL(glGenBuffers(1, &m_VBO));
	QY_GL(glGenBuffers(1, &m_EBO));

	QY_GL(glBindVertexArray(m_VAO));

	QY_GL(glBindBuffer(GL_ARRAY_BUFFER, m_VBO));

	QY_GL(glBufferData(GL_ARRAY_BUFFER, m_vecVert.size() * sizeof(QYVertex), &m_vecVert[0], GL_STATIC_DRAW));

	QY_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO));
	QY_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vecIndex.size() * sizeof(unsigned int), &m_vecIndex[0], GL_STATIC_DRAW));


	QY_GL(glEnableVertexAttribArray(0));
	QY_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QYVertex), (void*)0));

	QY_GL(glEnableVertexAttribArray(1));
	QY_GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QYVertex), (void*)offsetof(QYVertex, normal)));

	QY_GL(glEnableVertexAttribArray(2));
	QY_GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QYVertex), (void*)offsetof(QYVertex, texCoords)));

	QY_GL(glEnableVertexAttribArray(3));
	QY_GL(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(QYVertex), (void*)offsetof(QYVertex, tangent)));

	QY_GL(glEnableVertexAttribArray(4));
	QY_GL(glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(QYVertex), (void*)offsetof(QYVertex, bitangent)));

	QY_GL(glBindVertexArray(0));
}

void QYMesh::DrawMesh(QYShader* pShader)
{
	unsigned int diffuseTex = 1;
	unsigned int specularTex = 1;
	unsigned int normalTex = 1;
	unsigned int heightTex = 1;
	for (unsigned int i = 0; i < m_vecTex.size(); i++)
	{
		QY_GL(glActiveTexture(GL_TEXTURE0 + i));

		std::string number;
		std::string name = m_vecTex[i].type;
		if (name == "texture_diffuse")
		{
			number = std::to_string(diffuseTex++);
		}
		else if (name == "texture_specular")
		{
			number = std::to_string(specularTex++);
		}
		else if (name == "texture_normal")
		{
			number = std::to_string(normalTex++);
		}
		else if (name == "texture_height")
		{
			number = std::to_string(heightTex++);
		}

		QY_GL(glUniform1i(glGetUniformLocation(pShader->GetID(), (name + number).c_str()), i));

		QY_GL(glBindTexture(GL_TEXTURE_2D, m_vecTex[i].id));
	}

	QY_GL(glBindVertexArray(m_VAO));
	QY_GL(glDrawElements(GL_TRIANGLES, m_vecIndex.size(), GL_UNSIGNED_INT, 0));
	QY_GL(glBindVertexArray(0));

	QY_GL(glActiveTexture(GL_TEXTURE0));
}
