# MES系统基础知识及对接开发指南

## 一、MES系统基础知识

### 1. MES系统定义
**MES（Manufacturing Execution System，制造执行系统）** 是位于上层计划管理系统（ERP）与底层工业控制之间的面向车间层的管理信息系统。

### 2. MES在半导体行业的核心功能
| 功能模块 | 作用                   | 半导体行业特点         |
| -------- | ---------------------- | ---------------------- |
| 设备管理 | 设备状态监控、维护管理 | 高精度设备，OEE计算    |
| 生产调度 | 工单排程、资源分配     | 复杂工艺流程，批量控制 |
| 物料追踪 | 晶圆批次跟踪、WIP管理  | 严格的批次追溯         |
| 质量管理 | SPC统计、异常处理      | 高良率要求，实时监控   |
| 数据采集 | 设备数据收集、参数记录 | SECS/GEM标准协议       |

### 3. 半导体设备与MES交互场景
```
设备端 → MES系统
├── 设备状态报告（开机、运行、维护、故障）
├── 生产开始/完成事件
├── 工艺参数上传
├── 报警信息推送
├── 物料消耗记录
└── 质量数据上报
```

## 二、对接MES接口的模块划分

### 1. 核心模块架构
```
┌─────────────────────────────────────────┐
│            MES对接功能模块               │
├─────────────────────────────────────────┤
│ 1. 配置管理模块    │ 2. 通信协议模块     │
│   - 参数配置       │   - HTTP/REST      │
│   - 规则配置       │   - SOAP/WebService│
│   - 映射配置       │   - TCP/Socket     │
├─────────────────────────────────────────┤
│ 3. 数据转换模块    │ 4. 消息队列模块     │
│   - 格式转换       │   - 异步处理       │
│   - 数据验证       │   - 批量发送       │
│   - 映射关系       │   - 重试机制       │
├─────────────────────────────────────────┤
│ 5. 事件管理模块    │ 6. 状态监控模块     │
│   - 事件触发       │   - 连接状态       │
│   - 优先级处理     │   - 发送状态       │
│   - 过滤规则       │   - 性能统计       │
└─────────────────────────────────────────┘
```

### 2. 详细模块设计

#### 模块1：配置管理模块
```cpp
class MESConfigManager {
public:
    // MES服务器配置
    struct ServerConfig {
        QString protocol;      // "HTTP"/"SOAP"/"TCP"
        QString host;
        int port;
        QString baseUrl;
        int timeoutMs;
    };
    
    // 认证配置
    struct AuthConfig {
        QString username;
        QString password;
        QString token;
        QString certificatePath;
    };
    
    // 数据映射配置
    struct MappingConfig {
        QMap<QString, QString> logLevelMapping;  // INFO→Information
        QMap<QString, QString> moduleMapping;    // vtm→VacuumTransferModule
        QMap<QString, int> eventCodeMapping;     // 报警代码映射
    };
};
```

#### 模块2：通信协议模块
```cpp
class MESCommunication {
public:
    virtual bool connect() = 0;
    virtual bool sendEvent(const MESEvent& event) = 0;
    virtual bool queryData(const QString& query) = 0;
};

// HTTP REST实现
class HTTPMESCommunication : public MESCommunication {
private:
    QNetworkAccessManager* networkManager;
    MESConfigManager::ServerConfig serverConfig;
    
public:
    bool sendEvent(const MESEvent& event) override {
        QJsonObject json = event.toJson();
        // 发送到MES REST API
    }
};

// SECS/GEM协议实现（半导体专用）
class SECSGEMCommunication : public MESCommunication {
    // 实现半导体设备标准协议
};
```

