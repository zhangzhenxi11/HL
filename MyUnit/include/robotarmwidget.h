#ifndef ROBOTARMWIDGET_H
#define ROBOTARMWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>  //并行
#include <QSequentialAnimationGroup>//串行
#include <QQueue>
#pragma execution_character_set("utf-8")
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif

namespace Ui {
class RobotArmWidget;
}
class MYCUSTOMLIB_API RobotArmWidget : public QWidget
{
    Q_OBJECT
    
    // ========== Qt 属性系统 ==========
    // 关节控制属性
    Q_PROPERTY(double baseXOffset READ getBaseXOffset WRITE setBaseXOffset NOTIFY baseXOffsetChanged)
    Q_PROPERTY(double baseRotation READ getBaseRotation WRITE setBaseRotation NOTIFY baseRotationChanged)
    Q_PROPERTY(double shoulderAngle READ getShoulderAngle WRITE setShoulderAngle NOTIFY shoulderAngleChanged)
    Q_PROPERTY(double elbowAngle READ getElbowAngle WRITE setElbowAngle NOTIFY elbowAngleChanged)
    Q_PROPERTY(double wristAngle READ getWristAngle WRITE setWristAngle NOTIFY wristAngleChanged)
    // 机械臂2关节控制属性
    Q_PROPERTY(double shoulderAngle2 READ getShoulderAngle2 WRITE setShoulderAngle2 NOTIFY shoulderAngle2Changed)
    Q_PROPERTY(double elbowAngle2 READ getElbowAngle2 WRITE setElbowAngle2 NOTIFY elbowAngle2Changed)
    Q_PROPERTY(double wristAngle2 READ getWristAngle2 WRITE setWristAngle2 NOTIFY wristAngle2Changed)
    
    // 尺寸参数属性
    Q_PROPERTY(double baseRadius READ getBaseRadius WRITE setBaseRadius NOTIFY baseRadiusChanged)
    Q_PROPERTY(double upperArmLength READ getUpperArmLength WRITE setUpperArmLength NOTIFY upperArmLengthChanged)
    Q_PROPERTY(double forearmLength READ getForearmLength WRITE setForearmLength NOTIFY forearmLengthChanged)
    Q_PROPERTY(double handLength READ getHandLength WRITE setHandLength NOTIFY handLengthChanged)
    
    // 动画控制属性
    Q_PROPERTY(double animationSpeed READ getAnimationSpeed WRITE setAnimationSpeed NOTIFY animationSpeedChanged)
    Q_PROPERTY(double rotationSpeed READ getRotationSpeed WRITE setRotationSpeed NOTIFY rotationSpeedChanged)
    Q_PROPERTY(double extensionSpeed READ getExtensionSpeed WRITE setExtensionSpeed NOTIFY extensionSpeedChanged)
    Q_PROPERTY(double armSpeed READ getArmSpeed WRITE setArmSpeed NOTIFY armSpeedChanged)
    Q_PROPERTY(bool manualControlMode READ isManualControlMode WRITE setManualControlMode NOTIFY manualControlModeChanged)
    Q_PROPERTY(bool hasWafer READ getHasWafer WRITE setHasWafer NOTIFY hasWaferChanged)
    
    // 安全位置属性
    Q_PROPERTY(double safeShoulderAngle READ getSafeShoulderAngle WRITE setSafeShoulderAngle NOTIFY safeShoulderAngleChanged)
    Q_PROPERTY(double safeElbowAngle READ getSafeElbowAngle WRITE setSafeElbowAngle NOTIFY safeElbowAngleChanged)
    Q_PROPERTY(double safeWristAngle READ getSafeWristAngle WRITE setSafeWristAngle NOTIFY safeWristAngleChanged)
    
