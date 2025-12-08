# PM Recipe Widget Tab表格功能实现方案

## 功能需求

1. 在`pm_recipe_widget.ui`的`tabWidget`中，为四个tab页（PM1、PM2、PM3、PM4）添加`QTableWidget`
2. 每个`QTableWidget`的内容格式与主表格`pm_cavity_param_edit_tbw`一致
3. 实现"增加一项"、"删除选中项"、"清空数据"、"设置"按钮功能
4. 参考`slot_transfer_cycle_vtm_widget`中的实现

## 实现步骤

1. 重新生成UI头文件
2. 修改`pm_recipe_widget.h`头文件，添加槽函数声明
3. 修改`pm_recipe_widget.cpp`源文件，实现表格初始化和按钮事件处理
4. 编译和测试

## 技术要点

1. 表格初始化：使用统一函数初始化所有表格
2. 当前表格获取：根据选中tab页获取对应的表格
3. 信号槽连接：连接按钮信号到槽函数
4. 数据处理：实现表格数据的添加、删除、清空和保存

## 预期效果

1. 每个tab页显示一个与主表格格式一致的QTableWidget
2. 按钮功能正常工作，能够操作当前选中tab页的表格
3. 表格数据可以正确保存和加载
4. UI布局合理，表格自适应大小
5. 代码结构清晰，易于维护和扩展