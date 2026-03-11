#ifndef _XLH_FORTREND_DATAHISTORY_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_DATAHISTORY_SUBSYSTEM_INCLUDE_

#include <QWidget>
#include <QVector>

class QCustomPlot;

namespace FC {
class DataHistoryWidget;


class DataHistoryWidgetPrivate;
class DataHistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataHistoryWidget(QWidget *parent = 0);
    ~DataHistoryWidget();
    
protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    void updateHistoryChart(const QVector<double> &timeKeys,
                            const QVector<double> &velZ, const QVector<double> &velR,
                            const QVector<double> &posZ, const QVector<double> &posR);

public slots:
    void onQueryClicked();
    void populatePMChambers(int index);
    void populatePMChambers();

private:
	Q_DECLARE_PRIVATE(DataHistoryWidget)
	DataHistoryWidgetPrivate *d_ptr;
};

}
#endif // WIDGET_H
