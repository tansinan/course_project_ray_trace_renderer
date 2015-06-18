#ifndef RTRVIEWER_H
#define RTRVIEWER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

class RTRModel;

class RTRViewer : public QWidget
{
	Q_OBJECT
protected:
	RTRModel* model;
	QHBoxLayout* mainLayout;
	QVBoxLayout* renderParamaterLayout;
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent * event);
	QImage* renderResult;
public:
	explicit RTRViewer(QWidget *parent = 0);
	~RTRViewer();
signals:
public slots:
};

#endif // RTRVIEWER_H
