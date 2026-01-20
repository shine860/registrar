module;
#include "pqxx/pqxx"
export module registrar:dm.base;

import std;
using std::string;
using std::print;
using std::vector;
using std::unique_ptr;
using std::runtime_error;

export class RelationalBroker{
public:
public:
    RelationalBroker();
    virtual ~RelationalBroker() = default;

    void initConnection();
    pqxx::result query(const string& sql);
    // bool insert(const string& table, const vector<string>& cols, const vector<string>& vals);
    // bool drop(const string& table, const string& primary_key, const string& key_value);

    virtual void createTable() = 0;
    virtual void initData() = 0;

protected://静态连接指针，全剧共享一个连接
    static unique_ptr<pqxx::connection> m_conn;

};

unique_ptr<pqxx::connection> RelationalBroker::m_conn = nullptr;

RelationalBroker::RelationalBroker() {}

void RelationalBroker::initConnection() {
    if (m_conn && m_conn->is_open()) {
        print("数据库连接已初始化\n");
        return;
    }

    try {
        const string conn_str = "dbname=EnrollSys "
                                "user=enrolluser "
                                "password=123456 "
                                "hostaddr=127.0.0.1 "
                                "port=5432";

        m_conn = std::make_unique<pqxx::connection>(conn_str);

        if (m_conn->is_open()) {
            print("连接数据库成功: {}", m_conn->dbname());
        } else {
            throw runtime_error("数据库连接未成功");
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
