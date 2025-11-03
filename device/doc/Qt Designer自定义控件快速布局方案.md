# Qt Designer自定义控件快速布局方案

## 🎯 问题描述

**当前痛点**：在Qt Designer中调整自定义控件（如PM、TM、LoadLock、Robot等）的位置和大小时，只能通过鼠标拖拽和手动输入坐标，需要反复试验才能达到与实际设备布局一致的效果，**极其浪费时间**。

**期望效果**：将带图片的自定义控件拖到画布中，显示效果就和设计画面一致，无需反复调整。

## 💡 解决方案总览

| 方案 | 难度 | 效果 | 推荐度 | 适用场景 |
|------|------|------|--------|---------|
| **方案1：设置控件默认尺寸** | ⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | 最推荐，一劳永逸 |
| **方案2：使用UI模板文件** | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | 快速复用标准布局 |
| **方案3：坐标配置文件+脚本** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | 适合大量控件 |
| **方案4：自定义Designer插件** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐ | 终极方案，开发成本高 |
| **方案5：代码动态布局** | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ | 灵活但失去可视化 |

---

## 🌟 方案1：设置控件默认尺寸（最推荐）

### 原理

在自定义控件的构造函数中设置**sizeHint()**，Qt Designer会使用这个尺寸作为控件的初始大小。

### 实现步骤

#### 步骤1：修改控件头文件

```cpp
// gdt_pmwidget.h
class MYCUSTOMLIB_API PMGDTWidget : public QWidget {
    Q_OBJECT
public:
    explicit PMGDTWidget(QWidget *parent = 0);
    
    // ✅ 添加sizeHint()重载
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    
};
```

#### 步骤2：修改控件实现文件

```cpp
// gdt_pmwidget.cpp

// ✅ 返回控件的理想尺寸（基于实际图片尺寸）
QSize PMGDTWidget::sizeHint() const {
    // PM控件的标准图片尺寸是271x241
    return QSize(271, 241);
}

// ✅ 返回控件的最小尺寸（防止缩得太小）
QSize PMGDTWidget::minimumSizeHint() const {
    // 最小不低于标准尺寸的60%
    return QSize(163, 145);
}
```

#### 步骤3：为所有自定义控件添加默认尺寸

```cpp
// tm.h / tm.cpp
QSize TM::sizeHint() const {
    return QSize(371, 341);  // TM控件标准尺寸
}

// vtmrobot.h / vtmrobot.cpp
QSize vtmrobot::sizeHint() const {
    return QSize(250, 250);  // 机械手控件标准尺寸
}

// loadlockA.h / loadlockA.cpp
QSize LoadLockA::sizeHint() const {
    return QSize(261, 181);  // LoadLockA控件标准尺寸
}

// loadlockB.h / loadlockB.cpp
QSize LoadLockB::sizeHint() const {
    return QSize(241, 191);  // LoadLockB控件标准尺寸
}
```

### 如何确定理想尺寸

#### 方法1：基于图片原始尺寸

```cpp
// 在构造函数中加载图片并获取尺寸
PMGDTWidget::PMGDTWidget(QWidget *parent) : QWidget(parent) {
    QPixmap pixmap("image/image/PM1.png");
    // 将图片尺寸作为默认尺寸
    m_defaultSize = pixmap.size();
}

QSize PMGDTWidget::sizeHint() const {
    return m_defaultSize;
}
```

#### 方法2：基于当前UI中的实际尺寸

查看当前UI文件，提取控件的geometry：

```xml
<!-- fortrend_station_status_vtm_widget.ui -->
<widget class="PMGDTWidget" name="pm1_widget" native="true">
  <property name="geometry">
   <rect>
    <x>180</x>
    <y>340</y>
    <width>201</width>  <!-- ✅ 宽度 -->
    <height>241</height> <!-- ✅ 高度 -->
   </rect>
  </property>
</widget>
```

使用这些尺寸作为sizeHint()的返回值。

### 使用效果

```
修改前（Qt Designer中）：
1. 从控件栏拖拽PMGDTWidget到画布
2. 控件显示为默认的100x30（太小了）
3. 需要手动拉伸到271x241
4. 重复N次，浪费时间 ❌

修改后（Qt Designer中）：
1. 从控件栏拖拽PMGDTWidget到画布
2. 控件自动显示为271x241（完美！）
3. 直接拖到目标位置即可
4. 节省80%时间 ✅
```

