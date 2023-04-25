#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <string>
#include <iostream>

#include <boost/format.hpp>

#define TEST_DEFINE

using namespace std;

struct FileDBInfo
{
    string fileName;
    string md5;
    string size;
    string password;
    string picCount;
    string vedCount;
};

/// <summary>
/// 数据库操作，不做入参检查
/// </summary>
namespace DB_NS {
    int mainsfeg();

    // 建表
    bool CreTable(sqlite3 *db, const char *tableName);

    // 建库
    void DB_C(const char *_dbPath, sqlite3 *db);

    // 增列
    bool AddColumn(sqlite3 *db, const char * _tableName, const char *columnName);

    // 增行
    bool AddRow(sqlite3 *_db, const char *_tableName, 
                const vector<string> _columns, const vector<string> _values);

    inline void addRowToTable(sqlite3 *_db, const char *_tableName,
                const vector<string> _columns, const vector<string> _values)
    {
        string sqlCols;
        for (int i = 0; i < _columns.size();i++) {
            if (i == 0) {
                sqlCols += " (";
            }
            sqlCols = sqlCols + _columns[i];
            if (i != _columns.size() - 1) {
                sqlCols += ", ";
            }
            if (i == _columns.size() - 1) {
                sqlCols += ") ";
            }
        }

        string sqlVals;
        for (int i = 0; i < _values.size(); i++) {
            if (i == 0) {
                sqlVals += " (";
            }
            sqlVals = sqlVals + "'" + _values[i] + "'";
            if (i != _values.size() - 1) {
                sqlVals += ", ";
            }
            if (i == _values.size() - 1) {
                sqlVals += ") ";
            }
        }

        boost::format fmt("INSERT INTO %1% %2% VALUES %3%;");
        fmt %_tableName %sqlCols %sqlVals;
        string sql = fmt.str();

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            auto err =  sqlite3_errmsg(_db);
            return;
        }
        sqlite3_bind_text(stmt, 1, "value1", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, "value2", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, "value3", -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Error inserting row: " << sqlite3_errmsg(_db) << std::endl;
            return;
        }
        std::cout << "Row added successfully!" << std::endl;
        sqlite3_finalize(stmt);
    }

}

class SDB
{
public:
    sqlite3 *m_db;
    SDB(const char *_dbPath)
    {
        int rc = sqlite3_open(_dbPath, &m_db);
        if (rc != SQLITE_OK) {
            // 处理错误情况
        }
        Init();
    }
    void Init();
    ~SDB()
    {
        sqlite3_close(m_db);
    }
private:
    const string m_TABLE_NAME = "password";
    const vector<string> m_COLUMN_LIST ={
        "FILE_NAME",
        "MD5",
        "SIZE",
        "PASSWORD",
        "PIC_COUNT",
        "VED_COUNT"
    };
};

