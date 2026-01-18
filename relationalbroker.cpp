#include "relationalbroker.h"
#include <pqxx/pqxx>
unique_ptr<pqxx::connection> RelationalBroker::m_conn = nullptr;

RelationalBroker::RelationalBroker() {}

void RelationalBroker::initConnection() {
    if (m_conn && m_conn->is_open()) {
        print("数据库连接已初始化\n");
        return;
    }

    try {
        // 数据库连接配置（老师只需修改这里）
        const string conn_str = "dbname=EnrollSys "
                                "user=enrolluser "
                                "password=123456 "
                                "hostaddr=127.0.0.1 "
                                "port=5432";

        m_conn = std::make_unique<pqxx::connection>(conn_str);

        if (m_conn->is_open()) {
            print("连接数据库成功: {}", m_conn->dbname());
        } else {
            throw runtime_error("数据库连接未打开");
        }
    } catch (const std::exception& e) {
        print("数据库连接失败: {}\n", e.what());
        m_conn.reset();
    }
}

pqxx::result RelationalBroker::query(const string& sql) {
        if (!m_conn || !m_conn->is_open()) initConnection();
        try {
            pqxx::work tx(*m_conn);
            pqxx::result res = tx.exec(sql);
            tx.commit();
            return res;
        } catch (const std::exception& e) {
            std::cerr << "查询失败 | SQL：" << sql << "错误：" << e.what() << std::endl;
            return pqxx::result();
        }
}
bool RelationalBroker::insert(const string& table, const vector<string>& cols, const vector<string>& vals) {
    if (cols.size() != vals.size()) {
        throw runtime_error("字段数与值数不匹配");
    }

    string sql = "INSERT INTO " + table + " (";
    // 拼接字段名
    for (size_t i = 0; i < cols.size(); ++i) {
        sql += cols[i] + (i < cols.size()-1 ? ", " : ") VALUES (");
    }
    // 拼接值
    for (size_t i = 0; i < vals.size(); ++i) {
        sql += "'" + vals[i] + "'" + (i < vals.size()-1 ? ", " : ") ON CONFLICT DO NOTHING;");
    }

    return query(sql).affected_rows() > 0;
}
bool RelationalBroker::drop(const string& table, const string& primary_key, const string& key_value) {
    string sql = "DELETE FROM " + table + " WHERE " + primary_key + " = '" + key_value + "';";

    if (!m_conn || !m_conn->is_open()) initConnection();
    try {
        pqxx::work tx(*m_conn);
        pqxx::result res = tx.exec(sql);
        tx.commit();
        return res.affected_rows() > 0;
    } catch (const std::exception& e) {
        std::cerr << "删除失败 | 错误：{}" << e.what() << std::endl;
        return false;
    }
}
