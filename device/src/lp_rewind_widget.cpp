#include "lp_rewind_widget.h"
#include "EFEM/efem_loadport_subsystem.h"
#include "EFEM/efem_wafer_robot_subsystem.h"
#include "efem_ascii_api.h"
#include "Kernel/Fortrend/cassette.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/FortrendUI/cassette_widget.h"
#include "Kernel/kernel_subsystem_update_command.h"
#include "device/ui_lp_rewind_widget.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QLayoutItem>
#include <QMessageBox>
#include <QMetaObject>
#include <QTableWidgetItem>

#if _MSC_VER > 1600
#pragma execution_character_set("utf-8")
#endif

namespace {

std::shared_ptr<FC::EFEMLPSubsystem> findLoadPort(
    const std::shared_ptr<FC::IKernel>& kernel,
    const std::string& name)
{
    auto loadport = kernel->getKernelModule<FC::EFEMLPSubsystem>(name);
    if (loadport) {
        return loadport;
    }

    if (name == "LP1" || name == "LP2") {
        loadport = kernel->getKernelModule<FC::EFEMLPSubsystem>("E" + name);
        if (loadport) {
            return loadport;
        }
    }

    for (auto& sub : kernel->getKernelModules<FC::EFEMLPSubsystem>()) {
        if (!sub) {
            continue;
        }
        if (sub->getName() == name) {
            return sub;
        }
        if ((name == "LP1" || name == "LP2") && sub->getName() == "E" + name) {
            return sub;
        }
    }
    return nullptr;
}

void clearLayout(QLayout* layout)
{
    if (!layout) {
        return;
    }

    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            delete widget;
        }
        if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout);
            delete childLayout;
        }
        delete item;
    }
}

QLabel* createPlaceholderLabel(const QString& text, QWidget* parent)
{
    QLabel* label = new QLabel(text, parent);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setStyleSheet("QLabel { color: rgb(120, 120, 120); border: 1px dashed rgb(180, 180, 180); padding: 16px; }");
    return label;
}

QComboBox* createStationCombo(QWidget* parent, const QString& currentText)
{
    QComboBox* combo = new QComboBox(parent);
    combo->addItem("LP1");
    combo->addItem("LP2");
    combo->setCurrentText(currentText);
    return combo;
}

QComboBox* createSlotCombo(QWidget* parent, int currentSlot)
{
    QComboBox* combo = new QComboBox(parent);
    combo->setMaxVisibleItems(25);
    for (int slot = 1; slot <= 25; ++slot) {
        combo->addItem(QString::number(slot));
    }
    combo->setCurrentText(QString::number(currentSlot));
    return combo;
}

} // namespace

namespace FC {

class QLPRewindWidgetPrivate
{
public:
    Q_DECLARE_PUBLIC(QLPRewindWidget)

    QLPRewindWidgetPrivate(QLPRewindWidget* p, const std::shared_ptr<IKernel>& kernel)
        : ui(new Ui::LPRewindWidget)
        , kernel(kernel)
        , q_ptr(p)
    {
    }

    ~QLPRewindWidgetPrivate()
    {
        delete ui;
    }

    Ui::LPRewindWidget* ui;
    std::shared_ptr<IKernel> kernel;
    std::shared_ptr<FortrendCassetteManager> cassManager;
    std::shared_ptr<EFEMAsciiApi> api;
    std::shared_ptr<EFEMLPSubsystem> lp1;
    std::shared_ptr<EFEMLPSubsystem> lp2;
    std::shared_ptr<EFEMWaferRobotSubsystem> ewtr;
    QString currentSourceLp = "LP1";
    QLPRewindWidget* q_ptr;
};

QLPRewindWidget::QLPRewindWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new QLPRewindWidgetPrivate(this, kernel))
{
    Q_D(QLPRewindWidget);
    d->ui->setupUi(this);
    d->cassManager = kernel->getKernelModule<FortrendCassetteManager>();
    d->lp1 = findLoadPort(kernel, "LP1");
    d->lp2 = findLoadPort(kernel, "LP2");
    d->api = d->lp1 ? d->lp1->api : (d->lp2 ? d->lp2->api : nullptr);
    d->ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
    initUi();
}

QLPRewindWidget::~QLPRewindWidget()
{
    Q_D(QLPRewindWidget);
    if (d->api) {
        d->api->removeCommandObserver(this);
    }
    delete d_ptr;
}