#### 模块3：数据转换模块
```cpp
class DataTransformer {
public:
    // 日志转MES事件
    static MESEvent transformLogToEvent(const LogEntry& log) {
        MESEvent event;
        event.eventId = generateEventId();
        event.equipmentId = getEquipmentId();
        event.eventType = mapLogLevelToEventType(log.level);
        event.eventCode = getEventCode(log.module, log.message);
        event.timestamp = log.timestamp;
        event.description = log.message;
        event.parameters = extractParameters(log.message);
        return event;
    }
    
private:
    static QString mapLogLevelToEventType(const QString& logLevel) {
        static QMap<QString, QString> mapping = {
            {"INFO", "Information"},
            {"WARN", "Warning"},
            {"ERROR", "Alarm"}
        };
        return mapping.value(logLevel, "Information");
    }
};
```

#### 模块4：消息队列模块
```cpp
class MESMessageQueue : public QObject {
    Q_OBJECT
private:
    QQueue<MESEvent> pendingEvents;
    QTimer* batchTimer;
    bool isSending = false;
    
public slots:
    void enqueueEvent(const MESEvent& event) {
        pendingEvents.enqueue(event);
        if (pendingEvents.size() >= batchSize) {
            processBatch();
        }
    }
    
    void processBatch() {
        if (isSending || pendingEvents.isEmpty()) return;
        
        isSending = true;
        QList<MESEvent> batch;
        for (int i = 0; i < batchSize && !pendingEvents.isEmpty(); ++i) {
            batch.append(pendingEvents.dequeue());
        }
        
        sendBatchToMES(batch);
    }
};
```

## 三、需要向客户提出的需求清单

### 1. 接口协议需求
```
□ 请提供MES系统支持的接口协议类型：
  □ HTTP REST API
  □ SOAP WebService  
  □ TCP Socket
  □ SECS/GEM（半导体标准）
  □ 其他：________________

□ 请提供具体的接口地址和端口信息
□ 请说明是否需要SSL/TLS加密
```

### 2. 认证授权需求
```
□ 请提供认证方式：
  □ 用户名/密码
  □ Token认证
  □ 证书认证
  □ IP白名单
  □ 其他：________________

□ 请提供测试环境的认证凭据
□ 请说明Token的刷新机制和有效期
```

### 3. 数据格式需求
```
□ 请提供事件数据的标准格式：
  □ JSON Schema定义
  □ XML Schema定义
  □ 字段映射表

□ 关键字段要求：
  □ 设备标识字段名：________________
  □ 时间戳格式：________________
  □ 事件类型编码规则：________________
  □ 报警级别映射关系：________________
```

### 4. 业务规则需求
```
□ 请说明需要上传的日志类型：
  □ 所有日志
  □ 仅WARN和ERROR级别
  □ 特定模块的日志（请列出）：________________

□ 请提供事件代码对照表：
  □ 设备状态事件代码
  □ 工艺参数事件代码  
  □ 报警事件代码
  □ 维护事件代码

□ 实时性要求：
  □ 实时上传（<1秒）
  □ 批量上传（请说明批量和间隔）
  □ 定时上传
```

### 5. 测试验证需求
```
□ 请提供MES测试环境访问权限
□ 请提供接口测试用例
□ 请提供数据验证工具或方法
□ 请指定对接测试负责人和联系方式
```

### 6. 性能与可靠性需求
```
□ 请说明MES系统的服务时间（7×24小时？）
□ 请说明网络中断时的处理要求：
  □ 本地缓存后重传
  □ 丢弃过期数据
  □ 其他：________________

□ 请说明数据量的预估：
  □ 每秒最大事件数：________________
  □ 每日总数据量：________________
```

## 四、VTM项目特定的MES对接需求分析

### 1. 基于实际日志的事件分类

从`2025-10-17.txt`日志分析，系统产生的关键事件类型：

