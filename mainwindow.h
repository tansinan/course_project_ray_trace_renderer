#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include "RTRRenderer.h"
#include "RTRCamera.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class RTRViewer;
class QPushButton;

class MainWindow : public QMainWindow
{
	Q_OBJECT
protected:
	QImage renderResult;
	QVBoxLayout* mainLayout;
	QHBoxLayout* renderLayout;
	QLabel* labelFile;
	QLineEdit* editFile;
	QLabel* labelRenderPass;
	QLineEdit* editRenderPass;
	QPushButton* renderButton;
	QPushButton* changePaintModeButton;
	QHBoxLayout* saveLayout;
	QLabel* labelSaveFile;
	QLineEdit* editSaveFile;
	QPushButton* saveButton;
	RTRViewer* viewerWidget;
	RTRModel model;
	RTRRenderer renderer;
	RTRCamera camera;
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
public slots:
	void onRenderStatusChanged();
	void onRenderFinished();
	void onRenderButtonPressed();
	void saveImage();
};

#endif // MAINWINDOW_H