### 优势

- ✅ **一次修改，永久生效**
- ✅ **Qt Designer原生支持**
- ✅ **符合Qt最佳实践**
- ✅ **代码简单，易维护**
- ✅ **运行时也能使用（如QLayout自动布局）**

---

## 🎨 方案2：使用UI模板文件

### 原理

创建一个包含所有控件标准布局的模板UI文件，每次需要时复制并修改。

### 实现步骤

#### 步骤1：创建模板文件

```xml
<!-- vtm_layout_template.ui -->
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>VTMLayoutTemplate</class>
 <widget class="QWidget" name="VTMLayoutTemplate">
  <property name="geometry">
   <rect><x>0</x><y>0</y><width>1985</width><height>678</height></rect>
  </property>
  
  <!-- ✅ 预设好的PM1控件（标准位置和尺寸） -->
  <widget class="PMGDTWidget" name="pm1_widget" native="true">
   <property name="geometry">
    <rect><x>180</x><y>340</y><width>201</width><height>241</height></rect>
   </property>
  </widget>
  
  <!-- ✅ 预设好的PM2控件 -->
  <widget class="PMGDTWidget" name="pm2_widget" native="true">
   <property name="geometry">
    <rect><x>260</x><y>200</y><width>231</width><height>251</height></rect>
   </property>
  </widget>
  
  <!-- ✅ 预设好的TM控件 -->
  <widget class="TM" name="tm_widget" native="true">
   <property name="geometry">
    <rect><x>350</x><y>250</y><width>371</width><height>341</height></rect>
   </property>
   
   <!-- ✅ TM内的Robot也预设好 -->
   <widget class="vtmrobot" name="robot_widget" native="true">
    <property name="geometry">
     <rect><x>50</x><y>40</y><width>250</width><height>250</height></rect>
    </property>
   </widget>
  </widget>
  
  <!-- ✅ 其他控件... -->
 </widget>
</ui>
```

#### 步骤2：使用模板

```bash
# 每次新建UI时，复制模板
cp vtm_layout_template.ui new_vtm_interface.ui

# 在Qt Designer中打开new_vtm_interface.ui
# 所有控件已经在正确位置，直接微调即可
```

### 模板文件建议结构

```
d:\HLPrj\HL\device\ui_templates\
├── vtm_full_layout_template.ui      # 完整VTM布局（PM+TM+LoadLock）
├── vtm_minimal_template.ui          # 最小VTM布局（仅核心控件）
├── efem_layout_template.ui          # EFEM区域布局
├── pm_cluster_template.ui           # PM腔体簇布局
└── control_panel_template.ui        # 控制面板布局
```

### 优势

- ✅ **快速复用标准布局**
- ✅ **保证一致性**
- ✅ **可以创建多种预设**
- ✅ **适合团队协作**

### 劣势

- ⚠️ 需要维护多个模板文件
- ⚠️ 模板修改后需要重新复制

---

## 🔧 方案3：坐标配置文件 + Python脚本

### 原理

将控件的标准位置和尺寸保存在配置文件中，通过脚本自动修改UI文件的XML。

### 实现步骤

#### 步骤1：创建配置文件

```json
// vtm_widget_layout.json
{
  "canvas": {
    "width": 1985,
    "height": 678
  },
  "widgets": [
    {
      "class": "PMGDTWidget",
      "name": "pm1_widget",
      "x": 180,
      "y": 340,
      "width": 201,
      "height": 241,
      "comment": "PM1腔体 - 左下位置"
    },
    {
      "class": "PMGDTWidget",
      "name": "pm2_widget",
      "x": 260,
      "y": 200,
      "width": 231,
      "height": 251,
      "comment": "PM2腔体 - 左上位置"
    },
    {
      "class": "TM",
      "name": "tm_widget",
      "x": 350,
      "y": 250,
      "width": 371,
      "height": 341,
      "comment": "传输腔 - 中心位置",
      "children": [
        {
          "class": "vtmrobot",
          "name": "robot_widget",
          "x": 50,
          "y": 40,
          "width": 250,
          "height": 250,
          "comment": "TM内的机械手"
        }
      ]
    },
    {
      "class": "LoadLockA",
      "name": "loadlockA_widget",
      "x": 229,
      "y": 484,
      "width": 261,
      "height": 181,
      "comment": "负载锁A - 左侧"
    },
    {
      "class": "LoadLockB",
      "name": "loadlockB_widget",
      "x": 520,
      "y": 479,
      "width": 241,
      "height": 191,
      "comment": "负载锁B - 右侧"
    }
  ]
}
```