```cpp
// MES事件分类定义（基于VTM项目实际日志 - 2025-10-17 & 2025-08-28）
enum class MESEventCategory {
    SYSTEM_LIFECYCLE,      // 系统生命周期事件（启动/关闭/复位）
    MODULE_CONFIG,         // 模块配置事件
    DEVICE_STATE,          // 设备状态变化
    VACUUM_MONITORING,     // 真空度监控（关键工艺参数）
    COMMUNICATION_ERROR,   // 通信异常
    PROCESS_DATA,          // 工艺数据
    ALARM_WARNING,         // 报警告警
    ROBOT_MOTION,          // 机械手运动事件
    LOADPORT_OPERATION,    // 负载端口操作
    ALIGNER_OPERATION,     // 对准器操作
    WAFER_TRACKING,        // 晶圆追踪
    RECIPE_EXECUTION,      // 配方执行
    VALVE_CONTROL,         // 阀门控制
    TIMEOUT_EVENT          // 超时事件
};

// 实际日志事件映射表（扩展版 - 基于 8_22日志.xlsx Sheet7）
struct VTMLogToMESMapping {
    // 1. 系统生命周期事件
    {"core module start",       MESEventCategory::SYSTEM_LIFECYCLE, "设备启动"},
    {"core module end",         MESEventCategory::SYSTEM_LIFECYCLE, "设备关闭"},
    {"reset:整机复位开始",       MESEventCategory::SYSTEM_LIFECYCLE, "整机复位开始"},
    {"Reset:复位命令开始执行",   MESEventCategory::SYSTEM_LIFECYCLE, "复位命令执行"},
    {"Reset:复位命令执行结束",   MESEventCategory::SYSTEM_LIFECYCLE, "复位命令完成"},
    
    // 2. 模块配置事件
    {"Configure module: WTR",   MESEventCategory::MODULE_CONFIG, "机械手模块配置完成"},
    {"Configure module: LLA",   MESEventCategory::MODULE_CONFIG, "负载锁A配置完成"},
    {"Configure module: LLB",   MESEventCategory::MODULE_CONFIG, "负载锁B配置完成"},
    {"Configure module: PM1",   MESEventCategory::MODULE_CONFIG, "工艺腔1配置完成"},
    {"Configure module: PM2",   MESEventCategory::MODULE_CONFIG, "工艺腔2配置完成"},
    {"Configure module: PM3",   MESEventCategory::MODULE_CONFIG, "工艺腔3配置完成"},
    {"Configure module: PM4",   MESEventCategory::MODULE_CONFIG, "工艺腔4配置完成"},
    {"Configure module: TM",    MESEventCategory::MODULE_CONFIG, "传输腔配置完成"},
    
    // 3. 设备状态变化
    {"change control mode to local online", MESEventCategory::DEVICE_STATE, "控制模式切换"},
    
    // 4. 真空度监控（关键工艺参数 - 每2秒产生）
    {"LLA_vacuum:",             MESEventCategory::VACUUM_MONITORING, "负载锁A真空度"},
    {"LLB_vacuum:",             MESEventCategory::VACUUM_MONITORING, "负载锁B真空度"},
    {"TM_vacuum:",              MESEventCategory::VACUUM_MONITORING, "传输腔真空度"},
    {"pipe_vacuum:",            MESEventCategory::VACUUM_MONITORING, "管路真空度"},
    
    // 5. 通信异常（关键错误）
    {"连接机械手服务器失败",      MESEventCategory::COMMUNICATION_ERROR, "机械手通信异常"},
    {"Can not open socket",    MESEventCategory::COMMUNICATION_ERROR, "EFEM通信异常"},
    {"Reconnection failed",     MESEventCategory::COMMUNICATION_ERROR, "重连失败"},
    {"上使能命令发生错误",       MESEventCategory::COMMUNICATION_ERROR, "机械手使能错误"},
    
    // 6. 工艺参数设置
    {"设置真空上限值",           MESEventCategory::PROCESS_DATA, "真空参数配置"},
    {"设置快充隔膜阀真空设定值",   MESEventCategory::PROCESS_DATA, "阀门参数配置"},
    {"角阀慢抽计时开始",         MESEventCategory::PROCESS_DATA, "真空抽取过程"},
    
    // 7. 机械手运动事件（新增）
    {"WTR:start command",       MESEventCategory::ROBOT_MOTION, "机械手命令开始"},
    {"WTR:设置速度命令",         MESEventCategory::ROBOT_MOTION, "机械手速度设置"},
    {"WTR:机械手HOME命令",       MESEventCategory::ROBOT_MOTION, "机械手归位"},
    {"MOV:HOME",                MESEventCategory::ROBOT_MOTION, "机械手HOME移动"},
    {"SET:RUN_SPEED",           MESEventCategory::ROBOT_MOTION, "机械手运行速度设置"},
    
    // 8. 负载端口操作（新增）
    {"MOV:INIT/LP1",            MESEventCategory::LOADPORT_OPERATION, "LP1初始化"},
    {"MOV:INIT/LP2",            MESEventCategory::LOADPORT_OPERATION, "LP2初始化"},
    {"MOV:OPEN/LP",             MESEventCategory::LOADPORT_OPERATION, "打开负载端口"},
    {"MOV:LOAD/WTR/LP",         MESEventCategory::LOADPORT_OPERATION, "从LP装载晶圆"},
    {"DOORSTAT/LP/CLOSED",      MESEventCategory::LOADPORT_OPERATION, "LP门关闭状态"},
    {"LL打开晶圆盒门",           MESEventCategory::LOADPORT_OPERATION, "打开晶圆盒门"},
    {"取LP晶圆",                MESEventCategory::LOADPORT_OPERATION, "取晶圆操作"},
    
    // 9. 对准器操作（新增）
    {"EALIGNER:Timeout",        MESEventCategory::ALIGNER_OPERATION, "对准器超时"},
    {"GET:MAPDT/ALIGNER",       MESEventCategory::ALIGNER_OPERATION, "获取对准器晶圆状态"},
    {"MOV:ALIGN",               MESEventCategory::ALIGNER_OPERATION, "对准器对准操作"},
    {"MOV:LOAD/WTR/ALIGNER",    MESEventCategory::ALIGNER_OPERATION, "装载晶圆到对准器"},
    {"MOV:UNLOAD/WTR/ALIGNER",  MESEventCategory::ALIGNER_OPERATION, "从对准器卸载晶圆"},
    {"放晶圆到寻边器",           MESEventCategory::ALIGNER_OPERATION, "放置晶圆到对准器"},
    
    // 10. 晶圆追踪（新增）
    {"wafer[] trans from",      MESEventCategory::WAFER_TRACKING, "晶圆传输追踪"},
    {"打开晶圆盒",               MESEventCategory::WAFER_TRACKING, "晶圆盒操作"},
    
    // 11. 配方执行（新增）
    {"Cycle:current FormulaName", MESEventCategory::RECIPE_EXECUTION, "配方执行"},
    {"Formula_Go_Up_And_Down",  MESEventCategory::RECIPE_EXECUTION, "配方：上下动作"},
    {"TaskManager:TaskID",      MESEventCategory::RECIPE_EXECUTION, "任务管理"},
    {"TransferSetup:Row",        MESEventCategory::RECIPE_EXECUTION, "传输设置"},
    
    // 12. 超时事件（新增）
    {"Timeout command:",         MESEventCategory::TIMEOUT_EVENT, "命令超时"},
    {"角阀慢抽计时开始，超时时间", MESEventCategory::TIMEOUT_EVENT, "工艺超时计时"},
    
    // 13. 其他信号灯控制
    {"SET:TOWER/YELLOW/ON",     MESEventCategory::DEVICE_STATE, "黄色信号灯亮"}
};
```

