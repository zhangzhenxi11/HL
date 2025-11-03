#include "robotarmwidget.h"
#include <cmath>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <qsettings.h>
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#pragma execution_character_set("utf-8")

RobotArmWidget::RobotArmWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(500, 600);
    setWindowTitle("半导体机械手控制系统");

    // 初始化工位位置
    stationPositions.resize(5);
    stationNames.clear();
    stationNames << "LoadLockA" << "LoadLockB" << "Aligner" << "loadPortA" << "loadPortB";
    stationHasWafer.clear();
    stationHasWafer << false << false << false << true << false; // loadPortA(3)有晶圆

    // 初始化机械臂参数（复位位置）
    currentStation = 3;     // 从 loadPortA 开始
    baseXOffset = 116;      // 复位 X轴偏移
    baseRotation = 13;      // 复位旋转角度

    // 安全位置角度（缩回姿态）
    // 约束：肩关节和肘关节夹角必须保持锐角（<90°）
    //safeShoulderAngle = -104;   // 范围：-107° ~ 131°
    safeShoulderAngle = -84;
    safeElbowAngle = 142;       // 范围：153° ~ -150°
    safeWristAngle = -106;      // 范围：-115° ± 2°

    // 伸展位置角度（通用伸展姿态 - 从数据总结）
    // 约束：保持肩肘夹角为锐角
    extendShoulderAngle = -120.0;   // 机械臂1伸展时肩关节约 -103° ~ -126°
    extendElbowAngle = -150.0;      // 机械臂1伸展时肘关节约 -131° ~ -157°（已修正到范围内）
    
    // 机械臂2伸展位置角度（对称关系）
    extendShoulderAngle2 = 60.0;    // 机械臂2伸展时肩关节角度（-120 + 180）
    extendElbowAngle2 = 150.0;      // 机械臂2伸展时肘关节角度（-150取负）

    // 机械臂1 - 初始设置为安全位置
    shoulderAngle = safeShoulderAngle;  // -104°
    elbowAngle = safeElbowAngle;        // 142°
    wristAngle = safeWristAngle;        // -106°
    hasWafer = false;
    
    // 机械臂2 - 初始设置为安全位置（对称180°，使腕关节坐标重叠）
    // 关键：肩关节相对机械臂1旋转180°，肘关节角度取负值，使腕关节位置重叠
//    shoulderAngle2 = safeShoulderAngle + 180;  // 76° (-104 + 180)
    shoulderAngle2 = 58;
    elbowAngle2 = -safeElbowAngle;             // -142° (对称关系)
    wristAngle2 = safeWristAngle;              // -106° (保持一致)
    hasWafer2 = false;

    rotationSpeed = 2.0;   // 提高速度
    extensionSpeed = 2.0;  // 提高速度
    actionType = IDLE;
    currentPoseIndex = 0;
    
    // 初始化动画序列（基于用户记录的真实数据）
    initAnimationSequence();

    // 尺寸参数
    baseRadius = 50;
    upperArmLength = 110;
    forearmLength = 110;
    handLength = 120;

    // 创建定时器
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &RobotArmWidget::updateAnimation);


    // 初始化手动控制模式
    manualControlMode = false;
}

void RobotArmWidget::initAnimationSequence()
{
    animationSequence.clear();
    
    // 基于用户实际记录的姿态数据构建动画序列
    animationSequence.append({QString::fromUtf8("平移到loadPortA工位"), -96.0, 13.0, -104.0, 150.0, -106.0});
    animationSequence.append({QString::fromUtf8("底座旋转手指指向工位中心"), -96.0, 105.0, -104.0, 150.0, -106.0});
    animationSequence.append({QString::fromUtf8("loadPortA伸出取片"), -96.0, 105.0, -129.0, -139.0, -106.0});
    animationSequence.append({QString::fromUtf8("loadPortA取片完成缩回"), -96.0, 105.0, -104.0, 148.0, -106.0});
    
    animationSequence.append({QString::fromUtf8("平移+旋转到Aligner"), -96.0, 14.0, -104.0, 152.0, -106.0});
    animationSequence.append({QString::fromUtf8("Aligner伸出放片"), -96.0, 14.0, -104.0, -171.0, -106.0});
    animationSequence.append({QString::fromUtf8("Aligner缩回到位"), -96.0, 14.0, -104.0, 139.0, -106.0});
    animationSequence.append({QString::fromUtf8("Aligner伸出取片"), -96.0, 14.0, -104.0, -177.0, -106.0});
    animationSequence.append({QString::fromUtf8("Aligner缩回取片完成"), -96.0, 14.0, -104.0, 134.0, -106.0});
    
    animationSequence.append({QString::fromUtf8("平移到loadportB工位中心"), 144.0, 14.0, -104.0, 134.0, -106.0});
    animationSequence.append({QString::fromUtf8("底座旋转手指指向LoadLockB工位中心"), 144.0, 283.0, -104.0, 134.0, -106.0});
    animationSequence.append({QString::fromUtf8("loadPortB伸出放片"), 144.0, 283.0, -122.0, -163.0, -106.0});
    animationSequence.append({QString::fromUtf8("loadPortB缩回放片完成"), 144.0, 283.0, -96.0, 159.0, -106.0});
    
    qDebug() << QString::fromUtf8("✅ 已加载") << animationSequence.size() << QString::fromUtf8("个动作姿态");
}

void RobotArmWidget::startAnimation()
{
    if (!animationTimer->isActive()) {
        qDebug() << "===== 动画启动 =====";
        qDebug() << QString::fromUtf8("总共") << animationSequence.size() << QString::fromUtf8("个姿态");
        
        currentPoseIndex = 0;
        animationTimer->start(30); // 约33fps
        actionType = MOVING_TO_STATION;
        
        qDebug() << "==================";
    }
}

