/**
 * @file            lp_rewind_widget.h
 * @brief           lp rewind widget
 * @author          TRAE
 */

#ifndef _LP_REWIND_WIDGET_INCLUDE_
#define _LP_REWIND_WIDGET_INCLUDE_

#include "Kernel/kernel.h"
#include "Kernel/FortrendUI/fortrend_ui_macros.h"
#include <QString>
#include <QWidget>
#include <memory>

namespace FC {

class Cassette;
class EFEMAsciiApi;
class EFEMWaferRobotSubsystem;
class QLPRewindWidgetPrivate;

class QLPRewindWidget : public QWidget
{
    Q_OBJECT
public:
    QLPRewindWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent = NULL);
    ~QLPRewindWidget();

private slots:
    void onModeChanged(int index);
    void onSourceLpChanged(int index);
    void onAddTableRow();
    void onRemoveTableRow();
    void onClearRecipe();
    void onSaveRecipe();
    void onLoadRecipe();
    void onPreviewRewind();
    void onPreviewHold();
    void onPreviewRestr();
    void onPreviewAbort();
    void onRefreshMapping();
    void onAsciiMessageArrived(QString rawMessage, int type, int base, QString packedParams);

private:
    void initUi();
    void initPlaceholderTable();
    void addTableRow(int sourceSlot = 1, int targetSlot = 1);
    void initCassetteWidgets();
    void initApiObserver();
    void updateSourceTarget();
    void updateModeState();
    void updateTablePreview();
    bool validateBeforeRewind(QString& detailText) const;
    void requestRefresh(bool includeRobotState, const QString& detailText);
    bool sendAsciiCommand(const QString& commandText, const QString& pendingStateText, const QString& pendingDetailText);
    bool cassetteHasWafer(const std::shared_ptr<Cassette>& cassette) const;
    bool cassetteIsEmpty(const std::shared_ptr<Cassette>& cassette) const;
    bool robotHasWafer() const;
    void setStatusText(const QString& stateText, const QString& commandText, const QString& detailText);

private:
    Q_DECLARE_PRIVATE(QLPRewindWidget)
    QLPRewindWidgetPrivate* d_ptr;
};

}

#endif
