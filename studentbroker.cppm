// Module
// File: studentbroker.cppm   Version: 0.1.0   License: AGPLv3
// Created: 苏茜（2024051604029）   3236863614@qq.com   2026-01-24 21:35:16
// Description:student的代管者，继承基类
module;
#include "pqxx/pqxx"
export module registrar:dm.studentbroker;

import std;
import :dm.base;
import :domain.student;

export class StudentBroker : public RelationalBroker {
public:
    static StudentBroker& singleton();
    std::shared_ptr<class Student> findById(const string& sid);
    void createTable() override;
    void initData() override;
    bool isStudentExists(const string& sid);
    std::string trim(const std::string& s);
private:
    StudentBroker();
    StudentBroker(const StudentBroker&) = delete;
    StudentBroker& operator=(const StudentBroker&) = delete;
    std::vector<std::shared_ptr<class Student>> _students;
};

//构造函数
StudentBroker::StudentBroker()
{
    initConnection();
}
//单例
StudentBroker &StudentBroker::singleton()
{
    static StudentBroker instance;
    return instance;
}
//创建数据表
void StudentBroker::createTable()
{
    print("创建学生表...\n");
    std::string sql = R"(
          CREATE TABLE IF NOT EXISTS Student (
              id VARCHAR(20) PRIMARY KEY,
              name VARCHAR(50) NOT NULL,
              gender VARCHAR(10) NOT NULL,
              dept VARCHAR(40) NOT NULL,
              grade INT NOT NULL,
              major VARCHAR(50) NOT NULL,
              studentclass VARCHAR(50) NOT NULL,
              creditlimit DOUBLE PRECISION NOT NULL
          )
      )";
      query(sql);
}

//初始化数据
void StudentBroker::initData()

{
    //先清空数据
     // query("DELETE  FROM Student;");
    _students.clear();

    print("插入初始化数据...\n");
    vector<Student> teststudents = {
        {"2018001", "Jake", "男", "信息安全", 2018, "信息安全", "信安1801", 20.0},
        {"2018002", "Noah", "女", "计算机科学与技术", 2018, "计算机科学与技术", "计科1801", 20.0},
        {"2018003", "Olivia", "女", "计算机科学与技术", 2018, "计算机科学与技术", "计科1802", 20.0},
        {"2018004", "Ethan", "男", "计算机科学与技术", 2019, "计算机科学与技术", "计科1901", 20.0},
        {"2018005", "Liam", "男", "信息管理", 2018, "信息管理", "信管1801", 20.0},
        {"2018006", "Zoe", "男", "数据科学与大数据技术", 2018, "数据科学", "数据1801", 20.0}
        };

    try {
           pqxx::work tx(*m_conn);
           for(auto& s : teststudents) {
               tx.exec(
                   "INSERT INTO Student(id, name, gender, dept, grade, major, studentclass, creditlimit) "
                   "VALUES($1, $2, $3, $4, $5, $6, $7, $8) ON CONFLICT(id) DO NOTHING",
                   pqxx::params{s.m_id, s.m_name, s.m_gender, s.m_dept,
                   s.m_grade, s.m_major, s.m_studentclass, std::to_string(s.m_creditLimit)});}
           tx.commit();

           // 加入内存缓存
           for(auto& s : teststudents) {
               _students.push_back(std::make_shared<Student>(s));
           }
       } catch(const std::exception& e) {
           std::cerr << "初始化学生数据失败: " << e.what() << std::endl;
       }
}

//通过id来查找学生
std::shared_ptr<Student> StudentBroker::findById(const string& sid)
{
    for(auto& s:_students){
        if(s->hasId(sid)) return s;
    }
    string sql = "SELECT id, name, gender,dept, grade FROM Student WHERE id = '" + sid + "';";
    pqxx::result res = this->query(sql);
    if (res.empty()) {
        std::cerr << "未找到学生：" << sid << "\n";
        return nullptr;
    }
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT * FROM Student WHERE id=$1", pqxx::params{sid});
        tx.commit();

        if(!res.empty()) {
            auto r = res[0];
            // 构造函数参数: id, name, gender, dept, grade, major, studentclass, creditLimit
            auto s = std::make_shared<Student>(
                        r["id"].as<std::string>(),
                        r["name"].as<std::string>(),
                        r["gender"].as<std::string>(),
                        r["dept"].as<std::string>(),
                        r["grade"].as<int>(),              // int
                        r["major"].as<std::string>(),      // string
                        r["studentclass"].as<std::string>(), // string
                        r["creditlimit"].as<double>()      // double
                        );
            _students.push_back(s);
            return s;
        }
    } catch(const std::exception& e) {
        std::cerr << "查询学生失败: " << e.what() << std::endl;
    }
    return nullptr;
}

//判断学生是否存在
bool StudentBroker::isStudentExists(const std::string& sid) {
  return findById(sid)!=nullptr;
}
//消除输入时首位无效字符
std::string StudentBroker::trim(const std::string& s) {
    if (s.empty()) return s;
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}