    // 伸展位置属性
    Q_PROPERTY(double extendShoulderAngle READ getExtendShoulderAngle WRITE setExtendShoulderAngle NOTIFY extendShoulderAngleChanged)
    Q_PROPERTY(double extendElbowAngle READ getExtendElbowAngle WRITE setExtendElbowAngle NOTIFY extendElbowAngleChanged)

    
public:
    explicit RobotArmWidget(QWidget *parent = nullptr);
    //动画开始，停止，重置
    void startAnimation();
    void pauseAnimation();
    void resetAnimation();
    void loadAnimationFromJson(const QString &jsonFilePath);  // 新增：从 JSON 加载动画
    void loadStationRobotStatusFromJson(const QString& jsonFilePath,int stationId, QString& status); //加载不同工位的状态

    void startAnimationGroup();
    void clearAnimationGroup();
    //void pauseAnimationGroup();
    void animationPauseGroup();
    void animationResumeGroup();
    void animationAbortGroup();
    void reset(); //复位

signals:
    void animationStateChanged(const QString &state);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;
private:
    //动画
    void updateAnimation();
     // 更新关节位置
    void updateJointPositions();
    // 新的动画状态机函数（基于用户记录的姿态数据）
    void moveToNextPose();  // 移动到下一个姿态
    void drawRobotArm(QPainter &painter);
    void drawRobotArm2(QPainter &painter);
    // 更新机械臂2的关节位置
    void updateJointPositions2();
    void drawStation(QPainter &painter, int stationIndex);
    void drawJoint(QPainter &painter, const QPointF &center, double radius);
    void drawArmSegment(QPainter &painter, const QPointF &start, const QPointF &end, double width);
    // 添加半导体机械手特有的方法
    void drawEndEffector(QPainter &painter, const QPointF &wristPos, double rotation, bool hasWaferFlag);
    // 如果 Qt 版本太低，可以自己定义
    qreal qDegreesToRadians(qreal degrees);

    

public:

    void SetName(std::string Name);
    int  getCurrentStation() const;
    void setCurrentStation(int station);
    int  getCurrentArm() const;
    void setCurrentArm(int arm);
    // 设置底座参数
    void setBaseXOffset(double offset);    // 设置底座X轴偏移
    void setBaseRotation(double angle);    // 设置底座旋转

    // 获取当前关节角度
    double getBaseXOffset() const;         // 获取底座X轴偏移
    double getBaseRotation() const;

    // 设置机械臂1关节角度
    void setShoulderAngle(double angle);
    void setElbowAngle(double angle);
    void setWristAngle(double angle);
    
    // 设置机械臂2关节角度
    void setShoulderAngle2(double angle);
    void setElbowAngle2(double angle);
    void setWristAngle2(double angle);

    // 设置所有关节角度
    void setAllJoints(double baseAngle, double shoulderAngle, double elbowAngle,double wristAngle);

    // 获取机械臂1关节角度
    double getShoulderAngle() const;
    double getElbowAngle() const;
    double getWristAngle() const;
    
    // 获取机械臂2关节角度
    double getShoulderAngle2() const;
    double getElbowAngle2() const;
    double getWristAngle2() const;
    
    // 获取尺寸参数
    double getBaseRadius() const { return baseRadius; }
    double getUpperArmLength() const { return upperArmLength; }
    double getForearmLength() const { return forearmLength; }
    double getHandLength() const { return handLength; }
    
    // 设置尺寸参数
    void setBaseRadius(double radius);
    void setUpperArmLength(double length);
    void setForearmLength(double length);
    void setHandLength(double length);
    
    // 获取动画控制参数
    bool isManualControlMode() const { return manualControlMode; }
    bool getHasWafer() const { return hasWafer; }
    bool getHasWafer2() const { return hasWafer2; }
    
    // 设置动画控制参数
    void setHasWafer(bool has);
    void setHasWafer2(bool has);
    
    // 获取安全位置角度
    double getSafeShoulderAngle() const { return safeShoulderAngle; }
    double getSafeElbowAngle() const { return safeElbowAngle; }
    double getSafeWristAngle() const { return safeWristAngle; }
    