void RobotArmWidget::pauseAnimation()
{
    animationTimer->stop();
    actionType = IDLE;
    update();
}

void RobotArmWidget::resetAnimation()
{
    animationTimer->stop();
    
    // 复位到第一个姿态
    currentPoseIndex = 0;
    if (!animationSequence.isEmpty()) {
        const Pose &firstPose = animationSequence[0];
        baseXOffset = firstPose.baseX;
        baseRotation = firstPose.baseRot;
        shoulderAngle = firstPose.shoulder;
        elbowAngle = firstPose.elbow;
        wristAngle = firstPose.wrist;
    }

    hasWafer = false;
    stationHasWafer = {false, false, false, true, false};
    actionType = IDLE;
    update();
}

// 从 JSON 文件加载动画序列
void RobotArmWidget::loadAnimationFromJson(const QString &jsonFilePath)
{
    QFile file(jsonFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << QString::fromUtf8("❌ 无法打开文件:") << jsonFilePath;
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qDebug() << QString::fromUtf8("❌ JSON格式错误");
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray posesArray = root["poses"].toArray();
    
    if (posesArray.isEmpty()) {
        qDebug() << QString::fromUtf8("❌ JSON中没有姿态数据");
        return;
    }
    
    // 清空并加载新的动画序列
    animationSequence.clear();
    
    double prevElbow = 0;
    bool firstPose = true;
    
    for (const QJsonValue &value : posesArray) {
        QJsonObject poseObj = value.toObject();
        
        Pose pose;
        pose.name = poseObj["name"].toString();
        pose.baseX = poseObj["baseX"].toDouble();
        pose.baseRot = poseObj["baseRotation"].toDouble();
        pose.shoulder = poseObj["shoulderAngle"].toDouble();
        pose.elbow = poseObj["elbowAngle"].toDouble();
        pose.wrist = poseObj["wristAngle"].toDouble();
        
        // 关键修复：归一化肘关节角度，避免跨越180°边界
        if (!firstPose) {
            double diff = pose.elbow - prevElbow;
            
            // 如果角度差超过180°，说明跨越了边界
            if (diff > 180) {
                pose.elbow -= 360;  // 从正边界跳到负边界
                qDebug() << QString::fromUtf8("🔄 修正肘关节角度:") << poseObj["elbowAngle"].toDouble() << "--> " << pose.elbow;
            } else if (diff < -180) {
                pose.elbow += 360;  // 从负边界跳到正边界
                qDebug() << QString::fromUtf8("🔄 修正肘关节角度:") << poseObj["elbowAngle"].toDouble() << " --> " << pose.elbow;
            }
        }
        
        prevElbow = pose.elbow;
        firstPose = false;
        
        animationSequence.append(pose);
    }
    
    qDebug() << QString::fromUtf8("✅ 从 JSON 加载了") << animationSequence.size() << QString::fromUtf8("个姿态");
    qDebug() << QString::fromUtf8("💡 已自动修正肘关节角度跨越180°边界问题");
    qDebug() << QString::fromUtf8("📁 文件:") << jsonFilePath;
    
    // 复位到第一个姿态
    resetAnimation();
}

void RobotArmWidget::resizeEvent(QResizeEvent *event)
{
       Q_UNUSED(event);
       // 重新计算中心位置和腔体半径
       baseCenter = QPointF(width() / 2, height() / 2);
       chamberRadius = qMin(width(), height()) * 0.35;

       // 定义5个工位的位置 (矩形EFEM布局)
       // 矩形腔体参数
       double rectWidth = 729.0;
       double rectHeight = 254.0;
       double rectLeft = baseCenter.x() - 300;
       double rectTop = baseCenter.y() - 200;
       double stationSize = 50;  // 工位矩形大小
       
       // LoadLockA - 矩形外上方左侧
       stationPositions[0] = QPointF(rectLeft + 200, rectTop - stationSize);
       
       // LoadLockB - 矩形外上方右侧
       stationPositions[1] = QPointF(rectLeft + rectWidth - 100, rectTop - stationSize);
       
       // Aligner - 矩形内最左侧
       stationPositions[2] = QPointF(rectLeft + 60, baseCenter.y());
       
       // loadPortA - 矩形外下方左侧
       stationPositions[3] = QPointF(rectLeft + 200, rectTop + rectHeight + stationSize );
       
       // loadPortB - 矩形外下方右侧
       stationPositions[4] = QPointF(rectLeft + rectWidth - 100, rectTop + rectHeight + stationSize);

       // 更新关节位置
       updateJointPositions();
       updateJointPositions2();

       update();
}

void RobotArmWidget::updateJointPositions()
{
    // 计算实际底座位置（考虑X轴偏移）
    QPointF actualBaseCenter = QPointF(baseCenter.x() + baseXOffset, baseCenter.y());

    // 底座旋转角度（弧度）
    double baseRad = qDegreesToRadians(baseRotation);

    // ==== 机械臂1 ====
    // 1. 肩关节位置 = 底座中心（底座旋转不改变肩关节相对底座的位置）
    shoulderPos = actualBaseCenter;

    // 2. 肘关节位置 - 受底座旋转和肩关节角度共同影响
    //    实际上臂方向 = 底座旋转角度 + 肩关节相对角度
    double shoulderRad = qDegreesToRadians(shoulderAngle);
    double actualShoulderAngle = baseRad + shoulderRad;  // 底座旋转带动肩关节一起转
    
    elbowPos = QPointF(
        shoulderPos.x() + upperArmLength * cos(actualShoulderAngle),
        shoulderPos.y() + upperArmLength * sin(actualShoulderAngle)
    );

    // 3. 腕关节位置 - 受底座旋转、肩关节和肘关节角度共同影响
    //    实际前臂方向 = 底座旋转 + 肩关节相对角度 + 肘关节相对角度
    double elbowRad = qDegreesToRadians(elbowAngle);
    double actualForearmAngle = actualShoulderAngle + elbowRad;  // 累加角度
    
    wristPos = QPointF(
        elbowPos.x() + forearmLength * cos(actualForearmAngle),
        elbowPos.y() + forearmLength * sin(actualForearmAngle)
    );
}

// 更新机械臂2的关节位置
// 关键：机械臂2的腕关节完全独立，只是在复位状态下与机械臂1坐标重叠
void RobotArmWidget::updateJointPositions2()
{
    // 计算实际底座位置（考虑X轴偏移）
    QPointF actualBaseCenter = QPointF(baseCenter.x() + baseXOffset, baseCenter.y());

    // 底座旋转角度（弧度）
    double baseRad = qDegreesToRadians(baseRotation);

    // ==== 机械臂2（上臂） - 完全独立的运动学计算 ====
    // 1. 肩关节位置 = 底座中心（两个机械臂共享同一个底座肩关节）
    shoulderPos2 = actualBaseCenter;

    // 2. 肘关节位置 - 受底座旋转和肩关节角度2共同影响
    double shoulderRad2 = qDegreesToRadians(shoulderAngle2);
    double actualShoulderAngle2 = baseRad + shoulderRad2;  // 底座旋转带动肩关节一起转
    
    elbowPos2 = QPointF(
        shoulderPos2.x() + upperArmLength * cos(actualShoulderAngle2),
        shoulderPos2.y() + upperArmLength * sin(actualShoulderAngle2)
    );

    // 3. 腕关节位置 - 独立计算，受底座旋转、肩关节和肘关节角度2共同影响
    //    在复位状态下，通过合适的关节角度设置，使其与机械臂1的腕关节位置重叠
    double elbowRad2 = qDegreesToRadians(elbowAngle2);
    double actualForearmAngle2 = actualShoulderAngle2 + elbowRad2;  // 累加角度
    
    wristPos2 = QPointF(
        elbowPos2.x() + forearmLength * cos(actualForearmAngle2),
        elbowPos2.y() + forearmLength * sin(actualForearmAngle2)
    );
}

void RobotArmWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置中心点
    baseCenter = QPointF(width() / 2, height() / 2);
    chamberRadius = qMin(width(), height()) * 0.35;

    // 更新关节位置
    updateJointPositions();
    updateJointPositions2();

    // 绘制背景
    painter.fillRect(rect(), QColor(240, 245, 250));

    //// 绘制网格
    //painter.setPen(QPen(QColor(200, 220, 230), 1));
    //for (int x = 0; x < width(); x += 20) {
    //    painter.drawLine(x, 0, x, height());
    //}
    //for (int y = 0; y < height(); y += 20) {
    //    painter.drawLine(0, y, width(), y);
    //}

    //绘制矩形efem
    painter.setPen(QPen(QColor(50, 50, 100), 3));
    painter.setBrush(QColor(220, 230, 255, 180));
    QRectF rectangle2(baseCenter.x()-300, baseCenter.y()-200, 550.0, 350.0);
    painter.drawRect(rectangle2);

    // 绘制工位
    for (int i = 0; i < 5; ++i) {
        drawStation(painter, i);
    }

    // 绘制机械臂
    drawRobotArm(painter);   // 机械臂1
    drawRobotArm2(painter);  // 机械臂2

    // 绘制状态信息
    painter.setPen(Qt::darkBlue);
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    QString statusText;
    if (animationTimer->isActive()) {
        statusText = "运行中: " + stationNames[currentStation];
    } else {
        statusText = "已暂停";
    }

    painter.drawText(10, 20, statusText);

    // 绘制操作说明
    painter.setPen(Qt::darkGray);
    font.setPointSize(8);
    font.setBold(false);
    painter.setFont(font);
    painter.drawText(10, height() - 60, QString::fromUtf8("双臂机械手 - 俯视图腕关节重叠"));
    painter.drawText(10, height() - 40, "操作流程:loadPortA → Aligner→ LoadLockA");
}

