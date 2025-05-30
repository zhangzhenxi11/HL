#ifndef _XLH_FORTREND_DATA_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_DATA_SUBSYSTEM_INCLUDE_

#include <QWidget>

#include <QWebEngineView>
#include <QtWebEngine/QtWebEngine>

#include <QWebEngineSettings>
namespace FC {
class DataWidget;


class DataWidgetPrivate;
class DataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataWidget(QWidget *parent = 0);
    ~DataWidget();
public:
    void resizeEvent(QResizeEvent *event);
    void httpUpdate(const QList<QString> &name, const QList<int> &data);

    QList<int> httpdata;//单个线条的值
    QList<QString> lineName;//线条名称
private:
    QList<QString> httpname;//x 时间轴


public slots:
    void onclick();



private:
	Q_DECLARE_PRIVATE(DataWidget)
	DataWidgetPrivate *d_ptr;
};

}
#endif // WIDGET_H

