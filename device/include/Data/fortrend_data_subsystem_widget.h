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
#include <QVector>

class QCustomPlot;

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
    void updateChart(QCustomPlot* plot, const QVector<double> &timeKeys, 
                     const QVector<double> &dataZ, const QVector<double> &dataR);

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