### 2. VTM设备特有的上报需求

#### 2.1 真空系统监控数据

```cpp
// 真空监控数据结构（每2秒上报一次）
struct VacuumMonitoringData {
    QString equipmentId;           // 设备ID
    QDateTime timestamp;           // 时间戳
    
    // 各腔室真空度（Pa）
    double llaVacuum;             // LoadLock A真空度
    double llbVacuum;             // LoadLock B真空度
    double tmVacuum;              // 传输腔真空度
    double pipeVacuum;            // 管路真空度
    
    // 阈值配置
    double upperLimit;            // 真空上限（10 Pa）
    double extractionSetValue;    // 抽真空设定值（10 Pa）
    double fastDiaphragmValve;    // 快充隔膜阀设定值（31000 Pa）
};

// MES上报格式（JSON）
{
    "eventType": "VacuumMonitoring",
    "equipmentId": "VTM-001",
    "timestamp": "2025-10-17T17:03:34.731",
    "data": {
        "chambers": [
            {"name": "LLA", "vacuum": 100000.0, "unit": "Pa", "status": "ATMOSPHERIC"},
            {"name": "LLB", "vacuum": 100000.0, "unit": "Pa", "status": "ATMOSPHERIC"},
            {"name": "TM",  "vacuum": 100000.0, "unit": "Pa", "status": "ATMOSPHERIC"},
            {"name": "PIPE","vacuum": 100000.0, "unit": "Pa", "status": "ATMOSPHERIC"}
        ],
        "thresholds": {
            "upperLimit": 10.0,
            "extractionSetValue": 10.0,
            "fastDiaphragmValve": 31000.0
        }
    }
}
```

