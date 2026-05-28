#pragma once
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "UnifiedWaferTask.h"
#include  "kernel/kernel_log.h"

namespace FC {

    // 前置声明
    class slot_transfer_cycle_vtm_widget;
    struct UnifiedWaferTask;

    class TaskManager {
    public:
        // 单例模式访问
        static TaskManager& getInstance();

        // 删除拷贝构造函数和赋值操作符
        TaskManager(const TaskManager&) = delete;
        void operator=(const TaskManager&) = delete;

        // 添加新任务：一片晶圆是一个task
        void addTask(const UnifiedWaferTask& task);

        void popTask(const UnifiedWaferTask& task);

        void addWorkTask(const UnifiedWaferTask& task);

        // 获取特定位置的任务
        std::vector<UnifiedWaferTask> getTasksByLocation(UnifiedWaferTask::Location location);

        //获取特定位置的任务
        std::vector<UnifiedWaferTask> getTasksByLocation(std::vector<UnifiedWaferTask> tasks, UnifiedWaferTask::Location location);


        // 通过任务ID获取任务状态和类型
        const std::pair<UnifiedWaferTask::TaskType, UnifiedWaferTask::Status>& getTaskStatusAndType(int taskId);

        UnifiedWaferTask::TaskType& getTaskType(int taskId);

        //获取所有任务
        std::vector<UnifiedWaferTask>& getAllTasks();

        std::vector<UnifiedWaferTask>& getAllWorksTasks();

        int getAllTasksSize();

        std::vector<int> getAllTaskIDs();
            
        //清空任务
        void clearTasks();

        // 获取待处理的任务
        std::vector<UnifiedWaferTask> getPendingTasks();

        UnifiedWaferTask getByIDFindTask(int taskID);

        //查找机械手上的任务
		UnifiedWaferTask getRobotTaskInfo(int arm);

        // 检查是否有待处理任务
        bool hasPendingTasks();

        // 通用任务查询函数
        std::vector<UnifiedWaferTask> getTasksByTypeAndStatus(
            UnifiedWaferTask::TaskType type,
            UnifiedWaferTask::Status status);

        //能否找到此条件下任务
        bool  getByTypeAndStatus(
            UnifiedWaferTask::TaskType type,
            UnifiedWaferTask::Status status);


        // 带位置过滤的通用任务查询函数
        std::vector<UnifiedWaferTask> getTasksByTypeStatusAndLocation(
            UnifiedWaferTask::TaskType type,
            UnifiedWaferTask::Status status,
            UnifiedWaferTask::Location location);

        //检测有非初始状态的任务
        bool detectionHasNoInitialTypeTasks();

        //类似写实时的三色灯逻辑，只是每个步骤是组合动作而已

        //状态转变 
        // 第一次循环  0---> 1---> 5（2） ---> 6  ---> 9---> 10 --->11 --->7 --->8（3）
        // 第二次循环  4---> 0---> 1---> 5（2） ---> 6  ---> 9---> 10 --->11--->7 --->8（3）

         /*-----------------------------------------EFEM-------------------------------------------------------------*/
        //获取待EFEM 初始状态的任务
        std::vector<UnifiedWaferTask> getEfemUnkownStatusTasks();//0

        //分给LLA的任务
        std::vector<UnifiedWaferTask> getEfemUnkownStatusLLATasks();

        //分给LLB的任务
        std::vector<UnifiedWaferTask> getEfemUnkownStatusLLBTasks();


        //获取待EFEM待上料的任务 QUEUED (根据LLK MAPPING决定，要上料的晶圆)
        std::vector<UnifiedWaferTask> getEfemPendingTasks();//1

        //获取EFEM上料完成的任务 COMPLETED (实际上到LL上的晶圆数量)和LockPendingTasks 一样
        std::vector<UnifiedWaferTask> getEfemCompletedTasks();//2

        //获取EFEM待下料的任务
        std::vector<UnifiedWaferTask> getEfemRuturnPendingTasks();//3

        //获取EFEM下料完成的任务，(成功回到LP)   下次循环重新更新为EfemPendingTasks
        std::vector<UnifiedWaferTask> getEfemRuturnCompletedTasks();//4

        /*-----------------------------------------LoadLock-------------------------------------------------------------*/
        //获取Loadlock待工艺的任务   （破完真空,casste门开,efem放到LL槽上，待抽真空，待取走的晶圆） QUEUED 
        std::vector<UnifiedWaferTask> getLoadLockPendingTasks(std::string LLName);//5

