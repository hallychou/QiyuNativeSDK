/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/

#include "QYModel.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "QYShader.h"
#include "opencv2/opencv.hpp"
#include "QiyuApi.h"
#include "QYUtil.h"

QYModel::QYModel(const std::string& strPath)
	: m_bHasTexture(false)
{
	bool b = LoadModel(strPath);
}

QYModel::~QYModel()
{
	Release();
}

void QYModel::DrawHandleLR(bool bMultiView, float g_fTrackingOffset_, const glm::mat4& matProj, /*const */glm::mat4& matView_, QYShader* pShader, const qiyu_ControllerData& ctrlData, bool bIsHandleR)
{
	const qiyu_Vector3& pos = ctrlData.position;
	const qiyu_Quaternion& rot = ctrlData.rotation;
	int button_ = ctrlData.button;
	int buttonTouch_ = ctrlData.buttonTouch;
	// int batteryLevel_ = ctrlData.batteryLevel;
	float triggerForce_ = ctrlData.triggerForce;
	float gripForce_ = ctrlData.gripForce;
	const qiyu_Vector2& joyStickPos_ = ctrlData.joyStickPos;
	//
	static float fix_fScale = 0.001f;
	static glm::mat4 fix_matScale = glm::scale(glm::mat4(1.0f), glm::vec3(fix_fScale));
	static glm::mat4 fix_matRotY_Scale = fix_matScale;
	static float s_fABXY_TransY = -0.005f;
	static glm::mat4 s_matABXY_TransY = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, s_fABXY_TransY, 0.0f));
	static float s_fGrip_TransX = 0.004f;
	static float s_fTrigger_RotXdegree = -10.0f;
	static float s_fJoystick_RotXdegree = 45.0f;
	static float s_fJoystick_RotZdegree = 45.0f;
	//
	glm::fquat quat = glm::fquat(rot.w, rot.x, rot.y, rot.z);// glm quat is (w)(xyz)
	glm::mat4 matRotate = glm::mat4_cast(quat);
	float pos_y = 0.0f;
	qiyu_TrackingOriginMode trackMode = qiyu_GetTrackingOriginMode();
	switch (trackMode)
	{
	case qiyu_TrackingOriginMode::TM_Device:
		pos_y = pos.y;
		break;
	case qiyu_TrackingOriginMode::TM_Ground:
		pos_y = pos.y - g_fTrackingOffset_;//!
		break;
	default:
		assert(0 && "ILLEGAL qiyu_TrackingOriginMode");
		break;
	}
	glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos_y, pos.z));
	glm::mat4 matWorld = matTranslate * matRotate * fix_matRotY_Scale;

	pShader->SetMatrix4("u_matModel", matWorld);
	if (bMultiView)
	{
    	pShader->SetMat4fv("u_matView[0]", 2, glm::value_ptr(matView_[0]));//
	}
	else
	{
		pShader->SetMatrix4("u_matView", matView_);
	}
	pShader->SetMatrix4("u_matProj", matProj);
	pShader->SetVector3("u_lightPos", glm::vec3(0.0f, 0.0f, 0.0f));
	pShader->SetVector3("u_clrLight", glm::vec3(1.0f, 1.0f, 1.0f));
	pShader->SetVector3("u_viewPos", glm::vec3(0, 0, 0));
	pShader->SetVector3("u_clrPressBtn", glm::vec3(1.0f, 1.0f, 1.0f));


	glm::vec3 localMinCenter(m_minXYZ.x + (m_maxXYZ.x - m_minXYZ.x) / 2.0f, m_minXYZ.y, m_minXYZ.z);
	glm::vec3 localMaxCenter(m_minXYZ.x + (m_maxXYZ.x - m_minXYZ.x) / 2.0f, m_maxXYZ.y, m_maxXYZ.z);
	glm::vec3 localRayPos(m_minXYZ.x + (m_maxXYZ.x - m_minXYZ.x) / 2.0f, m_minXYZ.y, m_minXYZ.z);
	glm::vec4 worldMinCenter = matWorld * glm::vec4(localMinCenter, 1.0);
	glm::vec4 worldMaxCenter = matWorld * glm::vec4(localMaxCenter, 1.0);
	glm::vec4 worldRayPos = matWorld * glm::vec4(localRayPos, 1.0);
	glm::vec4 worldRayDir = glm::normalize(worldMinCenter - worldMaxCenter);
	m_vRayStartWldPos = glm::vec3(worldRayPos.x, worldRayPos.y, worldRayPos.z);
	m_vRayWldDir = glm::vec3(worldRayDir.x, worldRayDir.y, worldRayDir.z);

	bool bResetColor = false;
	bool bResetMatrix = false;
	for (unsigned int i = 0; i < m_vecMesh.size(); i++)
	{
		QYMesh& curMesh = m_vecMesh[i];

		bResetColor = false;
		bResetMatrix = false;
		bool b_button = button_ & curMesh.m_eButtonType;
		bool b_buttonTouch = buttonTouch_ & curMesh.m_eButtonType;
		if (b_buttonTouch)
		{
			bResetColor = true;
			pShader->SetVector3("u_clrPressBtn", glm::vec3(1.0f, 2.0f, 1.0f));
		}
		if (b_button)
		{
			bResetColor = true;
			bResetMatrix = true;
			pShader->SetVector3("u_clrPressBtn", glm::vec3(1.0f, 1.0f, 2.0f));

			glm::mat4 wld = matTranslate * matRotate * s_matABXY_TransY * fix_matRotY_Scale;
			pShader->SetMatrix4("u_matModel", wld);
		}
		switch (curMesh.m_eButtonType)
		{
		case BT_None:
		{
		}
		break;
		case BT_Trigger:
		{
			if (triggerForce_ > 0.0f)
			{
				bResetColor = true;
				bResetMatrix = true;
				pShader->SetVector3("u_clrPressBtn", glm::vec3(2.0f, 1.0f, 1.0f));

				glm::mat4 matTrigger_RotX = glm::rotate(glm::mat4(1.0f), glm::radians(triggerForce_ * s_fTrigger_RotXdegree), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::mat4 wld = matTranslate * matRotate * matTrigger_RotX * fix_matRotY_Scale;
				pShader->SetMatrix4("u_matModel", wld);
			}
		}
		break;
		case BT_Grip:
		{
			if (gripForce_ > 0.0f)
			{
				bResetColor = true;
				bResetMatrix = true;
				pShader->SetVector3("u_clrPressBtn", glm::vec3(2.0f, 1.0f, 1.0f));

				glm::mat4 matPressGrip = glm::translate(glm::mat4(1.0f), glm::vec3((bIsHandleR ? gripForce_ : -gripForce_) * s_fGrip_TransX, 0.0f, 0.0f));
				glm::mat4 wld = matTranslate * matRotate * matPressGrip * fix_matRotY_Scale;
				pShader->SetMatrix4("u_matModel", wld);
			}
		}
		break;
		case BT_Home_Menu:
		{
		}
		break;
		case BT_A_X:
		{
		}
		break;
		case BT_B_Y:
		{
		}
		break;
		case BT_JoyStick:
		{
			if (joyStickPos_.x != 0.0f || joyStickPos_.y != 0.0f)
			{
				bResetColor = true;
				bResetMatrix = true;
				pShader->SetVector3("u_clrPressBtn", glm::vec3(2.0f, 1.0f, 1.0f));

				glm::mat4 matJoystick_Rot = glm::mat4(1.0f);

				if (joyStickPos_.x != 0.0f)
				{
					static glm::vec3 realAxisZ = glm::vec3(0.0f, 0.0f, 1.0f);
					glm::mat4 rotateFirst = glm::rotate(glm::mat4(1.0f), glm::radians(joyStickPos_.x * s_fJoystick_RotZdegree), realAxisZ);//matJoystick_RotZ
					glm::mat4 matParents_r = glm::mat4(curMesh.m_matParents_.a1, curMesh.m_matParents_.b1, curMesh.m_matParents_.c1, curMesh.m_matParents_.d1,
						curMesh.m_matParents_.a2, curMesh.m_matParents_.b2, curMesh.m_matParents_.c2, curMesh.m_matParents_.d2,
						curMesh.m_matParents_.a3, curMesh.m_matParents_.b3, curMesh.m_matParents_.c3, curMesh.m_matParents_.d3,
						curMesh.m_matParents_.a4, curMesh.m_matParents_.b4, curMesh.m_matParents_.c4, curMesh.m_matParents_.d4);//FIXME! temp
					glm::mat4 matParents_r2 = glm::inverse(matParents_r);
					rotateFirst = matParents_r * rotateFirst * matParents_r2;
					matJoystick_Rot *= rotateFirst;
				}

				if (joyStickPos_.y != 0.0f)
				{
					static glm::vec3 realAxisX = glm::vec3(1.0f, 0.0f, 0.0f);
					glm::mat4 rotateFirst = glm::rotate(glm::mat4(1.0f), glm::radians(joyStickPos_.y * s_fJoystick_RotXdegree), realAxisX);//matJoystick_RotX
					glm::mat4 matParents_r = glm::mat4(curMesh.m_matParents_.a1, curMesh.m_matParents_.b1, curMesh.m_matParents_.c1, curMesh.m_matParents_.d1,
						curMesh.m_matParents_.a2, curMesh.m_matParents_.b2, curMesh.m_matParents_.c2, curMesh.m_matParents_.d2,
						curMesh.m_matParents_.a3, curMesh.m_matParents_.b3, curMesh.m_matParents_.c3, curMesh.m_matParents_.d3,
						curMesh.m_matParents_.a4, curMesh.m_matParents_.b4, curMesh.m_matParents_.c4, curMesh.m_matParents_.d4);//FIXME! temp
					glm::mat4 matParents_r2 = glm::inverse(matParents_r);
					rotateFirst = matParents_r * rotateFirst * matParents_r2;
					matJoystick_Rot *= rotateFirst;
				}

				glm::mat4 wld = matTranslate * matRotate * fix_matRotY_Scale * matJoystick_Rot;
				pShader->SetMatrix4("u_matModel", wld);
			}
		}
		break;
		default:
		{
		}
		break;
		}

		curMesh.DrawMesh(pShader);

		//! reset
		if (bResetColor)
		{
			pShader->SetVector3("u_clrPressBtn", glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (bResetMatrix)
		{
			pShader->SetVector3("u_clrPressBtn", glm::vec3(1.0f, 1.0f, 1.0f));
			pShader->SetMatrix4("u_matModel", matWorld);
		}
	}
}

void QYModel::DrawTeapot(QYShader* pShader)
{
	for (unsigned int i = 0; i < m_vecMesh.size(); i++)
	{
		QYMesh& curMesh = m_vecMesh[i];
		curMesh.DrawMesh(pShader);
	}
}

void QYModel::Release()
{
	for (QYMesh &mesh : m_vecMesh)
	{
		mesh.Release();
	}
}

bool QYModel::LoadModel(const std::string& strPath)
{
	Assimp::Importer importer;
	const aiScene* aiScene_ = importer.ReadFile(strPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!aiScene_ || aiScene_->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene_->mRootNode)
	{
		return false;
	}
	m_strDir = strPath.substr(0, strPath.find_last_of('/'));
	ProcessNode(aiScene_->mRootNode->mTransformation, aiScene_->mRootNode, aiScene_);
	return true;
}

void QYModel::ProcessNode(const aiMatrix4x4& matParents_, const aiNode* aiNode_, const aiScene* aiScene_)
{
	for (unsigned int i = 0; i < aiNode_->mNumMeshes; i++)
	{
		const aiMesh* mesh = aiScene_->mMeshes[aiNode_->mMeshes[i]];
		if (mesh != nullptr)
		{
			m_vecMesh.push_back(ProcessMesh(matParents_, mesh, aiScene_));
		}
	}

	for (unsigned int i = 0; i < aiNode_->mNumChildren; i++)
	{
		ProcessNode(matParents_ * aiNode_->mChildren[i]->mTransformation, aiNode_->mChildren[i], aiScene_);
	}
}

void QYModel::UpdateMinMaxXYZ(glm::vec3 pos)
{
	m_maxXYZ.x = pos.x > m_maxXYZ.x ? pos.x : m_maxXYZ.x;
	m_maxXYZ.y = pos.y > m_maxXYZ.y ? pos.y : m_maxXYZ.y;
	m_maxXYZ.z = pos.z > m_maxXYZ.z ? pos.z : m_maxXYZ.z;

	m_minXYZ.x = pos.x < m_minXYZ.x ? pos.x : m_minXYZ.x;
	m_minXYZ.y = pos.y < m_minXYZ.y ? pos.y : m_minXYZ.y;
	m_minXYZ.z = pos.z < m_minXYZ.z ? pos.z : m_minXYZ.z;
}

QYMesh QYModel::ProcessMesh(const aiMatrix4x4& matParents_, const aiMesh* aiMesh_, const aiScene* aiScene_)
{
	std::vector<QYVertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<QYTexture> textures;

	for (unsigned int i = 0; i < aiMesh_->mNumVertices; i++)
	{
		QYVertex vertex;
		glm::vec3 vector;
		aiVector3D vector_, vector2_;

		vector.x = aiMesh_->mVertices[i].x;
		vector.y = aiMesh_->mVertices[i].y;
		vector.z = aiMesh_->mVertices[i].z;

		vector_ = aiVector3D(vector.x, vector.y, vector.z);
		vector2_ = matParents_ * vector_;
		vertex.position = glm::vec3(vector2_.x, vector2_.y, vector2_.z);

		UpdateMinMaxXYZ(vertex.position);

		vector.x = aiMesh_->mNormals[i].x;
		vector.y = aiMesh_->mNormals[i].y;
		vector.z = aiMesh_->mNormals[i].z;

		vector_ = aiVector3D(vector.x, vector.y, vector.z);
		vector2_ = matParents_ * vector_;
		vertex.normal = glm::vec3(vector2_.x, vector2_.y, vector2_.z);

		if (aiMesh_->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = aiMesh_->mTextureCoords[0][i].x;
			vec.y = aiMesh_->mTextureCoords[0][i].y;
			vertex.texCoords = vec;
		}
		else
		{
			vertex.texCoords = glm::vec2(0.0f, 0.0f);
		}

		vector.x = aiMesh_->mTangents[i].x;
		vector.y = aiMesh_->mTangents[i].y;
		vector.z = aiMesh_->mTangents[i].z;

		vector_ = aiVector3D(vector.x, vector.y, vector.z);
		vector2_ = matParents_ * vector_;
		vertex.tangent = glm::vec3(vector2_.x, vector2_.y, vector2_.z);

		vector.x = aiMesh_->mBitangents[i].x;
		vector.y = aiMesh_->mBitangents[i].y;
		vector.z = aiMesh_->mBitangents[i].z;

		vector_ = aiVector3D(vector.x, vector.y, vector.z);
		vector2_ = matParents_ * vector_;
		vertex.bitangent = glm::vec3(vector2_.x, vector2_.y, vector2_.z);

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < aiMesh_->mNumFaces; i++)
	{
		aiFace face = aiMesh_->mFaces[i];

		//if (face.mNumIndices != 3)
		//	continue;
		assert(face.mNumIndices == 3);
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	const aiMaterial* mtl = aiScene_->mMaterials[aiMesh_->mMaterialIndex];

	std::vector<QYTexture> diffuseMaps = LoadMaterialTextures(mtl, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

	std::vector<QYTexture> specularMaps = LoadMaterialTextures(mtl, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	std::vector<QYTexture> normalMaps = LoadMaterialTextures(mtl, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	std::vector<QYTexture> heightMaps = LoadMaterialTextures(mtl, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	return QYMesh(matParents_, aiMesh_->mName.C_Str(), vertices, indices, textures);
}

std::vector<QYTexture> QYModel::LoadMaterialTextures(const aiMaterial* pMtl, aiTextureType texType, const std::string& strTypeName)
{
	std::vector<QYTexture> textures;
	for (unsigned int i = 0; i < pMtl->GetTextureCount(texType); i++)
	{
		aiString str;
		//pMtl->GetTexture(texType, i, &str);//FIXME! temp
		str = "handle_D.png";//FIXME! temp
		bool skip = false;
		for (unsigned int j = 0; j < m_vecLoadedTex.size(); j++)
		{
			if (std::strcmp(m_vecLoadedTex[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(m_vecLoadedTex[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			QYTexture texture;
			texture.id = LoadTexture(str.C_Str(), this->m_strDir);
			texture.type = strTypeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			m_vecLoadedTex.push_back(texture);
		}
	}
	return textures;
}

unsigned int QYModel::LoadTexture(const char* szPath, const std::string& strDir)
{
	std::string filename(szPath);
	filename = strDir + '/' + filename;

	unsigned int textureID;
	QY_GL(glGenTextures(1, &textureID));

	int width, height, nrComponents;
	unsigned char *data = nullptr;

	cv::Mat textureImage = cv::imread(filename);
	if (!textureImage.empty())
	{
		m_bHasTexture = true;

		cv::cvtColor(textureImage, textureImage, CV_BGR2RGB);

		QY_GL(glBindTexture(GL_TEXTURE_2D, textureID));
		QY_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureImage.cols,
							textureImage.rows, 0, GL_RGB, GL_UNSIGNED_BYTE,
							textureImage.data));
		QY_GL(glGenerateMipmap(GL_TEXTURE_2D));
		QY_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		QY_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		QY_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
		QY_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
	}

	return textureID;
}
