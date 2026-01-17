#include "Data/fortrend_data_subsystem_helper.h"

#include "kernel/kernel_log.h"
#include "Kernel/sqlite/sqlite3.h"
#include "Kernel/kernel_exception.h"

#include "Poco/Format.h"
#include <ctime>
#include <chrono>
#include <vector>
#include <string>


//sqlite callback
static int sql_callback(void *data, int col_count, char **colValue, char **colName){
	int i;
	std::vector<std::vector<std::string>>& srore = *(std::vector<std::vector<std::string>>*)data;
	std::vector<std::string> item;
	for (i = 0; i<col_count; i++){
		item.push_back(colValue[i]);//    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	if (item.size() > 0){
		srore.push_back(item);
	}
	return 0;
}

static int insert_callback(void *data, int col_count, char **colValue, char **colName){
    // For INSERT operations, we typically don't need to process the result rows
    return 0;
}
namespace FC{

	class DataSubSystemHelperPrivate{
	public:
		DataSubSystemHelperPrivate(DataSubSystemHelper* p);
		~DataSubSystemHelperPrivate();
		void createTable(const std::string& create_sql);
		void exceSql(const std::string& sql) throw(KernelException);
		std::vector<std::vector<std::string>> querySql(const std::string& sql) throw(KernelException);

	public:
		DataSubSystemHelper* p;
		//database
		bool db_en = true;
		std::string db_file;
		sqlite3 *db = 0;
	};
	DataSubSystemHelperPrivate::DataSubSystemHelperPrivate(DataSubSystemHelper* p) :
		p(p){

	}
	DataSubSystemHelperPrivate::~DataSubSystemHelperPrivate(){
		if (db){
			sqlite3_close(db);//释放
		}
	}


	void DataSubSystemHelperPrivate::createTable(const std::string& create_sql){
		if (!db_en || !db) return;  //db disabled or  not opened

		char* zErrMsg;
		int rc = sqlite3_exec(db, create_sql.c_str(),
			0, 0, &zErrMsg);
		if (rc){
			sqlite3_free(zErrMsg);
		}
	}

	void DataSubSystemHelperPrivate::exceSql(const std::string& sql) throw(KernelException){
		if (!db_en || !db) return;  //db disabled or  not opened


		char* zErrMsg;
		int rc = sqlite3_exec(db, sql.c_str(),
			0, 0, &zErrMsg);
		//logInform("Data", "sql %d %s", rc,sql.c_str());
		if (rc){
			//fprintf(stderr, "SQL error: %s\n", zErrMsg);
			std::string message = zErrMsg;
			sqlite3_free(zErrMsg);
			throw KernelSysException(__FILE__, KernelSysException::KR_SYSTEM_DATABASE_ERROR, Poco::format("can't execute sql : %s\n", message));
		}
	}

	std::vector<std::vector<std::string>> DataSubSystemHelperPrivate::querySql(const std::string& sql){
		std::vector<std::vector<std::string>> ret;
		if (!db_en || !db) return ret;  //db disabled or  not opened
		char* zErrMsg;
		int rc = sqlite3_exec(db, sql.c_str(), sql_callback, &ret, &zErrMsg);
		if (rc != SQLITE_OK){
			//fprintf(stderr, "SQL error: %s\n", zErrMsg);
			std::string message = zErrMsg;
			sqlite3_free(zErrMsg);
			printf("exe sql: %s", zErrMsg);
			throw KernelSysException(__FILE__, KernelSysException::KR_SYSTEM_DATABASE_ERROR, Poco::format("can't execute sql : %s\n", message));
		}

		return ret;
	}

	DataSubSystemHelper::DataSubSystemHelper():
		d(new DataSubSystemHelperPrivate(this))
	{
	    // 数据库表结构将在opendb后初始化
	}

	int DataSubSystemHelper::opendb(const std::string& db_file){
		int rc = 0;
		rc = sqlite3_open(db_file.c_str(), &d->db);
		if (rc){
			sqlite3_close(d->db);
			throw KernelSysException(__FILE__, KernelSysException::KR_SYSTEM_DATABASE_ERROR, Poco::format("Can't open database: %s\n", sqlite3_errmsg(d->db)));
		}
		// 数据库打开后初始化表结构
		initializeDatabase();
		return rc;
	}


	DataSubSystemHelper::~DataSubSystemHelper()
	{
		if (d->db){
			sqlite3_close(d->db);//释放
		}
	}


	void DataSubSystemHelper::createTable(const std::string& create_sql){
		d->createTable(create_sql);
	}
	
	void DataSubSystemHelper::initializeDatabase() {
	    // 创建数据表用于存储实时数据
	    std::string create_table_sql = 
	        "CREATE TABLE IF NOT EXISTS realtime_data (" 
	        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
	        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
	        "pm_name TEXT NOT NULL, "
	        "acc_z REAL, "
	        "acc_r REAL, "
	        "vel_z REAL, "
	        "vel_r REAL, "
	        "pos_z REAL, "
	        "pos_r REAL, "
	        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
	        ");";
	    
	    d->createTable(create_table_sql);
	    
	    // 创建索引以提高查询性能
	    std::string create_index_sql = "CREATE INDEX IF NOT EXISTS idx_timestamp ON realtime_data(timestamp);";
	    d->exceSql(create_index_sql);
	    
	    create_index_sql = "CREATE INDEX IF NOT EXISTS idx_pm_name ON realtime_data(pm_name);";
	    d->exceSql(create_index_sql);
	}
	
	void DataSubSystemHelper::insertRealtimeData(const std::string& pm_name, 
	                                           double acc_z, double acc_r,
	                                           double vel_z, double vel_r,
	                                           double pos_z, double pos_r) {
	    std::string insert_sql = Poco::format(
	        "INSERT INTO realtime_data (pm_name, acc_z, acc_r, vel_z, vel_r, pos_z, pos_r) "
	        "VALUES('%s', %.6f, %.6f, %.6f, %.6f, %.6f, %.6f);",
	        pm_name, acc_z, acc_r, vel_z, vel_r, pos_z, pos_r);
	    
	    d->exceSql(insert_sql);
	}
	
	std::vector<std::vector<std::string>> DataSubSystemHelper::queryHistoricalData(
	    const std::string& pm_name, 
	    const std::string& start_time, 
	    const std::string& end_time) {
	    std::string query_sql = Poco::format(
	        "SELECT timestamp, acc_z, acc_r, vel_z, vel_r, pos_z, pos_r "
	        "FROM realtime_data "
	        "WHERE pm_name = '%s' AND timestamp BETWEEN '%s' AND '%s' "
	        "ORDER BY timestamp ASC;",
	        pm_name, start_time, end_time);
	    
	    return d->querySql(query_sql);
	}
	
	std::vector<std::vector<std::string>> DataSubSystemHelper::queryAllPMNames() {
	    std::string query_sql = "SELECT DISTINCT pm_name FROM realtime_data ORDER BY pm_name ASC;";
	    
	    return d->querySql(query_sql);
	}

	void DataSubSystemHelper::exce(const std::string& sql) throw(KernelException){
		d->exceSql(sql);
	}
	std::vector<std::vector<std::string>> DataSubSystemHelper::query(const std::string& sql) throw(KernelException){
		return d->querySql(sql);
	}
	
}