void RobotArmWidget::drawStation(QPainter &painter, int index)
{
    QPointF station = stationPositions[index];
    double stationWidth = 80;
    double stationHeight = 50;

    // 绘制矩形工位
    painter.setPen(QPen(Qt::darkGray, 2));
    painter.setBrush(QColor(200, 200, 210));
    QRectF stationRect(station.x() - stationWidth/2, station.y() - stationHeight/2, 
                       stationWidth, stationHeight);
    painter.drawRect(stationRect);

    // 工位名称
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(9);
    painter.setFont(font);
    painter.drawText(stationRect, Qt::AlignCenter, stationNames[index]);

    // 工位状态 (晶圆) - 在矩形工位上方显示
    if (stationHasWafer[index]) {
        painter.setPen(QPen(QColor(70, 130, 180), 2));
        painter.setBrush(QColor(100, 180, 255, 200));
        QPointF waferPos(station.x(), station.y() - stationHeight/2 - 20);
        painter.drawEllipse(waferPos, 15, 15);

        // 绘制晶圆细节
        painter.setPen(QPen(QColor(50, 100, 150), 1));
        painter.drawEllipse(waferPos, 10, 10);
        painter.drawLine(waferPos.x() - 12, waferPos.y(), waferPos.x() + 12, waferPos.y());
        painter.drawLine(waferPos.x(), waferPos.y() - 12, waferPos.x(), waferPos.y() + 12);
    }
}

