#include "RTRModel.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QImage>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include "RTRGeometry.h"
#include "RTRTexture.h"

RTRModel::RTRModel()
{

}

RTRModel::~RTRModel()
{

}

bool RTRModel::loadModelFromObjFile(const QString& filePath)
{
	QFileInfo info(filePath);
	modelPath = info.dir().path();
	QFile objModelFile(filePath);
	if(!objModelFile.open(QIODevice::ReadOnly))
	{
		return false;
	}
	QTextStream objModelStream(&objModelFile);
	QString currentObjectName = "";
	QString currentGroupName = "";
	QString materialName = "";
	while(!objModelStream.atEnd())
	{
		QString line = objModelStream.readLine().trimmed();
		if (line.isEmpty()) continue;
		if(line[0]=='#')
		{
			qDebug() << "Ignoring Comment Line:" << line;
			continue;
		}
		QStringList param = line.split(' ', QString::SkipEmptyParts);
		QString command = param[0];
		command.toLower();
		param.removeFirst();
		if (command == "mtllib")
		{
			loadMaterialLibraryFromMtlFile(modelPath + "/" + param[0]);
		}
		else if (command == "usemtl")
		{
			materialName = param[0];
		}
		else if (command == "o")
		{
			currentObjectName = param[0];
			//qDebug() << "Ignoring Unsupported Feature : Object" << objectName;
		}
		else if (command == "g")
		{
			currentGroupName = param[0];
		}
		else if (command == "s")
		{
			//TODO : Smooth Shading Support 
		}
		else if (command == "v")
		{
			vertexPositions.append(RTRVector3D(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
		}
		else if (command == "vn")
		{
			vertexNormals.append(RTRVector3D(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
			vertexNormals.back().vectorNormalize();
		}
		else if (command == "vt")
		{
			vertexUVPositions.append(RTRVector2D(param[0].toDouble(), param[1].toDouble()));
		}
		else if (command == "f")
		{
			RTRFace face;
			face.objectName = currentObjectName;
			face.groupName = currentGroupName;
			face.materialName = materialName;
			for (int i = 0; i < param.size(); i++)
			{
				QStringList list = param[i].split("/");
				switch (list.size())
				{
				case 1:
					face.addVertex(list[0].toInt());
					break;
				case 2:
					face.addVertex(list[0].toInt());
					face.uvCoordinates.append(list[1].toInt());
					break;
				case 3:
					face.addVertex(list[0].toInt());
					if(!list[1].isEmpty()) face.uvCoordinates.append(list[1].toInt());
					face.normals.append(list[2].toInt());
					break;
				default:
					break;
				}
			}
			faces.append(face);
		}
		else
		{
			qDebug() << "Ignoring Unsupported Line : " << command << param;
		}
	}
	objModelFile.close();
	return true;
}

bool RTRModel::loadMaterialLibraryFromMtlFile(const QString& filePath)
{
	QFile mtlFile(filePath);
	if (!mtlFile.open(QIODevice::ReadOnly))
	{
		return false;
	}
	QTextStream mtlFileStream(&mtlFile);
	QString currentMaterialName;
	RTRMaterial* currentMaterial = NULL;
	while (!mtlFileStream.atEnd())
	{
		QString line = mtlFileStream.readLine().trimmed();
		if (line.isEmpty()) continue;
		if (line[0] == '#')
		{
			qDebug() << "Ignoring Comment Line:" << line;
			continue;
		}
		QStringList param = line.split(' ', QString::SkipEmptyParts);
		QString command = param[0];
		command = command.toLower();
		param.removeFirst();
		if (command == "newmtl")
		{
			if (currentMaterial != NULL)
			{
				materialLibrary.insert(currentMaterialName, currentMaterial);
			}
			currentMaterialName = param[0];
			currentMaterial = new RTRMaterial();
		}
		else
		{
			if (currentMaterial == NULL)
			{
				mtlFile.close();
				return false;
			}
			if (command == "kd")
				currentMaterial->setColorProperty("diffuse", RTRColor(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
			else if (command == "map_kd")
				currentMaterial->setTextureProperty("diffuse", modelPath + "/" + param[0]);
		}
	}
	if (currentMaterial != NULL)
	{
		materialLibrary.insert(currentMaterialName, currentMaterial);
	}
	mtlFile.close();
	return true;
}