#include <QMenuBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QIntValidator>
#include "MainWindow.h"
#include "RTRViewer.h"
#include "RTRRenderer.h"

MainWindow::MainWindow(QWidget *parent)
    :renderResult(640, 480, QImage::Format_ARGB32)
	,renderer(&renderResult)
	,QMainWindow(parent)
{

	mainLayout = new QVBoxLayout();
	setCentralWidget(new QWidget());
	centralWidget()->setLayout(mainLayout);

	renderLayout = new QHBoxLayout();

	labelFile = new QLabel(".obj Model File Name:");
	renderLayout->addWidget(labelFile);
	editFile = new QLineEdit();
	renderLayout->addWidget(editFile);
	labelRenderPass = new QLabel("Passes to render:");
	renderLayout->addWidget(labelRenderPass);
	editRenderPass = new QLineEdit();
	editRenderPass->setMaxLength(6);
	renderLayout->addWidget(editRenderPass);
	renderButton = new QPushButton("Render");
	renderLayout->addWidget(renderButton);
	mainLayout->addLayout(renderLayout);

	saveLayout = new QHBoxLayout();
	labelSaveFile = new QLabel("Save Result To : ");
	saveLayout->addWidget(labelSaveFile);
	editSaveFile = new QLineEdit();
	saveLayout->addWidget(editSaveFile);
	saveButton = new QPushButton("Save");
	saveButton->setEnabled(false);
	saveLayout->addWidget(saveButton);
	mainLayout->addLayout(saveLayout);

	editRenderPass->setText("10");
	editRenderPass->setValidator(new QIntValidator(1, 10000));
	viewerWidget = new RTRViewer(this, &renderer);
	mainLayout->addWidget(viewerWidget);
	changePaintModeButton = new QPushButton("...");
	connect(renderButton, SIGNAL(clicked()), this, SLOT(onRenderButtonPressed()));
	connect(saveButton,SIGNAL(clicked()),this,SLOT(saveImage()));
	connect(&renderer,SIGNAL(renderStatusChanged()),this,SLOT(onRenderStatusChanged()));
	connect(&renderer, SIGNAL(renderFinished()),this,SLOT(onRenderFinished()));
	mainLayout->addWidget(changePaintModeButton);
}

void MainWindow::onRenderStatusChanged()
{
	viewerWidget->updateDisplay();
	changePaintModeButton->setText(QString("%1 / %2 pass finished").arg(renderer.currentPass).
		arg(renderer.targetPass));
}

void MainWindow::onRenderFinished()
{
	viewerWidget->updateDisplay(true);
	QMessageBox::information(this, "Message","Render Finished.\n Due to memory leak bug, please restart to render another image.\n"
											 "You can now save image file to wherever you want.");
	renderButton->setEnabled(false);
	saveButton->setEnabled(true);
}

void MainWindow::onRenderButtonPressed()
{
	if (!model.loadModelFromObjFile(editFile->text()))
	{
		QMessageBox::warning(this, "Warning", QString(
								 "Fail to open model file:\n%1").arg(editFile->text()));
		return;
	}
	bool invalidMaterial = false;
	if (model.materialLibrary.find("@@world") == model.materialLibrary.end())
	{
		invalidMaterial = true;
	}
	else
	{
		RTRMaterial* worldMaterial = model.materialLibrary["@@world"];
		if (worldMaterial->getPropertyType("camera_position") == RTRMaterial::TYPE_COLOR)
		{
			RTRColor temp = worldMaterial->getColorAt("camera_position", 0, 0);
			camera.cameraPosition = RTRVector3D(temp.r(), temp.g(), temp.b());
		}
		else invalidMaterial = true;
		if (worldMaterial->getPropertyType("camera_angle") == RTRMaterial::TYPE_COLOR)
		{
			RTRColor temp = worldMaterial->getColorAt("camera_angle", 0, 0);
			camera.cameraAngle = RTRVector3D(temp.r(), temp.g(), temp.b());
		}
		else invalidMaterial = true;
		if (worldMaterial->getPropertyType("image_distance") == RTRMaterial::TYPE_COLOR)
		{
			camera.imageDistance = worldMaterial->getColorAt("image_distance", 0, 0).r();
		}
		else invalidMaterial = true;
        camera.offset.x() = 320;
        camera.offset.y() = 240;
        camera.imageDistance = camera.offset.x() / qTan(qDegreesToRadians(110.0 / 2));
		camera.evaluateRotationMatrix();
	}
	if(invalidMaterial)
	{
		QMessageBox::warning(this, "Warning", QString(
							 "Model File:%1 is not valid.\n This renderer requires mtl files to be modified to include camera info.\n"
							 "See the documentation for more detailed instructions.").arg(editFile->text()));
		return;
	}
	QMessageBox::information(this, "Message","Model file load succeeded.");
	renderer.render(&model, &camera, editRenderPass->text().toInt());
	renderButton->setEnabled(false);
}

MainWindow::~MainWindow()
{

}

void MainWindow::saveImage()
{
    if(renderer.image->save(editSaveFile->text()))
    {
        QMessageBox::information(this, "Message","File save succeeded!");
    }
    else
    {
        QMessageBox::warning(this, "Warning","File save failed!");
    }
}