#### 2.2 设备通信状态监控

```cpp
// 通信状态事件（关键异常必须实时上报）
struct CommunicationAlarm {
    QString deviceName;           // "WTR"/"EFEMClient"
    QString errorMessage;         // 详细错误信息
    QString location;             // 代码位置
    int reconnectAttempts;        // 重连次数
    bool isCritical;              // 是否严重
};

// 实际日志示例：
// [ERROR] WTR:连接机械手服务器失败！
// Location : D:\HLPrj\HL\device\src\SunwayRobot\sunway_subsystem_helper.cpp
// Detail   : 无法连接机械手服务器: 13000

// MES上报格式
{
    "eventType": "CommunicationAlarm",
    "severity": "ERROR",
    "equipmentId": "VTM-001",
    "timestamp": "2025-10-17T17:03:35.069",
    "alarm": {
        "device": "WTR",
        "message": "连接机械手服务器失败",
        "detail": "无法连接机械手服务器: 13000",
        "location": "sunway_subsystem_helper.cpp",
        "reconnectAttempts": 0,
        "impact": "晶圆传输功能不可用"
    }
}
```

### 3. 日志解析与过滤策略

#### 3.1 日志格式解析

```cpp
// VTM日志格式：[级别](时间戳) 模块:消息内容
// 示例：[INFO](2025-10-17 17:03:32.414) core module start.

class VTMLogParser {
public:
    struct ParsedLog {
        QString level;            // "INFO"/"WARN"/"ERROR"
        QDateTime timestamp;      // 2025-10-17 17:03:32.414
        QString module;           // "core"/"WTR"/"LLA"等
        QString message;          // 消息内容
        
        // 扩展字段（从message中提取）
        QMap<QString, QString> parameters;
    };
    
    static ParsedLog parse(const QString& logLine) {
        // 正则表达式：\[(\w+)\]\(([^)]+)\)\s+(\w+):(.+)
        QRegularExpression regex(R"(\[(\w+)\]\(([^)]+)\)\s+(\w+):(.+))");
        auto match = regex.match(logLine);
        
        ParsedLog result;
        if (match.hasMatch()) {
            result.level = match.captured(1);
            result.timestamp = QDateTime::fromString(match.captured(2), 
                                                    "yyyy-MM-dd HH:mm:ss.zzz");
            result.module = match.captured(3);
            result.message = match.captured(4).trimmed();
            
            // 提取真空度数值
            if (result.message.contains("_vacuum:")) {
                QRegularExpression vacuumRegex(R"((\w+)_vacuum:(\d+\.\d+))");
                auto vacMatch = vacuumRegex.match(result.message);
                if (vacMatch.hasMatch()) {
                    result.parameters["chamber"] = vacMatch.captured(1);
                    result.parameters["value"] = vacMatch.captured(2);
                }
            }
        }
        return result;
    }
};
```

