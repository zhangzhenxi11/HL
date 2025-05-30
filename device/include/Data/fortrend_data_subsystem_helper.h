#ifndef _XLH_FORTREND_DATA_SUBSYSTEM_HELPER_INCLUDE_
#define _XLH_FORTREND_DATA_SUBSYSTEM_HELPER_INCLUDE_

#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_exception.h"
#include <cmath>


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
private:
	DECLARE_PRIVATE(DataSubSystemHelper);
};

}
#endif // WIDGET_H

