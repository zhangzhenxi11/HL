## 问题概览
- 多个语法错误集中在 280–560 行，表现为“非法 continue”“缺少分号/大括号”“this 只能在成员函数中使用”，以及大量标识符未声明。
- 这些一般源于：
  - 在非循环语句块中使用 continue（应使用 break），导致编译器进入异常解析模式。
  - Qt 的 foreach 宏未用大括号包裹或头文件缺失，宏展开后破坏作用域，使后续成员函数体被误判为全局代码。
  - 缺少必要的头文件（如 QMap），以及 UI 头文件的包含路径不一致。
  - 一旦前面出现作用域破坏，所有后续使用 Q_D(this)、d->ui、return 等都会报错（你看到的密集错误就是该连锁反应）。

## 代码现状核查
- 我已通读当前文件 [pm_recipe_widget.cpp](file:///d:/HLPrj/HL/device/src/pm_recipe_widget.cpp)；类与命名空间结构完整，case 2 位置在 [pm_recipe_widget.cpp:L145](file:///d:/HLPrj/HL/device/src/pm_recipe_widget.cpp#L141-L148)，函数/大括号配对正常。
- 头文件在 [pm_recipe_widget.h](file:///d:/HLPrj/HL/device/include/pm_recipe_widget.h) 定义 Q_OBJECT 与槽函数声明，UI 头生成在 [ui_pm_recipe_widget.h](file:///d:/HLPrj/HL/device/src/ui_pm_recipe_widget.h)。
- 现有实现中使用了 QMap 与 foreach 宏，并在多处使用 continue。

## 具体修复项
1. 修复 foreach 宏的使用
   - 将
     - foreach(const QString & str, AngleMap.keys()) dcb->addItem(str, AngleMap.value(str));
   - 改为
     - foreach (const QString &str, AngleMap.keys()) { dcb->addItem(str, AngleMap.value(str)); }
   - 或直接改用范围 for：for (const auto &str : AngleMap.keys()) { … }
2. 补齐缺失头文件
   - 在 pm_recipe_widget.cpp 顶部增加：#include <QMap>
3. 校正 UI 头文件包含路径
   - 目前包含的是 "device/ui_pm_recipe_widget.h"，实际文件在 device/src；若工程不含 device 作为包含根目录，改为 "ui_pm_recipe_widget.h" 或将工程包含目录指向 device/src。
4. 审查并修正 continue 的位置
   - 仅允许在 for/while 中使用；若出现在 switch 或非循环环境，改为 break。
   - 现文件 onLoadParameters 中的 continue 在 for 内是合法的，但我将逐行检查其它函数，确保没有误用。
5. 统一使用显式的大括号
   - 对所有单语句的 for/if/foreach 补全 {}，避免宏或格式化引起的解析歧义。
6. 保证所有成员实现位于命名空间 FC 与类成员函数体内
   - 若编译器仍然报“this 只能在成员函数中使用”，通常表示先前的括号/宏展开破坏了作用域，上述 1–5 项修复后应消除。

## 验证步骤
- 重新编译 libdevice_plugin，观察首个错误位置是否清零。
- 若仍有错误：
  - 先定位第一个报错行，优先修正它（后续错误通常是连锁反应）。
  - 如与 UI 头相关，确认生成器版本与包含路径一致；必要时清理并重新生成 ui_pm_recipe_widget.h。
- 运行界面：
  - 确认四个 Tab 上表格创建正常，增删/清空/加载与保存 JSON 功能工作正常。
  - 点击“Start Cycle/Stop Cycle”，验证线程状态与按钮使能切换。

## 预期改动文件
- d:\HLPrj\HL\device\src\pm_recipe_widget.cpp（修正 foreach、大括号、继续语句与包含）
- 构建设置或包含路径（仅当需要修正 "device/ui_pm_recipe_widget.h" 的查找路径时）

请确认以上方案，我将按步骤实施修复并完成编译验证。