void RobotArmWidget::drawJoint(QPainter &painter, const QPointF &center, double radius)
{
    // 关节外部
    painter.setPen(QPen(Qt::darkGray, 2));
    painter.setBrush(QColor(120, 120, 140));
    painter.drawEllipse(center, radius, radius);

    // 关节内部
    painter.setPen(QPen(Qt::lightGray, 1));
    painter.setBrush(QColor(180, 180, 200));
    painter.drawEllipse(center, radius * 0.7, radius * 0.7);

    // 关节中心
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(90, 90, 110));
    painter.drawEllipse(center, radius * 0.3, radius * 0.3);

    // 关节螺栓
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::black);
    painter.drawEllipse(center, radius * 0.15, radius * 0.15);
}

// 绘制上臂
void RobotArmWidget::drawArmSegment(QPainter &painter, const QPointF &start, const QPointF &end, double width)
{
    // 计算线段的方向向量
    QPointF direction = end - start;
    double length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());

    if (length == 0) return;

    // 计算垂直方向的单位向量
    QPointF perpendicular(-direction.y() / length, direction.x() / length);
    perpendicular *= width / 2;

    // 创建臂段的四边形路径
    QPainterPath path;
    path.moveTo(start + perpendicular);
    path.lineTo(end + perpendicular);
    path.lineTo(end - perpendicular);
    path.lineTo(start - perpendicular);
    path.closeSubpath();

    // 设置画笔和画刷
    painter.setPen(QPen(QColor(80, 80, 100), 1));
    painter.setBrush(QColor(100, 100, 120));

    // 绘制臂段
    painter.drawPath(path);

    // 添加臂段细节（加强筋）
    painter.setPen(QPen(QColor(60, 60, 80), 1));

    // 在臂段中间绘制一条加强线
    QPointF midStart = start + direction * 0.3;
    QPointF midEnd = start + direction * 0.7;

    painter.drawLine(midStart + perpendicular * 0.8, midEnd + perpendicular * 0.8);
    painter.drawLine(midStart - perpendicular * 0.8, midEnd - perpendicular * 0.8);

    // 在臂段两端绘制连接环
    painter.setBrush(QColor(90, 90, 110));
    painter.drawEllipse(start, width/2, width/2);
    painter.drawEllipse(end, width/2, width/2);
}

void RobotArmWidget::drawRobotArm(QPainter &painter)
{
    // 保存当前状态
    painter.save();

    // 计算实际底座位置（考虑X轴偏移）
    QPointF actualBaseCenter = QPointF(baseCenter.x() + baseXOffset, baseCenter.y());

    // 1. 绘制底座（最大圆）
    painter.setPen(QPen(Qt::darkGray, 3));
    painter.setBrush(QColor(100, 100, 120));
    painter.drawEllipse(actualBaseCenter, baseRadius, baseRadius);

    // 底座细节
    painter.setPen(QPen(Qt::lightGray, 2));
    painter.drawEllipse(actualBaseCenter, baseRadius-10, baseRadius-10);
    painter.drawEllipse(actualBaseCenter, baseRadius-20, baseRadius-20);

    // 绘制X轴导轨（显示底座可平移）
    painter.setPen(QPen(QColor(150, 150, 150), 2, Qt::DashLine));
    painter.drawLine(baseCenter.x() - 300, baseCenter.y(), 
                     baseCenter.x() + 300, baseCenter.y());

    // 2. 肩关节位置 = 底座中心
    shoulderPos = actualBaseCenter;
    
    // 绘制肩关节
    drawJoint(painter, shoulderPos, 15);

    // 3. 绘制上臂（肩关节到肘关节）- 受底座旋转影响
    drawArmSegment(painter, shoulderPos, elbowPos, 20);

    // 4. 绘制肘关节
    drawJoint(painter, elbowPos, 12);

    // 5. 绘制前臂（肘关节到腕关节）
    drawArmSegment(painter, elbowPos, wristPos, 16);

    // 6. 绘制腕关节
    drawJoint(painter, wristPos, 10);

    // 7. 绘制末端执行器（铲子形状）- 末端朝向受底座旋转影响
    double actualWristAngle = baseRotation + wristAngle;  // 底座旋转 + 腕关节相对角度
    drawEndEffector(painter, wristPos, actualWristAngle, hasWafer);

    // 恢复原始状态
    painter.restore();
}

void RobotArmWidget::drawRobotArm2(QPainter &painter)
{
    // 保存当前状态
    painter.save();

    // 计算实际底座位置（考虑X轴偏移）
    QPointF actualBaseCenter = QPointF(baseCenter.x() + baseXOffset, baseCenter.y());

    // 机械臂2使用与机械臂1不同的颜色，以区分
    // 3. 绘制上臂（肩关节到肘关节）
    painter.setPen(QPen(QColor(100, 80, 80), 1));
    painter.setBrush(QColor(150, 100, 100));
    drawArmSegment(painter, shoulderPos2, elbowPos2, 20);

    // 4. 绘制肘关节
    drawJoint(painter, elbowPos2, 12);

    // 5. 绘制前臂（肘关节到腕关节）
    //    腕关节位置独立计算，在复位状态下与机械臂1重叠
    painter.setPen(QPen(QColor(100, 80, 80), 1));
    painter.setBrush(QColor(150, 100, 100));
    drawArmSegment(painter, elbowPos2, wristPos2, 16);

    // 6. 绘制腕关节（独立的关节）
    drawJoint(painter, wristPos2, 10);

    // 7. 绘制末端执行器（铲子形状） - 使用独立的腕关节角度
    //    机械臂2有自己独立的wristAngle2参数
    double actualWristAngle2 = baseRotation + wristAngle2;  // 底座旋转 + 腕关节2相对角度
    drawEndEffector(painter, wristPos2, actualWristAngle2, hasWafer2);

    // 恢复原始状态
    painter.restore();
}


