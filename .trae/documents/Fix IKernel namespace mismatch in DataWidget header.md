编译错误是由头文件中 `IKernel` 前置声明的 **命名空间不匹配** 引起的。

### 错误分析
1.  **在头文件 (`fortrend_data_subsystem_widget.h`) 中**:
    ```cpp
    class IKernel; // <--- 声明在 全局命名空间 (::IKernel)

    namespace FC {
        class DataWidget : public QWidget {
            // 构造函数实际上使用了 std::shared_ptr<::IKernel>
            explicit DataWidget(const std::shared_ptr<IKernel>& kernel, ...); 
        }
    }
    ```

2.  **在源文件 (`fortrend_data_subsystem_widget.cpp`) 中**:
    ```cpp
    #include "Kernel/kernel.h" // 这个文件定义了 FC::IKernel
    namespace FC {
        // 这里的构造函数实现使用的是 FC::IKernel (因为在 namespace FC 内部)
        DataWidget::DataWidget(const std::shared_ptr<IKernel>& kernel, ...) { ... } 
    }
    ```

3.  **结果**:
    编译器认为 `::IKernel` 和 `FC::IKernel` 是两个完全不同的类型。因此，`.cpp` 文件中定义的构造函数（接受 `FC::IKernel`）与 `.h` 文件中声明的构造函数（接受 `::IKernel`）**签名不匹配**，导致报错：“未在 'FC::DataWidget' 中找到重载的成员函数”。

### 解决方案
我需要将 `fortrend_data_subsystem_widget.h` 中的前置声明 `class IKernel;` 移动到 `namespace FC` 代码块的**内部**。

我将立即执行此修复。