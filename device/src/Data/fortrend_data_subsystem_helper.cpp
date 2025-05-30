#include "Data/fortrend_data_subsystem_helper.h"

#include "kernel/kernel_log.h"
#include "Kernel/sqlite/sqlite3.h"
#include "Kernel/kernel_exception.h"

#include "Poco/Format.h"


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
	}

	int DataSubSystemHelper::opendb(const std::string& db_file){
		int rc = 0;
		rc = sqlite3_open(db_file.c_str(), &d->db);
		if (rc){
			sqlite3_close(d->db);
			throw KernelSysException(__FILE__, KernelSysException::KR_SYSTEM_DATABASE_ERROR, Poco::format("Can't open database: %s\n", sqlite3_errmsg(d->db)));
		}
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

	void DataSubSystemHelper::exce(const std::string& sql) throw(KernelException){
		d->exceSql(sql);
	}
	std::vector<std::vector<std::string>> DataSubSystemHelper::query(const std::string& sql) throw(KernelException){
		return d->querySql(sql);
	}
	
}