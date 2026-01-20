module;
#include "pqxx/pqxx"
export module registrar:dm.studentbroker;

import std;
import :dm.base;
import :domain.student;

export class StudentBroker : public RelationalBroker {
public:
    static StudentBroker& singleton();
    std::shared_ptr<class Student> findById(string& sid);
    void createTable() override;
    void initData() override;
    bool isStudentExists(string& sid);
private:
    StudentBroker();
    StudentBroker(const StudentBroker&) = delete;
    StudentBroker& operator=(const StudentBroker&) = delete;
    std::vector<std::shared_ptr<class Student>> _students;
};


StudentBroker::StudentBroker()
{
    initConnection();
}

StudentBroker &StudentBroker::singleton()
{
    static StudentBroker instance;
    return instance;
}

void StudentBroker::createTable()
{
    print("创建学生表...\n");
    std::string sql = R"(
          CREATE TABLE IF NOT EXISTS Student (
              id VARCHAR(20) PRIMARY KEY,
              name VARCHAR(50) NOT NULL,
              gender VARCHAR(10) NOT NULL,
              dept VARCHAR(40) NOT NULL,
              grade INT NOT NULL,            -- 对应 m_grade (int)
              major VARCHAR(50) NOT NULL,    -- 对应 m_major
              studentclass VARCHAR(50) NOT NULL, -- 对应 m_studentclass
              creditlimit DOUBLE PRECISION NOT NULL -- 对应 m_creditLimit
          )
      )";
      query(sql);
}


void StudentBroker::initData()
{
    //先清空数据
    query("DELETE  FROM Student;");
    _students.clear();

    print("插入初始化数据...\n");
    vector<Student> teststudents = {
        {"2018001", "李勇", "男", "信息安全", 2018, "信息安全", "信安1801", 20.0},
        {"2018002", "刘晨", "女", "计算机科学与技术", 2018, "计算机科学与技术", "计科1801", 20.0},
        {"2018003", "王敏", "女", "计算机科学与技术", 2018, "计算机科学与技术", "计科1802", 20.0},
        {"2018004", "张立", "男", "计算机科学与技术", 2019, "计算机科学与技术", "计科1901", 20.0},
        {"2018005", "陈新齐", "男", "信息管理", 2018, "信息管理", "信管1801", 20.0},
        {"2018006", "赵明", "男", "数据科学与大数据技术", 2018, "数据科学", "数据1801", 20.0}
        };

    try {
           pqxx::work tx(*m_conn);
           for(auto& s : teststudents) {
               tx.exec(
                   "INSERT INTO Student(id, name, gender, dept, grade, major, studentclass, creditlimit) "
                   "VALUES($1, $2, $3, $4, $5, $6, $7, $8)",
                   pqxx::params{s.m_id, s.m_name, s.m_gender, s.m_dept,
                   s.m_grade, s.m_major, s.m_studentclass, s.m_creditLimit});
            }
           tx.commit();

           // 加入内存缓存
           for(auto& s : teststudents) {
               _students.push_back(std::make_shared<Student>(s));
           }
       } catch(const std::exception& e) {
           std::cerr << "初始化学生数据失败: " << e.what() << std::endl;
       }
}
std::shared_ptr<Student> StudentBroker::findById(string& sid)
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


bool StudentBroker::isStudentExists(std::string& sid) {
  return findById(sid)!=nullptr;
}
