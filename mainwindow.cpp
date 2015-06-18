#include <QMenuBar>
#include <QVBoxLayout>
#include <QPushButton>
#include "MainWindow.h"
#include "RTRViewer.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	menuBar = new QMenuBar();
	setMenuBar(menuBar);

	menuDraw = new QMenu("Draw");
	menuBar->addMenu(menuDraw);

	actionDrawLine = new QAction("DrawLine",menuDraw);
	menuDraw->addAction(actionDrawLine);

	graphicsWidgetLayout = new QVBoxLayout();
	setCentralWidget(new QWidget());
	centralWidget()->setLayout(graphicsWidgetLayout);
	viewerWidget = new RTRViewer();
	graphicsWidgetLayout->addWidget(viewerWidget);
	changePaintModeButton = new QPushButton("Switch mode");
	//connect(changePaintModeButton,SIGNAL(clicked()),graphicsWidget,SLOT(switchMode()));
	graphicsWidgetLayout->addWidget(changePaintModeButton);
}

MainWindow::~MainWindow()
{

}
