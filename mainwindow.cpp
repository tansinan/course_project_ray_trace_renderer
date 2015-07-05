#include <QMenuBar>
#include <QVBoxLayout>
#include <QPushButton>
#include "MainWindow.h"
#include "RTRViewer.h"
#include "RTRRenderer.h"

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
	viewerWidget = new RTRViewer(this);
	graphicsWidgetLayout->addWidget(viewerWidget);
	changePaintModeButton = new QPushButton("Switch mode");
	//connect(changePaintModeButton,SIGNAL(clicked()),graphicsWidget,SLOT(switchMode()));
	graphicsWidgetLayout->addWidget(changePaintModeButton);
}

void MainWindow::onRenderStatusChanged()
{
	changePaintModeButton->setText(QString("%1 / %2 pass finished").arg(viewerWidget->getRenderer()->currentPass).
		arg(viewerWidget->getRenderer()->targetPass));
}

MainWindow::~MainWindow()
{

}