void RobotArmWidget::drawEndEffector(QPainter &painter, const QPointF &wristPos, double rotation, bool hasWaferFlag)
{
       painter.save();
       painter.translate(wristPos.x(), wristPos.y());
       painter.rotate(rotation);

       // 绘制手柄（前臂到手指的连接杆）
       QRectF handleRect(-6, 0, 12, -handLength);
       painter.setPen(Qt::NoPen);
       painter.setBrush(QColor(90, 90, 110));
       painter.drawRect(handleRect);

       // 绘制铲形手指基座
       QRectF baseRect(-20, -handLength, 40, 10);
       painter.setPen(QPen(QColor(70, 70, 90), 1));
       painter.setBrush(QColor(100, 100, 130));
       painter.drawRect(baseRect);

       // 绘制U形凹陷的铲形手指
       QPainterPath bladePath;

       // 左侧支撑臂
       bladePath.moveTo(-20, -handLength);
       bladePath.lineTo(-20, -handLength - 25);
       bladePath.lineTo(-15, -handLength - 30);

       // U形凹陷底部（半圆形）
       bladePath.arcTo(-15, -handLength - 35, 30, 25, 180, 180);

       // 右侧支撑臂
       bladePath.lineTo(15, -handLength - 30);
       bladePath.lineTo(20, -handLength - 25);
       bladePath.lineTo(20, -handLength);

       bladePath.closeSubpath();

       painter.setPen(QPen(QColor(70, 70, 90), 1));
       painter.setBrush(QColor(100, 100, 130));
       painter.drawPath(bladePath);

       // 绘制晶圆片位置（U形凹陷中）
         QPointF waferCenter(0, -handLength - 25);

         // 如果持有晶圆，在U形凹陷中绘制晶圆
         if (hasWaferFlag) {
             // 绘制晶圆片（在U形凹陷中）
             painter.setPen(QPen(QColor(70, 130, 180), 2));
             painter.setBrush(QColor(100, 180, 255, 200));
             painter.drawEllipse(waferCenter, 12, 12);

             // 晶圆细节
             painter.setPen(QPen(QColor(50, 100, 150), 1));
             painter.drawEllipse(waferCenter, 8, 8);
             painter.drawEllipse(waferCenter, 4, 4);
             painter.drawLine(waferCenter - QPointF(8, 0), waferCenter + QPointF(8, 0));
             painter.drawLine(waferCenter - QPointF(0, 8), waferCenter + QPointF(0, 8));
         } else {
             // 绘制空的U形凹陷（晶圆槽）
             painter.setPen(QPen(QColor(150, 150, 170), 1));
             painter.setBrush(Qt::NoBrush);
             painter.drawEllipse(waferCenter, 12, 12);
         }

         // 绘制支撑臂上的防滑纹路
         painter.setPen(QPen(QColor(80, 80, 100), 1));

         // 左侧支撑臂纹路
         for (int i = 0; i < 3; i++) {
             painter.drawLine(QPointF(-18, -handLength - 5 - i*5),
                              QPointF(-18, -handLength - 10 - i*5));
         }

         // 右侧支撑臂纹路
         for (int i = 0; i < 3; i++) {
             painter.drawLine(QPointF(18, -handLength - 5 - i*5),
                              QPointF(18, -handLength - 10 - i*5));
         }

         painter.restore();

}



qreal RobotArmWidget::qDegreesToRadians(qreal degrees)
{
     return degrees * (M_PI / 180.0);
}

void RobotArmWidget::setBaseXOffset(double offset)
{
    if (qAbs(baseXOffset - offset) < 0.01) return;  // 避免重复更新
    
    baseXOffset = offset;
    updateJointPositions();
    update();
    emit baseXOffsetChanged(offset);  // 发出属性变化信号
    emit jointsChanged(baseXOffset, baseRotation, shoulderAngle, elbowAngle, wristAngle);
}

void RobotArmWidget::setBaseRotation(double angle)
{
       // 规范化角度到0-360范围
       double normalizedAngle = fmod(angle, 360.0); //求余数
       if (normalizedAngle < 0) normalizedAngle += 360.0;
       
       if (qAbs(baseRotation - normalizedAngle) < 0.01) return;  // 避免重复更新
       
       baseRotation = normalizedAngle;
       updateJointPositions();
       update();
       emit baseRotationChanged(baseRotation);  // 发出属性变化信号
       emit jointsChanged(baseXOffset, baseRotation, shoulderAngle, elbowAngle, wristAngle);
}

void RobotArmWidget::setShoulderAngle(double angle)
{
        // 限制在-180到180范围
        double normalizedAngle = fmod(angle, 360.0);
        if (normalizedAngle > 180) normalizedAngle -= 360;
        if (normalizedAngle < -180) normalizedAngle += 360;
        
        if (qAbs(shoulderAngle - normalizedAngle) < 0.01) return;

        shoulderAngle = normalizedAngle;
        updateJointPositions();
        update();
        emit shoulderAngleChanged(shoulderAngle);  // 发出属性变化信号
        emit jointsChanged(baseXOffset, baseRotation, shoulderAngle, elbowAngle, wristAngle);
}