    // 设置安全位置角度
    void setSafeShoulderAngle(double angle);
    void setSafeElbowAngle(double angle);
    void setSafeWristAngle(double angle);
    
    // 获取伸展位置角度
    double getExtendShoulderAngle() const { return extendShoulderAngle; }
    double getExtendElbowAngle() const { return extendElbowAngle; }
    double getExtendShoulderAngle2() const { return extendShoulderAngle2; }  // 新增：机械臂2
    double getExtendElbowAngle2() const { return extendElbowAngle2; }        // 新增：机械臂2
    
    // 设置伸展位置角度
    void setExtendShoulderAngle(double angle);
    void setExtendElbowAngle(double angle);
    void setExtendShoulderAngle2(double angle);  // 新增：机械臂2
    void setExtendElbowAngle2(double angle);     // 新增：机械臂2
    
    // 双臂同步取放片功能
    void pickWafer(int stationId,int armIndex = 1);     // 取片动作（armIndex: 1=下臂, 2=上臂）
    void placeWafer(int stationId, int armIndex = 1);    // 放片动作（armIndex: 1=下臂, 2=上臂）
    void extendToStation(int stationId, int armIndex = 1); // 伸出到工位（armIndex: 1=下臂, 2=上臂）
    void retractFromStation(int stationId, int armIndex = 1); // 从工位缩回（armIndex: 1=下臂, 2=上臂）
    void stationRobotStatus(int stationId);
    // 模式控制
    void setAnimationMode(bool enable);
    void setManualControlMode(bool enable);
    void stepAnimation();

    // 预设管理
    void savePreset(const QString &name);
    void loadPreset(const QString &name);

    // 设置动画速度（1.0为正常速度）
    void setAnimationSpeed(double speed);

    // 获取当前动画速度
    double getAnimationSpeed() const;

    void setArmSpeed(int speed);

    int getArmSpeed()const;

    //设置旋转速度
    void setRotationSpeed(double speed);
    // 获取旋转速度
    double getRotationSpeed()const;

    //设置伸缩速度
    void setExtensionSpeed(double speed);
    // 获取伸缩速度
    double getExtensionSpeed()const;

signals:
    void jointsChanged(double baseX, double baseRot, double shoulder, double elbow, double wrist);
    void animationFinished();
    
    // 属性变化信号
    void baseXOffsetChanged(double offset);
    void baseRotationChanged(double angle);
    void shoulderAngleChanged(double angle);
    void elbowAngleChanged(double angle);
    void wristAngleChanged(double angle);
    // 机械臂2属性变化信号
    void shoulderAngle2Changed(double angle);
    void elbowAngle2Changed(double angle);
    void wristAngle2Changed(double angle);

    void baseRadiusChanged(double radius);
    void upperArmLengthChanged(double length);
    void forearmLengthChanged(double length);
    void handLengthChanged(double length);
    
    void animationSpeedChanged(double speed);
    void manualControlModeChanged(bool enabled);
    void hasWaferChanged(bool has);
    void rotationSpeedChanged(double speed);
    void extensionSpeedChanged(double speed);
    void armSpeedChanged(double speed);
    
    void safeShoulderAngleChanged(double angle);
    void safeElbowAngleChanged(double angle);
    void safeWristAngleChanged(double angle);
    
    void extendShoulderAngleChanged(double angle);
    void extendElbowAngleChanged(double angle);
    
    void signalERobotDialog(); // 定义一个信号
    void signalERobotReset(); // 定义一个信号
    void signalERobotGetStatus(); // 定义一个信号
    void signalERobotClearError(); // 定义一个信号

public:

    QSequentialAnimationGroup* animationGroup;
    QParallelAnimationGroup*   parallelAnimationGroup;