void QLPRewindWidget::initUi()
{
    Q_D(QLPRewindWidget);

    connect(d->ui->mode_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeChanged(int)));
    connect(d->ui->source_lp_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(onSourceLpChanged(int)));
    connect(d->ui->add_row_btn, SIGNAL(clicked()), this, SLOT(onAddTableRow()));
    connect(d->ui->remove_row_btn, SIGNAL(clicked()), this, SLOT(onRemoveTableRow()));
    connect(d->ui->clear_recipe_btn, SIGNAL(clicked()), this, SLOT(onClearRecipe()));
    connect(d->ui->save_recipe_btn, SIGNAL(clicked()), this, SLOT(onSaveRecipe()));
    connect(d->ui->load_recipe_btn, SIGNAL(clicked()), this, SLOT(onLoadRecipe()));
    connect(d->ui->rewind_btn, SIGNAL(clicked()), this, SLOT(onPreviewRewind()));
    connect(d->ui->hold_btn, SIGNAL(clicked()), this, SLOT(onPreviewHold()));
    connect(d->ui->restr_btn, SIGNAL(clicked()), this, SLOT(onPreviewRestr()));
    connect(d->ui->abort_btn, SIGNAL(clicked()), this, SLOT(onPreviewAbort()));
    connect(d->ui->refresh_mapping_btn, SIGNAL(clicked()), this, SLOT(onRefreshMapping()));

    initApiObserver();
    initPlaceholderTable();
    initCassetteWidgets();
    updateSourceTarget();
    updateModeState();
    setStatusText("待执行", "-", "已接入 ASCII 指令发送与基础回包处理，可执行整盒 REWIND、HOLD、RESTR、ABORT 和刷新 mapping。");
}

void QLPRewindWidget::initPlaceholderTable()
{
    Q_D(QLPRewindWidget);

    d->ui->rewind_table->setColumnCount(4);
    d->ui->rewind_table->setHorizontalHeaderLabels(QStringList()
        << "Source Station"
        << "Source Slot"
        << "Target Station"
        << "Target Slot");
    d->ui->rewind_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    d->ui->rewind_table->verticalHeader()->setDefaultSectionSize(28);
    d->ui->rewind_table->setAlternatingRowColors(true);
    d->ui->rewind_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->ui->rewind_table->setSelectionMode(QAbstractItemView::SingleSelection);
    d->ui->rewind_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    d->ui->rewind_table->setRowCount(0);
    addTableRow();
}

void QLPRewindWidget::addTableRow(int sourceSlot, int targetSlot)
{
    Q_D(QLPRewindWidget);

    const int row = d->ui->rewind_table->rowCount();
    const QString sourceLp = d->ui->source_lp_combo_box->currentText();
    const QString targetLp = (sourceLp == "LP1") ? "LP2" : "LP1";

    d->ui->rewind_table->insertRow(row);
    d->ui->rewind_table->setCellWidget(row, 0, createStationCombo(d->ui->rewind_table, sourceLp));
    d->ui->rewind_table->setCellWidget(row, 1, createSlotCombo(d->ui->rewind_table, sourceSlot));
    d->ui->rewind_table->setCellWidget(row, 2, createStationCombo(d->ui->rewind_table, targetLp));
    d->ui->rewind_table->setCellWidget(row, 3, createSlotCombo(d->ui->rewind_table, targetSlot));
    d->ui->rewind_table->selectRow(row);
}

void QLPRewindWidget::initCassetteWidgets()
{
    Q_D(QLPRewindWidget);

    clearLayout(d->ui->lp1_cassette_layout);
    clearLayout(d->ui->lp2_cassette_layout);

    if (!d->cassManager) {
        d->ui->lp1_cassette_layout->addWidget(createPlaceholderLabel("CassetteManager 未找到", this));
        d->ui->lp2_cassette_layout->addWidget(createPlaceholderLabel("CassetteManager 未找到", this));
        return;
    }

    auto lp1Cassette = d->lp1 ? d->cassManager->getCassette(d->lp1.get()) : nullptr;
    if (lp1Cassette) {
        lp1Cassette->setBoxId("LP1");
        d->ui->lp1_cassette_layout->addWidget(new QFortrendCassetteWidget(lp1Cassette, d->cassManager, this));
    }
    else {
        d->ui->lp1_cassette_layout->addWidget(createPlaceholderLabel("LP1 cassette 未就绪", this));
    }

    auto lp2Cassette = d->lp2 ? d->cassManager->getCassette(d->lp2.get()) : nullptr;
    if (lp2Cassette) {
        lp2Cassette->setBoxId("LP2");
        d->ui->lp2_cassette_layout->addWidget(new QFortrendCassetteWidget(lp2Cassette, d->cassManager, this));
    }
    else {
        d->ui->lp2_cassette_layout->addWidget(createPlaceholderLabel("LP2 cassette 未就绪", this));
    }
}