#### 步骤2：创建自动布局脚本

```python
# auto_layout_ui.py
import xml.etree.ElementTree as ET
import json
import sys

def apply_layout_from_config(ui_file, config_file):
    """根据配置文件自动调整UI中控件的位置和尺寸"""
    
    # 读取配置
    with open(config_file, 'r', encoding='utf-8') as f:
        config = json.load(f)
    
    # 解析UI文件
    tree = ET.parse(ui_file)
    root = tree.getroot()
    
    # 查找widget根节点
    widget_root = root.find('.//widget[@class="QWidget"]')
    
    # 应用每个控件的配置
    for widget_config in config['widgets']:
        class_name = widget_config['class']
        name = widget_config['name']
        
        # 在UI中查找对应控件
        xpath = f'.//widget[@class="{class_name}"][@name="{name}"]'
        widget = widget_root.find(xpath)
        
        if widget is not None:
            # 更新geometry
            geometry = widget.find('.//property[@name="geometry"]/rect')
            if geometry is not None:
                geometry.find('x').text = str(widget_config['x'])
                geometry.find('y').text = str(widget_config['y'])
                geometry.find('width').text = str(widget_config['width'])
                geometry.find('height').text = str(widget_config['height'])
                print(f"✅ 已更新 {name}: ({widget_config['x']}, {widget_config['y']}) {widget_config['width']}x{widget_config['height']}")
            
            # 处理子控件
            if 'children' in widget_config:
                for child_config in widget_config['children']:
                    child_xpath = f'.//widget[@class="{child_config["class"]}"][@name="{child_config["name"]}"]'
                    child = widget.find(child_xpath)
                    if child is not None:
                        child_geometry = child.find('.//property[@name="geometry"]/rect')
                        if child_geometry is not None:
                            child_geometry.find('x').text = str(child_config['x'])
                            child_geometry.find('y').text = str(child_config['y'])
                            child_geometry.find('width').text = str(child_config['width'])
                            child_geometry.find('height').text = str(child_config['height'])
                            print(f"  ✅ 已更新子控件 {child_config['name']}")
        else:
            print(f"⚠️  未找到控件: {name} ({class_name})")
    
    # 保存修改后的UI文件
    tree.write(ui_file, encoding='utf-8', xml_declaration=True)
    print(f"\n✅ UI文件已更新: {ui_file}")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("使用方法: python auto_layout_ui.py <ui_file> <config_file>")
        print("示例: python auto_layout_ui.py vtm_widget.ui vtm_widget_layout.json")
        sys.exit(1)
    
    ui_file = sys.argv[1]
    config_file = sys.argv[2]
    apply_layout_from_config(ui_file, config_file)
```

#### 步骤3：使用脚本

```bash
# 自动应用布局配置到UI文件
python auto_layout_ui.py fortrend_station_status_vtm_widget.ui vtm_widget_layout.json

# 输出：
# ✅ 已更新 pm1_widget: (180, 340) 201x241
# ✅ 已更新 pm2_widget: (260, 200) 231x251
# ✅ 已更新 tm_widget: (350, 250) 371x341
#   ✅ 已更新子控件 robot_widget
# ✅ 已更新 loadlockA_widget: (229, 484) 261x181
# ✅ 已更新 loadlockB_widget: (520, 479) 241x191
# 
# ✅ UI文件已更新: fortrend_station_status_vtm_widget.ui
```

### 工作流程

```
1. 在Qt Designer中随便拖拽控件到画布
2. 保存UI文件（控件位置不重要）
3. 运行脚本：python auto_layout_ui.py xxx.ui xxx_layout.json
4. 在Qt Designer中重新打开UI文件
5. 所有控件已自动调整到标准位置 ✅
```

### 优势

- ✅ **批量处理多个控件**
- ✅ **配置文件易于维护和版本控制**
- ✅ **可以为不同场景创建不同配置**
- ✅ **支持嵌套控件**
- ✅ **可以添加注释说明**

### 扩展功能

