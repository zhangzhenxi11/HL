/**
* @file            fortrend_data_subsystem_widget.h
* @brief           fortrend_data_subsystem_widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: Data

#ifndef _XLH_FORTREND_DATA_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_DATA_SUBSYSTEM_INCLUDE_

#include <QWidget>
#include <memory>
#include <string>

#include <QWebEngineView>
#include <QtWebEngine/QtWebEngine>
#include <QWebEngineSettings>

namespace FC {

class IKernel;
class DataWidgetPrivate;
class DataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataWidget(const std::shared_ptr<IKernel>& kernel, QWidget *parent = 0);
    ~DataWidget();
public:
    void resizeEvent(QResizeEvent *event);
    // Modified to support multiple charts/views
    void httpUpdate(QWebEngineView* view, const QList<QString> &name, 
                    const QList<double> &dataZ, const QList<double> &dataR,
                    const QString& zName, const QString& rName);

public slots:
    void onclick();
    void onSimulateTest();
    void onCycleStart(const std::string& pmName);
    void onCycleStop();

private:
	Q_DECLARE_PRIVATE(DataWidget)
	DataWidgetPrivate *d_ptr;
};

}
#endif // WIDGET_H
