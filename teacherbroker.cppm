module;
#include "pqxx/pqxx"
export module registrar:dm.teacherbroker;

import std;
import :dm.base;
import :domain.teacher;
using std::exception;
export class TeacherBroker : public RelationalBroker {
friend class TeachingSecretaryBroker;
public:
    static TeacherBroker& singleton();
    void createTable() override;
    void initData() override;
    std::shared_ptr<Teacher> findById(const string &tid);
    bool isTeacherExists(const std::string& tid);
    std::vector<std::shared_ptr<Teacher>> findAll();
private:
    TeacherBroker();
    TeacherBroker(const TeacherBroker&) = delete;
    TeacherBroker& operator=(const TeacherBroker&) = delete;

    std::vector<std::shared_ptr<Teacher>> _teachers;
};

TeacherBroker::TeacherBroker() {
    initConnection();
}

TeacherBroker& TeacherBroker::singleton() {
    static TeacherBroker instance;
    return instance;
}

void TeacherBroker::createTable() {
    std::print("创建教师表（Teacher）...\n");

    std::string sql = "CREATE TABLE IF NOT EXISTS Teacher("
                      "id VARCHAR(20) PRIMARY KEY,"          // 教师工号
                      "name VARCHAR(50) NOT NULL,"
                       "gender VARCHAR(8) NOT NULL,"
                      "department VARCHAR(50) NOT NULL,"     // 院系
                      "title VARCHAR(20) NOT NULL);";        // 职称
    try {
        query(sql);
        std::print("教师表创建成功\n");
    } catch (const std::exception& e) {
        std::cerr << "创建教师表失败：" << e.what() << std::endl;
        throw;
    }
}

void TeacherBroker::initData() {
    query("DELELE FROM Teacher;");
    _teachers.clear();

    std::print("初始化教师测试数据...\n");

    std::vector<Teacher> testTeachers = {
        {"2001", "张三", "男","计算机学院", "教授"},
        {"2002", "李四", "男","计算机学院", "副教授"},
        {"2003", "王五", "女","电子工程学院", "讲师"},
        {"2004", "赵六", "女","数学学院", "教授"}
    };
    try {
        pqxx::work tx(*m_conn);
        for(auto& t : testTeachers) {
            tx.exec("INSERT INTO Teacher(id, name, gender, dept, title) VALUES($1, $2, $3, $4, $5)",
                           pqxx::params{t.m_id, t.m_name, t.m_gender, t.m_dept, t.m_title});
        tx.commit();
        for(auto& t : testTeachers) _teachers.push_back(std::make_shared<Teacher>(t));
        }
    } catch(const std::exception& e) {
        std::cerr << "初始化教师表失败：" << e.what() << std::endl;
    }
}
std::shared_ptr<Teacher> TeacherBroker::findById(const std::string& tid) {
    for(auto& t : _teachers) if(t->hasId(tid)) return t;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT * FROM Teacher WHERE id=$1", pqxx::params{tid});
        tx.commit();
        if(!res.empty()) {
            auto r = res[0];
            auto t = std::make_shared<Teacher>(
                        r["id"].as<std::string>(), r["name"].as<std::string>(),
                        r["gender"].as<std::string>(), r["dept"].as<std::string>(),
                        r["title"].as<std::string>()
                        );
            _teachers.push_back(t);
            return t;
        }
    } catch(const std::exception& e) {
        std::cerr << "未找到：" << e.what() << std::endl;
    }
    return nullptr;
}


bool TeacherBroker::isTeacherExists(const std::string& tid) {
   return findById(tid)!=nullptr;
}
std::vector<std::shared_ptr<Teacher>> TeacherBroker::findAll() {
    if(_teachers.empty()) {
        try {
            pqxx::work tx(*m_conn);
            auto res = tx.exec("SELECT * FROM Teacher");
            tx.commit();
            for(auto r : res) {
                auto t = std::make_shared<Teacher>(
                    r["id"].as<std::string>(), r["name"].as<std::string>(),
                    r["gender"].as<std::string>(), r["dept"].as<std::string>(),
                    r["title"].as<std::string>()
                );
                _teachers.push_back(t);
            }
        } catch(const std::exception& e) {
            std::cerr << "初始化教师表失败：" << e.what() << std::endl;
        }
    }
    return _teachers;
}