```python
# 可以添加更多功能：

# 1. 导出当前UI布局到配置文件
python export_layout.py current.ui output.json

# 2. 对齐控件（左对齐、右对齐、居中等）
python align_widgets.py current.ui --align-left pm1,pm2,pm3

# 3. 等距分布控件
python distribute_widgets.py current.ui --horizontal pm1,pm2,pm3,pm4

# 4. 批量缩放控件
python scale_widgets.py current.ui --scale 0.8
```

---

## 🔌 方案4：自定义Qt Designer插件（终极方案）

### 原理

开发一个Qt Designer插件，提供"智能布局"功能，一键将控件按照实际设备布局排列。

### 实现步骤（概要）

#### 步骤1：创建Designer插件项目

```cpp
// vtm_designer_plugin.h
#include <QDesignerCustomWidgetInterface>

class VTMLayoutPlugin : public QObject, public QDesignerCustomWidgetInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
    
public:
    VTMLayoutPlugin(QObject *parent = nullptr);
    
    // 提供"智能布局"菜单项
    QString name() const override { return "VTM智能布局"; }
    
    // 右键菜单中添加"应用标准布局"选项
    void initialize(QDesignerFormEditorInterface *core) override;
    
    // 一键布局功能
    void applyStandardLayout(QDesignerFormWindowInterface *formWindow);
};
```

#### 步骤2：实现智能布局功能

```cpp
void VTMLayoutPlugin::applyStandardLayout(QDesignerFormWindowInterface *formWindow) {
    // 获取表单中的所有控件
    QWidgetList widgets = formWindow->mainContainer()->findChildren<QWidget*>();
    
    // 标准布局配置
    QMap<QString, QRect> standardLayout = {
        {"pm1_widget", QRect(180, 340, 201, 241)},
        {"pm2_widget", QRect(260, 200, 231, 251)},
        {"tm_widget", QRect(350, 250, 371, 341)},
        {"robot_widget", QRect(50, 40, 250, 250)},
        {"loadlockA_widget", QRect(229, 484, 261, 181)},
        {"loadlockB_widget", QRect(520, 479, 241, 191)}
    };
    
    // 应用布局
    for (QWidget *widget : widgets) {
        QString name = widget->objectName();
        if (standardLayout.contains(name)) {
            QRect geom = standardLayout[name];
            widget->setGeometry(geom);
        }
    }
    
    formWindow->setDirty(true);
}
```

#### 步骤3：编译和安装插件

```bash
# 编译插件
qmake vtm_designer_plugin.pro
make

# 安装到Qt Designer插件目录
cp libvtm_designer_plugin.so $QTDIR/plugins/designer/
```

### 使用效果

```
Qt Designer中：
1. 拖拽控件到画布（随便放）
2. 右键点击画布 → "应用VTM标准布局"
3. 所有控件瞬间移动到正确位置 ✅
```

### 优势

- ✅ **最专业的解决方案**
- ✅ **集成到Qt Designer，无缝使用**
- ✅ **可以提供多种预设布局**
- ✅ **团队共享插件，统一标准**

### 劣势

- ⚠️ 开发成本高（需要Qt插件开发知识）
- ⚠️ 需要为不同Qt版本编译
- ⚠️ 维护成本较高

---

## 💻 方案5：代码动态布局（编程方式）

### 原理

不使用Qt Designer的绝对定位，而是在代码中通过计算动态设置控件位置。

### 实现示例