        //获取Loadlock工艺完成的任务：（抽完真空，开传输门后,csr取到片, 下一步是传到Pm腔待加工的晶圆数量  COMPLETED）
        std::vector<UnifiedWaferTask> getLoadLockCompletedTasks(std::string LLName);//6


        //获取Loadlock待下料的任务：（检查真空,开传输门后，做完PM工艺后的,待放到LL的晶圆数量  从PM腔收集的QUEUED）
        std::vector<UnifiedWaferTask> getLoadLockReturnPendingTasks(std::string LLName);//7

        //获取Loadlock下料完成的任务 (做完PM工艺后的,放回到LL的晶圆数量)
        std::vector<UnifiedWaferTask> getLoadLockReturnCompletedTasks(std::string LLName);//8


        //获取Loadlock待工艺的任务   
        std::vector<UnifiedWaferTask> getLoadLockPendingTasks();//5

        //获取Loadlock工艺完成的任务：
        std::vector<UnifiedWaferTask> getLoadLockCompletedTasks();//6


        //获取Loadlock待下料的任务
        std::vector<UnifiedWaferTask> getLoadLockReturnPendingTasks();//7

        //获取Loadlock下料完成的任务
        std::vector<UnifiedWaferTask> getLoadLockReturnCompletedTasks();//8

        /*-----------------------------------------PM-------------------------------------------------------------*/
        //获取PM待工艺的任务 （csr已取到手片子，待放到PM取放片位的片子） QUEUED
        std::vector<UnifiedWaferTask> getPMPendingTasks(std::string PM); //9

        std::vector<UnifiedWaferTask> getPMProcessTasks(std::string PM); //10

        //获取PM工艺完成的任务 （工艺结束后，回到取放片位，待机械手取走的片子） COMPLETED
        std::vector<UnifiedWaferTask> getPMCompletedTasks(std::string PM); //11

        // 等待新任务（带超时）
        bool waitForTasks(int timeoutMs = 100);

        bool waitLLBForTasks(int timeoutMs = 100);
        // 检查是否有任务
        bool hasTasks();

        // 设置停止标志
        void stop();

        void start();

        bool isStopped() const;

        //为该料打上料完成标签，
        void setLoadingInPlaceFlag(int taskID,bool flag);

        bool getLoadingInPlaceFlag(int taskID);

        //获得经过LLA/LLB 的有上料标签
        bool CollectionPassedThroughLL(const std::string &LLName);

        bool hasLoadLockLowerPriorityReturn(const std::string& LLName);

        bool hasEfemUnloadInProgress(const std::string& LLName);

		//通过来源位置获取EFEM下料完成的任务
        std::vector<UnifiedWaferTask> getEfemRuturnCompletedTasksBySource(UnifiedWaferTask::Location source);

        bool canResetEfemReturnCompletedTasks(UnifiedWaferTask::Location source);

        bool hasInFlightTasksExceptCompletedEfemReturn();

        // 更新任务映射关系 ,根据taskId先移除旧映射
        void updateTaskMaps(int taskId, UnifiedWaferTask::TaskType type, UnifiedWaferTask::Status status);

        // 更新任务状态（线程安全）
        void updateTaskStatus(int taskId, UnifiedWaferTask::TaskType newTaskType,UnifiedWaferTask::Status newStatus);

        //将字符串转换为Location枚举
        UnifiedWaferTask::Location stringToLocation(const std::string& locStr);

        //taskId排序
        void lessTaskIdSortAlgorithm();

        void largeTaskIdSortAlgorithm();
  public:
    bool stopped_{ false };

    private:
        TaskManager() = default; // 私有构造函数
        ~TaskManager() = default;

        std::vector<UnifiedWaferTask> tasks_; //任务队列
        std::vector<UnifiedWaferTask> workTasks_;//实际的工作队列

        std::map<int, std::pair<UnifiedWaferTask::TaskType, UnifiedWaferTask::Status>> taskStatusMap_; //   taskID:{TYPE:STATUS}  任务状态映射
        std::map<UnifiedWaferTask::TaskType, std::map< UnifiedWaferTask::Status, std::vector<int>>> taskTypeStatusMap_; //最内层int是任务ID

        std::mutex mutex_;
        std::condition_variable cv_;
        //std::atomic<bool> stopped_{ false };
        int nextTaskId_ = 0;

    };

}