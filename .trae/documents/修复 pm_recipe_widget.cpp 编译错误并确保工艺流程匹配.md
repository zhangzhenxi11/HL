## 现象与定位
- 关键报错集中在 [pm_recipe_widget.cpp:L511](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L511)、[L618](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L618)、[L636](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L636)、[L639](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L639)。
- 与字符串相关错误：C2146（缺少“)”）、C2001（常量中有换行符）、C2660（函数不接受 2 个参数）出现在调用 `logFailedExcuteCommandHasError` 的行（如 L618）。
- 与语法/作用域相关错误：C2969（成员函数定义应以“}”结尾）、C2143（缺少“;”在“->/void”前）、C4430、C2238，出现在 `onStartCycle` 尾部（L636、L639），通常是前面一处未闭合导致的连锁解析失败。

## 根因分析（最可能）
- 中文字符串字面量中存在不可见分隔符或编码错配（如 U+2028/U+2029），导致编译器将第三个参数断行，从而报 C2001/C2146/C2660（错误列表中的“娴嬭瘯pm娴佺▼”是“测试pm流程”的乱码表现）。
- 上述异常使编译器在 `onStartCycle` 的 `});` 之后语法同步失败，引发 L636/L639 的“缺少分号/void 前缺少分号”等一串级联错误（解析崩溃的典型症状）。
- 验证：函数签名与所有调用均为 3 参数，且类内唯一声明，另一个同名函数位于不同类与文件；不存在重载冲突。[pm_recipe_widget.h:L96](file:///d:\HLPrj\HL\device\include\pm_recipe_widget.h#L96) 与 [pm_recipe_widget.cpp:L661](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L661) 一致；调用位于 [L554](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L554)、[L567](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L567)、[L581](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L581)、[L618](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L618)。

## 修复方案
- 将所有中文字面量改用 `QStringLiteral(...).toStdString()` 传入，避免源文件编码问题：
  - 例：`logFailedExcuteCommandHasError(pmSubsystem->getName(), QStringLiteral("移动到取放片面").toStdString(), QStringLiteral("测试pm流程").toStdString());`（应用于 L554/L567/L581/L618）。
- 或保持当前窄字符串方案但统一源文件编码：
  - 在 VS 中“文件→另存为→保存编码”为 UTF-8（带签名 BOM）。
  - 项目属性→C/C++→高级→源文件字符集 设为 `UTF-8`；保持文件头的 `#pragma execution_character_set("utf-8")`。
- 在 `logFailedExcuteCommandHasError` 的实现中，若编译器对 `Poco::format` 参数推导产生歧义，改为 `c_str()`：
  - 例：[L661-L664](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L661-L664) 替换为 `logFailed(station_name, Poco::format("%s %s命令执行失败， %s", station_name.c_str(), command_name.c_str(), process_name.c_str()));`
- 语法结构自检（确认无括号遗漏）：
  - 用 IDE 括号/大括号配对检查 `onStartCycle` 的 `try/catch`、`for`、各 `if` 与最末 `});`（区域 [L511-L637](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L511-L637)）。
  - 若仍报 L636/L639，则说明字符串问题未清除或存在不可见字符，按上一步统一编码并清理。
- 编辑并继续设置：
  - 针对“fortrend_pm_cavity_subsystem.cpp 未以 /ZI 编译”的提示，项目属性→C/C++→调试信息格式设为“编辑并继续（/ZI）”，或临时关闭热重载进行完整重建。

## 验证步骤
- 清理并重建 libdevice_plugin；确保不再出现 C2001/C2146/C2660；L636/L639 级联错误应随之消失。
- 运行流程，观察日志：应严格按“Z3 → 循环（Z2→旋转→Z1等待）→ Z3”输出；对应代码见 [L526-L625](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L526-L625)。
- 检查 UI 更新是否正常：`pm1_spx` 在循环内递增，[L543-L545](file:///d:\HLPrj\HL\device\src\pm_recipe_widget.cpp#L543-L545)。

## 变更摘要（将实施）
- 统一中文字符串构造方式（`QStringLiteral(...).toStdString()`）。
- 如需，调整 `Poco::format` 调用为 `c_str()` 以消除重载歧义。
- 保持/确认 UTF-8 编码设置；清理不可见控制字符。

请确认以上方案；确认后我将进行代码改造与配置调整，并重建验证。