### 4. 客户需求收集清单（VTM项目定制版）

```
□ MES系统是否支持SECS/GEM协议？（半导体行业标准）
  □ GEM 300（通用设备通信）
  □ SEMI E30（通用设备模型）
  □ SEMI E5（消息传输）
  □ 如不支持，采用何种替代协议：________________

□ 实时数据上传频率要求：
  □ 真空度数据：每_____秒上报一次（建议2-5秒）
  □ 设备状态：每_____秒上报一次（建议10-30秒）
  □ 报警事件：实时上报（<1秒）
  □ 工艺数据：每完成一片/批次上报

□ 网络环境：
  □ MES服务器IP地址：________________
  □ 端口号：________________
  □ 是否跨网段：□是 □否
  □ 防火墙策略：________________
```

## 五、开发实施建议

### 1. 分阶段实施计划（结合VTM项目）

```cpp
// 第一阶段：日志采集与本地验证（1周）
class Phase1_LogCollection {
    void develop() {
        // 1. 实现VTMLogParser日志解析器
        // 2. 实现MESUploadFilter过滤策略
        // 3. 本地文件输出验证（JSON格式）
        // 4. 单元测试：解析100种日志格式
        
        // 交付物：
        // - 日志解析模块（100%覆盖现有日志格式）
        // - 过滤规则配置文件
        // - 本地JSON输出文件供客户审核
    }
};

// 第二阶段：通信模块与测试环境对接（2周）
class Phase2_Communication {
    void develop() {
        // 1. 根据客户协议实现Communication接口
        // 2. 实现消息队列和批量发送
        // 3. 实现断线重连机制
        // 4. 与MES测试环境联调
        
        // 交付物：
        // - 通信模块（支持HTTP/SOAP/TCP）
        // - 重试机制（3次重试+本地缓存）
        // - 联调测试报告
    }
};

// 第三阶段：真空监控与工艺数据上报（1周）
class Phase3_ProcessData {
    void develop() {
        // 1. 实现真空度数据定时上报（每2秒）
        // 2. 实现变化检测（避免重复上报）
        // 3. 实现工艺参数提取
        // 4. 集成到FortrendDeviceKernel
        
        // 交付物：
        // - 真空监控模块
        // - 工艺数据采集模块
        // - 性能测试报告（CPU/内存/网络占用）
    }
};

// 第四阶段：生产环境部署与优化（1周）
class Phase4_Production {
    void develop() {
        // 1. 生产环境配置迁移
        // 2. 7×24小时稳定性测试
        // 3. 异常场景压力测试
        // 4. 性能优化（降低CPU占用<5%）
        
        // 交付物：
        // - 生产环境部署手册
        // - 运维监控Dashboard
        // - 问题处理手册
    }
};
```

### 2. 技术选型建议（VTM项目）

```cpp
```cpp
// 第一阶段：基础框架
class Phase1_MESModule {
    void develop() {
        // 1. 配置管理模块
        // 2. HTTP通信基础
        // 3. 简单的数据转换
        // 4. 本地测试模拟
    }
};

// 第二阶段：完整功能  
class Phase2_MESModule {
    void develop() {
        // 1. 支持多种协议
        // 2. 完善的数据映射
        // 3. 队列和重试机制
        // 4. 与测试环境联调
    }
};

// 第三阶段：优化完善
class Phase3_MESModule {
    void develop() {
        // 1. 性能优化
        // 2. 异常处理完善
        // 3. 生产环境部署
        // 4. 监控统计功能
    }
};
```

### 2. 技术选型建议
- **通信协议**：优先支持HTTP REST，扩展性最好
- **数据格式**：JSON，易于调试和处理
- **异步处理**：使用Qt的信号槽机制
- **配置存储**：INI文件或数据库
- **日志记录**：记录所有MES交互过程

通过以上需求收集，您可以获得开发MES对接功能所需的所有关键信息，确保开发工作有的放矢，避免后期返工。