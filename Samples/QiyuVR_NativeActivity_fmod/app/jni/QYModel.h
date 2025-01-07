/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/
#ifndef _QYMODEL_H_
#define _QYMODEL_H_

#include "QYMesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

struct qiyu_ControllerData;

class QYModel
{
public:
	QYModel(const std::string& strPath);
	~QYModel();
	void Release();

	const glm::vec3& GetRayStartWorldPos() const { return m_vRayStartWldPos; }
	const glm::vec3& GetRayWorldDir() const { return m_vRayWldDir; }

	void DrawHandleLR(bool bMultiView, float g_fTrackingOffset_, const glm::mat4& matProj, /*const */glm::mat4& matView_, QYShader* pShader, const qiyu_ControllerData& ctrlData, bool bIsHandleR);

	void DrawTeapot(QYShader* pShader);

	inline bool HasTextures() const { return m_bHasTexture; }

private:
	bool LoadModel(const std::string& strPath);

	void ProcessNode(const aiMatrix4x4& matParents_, const aiNode* aiNode_, const aiScene* aiScene_);

	void UpdateMinMaxXYZ(glm::vec3 pos);

	QYMesh ProcessMesh(const aiMatrix4x4& matParents_, const aiMesh* aiMesh_, const aiScene* aiScene_);

	std::vector<QYTexture> LoadMaterialTextures(const aiMaterial* pMtl, aiTextureType texType, const std::string& strTypeName);

	unsigned int LoadTexture(const char* szPath, const std::string& strDir);

private:
	std::vector<QYTexture> 	m_vecLoadedTex;
	std::vector<QYMesh> 	m_vecMesh;
	std::string 			m_strDir;
	glm::vec3 				m_maxXYZ;
	glm::vec3 				m_minXYZ;
	bool 					m_bHasTexture;
	glm::vec3 				m_vRayStartWldPos;
	glm::vec3 				m_vRayWldDir;
};


#endif//_QYMODEL_H_