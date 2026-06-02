# Checklist

## 数据结构
- [x] PMMotorRow结构已扩展两个Wait时间参数（每次工艺可能不同）
- [x] 参数：pre_process_wait_s, post_process_wait_s

## 界面修改
- [x] 电机运动参数表格(innerTable)已扩展到10行
- [x] 行8：前处理Wait时间/s (soak)
- [x] 行9：后处理Wait时间/s (time7)
- [x] 每列（每次工艺）可以设置不同的Wait时间

## 功能实现
- [x] 数据保存正确包含两个Wait时间参数（每列）
- [x] 数据加载正确恢复两个Wait时间参数（每列）

## 运动流程
- [x] 第i次工艺执行前处理Wait(第i列的pre_process_wait_s)
- [x] 第i次工艺执行工艺时间(第i次Dep)
- [x] 第i次工艺执行后处理Wait(第i列的post_process_wait_s)