void QLPRewindWidget::initApiObserver()
{
    Q_D(QLPRewindWidget);

    if (!d->api) {
        return;
    }

    d->api->addCommandObserver(this, [this](const std::string& rawMessage, const std::shared_ptr<EFEMAsciiApi::Command>& command) {
        QStringList params;
        for (auto& item : command->message->paramers) {
            params << QString::fromStdString(item);
        }

        QMetaObject::invokeMethod(this, "onAsciiMessageArrived", Qt::QueuedConnection,
            Q_ARG(QString, QString::fromStdString(rawMessage)),
            Q_ARG(int, static_cast<int>(command->type)),
            Q_ARG(int, static_cast<int>(command->message->base)),
            Q_ARG(QString, params.join(QChar(0x1F))));
    });
}

void QLPRewindWidget::updateSourceTarget()
{
    Q_D(QLPRewindWidget);

    d->currentSourceLp = d->ui->source_lp_combo_box->currentText();
    const QString targetLp = (d->currentSourceLp == "LP1") ? "LP2" : "LP1";
    d->ui->source_lp_value_label->setText(d->currentSourceLp);
    d->ui->target_lp_value_label->setText(targetLp);
    updateTablePreview();
}

void QLPRewindWidget::updateModeState()
{
    Q_D(QLPRewindWidget);

    const bool reservedMode = d->ui->mode_combo_box->currentIndex() == 1;
    d->ui->rewind_table->setEnabled(reservedMode);
    d->ui->add_row_btn->setEnabled(reservedMode);
    d->ui->remove_row_btn->setEnabled(reservedMode);
    d->ui->mode_hint_label->setText(reservedMode
        ? "当前为选片捯片预留模式：支持添加/删除行，并可在表格内通过下拉框选择 LP 与 1~25 槽位。"
        : "当前为全部捯片模式：表格不参与组包，仅保留后续选片编辑能力。");
}

void QLPRewindWidget::updateTablePreview()
{
    Q_D(QLPRewindWidget);

    const QString sourceLp = d->ui->source_lp_combo_box->currentText();
    const QString targetLp = (sourceLp == "LP1") ? "LP2" : "LP1";

    for (int row = 0; row < d->ui->rewind_table->rowCount(); ++row) {
        QComboBox* sourceCombo = qobject_cast<QComboBox*>(d->ui->rewind_table->cellWidget(row, 0));
        QComboBox* sourceSlotCombo = qobject_cast<QComboBox*>(d->ui->rewind_table->cellWidget(row, 1));
        QComboBox* targetCombo = qobject_cast<QComboBox*>(d->ui->rewind_table->cellWidget(row, 2));
        QComboBox* targetSlotCombo = qobject_cast<QComboBox*>(d->ui->rewind_table->cellWidget(row, 3));

        if (!sourceCombo || !sourceSlotCombo || !targetCombo || !targetSlotCombo) {
            continue;
        }

        if (sourceCombo->currentText().isEmpty()) {
            sourceCombo->setCurrentText(sourceLp);
        }
        if (targetCombo->currentText().isEmpty()) {
            targetCombo->setCurrentText(targetLp);
        }
        if (sourceSlotCombo->currentText().isEmpty()) {
            sourceSlotCombo->setCurrentText(QString::number(row + 1));
        }
        if (targetSlotCombo->currentText().isEmpty()) {
            targetSlotCombo->setCurrentText(QString::number(row + 1));
        }
    }
}