```cpp
// fortrend_station_status_vtm_widget.cpp

// ✅ 定义设备布局的标准坐标
struct DeviceLayout {
    QString name;
    QPoint pos;
    QSize size;
};

// VTM设备标准布局配置
static const QVector<DeviceLayout> VTM_STANDARD_LAYOUT = {
    {"pm1", QPoint(180, 340), QSize(201, 241)},
    {"pm2", QPoint(260, 200), QSize(231, 251)},
    {"pm3", QPoint(510, 200), QSize(241, 251)},
    {"pm4", QPoint(660, 340), QSize(221, 241)},
    {"tm", QPoint(350, 250), QSize(371, 341)},
    {"loadlockA", QPoint(229, 484), QSize(261, 181)},
    {"loadlockB", QPoint(520, 479), QSize(241, 191)}
};

void FortrendStationStatusVTMWidget::setupLayout() {
    // ✅ 创建控件时直接应用标准布局
    for (const DeviceLayout &layout : VTM_STANDARD_LAYOUT) {
        if (layout.name == "pm1") {
            pm1_widget->setGeometry(QRect(layout.pos, layout.size));
        }
        else if (layout.name == "pm2") {
            pm2_widget->setGeometry(QRect(layout.pos, layout.size));
        }
        // ... 其他控件
    }
    
    // ✅ 或者使用映射表
    QMap<QString, QWidget*> widgetMap = {
        {"pm1", pm1_widget},
        {"pm2", pm2_widget},
        {"pm3", pm3_widget},
        {"pm4", pm4_widget},
        {"tm", tm_widget},
        {"loadlockA", loadlockA_widget},
        {"loadlockB", loadlockB_widget}
    };
    
    for (const DeviceLayout &layout : VTM_STANDARD_LAYOUT) {
        if (widgetMap.contains(layout.name)) {
            widgetMap[layout.name]->setGeometry(QRect(layout.pos, layout.size));
        }
    }
}

// 构造函数中调用
FortrendStationStatusVTMWidget::FortrendStationStatusVTMWidget(QWidget *parent)
    : QWidget(parent)
{
    ui->setupUi(this);
    
    // ✅ 应用标准布局
    setupLayout();
}
```

### 高级用法：支持缩放

```cpp
void FortrendStationStatusVTMWidget::setupLayout(double scale = 1.0) {
    for (const DeviceLayout &layout : VTM_STANDARD_LAYOUT) {
        QPoint scaledPos(layout.pos.x() * scale, layout.pos.y() * scale);
        QSize scaledSize(layout.size.width() * scale, layout.size.height() * scale);
        
        if (widgetMap.contains(layout.name)) {
            widgetMap[layout.name]->setGeometry(QRect(scaledPos, scaledSize));
        }
    }
}
```

### 优势

- ✅ **完全程序化控制**
- ✅ **支持动态计算和缩放**
- ✅ **易于版本控制（代码而非XML）**
- ✅ **可以添加复杂逻辑**

### 劣势

- ⚠️ 失去Qt Designer可视化优势
- ⚠️ 调试布局需要重新编译
- ⚠️ 不如Designer直观

---

## 🎯 推荐方案组合

根据你的项目情况，我推荐**组合使用以下方案**：

### 短期方案（立即见效）：方案2 + 方案3

```bash
# 1. 创建UI模板文件（方案2）
# 手动在Qt Designer中调整一次完美布局，保存为模板

# 2. 提取配置文件（方案3）
python extract_layout.py vtm_template.ui vtm_layout.json

# 3. 后续新建UI时
cp vtm_template.ui new_interface.ui
# 或者
python auto_layout_ui.py new_interface.ui vtm_layout.json
```

### 中长期方案（最佳实践）：方案1

```cpp
// 为所有自定义控件添加sizeHint()
// 以下是需要修改的文件清单：

✅ d:\HLPrj\HL\MyUnit\include\gdt_pmwidget.h
✅ d:\HLPrj\HL\MyUnit\src\gdt_pmwidget.cpp

✅ d:\HLPrj\HL\MyUnit\include\tm.h
✅ d:\HLPrj\HL\MyUnit\src\tm.cpp

✅ d:\HLPrj\HL\MyUnit\include\vtmrobot.h
✅ d:\HLPrj\HL\MyUnit\src\vtmrobot.cpp

✅ d:\HLPrj\HL\MyUnit\include\loadlockA.h
✅ d:\HLPrj\HL\MyUnit\src\loadlockA.cpp

✅ d:\HLPrj\HL\MyUnit\include\loadlockB.h
✅ d:\HLPrj\HL\MyUnit\src\loadlockB.cpp

// 每个控件只需添加两个函数，一劳永逸！
```

---

## 📊 当前项目建议的控件默认尺寸

根据你当前的UI文件，我整理了各控件的推荐默认尺寸：

| 控件类 | 控件名 | 推荐宽度 | 推荐高度 | 依据 |
|--------|--------|---------|---------|------|
| **PMGDTWidget** | pm1_widget | 201 | 241 | 当前UI实际尺寸 |
| **PMGDTWidget** | pm2_widget | 231 | 251 | 当前UI实际尺寸 |
| **PMGDTWidget** | pm3_widget | 241 | 251 | 当前UI实际尺寸 |
| **PMGDTWidget** | pm4_widget | 221 | 241 | 当前UI实际尺寸 |
| **TM** | tm_widget | 371 | 341 | 当前UI实际尺寸 |
| **vtmrobot** | robot_widget | 250 | 250 | 当前UI实际尺寸 |
| **LoadLockA** | loadlockA_widget | 261 | 181 | 当前UI实际尺寸 |
| **LoadLockB** | loadlockB_widget | 241 | 191 | 当前UI实际尺寸 |