void RobotArmWidget::setElbowAngle(double angle)
{
       double normalizedAngle = fmod(angle, 360.0);
       if (normalizedAngle > 180) normalizedAngle -= 360;
       if (normalizedAngle < -180) normalizedAngle += 360;
       
       if (qAbs(elbowAngle - normalizedAngle) < 0.01) return;

       elbowAngle = normalizedAngle;
       updateJointPositions();
       update();
       emit elbowAngleChanged(elbowAngle);  // 发出属性变化信号
       emit jointsChanged(baseXOffset, baseRotation, shoulderAngle, elbowAngle, wristAngle);
}

void RobotArmWidget::setWristAngle(double angle)
{
    double normalizedAngle = fmod(angle,360);
    if (normalizedAngle > 180) normalizedAngle -= 360;
    if (normalizedAngle < -180) normalizedAngle += 360;
    
    if (qAbs(wristAngle - normalizedAngle) < 0.01) return;
    
    wristAngle = normalizedAngle;
    updateJointPositions();
    update();
    emit wristAngleChanged(wristAngle);  // 发出属性变化信号
    emit jointsChanged(baseXOffset, baseRotation, shoulderAngle, elbowAngle, wristAngle);
}



void RobotArmWidget::setAllJoints(double baseAngle, double shoulderAngle, double elbowAngle,double wristAngle)
{
//       baseRotation = baseAngle;
//       this->shoulderAngle = shoulderAngle;
//       this->elbowAngle = elbowAngle;
//       this->wristAngle = wristAngle;
//       updateJointPositions();
//       update();
//       emit jointsChanged(baseRotation, shoulderAngle, elbowAngle,wristAngle);

     setBaseRotation(baseAngle);
     setShoulderAngle(shoulderAngle);
     setElbowAngle(elbowAngle);
     setWristAngle(wristAngle);
}

double RobotArmWidget::getBaseXOffset() const
{
    return baseXOffset;
}

double RobotArmWidget::getBaseRotation() const
{
    return baseRotation;
}

double RobotArmWidget::getShoulderAngle() const
{
    return shoulderAngle;
}

double RobotArmWidget::getElbowAngle() const
{
    return elbowAngle;
}

double RobotArmWidget::getWristAngle() const
{
    return wristAngle;
}

// ==== 机械臂2关节角度控制 ====

void RobotArmWidget::setShoulderAngle2(double angle)
{
    double normalizedAngle = fmod(angle, 360.0);
    if (normalizedAngle > 180) normalizedAngle -= 360;
    if (normalizedAngle < -180) normalizedAngle += 360;
    
    if (qAbs(shoulderAngle2 - normalizedAngle) < 0.01) return;

    shoulderAngle2 = normalizedAngle;
    updateJointPositions2();
    update();
}

void RobotArmWidget::setElbowAngle2(double angle)
{
    double normalizedAngle = fmod(angle, 360.0);
    if (normalizedAngle > 180) normalizedAngle -= 360;
    if (normalizedAngle < -180) normalizedAngle += 360;
    
    if (qAbs(elbowAngle2 - normalizedAngle) < 0.01) return;

    elbowAngle2 = normalizedAngle;
    updateJointPositions2();
    update();
}

void RobotArmWidget::setWristAngle2(double angle)
{
    double normalizedAngle = fmod(angle, 360.0);
    if (normalizedAngle > 180) normalizedAngle -= 360;
    if (normalizedAngle < -180) normalizedAngle += 360;
    
    if (qAbs(wristAngle2 - normalizedAngle) < 0.01) return;
    
    wristAngle2 = normalizedAngle;
    updateJointPositions2();
    update();
}

double RobotArmWidget::getShoulderAngle2() const
{
    return shoulderAngle2;
}

double RobotArmWidget::getElbowAngle2() const
{
    return elbowAngle2;
}

double RobotArmWidget::getWristAngle2() const
{
    return wristAngle2;
}

void RobotArmWidget::setAnimationMode(bool enable)
{
    if (enable) {
            animationTimer->start(30);
        } else {
            animationTimer->stop();
        }

}

void RobotArmWidget::setManualControlMode(bool enable)
{
    manualControlMode = enable;
        if (enable) {
            animationTimer->stop();
        }
}

void RobotArmWidget::stepAnimation()
{
    if (manualControlMode) {
           updateAnimation();
           update();
       }
}

void RobotArmWidget::savePreset(const QString &name)
{
      QSettings settings;
      settings.beginGroup("RobotArmPresets");
      settings.setValue(name + "/base", baseRotation);
      settings.setValue(name + "/shoulder", shoulderAngle);
      settings.setValue(name + "/elbow", elbowAngle);
      settings.setValue(name + "/Wrist", wristAngle);
      settings.endGroup();
}

void RobotArmWidget::loadPreset(const QString &name)
{
    QSettings settings;
    settings.beginGroup("RobotArmPresets");
    setAllJoints(
        settings.value(name + "/base", 0.0).toDouble(),
        settings.value(name + "/shoulder", safeShoulderAngle).toDouble(),
        settings.value(name + "/elbow", safeElbowAngle).toDouble(),
         settings.value(name + "/Wrist", safeWristAngle).toDouble()
    );
    settings.endGroup();
}



void RobotArmWidget::setAnimationSpeed(double speed)
{
    rotationSpeed = 1.0 * speed;
    extensionSpeed = 1.0 * speed;
}

double RobotArmWidget::getAnimationSpeed() const
{
    return rotationSpeed; // 返回任一速度值即可
}

void RobotArmWidget::updateAnimation()
{
    if (actionType == MOVING_TO_STATION) {
        moveToNextPose();
    }
    update();
}