bool QLPRewindWidget::validateBeforeRewind(QString& detailText) const
{
    Q_D(const QLPRewindWidget);

    if (d->ui->mode_combo_box->currentIndex() != 0) {
        detailText = "当前仅支持整盒 REWIND，选片捯片仍为预留模式。";
        return false;
    }
    if (!d->api || !d->lp1 || !d->lp2 || !d->ewtr) {
        detailText = "ASCII API 或 LP1/LP2/EWTR 子系统未就绪。";
        return false;
    }

    auto sourceLp = (d->ui->source_lp_combo_box->currentText() == "LP1") ? d->lp1 : d->lp2;
    auto targetLp = (d->ui->source_lp_combo_box->currentText() == "LP1") ? d->lp2 : d->lp1;
    if (!sourceLp || !targetLp) {
        detailText = "未找到 Source/Target LP 子系统。";
        return false;
    }
    if (!sourceLp->hasBoxPresent() || !targetLp->hasBoxPresent()) {
        detailText = "执行 REWIND 前要求 Source 与 Target LP 都已上盒。";
        return false;
    }
    if (!sourceLp->hasDoorOpend() || !targetLp->hasDoorOpend()) {
        detailText = "执行 REWIND 前要求 Source 与 Target LP 都处于开门状态。";
        return false;
    }
    if (!d->cassManager) {
        detailText = "CassetteManager 未就绪，无法校验 Source/Target 盒内状态。";
        return false;
    }

    auto sourceCassette = d->cassManager->getCassette(sourceLp.get());
    auto targetCassette = d->cassManager->getCassette(targetLp.get());
    if (!sourceCassette || !targetCassette) {
        detailText = "Source 或 Target cassette 尚未建立，请先刷新 mapping。";
        return false;
    }
    if (!cassetteHasWafer(sourceCassette)) {
        detailText = "Source LP 当前没有可搬运晶圆。";
        return false;
    }
    if (!cassetteIsEmpty(targetCassette)) {
        detailText = "Target LP 必须为空盒后才能执行整盒 REWIND。";
        return false;
    }
    if (robotHasWafer()) {
        detailText = "EWTR 双臂上仍有片，不能执行 REWIND。";
        return false;
    }

    detailText = "前置校验通过，等待设备 ACK/INF/ABS 回包。";
    return true;
}

void QLPRewindWidget::requestRefresh(bool includeRobotState, const QString& detailText)
{
    Q_D(QLPRewindWidget);

    auto lp1 = d->lp1;
    auto lp2 = d->lp2;
    auto ewtr = d->ewtr;
    auto kernel = d->kernel;

    if (!kernel || !lp1 || !lp2) {
        setStatusText("刷新失败", d->ui->command_value_label->text(), "LP1/LP2 子系统未就绪，无法发起状态刷新。");
        return;
    }

    setStatusText("刷新中", d->ui->command_value_label->text(), detailText);
    kernel->submitTask([lp1, lp2, ewtr, includeRobotState]() {
        auto lp1Cmd = lp1->createUpdateCommand();
        lp1->startCommand(lp1Cmd);
        lp1Cmd->wait();

        auto lp2Cmd = lp2->createUpdateCommand();
        lp2->startCommand(lp2Cmd);
        lp2Cmd->wait();

        if (includeRobotState && ewtr) {
            auto ewtrCmd = ewtr->createUpdateCommand();
            ewtr->startCommand(ewtrCmd);
            ewtrCmd->wait();
        }
    });
}

bool QLPRewindWidget::sendAsciiCommand(const QString& commandText, const QString& pendingStateText, const QString& pendingDetailText)
{
    Q_D(QLPRewindWidget);

    if (!d->api) {
        setStatusText("发送失败", commandText, "ASCII API 未就绪，指令未发送。");
        return false;
    }

    const QByteArray bytes = commandText.toUtf8();
    if (!d->api->sendMessage(bytes.constData(), static_cast<unsigned int>(bytes.size()))) {
        setStatusText("发送失败", commandText, "sendMessage 返回失败，请检查 ASCII 通信连接。");
        return false;
    }

    setStatusText(pendingStateText, commandText, pendingDetailText);
    return true;
}

bool QLPRewindWidget::cassetteHasWafer(const std::shared_ptr<Cassette>& cassette) const
{
    if (!cassette) {
        return false;
    }

    for (int slot = 1; slot <= static_cast<int>(cassette->slotCount()); ++slot) {
        if (cassette->getMapping(slot) == Cassette::Present) {
            return true;
        }
    }
    return false;
}