**建议**：对于同一类控件（如PMGDTWidget），可以使用平均值作为默认尺寸：

```cpp
// PMGDTWidget的推荐默认尺寸
// 平均值：(201+231+241+221)/4 ≈ 224
//        (241+251+251+241)/4 ≈ 246
QSize PMGDTWidget::sizeHint() const {
    return QSize(224, 246);  // 或者使用最常用的尺寸
}
```

---

## 🔍 WPF对比学习

在WPF中，类似的问题有以下解决方案：

### WPF方案1：默认尺寸

```csharp
// 自定义控件
public class PMWidget : Control {
    static PMWidget() {
        // 设置默认尺寸
        WidthProperty.OverrideMetadata(
            typeof(PMWidget),
            new FrameworkPropertyMetadata(224.0)
        );
        HeightProperty.OverrideMetadata(
            typeof(PMWidget),
            new FrameworkPropertyMetadata(246.0)
        );
    }
}
```

### WPF方案2：样式模板

```xaml
<!-- 资源字典中定义标准布局 -->
<Style x:Key="VTMLayoutStyle" TargetType="Canvas">
    <Setter Property="Width" Value="1985"/>
    <Setter Property="Height" Value="678"/>
</Style>

<DataTemplate x:Key="VTMDeviceTemplate">
    <Canvas>
        <local:PMWidget Canvas.Left="180" Canvas.Top="340" Width="201" Height="241"/>
        <local:TMWidget Canvas.Left="350" Canvas.Top="250" Width="371" Height="341"/>
        <!-- ... -->
    </Canvas>
</DataTemplate>
```

### WPF方案3：ViewModel驱动

```csharp
public class DeviceLayoutViewModel {
    public ObservableCollection<DevicePosition> Devices { get; set; }
}

public class DevicePosition {
    public string Type { get; set; }
    public double X { get; set; }
    public double Y { get; set; }
    public double Width { get; set; }
    public double Height { get; set; }
}

// XAML绑定
<ItemsControl ItemsSource="{Binding Devices}">
    <ItemsControl.ItemsPanel>
        <ItemsPanelTemplate>
            <Canvas/>
        </ItemsPanelTemplate>
    </ItemsControl.ItemsPanel>
    <ItemsControl.ItemContainerStyle>
        <Style>
            <Setter Property="Canvas.Left" Value="{Binding X}"/>
            <Setter Property="Canvas.Top" Value="{Binding Y}"/>
            <Setter Property="Width" Value="{Binding Width}"/>
            <Setter Property="Height" Value="{Binding Height}"/>
        </Style>
    </ItemsControl.ItemContainerStyle>
</ItemsControl>
```

**对比总结**：
- Qt：通过`sizeHint()`提供默认尺寸，或通过代码设置geometry
- WPF：通过元数据覆盖设置默认尺寸，或通过样式/模板/数据绑定

两者思路相似，WPF更倾向于XAML声明式，Qt更倾向于代码式。

---

## ✅ 行动计划

建议你按以下顺序实施：

### 第1周：快速见效（方案2）
```bash
1. 在Qt Designer中手动调整一次完美布局
2. 保存为 vtm_layout_template.ui
3. 后续复制模板使用
估计节省时间：60%
```

### 第2周：中期优化（方案1）
```cpp
1. 为PMGDTWidget添加sizeHint()
2. 为TM添加sizeHint()
3. 为LoadLockA/B添加sizeHint()
4. 为vtmrobot添加sizeHint()
5. 重新编译自定义控件库
估计节省时间：80%
```

### 第3周：自动化工具（方案3）
```python
1. 创建配置文件 vtm_layout.json
2. 编写auto_layout_ui.py脚本
3. 集成到构建流程
估计节省时间：90%
```

### 长期（可选）：终极方案（方案4）
```
如果团队规模大，项目多，可考虑开发Designer插件
投资回报周期：约3-6个月
```

---

**日期**：2025-10-29  
**文档作者**：AI助手  
**适用项目**：HLPrj VTM设备控制系统