// 移动到下一个姿态
void RobotArmWidget::moveToNextPose()
{
    if (currentPoseIndex >= animationSequence.size()) {
        // 循环播放
        currentPoseIndex = 0;
        qDebug() << QString::fromUtf8("🔄 循环播放");
    }
    
    const Pose &targetPose = animationSequence[currentPoseIndex];
    
    // 计算距离
    double baseXDiff = qAbs(baseXOffset - targetPose.baseX);
    double baseRotDiff = qAbs(baseRotation - targetPose.baseRot);
    double shoulderDiff = qAbs(shoulderAngle - targetPose.shoulder);
    double elbowDiff = qAbs(elbowAngle - targetPose.elbow);
    double wristDiff = qAbs(wristAngle - targetPose.wrist);
    
    // 判断是否到达目标姿态
    if (baseXDiff < 0.5 && baseRotDiff < 0.5 && 
        shoulderDiff < 0.5 && elbowDiff < 0.5 && wristDiff < 0.5) {
        
        // 精确设置
        baseXOffset = targetPose.baseX;
        baseRotation = targetPose.baseRot;
        shoulderAngle = targetPose.shoulder;
        elbowAngle = targetPose.elbow;
        wristAngle = targetPose.wrist;
        
        qDebug() << QString::fromUtf8("✅ 完成:") << targetPose.name;
        
        // 移动到下一个姿态
        currentPoseIndex++;
        return;
    }
    
    // 平滑移动到目标（自适应步进）
    auto smoothMove = [](double &current, double target, double speed) {
        double diff = target - current;
        double step = qMin(qAbs(diff), speed);
        current += (diff > 0 ? step : -step);
    };
    
    smoothMove(baseXOffset, targetPose.baseX, rotationSpeed);
    smoothMove(baseRotation, targetPose.baseRot, rotationSpeed);
    smoothMove(shoulderAngle, targetPose.shoulder, extensionSpeed);
    smoothMove(elbowAngle, targetPose.elbow, extensionSpeed);
    smoothMove(wristAngle, targetPose.wrist, extensionSpeed);
}

// ========== 新增属性 Setter 方法实现 ==========

void RobotArmWidget::setBaseRadius(double radius)
{
    if (qAbs(baseRadius - radius) < 0.01 || radius <= 0) return;
    
    baseRadius = radius;
    update();
    emit baseRadiusChanged(radius);
}

void RobotArmWidget::setUpperArmLength(double length)
{
    if (qAbs(upperArmLength - length) < 0.01 || length <= 0) return;
    
    upperArmLength = length;
    updateJointPositions();
    update();
    emit upperArmLengthChanged(length);
}

void RobotArmWidget::setForearmLength(double length)
{
    if (qAbs(forearmLength - length) < 0.01 || length <= 0) return;
    
    forearmLength = length;
    updateJointPositions();
    update();
    emit forearmLengthChanged(length);
}

void RobotArmWidget::setHandLength(double length)
{
    if (qAbs(handLength - length) < 0.01 || length <= 0) return;
    
    handLength = length;
    update();
    emit handLengthChanged(length);
}

void RobotArmWidget::setHasWafer(bool has)
{
    if (hasWafer == has) return;
    
    hasWafer = has;
    update();
    emit hasWaferChanged(has);
}

void RobotArmWidget::setHasWafer2(bool has)
{
    if (hasWafer2 == has) return;
    
    hasWafer2 = has;
    update();
}

void RobotArmWidget::setSafeShoulderAngle(double angle)
{
    if (qAbs(safeShoulderAngle - angle) < 0.01) return;
    
    safeShoulderAngle = angle;
    emit safeShoulderAngleChanged(angle);
}

void RobotArmWidget::setSafeElbowAngle(double angle)
{
    if (qAbs(safeElbowAngle - angle) < 0.01) return;
    
    safeElbowAngle = angle;
    emit safeElbowAngleChanged(angle);
}

void RobotArmWidget::setSafeWristAngle(double angle)
{
    if (qAbs(safeWristAngle - angle) < 0.01) return;
    
    safeWristAngle = angle;
    emit safeWristAngleChanged(angle);
}

void RobotArmWidget::setExtendShoulderAngle(double angle)
{
    if (qAbs(extendShoulderAngle - angle) < 0.01) return;
    
    extendShoulderAngle = angle;
    emit extendShoulderAngleChanged(angle);
}

void RobotArmWidget::setExtendElbowAngle(double angle)
{
    if (qAbs(extendElbowAngle - angle) < 0.01) return;
    
    extendElbowAngle = angle;
    emit extendElbowAngleChanged(angle);
}

void RobotArmWidget::setExtendShoulderAngle2(double angle)
{
    if (qAbs(extendShoulderAngle2 - angle) < 0.01) return;
    
    extendShoulderAngle2 = angle;
    emit extendShoulderAngleChanged(angle);  // 可以复用现有信号
}

void RobotArmWidget::setExtendElbowAngle2(double angle)
{
    if (qAbs(extendElbowAngle2 - angle) < 0.01) return;
    
    extendElbowAngle2 = angle;
    emit extendElbowAngleChanged(angle);  // 可以复用现有信号
}

// ========== 双臂同步取放片功能实现 ==========