bool QLPRewindWidget::cassetteIsEmpty(const std::shared_ptr<Cassette>& cassette) const
{
    if (!cassette) {
        return false;
    }

    for (int slot = 1; slot <= static_cast<int>(cassette->slotCount()); ++slot) {
        if (cassette->getMapping(slot) == Cassette::Present) {
            return false;
        }
    }
    return true;
}

bool QLPRewindWidget::robotHasWafer() const
{
    Q_D(const QLPRewindWidget);

    if (d->ewtr && (d->ewtr->hasObject(0) || d->ewtr->hasObject(1))) {
        return true;
    }
    if (!d->cassManager || !d->ewtr) {
        return false;
    }

    auto robotCassette = d->cassManager->getCassette(d->ewtr.get());
    if (!robotCassette) {
        return false;
    }

    const int slotCount = static_cast<int>(robotCassette->slotCount());
    for (int slot = 1; slot <= slotCount; ++slot) {
        if (robotCassette->getMapping(slot) == Cassette::Present) {
            return true;
        }
    }
    return false;
}

void QLPRewindWidget::setStatusText(const QString& stateText, const QString& commandText, const QString& detailText)
{
    Q_D(QLPRewindWidget);

    d->ui->state_value_label->setText(stateText);
    d->ui->command_value_label->setText(commandText);
    d->ui->detail_value_label->setText(detailText);
}

void QLPRewindWidget::onModeChanged(int)
{
    updateModeState();
}

void QLPRewindWidget::onSourceLpChanged(int)
{
    updateSourceTarget();
}

void QLPRewindWidget::onAddTableRow()
{
    Q_D(QLPRewindWidget);

    if (d->ui->rewind_table->rowCount() >= 25) {
        setStatusText("表格已满", d->ui->command_value_label->text(), "选片表格最多支持 25 行，已对应 1~25 槽位范围。");
        return;
    }

    const int nextSlot = qMin(25, d->ui->rewind_table->rowCount() + 1);
    addTableRow(nextSlot, nextSlot);
}

void QLPRewindWidget::onRemoveTableRow()
{
    Q_D(QLPRewindWidget);

    int row = d->ui->rewind_table->currentRow();
    if (row < 0) {
        row = d->ui->rewind_table->rowCount() - 1;
    }
    if (row < 0) {
        return;
    }

    d->ui->rewind_table->removeRow(row);
    if (d->ui->rewind_table->rowCount() > 0) {
        d->ui->rewind_table->selectRow(qMax(0, row - 1));
    }
}

void QLPRewindWidget::onClearRecipe()
{
    setStatusText("配方占位", "-", "清除配方按钮已预留，首版暂未接入实际配方持久化逻辑。");
}

void QLPRewindWidget::onSaveRecipe()
{
    Q_D(QLPRewindWidget);
    setStatusText("配方占位", d->ui->command_value_label->text(), "保存配方按钮已预留，首版暂未接入文件保存逻辑。");
}

void QLPRewindWidget::onLoadRecipe()
{
    Q_D(QLPRewindWidget);
    setStatusText("配方占位", d->ui->command_value_label->text(), "加载配方按钮已预留，首版暂未接入文件加载逻辑。");
}

void QLPRewindWidget::onPreviewRewind()
{
    Q_D(QLPRewindWidget);
    QString detailText;
    if (!validateBeforeRewind(detailText)) {
        setStatusText("校验失败", d->ui->command_value_label->text(), detailText);
        return;
    }

    const QString sourceLp = d->ui->source_lp_combo_box->currentText();
    const QString targetLp = d->ui->target_lp_value_label->text();
    const QString command = QString("MOV:REWIND/%1/%2;").arg(sourceLp, targetLp);
    sendAsciiCommand(command, "等待 ACK", detailText);
}

void QLPRewindWidget::onPreviewHold()
{
    sendAsciiCommand("MOV:HOLD;", "等待 ACK", "已发送 HOLD 指令，等待设备 ACK/INF/ABS 回包。");
}

void QLPRewindWidget::onPreviewRestr()
{
    sendAsciiCommand("MOV:RESTR;", "等待 ACK", "已发送 RESTR 指令，等待设备 ACK/INF/ABS 回包。");
}

void QLPRewindWidget::onPreviewAbort()
{
    if (QMessageBox::question(this, "确认中止", "确认发送 MOV:ABORT; 吗？") != QMessageBox::Yes) {
        return;
    }

    sendAsciiCommand("MOV:ABORT;", "等待 ACK", "已发送 ABORT 指令，等待设备 ACK/INF/ABS 回包。");
}

