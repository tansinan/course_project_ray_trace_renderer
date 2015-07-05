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

RTRModelVertex* RTRModel::addVertex(const RTRVector3D& coordinate)
{
	RTRModelVertex* vertex = new RTRModelVertex();
	vertex->position = coordinate;
	vertices.insert(vertex);
	return vertex;
}

RTRModelVertex* RTRModel::addVertex(double _x, double _y, double _z)
{
	return addVertex(RTRVector3D(_x, _y, _z));
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
	QVector<RTRModelVertex*> tempVertexPositions;
	QVector<RTRVector3D> tempVertexNormals;
	QVector<RTRVector2D> tempVertexUVPositions;
	bool currentSmoothShading = true;
	while(!objModelStream.atEnd())
	{
		//Read a line, spaces and begin and end are trimmed
		QString line = objModelStream.readLine().trimmed();

		//Ignore empty lines and comment lines
		if (line.isEmpty()) continue;
		if (line[0] == '#') continue;


		QStringList param = line.split(' ', QString::SkipEmptyParts);
		QString command = param[0];
		command.toLower();
		param.removeFirst();
		if (command == "mtllib") loadMaterialLibraryFromMtlFile(modelPath + "/" + param[0]);
		else if (command == "usemtl") materialName = param[0];
		else if (command == "o") currentObjectName = param[0];
		else if (command == "g")
		{
			currentGroupName = param[0];
		}
		else if (command == "s")
		{
			if (param[0] == "off") currentSmoothShading = false;
			else currentSmoothShading = true;
		}
		else if (command == "v")
		{
			tempVertexPositions.append(
				addVertex(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
		}
		else if (command == "vn")
		{
			tempVertexNormals.append(RTRVector3D(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
			tempVertexNormals.back().vectorNormalize();
		}
		else if (command == "vt")
		{
			tempVertexUVPositions.append(RTRVector2D(param[0].toDouble(), param[1].toDouble()));
		}
		else if (command == "f")
		{
			RTRModelPolygen* poly = new RTRModelPolygen();
			poly->objectName = currentObjectName;
			poly->groupName = currentGroupName;
			poly->materialName = materialName;
			poly->smoothShading = currentSmoothShading;
			for (int i = 0; i < param.size(); i++)
			{
				QStringList list = param[i].split("/");
				switch (list.size())
				{
				case 1:
					poly->vertices.append(tempVertexPositions[list[0].toInt() - 1]);
					break;
				case 2:
					poly->vertices.append(tempVertexPositions[list[0].toInt() - 1]);
					poly->uvMaps.append(tempVertexUVPositions[list[1].toInt() - 1]);
					break;
				case 3:
					poly->vertices.append(tempVertexPositions[list[0].toInt() - 1]);
					if (!list[1].isEmpty()) poly->uvMaps.append(tempVertexUVPositions[list[1].toInt() - 1]);
					poly->normals.append(tempVertexNormals[list[2].toInt() - 1]);
					break;
				default:
					break;
				}
			}
			polygens.insert(poly);
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
			else if (command == "ka")
				currentMaterial->setColorProperty("ambient", RTRColor(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
			else if (command == "ks")
				currentMaterial->setColorProperty("specular", RTRColor(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
			else if (command == "reflection_rate")
				currentMaterial->setColorProperty("reflection_rate", RTRColor(param[0].toDouble(), param[0].toDouble(), param[0].toDouble()));
			else if (command == "reflection_color")
				currentMaterial->setColorProperty("reflection_color", RTRColor(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
			else if (command == "reflection_glossiness")
				currentMaterial->setColorProperty("reflection_glossiness", RTRColor(param[0].toDouble(), param[0].toDouble(), param[0].toDouble()));
			else if (command == "refraction_rate")
				currentMaterial->setColorProperty("refraction_rate", RTRColor(param[0].toDouble(), param[0].toDouble(), param[0].toDouble()));
			else if (command == "refraction_index")
				currentMaterial->setColorProperty("refraction_index", RTRColor(param[0].toDouble(), param[0].toDouble(), param[0].toDouble()));
			else if (command == "refraction_color")
				currentMaterial->setColorProperty("refraction_color", RTRColor(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
			else if (command == "refraction_glossiness")
				currentMaterial->setColorProperty("refraction_glossiness", RTRColor(param[0].toDouble(), param[0].toDouble(), param[0].toDouble()));
			else if (command == "fod_aperture")
				currentMaterial->setColorProperty("fod_aperture", RTRColor(param[0].toDouble(), param[0].toDouble(), param[0].toDouble()));
			else if (command == "fod_focus")
				currentMaterial->setColorProperty("fod_focus", RTRColor(param[0].toDouble(), param[0].toDouble(), param[0].toDouble()));
		}
	}
	if (currentMaterial != NULL)
	{
		materialLibrary.insert(currentMaterialName, currentMaterial);
	}
	mtlFile.close();
	return true;
}

bool RTRModel::saveModelToObjFile(const QString& filePath)
{
	return false;
	/*QFile objOutputFile(filePath);
	if (!objOutputFile.open(QIODevice::WriteOnly))
	{
		return false;
	}
	QTextStream objOutputStream(&objOutputFile);
	for (int i = 0; i < vertexPositions.size(); i++)
	{
		objOutputStream << "v " << vertexPositions[i].x() << " " << vertexPositions[i].y() << " " << vertexPositions[i].z() << "\n";
	}
	for (int i = 0; i < faces.size(); i++)
	{
		objOutputStream << "f ";
		for (int j = 0; j < faces[i].vertices.size(); j++)
		{
			objOutputStream << faces[i].vertices[j] << " ";
		}
		objOutputStream << "\n";
		//<< faces[i].vertices[]
	}
	objOutputFile.close();
	return false;*/
}

RTRModelPolygen* RTRModel::addPolygen(const QVector<RTRModelVertex*>& vertices)
{
	RTRModelPolygen* poly = new RTRModelPolygen();
	poly->vertices = vertices;
	return poly;
}

RTRModelPolygen* RTRModel::addPolygen(const QVector<RTRModelVertex*>& vertices, const QVector<RTRVector2D>& uvMaps)
{
	RTRModelPolygen* poly = new RTRModelPolygen();
	poly->vertices = vertices;
	poly->uvMaps = uvMaps;
	return poly;
}