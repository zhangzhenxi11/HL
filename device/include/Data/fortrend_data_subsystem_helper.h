#ifndef _XLH_FORTREND_DATA_SUBSYSTEM_HELPER_INCLUDE_
#define _XLH_FORTREND_DATA_SUBSYSTEM_HELPER_INCLUDE_

#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_exception.h"
#include <cmath>
#include <vector>
#include <string>

#include "Poco/StringTokenizer.h"

namespace FC {
class DataSubSystemHelper;


class DataSubSystemHelperPrivate;
class DataSubSystemHelper 
{
public:
	DECLARE_PTR(DataSubSystemHelper)
public:
    explicit DataSubSystemHelper();
    ~DataSubSystemHelper();
	void createTable(const std::string& create_sql);
	void exce(const std::string& sql) throw(KernelException);
	std::vector<std::vector<std::string>> query(const std::string& sql) throw(KernelException);
	int opendb(const std::string& db_file);
    
    // 新增方法用于历史数据管理
    void initializeDatabase();
    void insertRealtimeData(const std::string& pm_name, 
                           double acc_z, double acc_r,
                           double vel_z, double vel_r,
                           double pos_z, double pos_r);
    std::vector<std::vector<std::string>> queryHistoricalData(
        const std::string& pm_name, 
        const std::string& start_time, 
        const std::string& end_time);
    std::vector<std::vector<std::string>> queryAllPMNames();
    
private:
	DECLARE_PRIVATE(DataSubSystemHelper);
};

}
#endif // WIDGET_H