void QLPRewindWidget::onRefreshMapping()
{
    updateTablePreview();
    requestRefresh(true, "已触发 LP1/LP2/EWTR 的 ASCII 状态与 mapping 刷新，请等待 STATE/MAPDT 回包。");
}

void QLPRewindWidget::onAsciiMessageArrived(QString rawMessage, int type, int base, QString packedParams)
{
    Q_D(QLPRewindWidget);

    const QString displayMessage = rawMessage.endsWith(";") ? rawMessage : (rawMessage + ";");
    const QStringList params = packedParams.isEmpty() ? QStringList() : packedParams.split(QChar(0x1F));
    const EFEMAsciiApi::Type messageType = static_cast<EFEMAsciiApi::Type>(type);
    const EFEMAsciiApi::Base messageBase = static_cast<EFEMAsciiApi::Base>(base);

    if (messageBase == EFEMAsciiApi::MAPDT || messageBase == EFEMAsciiApi::STATE) {
        if (params.isEmpty()) {
            return;
        }

        const QString station = params.first();
        if (station != "LP1" && station != "LP2" && station != "WTR") {
            return;
        }

        updateTablePreview();
        if (messageBase == EFEMAsciiApi::MAPDT) {
            setStatusText(d->ui->state_value_label->text(), d->ui->command_value_label->text(),
                QString("收到 %1 的 MAPDT 回包，已刷新 cassette 与 mapping 展示。").arg(station));
        }
        return;
    }

    if (messageBase == EFEMAsciiApi::REWIND) {
        if (messageType == EFEMAsciiApi::ACK) {
            setStatusText("执行中", displayMessage, "设备已 ACK REWIND，等待执行完成。");
        }
        else if (messageType == EFEMAsciiApi::INF) {
            setStatusText("已完成", displayMessage, "REWIND 已完成，正在自动刷新 LP1/LP2/EWTR 状态。");
            requestRefresh(true, "REWIND 完成，已自动刷新 LP1/LP2/EWTR 状态与 mapping。");
        }
        else if (messageType == EFEMAsciiApi::ABS) {
            setStatusText("执行失败", displayMessage, "REWIND 返回 ABS，请检查设备报警与前置条件。");
            requestRefresh(true, "REWIND 返回 ABS，已自动刷新 LP1/LP2/EWTR 状态。");
        }
        return;
    }

    if (messageBase == EFEMAsciiApi::HOLD) {
        if (messageType == EFEMAsciiApi::ACK) {
            setStatusText("暂停中", displayMessage, "设备已 ACK HOLD，等待暂停完成。");
        }
        else if (messageType == EFEMAsciiApi::INF) {
            setStatusText("已暂停", displayMessage, "设备返回 HOLD 完成。");
        }
        else if (messageType == EFEMAsciiApi::ABS) {
            setStatusText("暂停失败", displayMessage, "设备返回 HOLD ABS，请检查当前运行状态。");
        }
        return;
    }

    if (messageBase == EFEMAsciiApi::RESTR) {
        if (messageType == EFEMAsciiApi::ACK) {
            setStatusText("恢复中", displayMessage, "设备已 ACK RESTR，等待恢复完成。");
        }
        else if (messageType == EFEMAsciiApi::INF) {
            setStatusText("已恢复", displayMessage, "设备返回 RESTR 完成。");
        }
        else if (messageType == EFEMAsciiApi::ABS) {
            setStatusText("恢复失败", displayMessage, "设备返回 RESTR ABS，请检查当前运行状态。");
        }
        return;
    }

    if (messageBase == EFEMAsciiApi::ABORT) {
        if (messageType == EFEMAsciiApi::ACK) {
            setStatusText("中止中", displayMessage, "设备已 ACK ABORT，等待中止完成。");
        }
        else if (messageType == EFEMAsciiApi::INF) {
            setStatusText("已中止", displayMessage, "设备返回 ABORT 完成，正在自动刷新 LP1/LP2/EWTR 状态。");
            requestRefresh(true, "ABORT 完成，已自动刷新 LP1/LP2/EWTR 状态与 mapping。");
        }
        else if (messageType == EFEMAsciiApi::ABS) {
            setStatusText("中止失败", displayMessage, "设备返回 ABORT ABS，请检查当前运行状态。");
        }
    }
}

} // namespace FC
