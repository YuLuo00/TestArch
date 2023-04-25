#include <sqlite3.h>
#include <string>

#include <iostream>
#include <format>

#include "DB.h"

#include <boost/format.hpp>



using namespace std;
#pragma comment(lib, "sqlite3.lib")

int callback(void *data, int argc, char **argv, char **azColName)
{
    return 0;
}

namespace DB_NS {

    int mainsfeg()
    {
        //std::string name = "Alice";
        //int age = 30;

        //std::string result = std::format("My name is {} and I'm {} years old.", name, age);

        //std::cout << result << std::endl; // 输出："My name is Alice and I'm 30 years old."

        return 0;
    }

    // 建表
    bool CreTable(sqlite3 *db, const char *tableName)
    {
        // 查表回调
        auto callback = [](void *data, int argc, char **argv, char **azColName)
        {
            return 0;
        };

        boost::format tmp("SELECT name FROM sqlite_master WHERE type='table' AND name='%1%'");

        // 查表
        string sqlStr = (tmp % tableName).str();
       
        int rc = sqlite3_exec(db, sqlStr.c_str(), callback, NULL, NULL);
        if (rc == SQLITE_OK) {
            // 表格存在
        }
        // 建表
        else {
            boost::format tmp1("CREATE TABLE %1% (id INTEGER PRIMARY KEY, name id);");
            string sqlCre = (tmp1 % tableName).str();
            rc = sqlite3_exec(db, sqlCre.c_str(), callback, NULL, NULL);
            if (rc != SQLITE_OK) {
                // 处理错误情况
                return false;
            }
        }
        return true;
    }


    // 建库
    void DB_C(const char *_dbPath, sqlite3 *db)
    {

    }

    // 增列
    bool AddColumn(sqlite3 *_db, const char * _tableName, const char *_columnName)
    {
        boost::format sqlTmp("ALTER TABLE %1% ADD COLUMN %2% TEXT;");
        string sql = (sqlTmp % _tableName % _columnName).str();
        auto callback = [](void *data, int argc, char **argv, char **azColName)
        {
            return 0;
        };

        int rc = sqlite3_exec(_db, sql.c_str(), callback, NULL, NULL);
        if (rc != SQLITE_OK) {
            /* 错误处理 */
            return false;
        }

        return true;
    }

    vector<string> countValuesInColumn(sqlite3 *db, const char* _tableName, const char *_columnName)
    {
        vector<string> result;

        sqlite3_stmt *stmt;
        boost::format sqlTmp("SELECT %1%, COUNT(*) FROM %2% GROUP BY %1% ORDER BY COUNT(*) DESC;");
        string sql = (sqlTmp % _columnName % _tableName % _columnName).str();

        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
            return result;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *value = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            int count = sqlite3_column_int(stmt, 1);
            std::cout << value << ": " << count << std::endl;

            result.push_back(value);
        }

        sqlite3_finalize(stmt);

        return result;
    }
}

void SDB::Init()
{
    DB_NS::CreTable(m_db, m_TABLE_NAME.c_str());
    for (auto column : m_COLUMN_LIST) {
        DB_NS::AddColumn(m_db, m_TABLE_NAME.c_str(), column.c_str());
    }
    vector<string> vals ={"test1", "test1", "test1", "test1", "test1", "test6"};
    DB_NS::addRowToTable(m_db, m_TABLE_NAME.c_str(), m_COLUMN_LIST, vals);
}