// 取片动作：上臂和前臂同步伸出，手指朝向工位取片
void RobotArmWidget::pickWafer(int armIndex)
{
    qDebug() << QString::fromUtf8("====== 机械臂") << armIndex << QString::fromUtf8("取片动作 ======");
    
    if (armIndex == 1) {
        // 机械臂1（下臂）取片
        // 同步动作：肩关节前伸 + 肘关节同步伸出
        double targetShoulder = extendShoulderAngle;  // 使用伸展位置角度
        double targetElbow = extendElbowAngle;        // 使用伸展位置角度
        
        setShoulderAngle(targetShoulder);
        setElbowAngle(targetElbow);
        
        // 更新关节位置
        updateJointPositions();
        
        // 设置持有晶圆标志
        setHasWafer(true);
        
        qDebug() << QString::fromUtf8("机械臂1 取片完成:") 
                 << QString::fromUtf8("肩关节=") << targetShoulder << "°, "
                 << QString::fromUtf8("肘关节=") << targetElbow << "°";
    } 
    else if (armIndex == 2) {
        // 机械臂2（上臂）取片
        // 同步动作：肩关节前伸 + 肘关节同步伸出
        // 注意：机械臂2的角度需要根据对称关系调整
        double targetShoulder2 = extendShoulderAngle2;  // 对称180°
        double targetElbow2 = extendElbowAngle2;             // 肘关节取负值
        
        setShoulderAngle2(targetShoulder2);
        setElbowAngle2(targetElbow2);
        
        // 更新关节位置
        updateJointPositions2();
        
        // 设置持有晶圆标志
        setHasWafer2(true);
        
        qDebug() << QString::fromUtf8("机械臂2 取片完成:") 
                 << QString::fromUtf8("肩关节=") << targetShoulder2 << "°, "
                 << QString::fromUtf8("肘关节=") << targetElbow2 << "°";
    }
    
    update();  // 刷新显示
}

// 放片动作：上臂和前臂同步伸出，手指朝向工位放片
void RobotArmWidget::placeWafer(int armIndex)
{
    qDebug() << QString::fromUtf8("====== 机械臂") << armIndex << QString::fromUtf8("放片动作 ======");
    
    if (armIndex == 1) {
        // 机械臂1（下臂）放片
        // 同步动作：肩关节前伸 + 肘关节同步伸出
        double targetShoulder = extendShoulderAngle;
        double targetElbow = extendElbowAngle;
        
        setShoulderAngle(targetShoulder);
        setElbowAngle(targetElbow);
        
        // 更新关节位置
        updateJointPositions();
        
        // 清除持有晶圆标志
        setHasWafer(false);
        
        qDebug() << QString::fromUtf8("机械臂1 放片完成:") 
                 << QString::fromUtf8("肩关节=") << targetShoulder << "°, "
                 << QString::fromUtf8("肘关节=") << targetElbow << "°";
    } 
    else if (armIndex == 2) {
        // 机械臂2（上臂）放片
        double targetShoulder2 = extendShoulderAngle2;
        double targetElbow2 = extendElbowAngle2;
        
        setShoulderAngle2(targetShoulder2);
        setElbowAngle2(targetElbow2);
        
        // 更新关节位置
        updateJointPositions2();
        
        // 清除持有晶圆标志
        setHasWafer2(false);
        
        qDebug() << QString::fromUtf8("机械臂2 放片完成:") 
                 << QString::fromUtf8("肩关节=") << targetShoulder2 << "°, "
                 << QString::fromUtf8("肘关节=") << targetElbow2 << "°";
    }
    
    update();  // 刷新显示
}

// 伸出到工位：上臂和前臂同步伸出
void RobotArmWidget::extendToStation(int armIndex)
{
    qDebug() << QString::fromUtf8("====== 机械臂") << armIndex << QString::fromUtf8("伸出到工位 ======");
    
    if (armIndex == 1) {
        // 机械臂1伸出
        double targetShoulder = extendShoulderAngle;
        double targetElbow = extendElbowAngle;
        
        setShoulderAngle(targetShoulder);
        setElbowAngle(targetElbow);
        updateJointPositions();
        
        qDebug() << QString::fromUtf8("机械臂1 伸出完成:") 
                 << QString::fromUtf8("肩关节=") << targetShoulder << "°, "
                 << QString::fromUtf8("肘关节=") << targetElbow << "°";
    } 
    else if (armIndex == 2) {
        // 机械臂2伸出
        double targetShoulder2 = extendShoulderAngle2;
        double targetElbow2 = extendElbowAngle2;
        
        setShoulderAngle2(targetShoulder2);
        setElbowAngle2(targetElbow2);
        updateJointPositions2();
        
        qDebug() << QString::fromUtf8("机械臂2 伸出完成:") 
                 << QString::fromUtf8("肩关节=") << targetShoulder2 << "°, "
                 << QString::fromUtf8("肘关节=") << targetElbow2 << "°";
    }
    
    update();
}

// 从工位缩回：上臂和前臂同步缩回
void RobotArmWidget::retractFromStation(int armIndex)
{
    qDebug() << QString::fromUtf8("====== 机械臂") << armIndex << QString::fromUtf8("从工位缩回 ======");
    
    if (armIndex == 1) {
        // 机械臂1缩回到安全位置
        double targetShoulder = safeShoulderAngle;
        double targetElbow = safeElbowAngle;
        
        setShoulderAngle(targetShoulder);
        setElbowAngle(targetElbow);
        updateJointPositions();
        
        qDebug() << QString::fromUtf8("机械臂1 缩回完成:") 
                 << QString::fromUtf8("肩关节=") << targetShoulder << "°, "
                 << QString::fromUtf8("肘关节=") << targetElbow << "°";
    } 
    else if (armIndex == 2) {
        // 机械臂2缩回到安全位置
        double targetShoulder2 = safeShoulderAngle + 142;
        //double targetShoulder2 = safeShoulderAngle;
        double targetElbow2 = -safeElbowAngle;
        
        setShoulderAngle2(targetShoulder2);
        setElbowAngle2(targetElbow2);
        updateJointPositions2();
        
        qDebug() << QString::fromUtf8("机械臂2 缩回完成:") 
                 << QString::fromUtf8("肩关节=") << targetShoulder2 << "°, "
                 << QString::fromUtf8("肘关节=") << targetElbow2 << "°";
    }
    
    update();
}
