#ifndef _XLH_FORTREND_DATAHISTORY_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_DATAHISTORY_SUBSYSTEM_INCLUDE_

#include <QWidget>

#include <QWebEngineView>
#include <QtWebEngine/QtWebEngine>
#include <QWebEngineSettings>
#include <QHash>


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
    bool eventFilter(QObject *obj, QEvent *event) override;
    void httpUpdate(const QList<QString> &name, const QList<int> &data);
    void httpUpdateMultiple(const QList<QString> &timestamps, 
                           const QList<int> &dataZ, 
                           const QList<int> &dataR,
                           const QString &nameZ,
                           const QString &nameR);
    void httpUpdateMultipleSeries(const QList<QString> &timestamps,
                                 const QList<int> &velZ,
                                 const QList<int> &velR,
                                 const QList<int> &posZ,
                                 const QList<int> &posR);

	QHash<QString, QHash<QString,int>> Data;

    QList<int> httpdata;//单个线条的值
    QList<QString> lineName;//线条名称
private:
    QList<QString> httpname;//x 时间轴


public slots:
    void onclick();
	void onSelect();

    void onQueryClicked();

    void populatePMChambers(int index);

    void populatePMChambers();
private:
	Q_DECLARE_PRIVATE(DataHistoryWidget)
	DataHistoryWidgetPrivate *d_ptr;
};

}
#endif // WIDGET_H

