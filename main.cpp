#include "MainWindow.h"
#include <QApplication>
#include <RTRModel.h>
#include <QDebug>
#include "RTRMatrix.h"
#include <QtGlobal>
#include "RTRGeometry.h"
#include <ctime>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	qsrand(time(0)^clock());
	return a.exec();
}