    enum ActionType {
        IDLE,
        MOVING_TO_STATION,    // 平移+旋转到工位
        EXTENDING,            // 伸出取放片
        RETRACTING            // 缩回
    };
    //ELP1,ELP2,LLA,LLB,EALIGNER
    enum stationID {
        STATIONIDLP1=1,
        STATIONIDLP2 = 2,
        STATIONIDELK1,
        STATIONIDELK2,
        STATIONIDEALIGNER,
        STATIONORIGIN,
    };

    // 姿态数据结构
    struct Pose {
        QString name;
        double baseX;
        double baseRot;
        double shoulder;
        double elbow;
        double wrist;
    };

    // 动画定时器
    QTimer *animationTimer;

    // 机械臂关节参数
    QPointF baseCenter;    // 底座中心位置（固定参考点）
    double baseXOffset;    // 底座X轴平移偏移量（线性导轨）
    double baseRotation;   // 底座旋转角度(度)（整体旋转）
    int targetBaseX = 0;
    int targetBaseRot = 0;

    //double currentbaseXOffset;//实时X轴平移偏移量
    //double currentbaseRotation;//实时底座旋转角度

    // 机械臂1关节角度
    double shoulderAngle;  // 肩关节角度(度)
    double elbowAngle;     // 肘关节角度(度)
    double wristAngle;     // 腕关节角度(度)
    
    // 机械臂2关节角度（独立控制）
    double shoulderAngle2;  // 肩关节角度(度)
    double elbowAngle2;     // 肘关节角度(度)
    double wristAngle2;     // 腕关节角度(度)

    // 角度参数
    double safeShoulderAngle;   // 安全位置肩关节角度
    double safeElbowAngle;      // 安全位置肘关节角度
    double safeWristAngle;      // 安全位置腕关节角度
    
    // 机械臂1伸展位置角度
    double extendShoulderAngle; // 伸展位置肩关节角度
    double extendElbowAngle;    // 伸展位置肘关节角度
    
    // 机械臂2伸展位置角度
    double extendShoulderAngle2; // 机械臂2伸展位置肩关节角度
    double extendElbowAngle2;    // 机械臂2伸展位置肘关节角度

    // 动画参数
    int currentArm;                     //记录当前取放手爪
    int currentStation;                 // 当前目标工位
    ActionType actionType;
    int currentPoseIndex;               // 当前姿态索引
    QVector<Pose> animationSequence;    // 动画序列
    
    // 动画队列相关
    struct AnimationTask {
        QString type;       // "retract", "pick", "place", "extend"
        int stationId;
        int armIndex;
    };

    QQueue<AnimationTask> animationQueue;  // 动画任务队列

    bool isAnimationRunning;               // 动画是否正在运行
    
    // 初始化动画序列（基于用户记录的数据）
    void initAnimationSequence();   // 当前动作类型
    
    // 动画队列管理方法
    void queueAnimation(const QString& type, int stationId, int armIndex);
    void processNextAnimation();
    double rotationSpeed;    // 旋转速度(度/帧)
    double extensionSpeed;   // 伸缩速度(单位/帧)
    bool hasWafer;           // 机械手是否持有晶圆
    bool manualControlMode;  // 手动控制模式
    double armSpeed;         //机械臂整体速度

    // 工位信息
    QVector<QPointF> stationPositions;
    QStringList stationNames;
    QVector<bool> stationHasWafer;
    QVector<QPointF> normals; // 存储每条边的法线方向
    // 尺寸参数
    double chamberRadius;
    double baseRadius;
    double upperArmLength;//上臂长
    double forearmLength; //前臂长
    double handLength;    //手指长

    // 机械臂1计算关节位置
    QPointF shoulderPos;//肩关节位置
    QPointF elbowPos;//肘关节位置
    QPointF wristPos;//腕关节位置

    // 机械臂2计算关节位置
    QPointF shoulderPos2;//肩关节位置
    QPointF elbowPos2;//肘关节位置
    QPointF wristPos2;//腕关节位置
    
    // 机械臂2是否持有晶圆
    bool hasWafer2;
    std::string name;

};

#endif // ROBOTARMWIDGET_H
