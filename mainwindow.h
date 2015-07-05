#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QVBoxLayout;
class RTRViewer;
class QPushButton;

class MainWindow : public QMainWindow
{
	Q_OBJECT
protected:
	QMenuBar* menuBar;
	QMenu* menuDraw;
	QAction* actionDrawLine;
	QAction* actionDrawEllipse;


	QVBoxLayout* graphicsWidgetLayout;
	QPushButton* changePaintModeButton;
	RTRViewer* viewerWidget;
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
public slots:
	void onRenderStatusChanged();

};

#endif // MAINWINDOW_H
