module;
#include "pqxx/pqxx"
export module registrar:dm.studentbroker;

import std;
import :dm.base;
import registrar:domian.student;

export class StudentBroker : public RelationalBroker {
public:
    static StudentBroker& singleton();
    std::shared_ptr<class Student> findById(const string& sid);
    // 1. 添加学生（调用基类insert方法，直接拼接SQL）
    void createTable() override;
    void initData() override;
    bool addStudent(std::shared_ptr<class Student>& student);
    bool deleteStudent(const string& sid);
    bool isStudentExists(const string& sid);
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
    string sql="CREATE TABLE IF NOT EXISTS Student("
               "id VARCHAR(20) PRIMARY KEY,"
               "name VARCHAR(50) NOT NULL,"
               "gender VARCHAR(10) NOT NULL,"
               "dept VARCHAR(40) NOT NULL,"
               "grade VARCHAR(20) NOT NULL);";
    query(sql);
    print("学生表创建完成...\n");
}

void StudentBroker::initData()
{
    //先清空数据
    query("DELETE  FROM Student;");
    _students.clear();

    print("插入初始化数据...\n");
    vector<Student> teststudents = {
            {"2018001","李勇","男","信息安全","2018"},
            {"2018002","刘晨","女","计算机科学与技术","2018"},
            {"2018003","王敏","女","计算机科学与技术","2018"},
            {"2018004","张立","男","计算机科学与技术","2018"},
            {"2018005","陈新齐","男","信息管理","2018"},
            {"2018006","赵明","男","数据科学与大数据技术","2018"}
        };
    int successCount = 0;
    for (auto& stu : testStudents) {
        if (addStudent(stu)) {
            successCount++;
        }
    }
    _students.clear();
    for (auto& stu : testStudents) {
        _students.push_back(std::make_shared<Student>(stu));
    }
    print("学生数据初始化完成，成功插入{} 条数据",successCount);

}
std::shared_ptr<Student> StudentBroker::findById(string& sid)
{
    for(auto& student:_students){
        if(student->hasId(sid)) return student;
    }
    string sql = "SELECT id, name, gender,dept, grade FROM Student WHERE id = '" + sid + "';";
    pqxx::result res = this->query(sql);
    if (res.empty()) {
        cerr << "未找到学生：" << sid << "\n";
        return nullptr;
    }
    // 3. 数据库查询结果转Student对象
    auto row = res[0];
    auto student = std::make_shared<Student>(
                row["id"].as<std::string>(),
                row["name"].as<std::string>(),
                row["gender"].as<std::string>(),
                row["dept"].as<std::string>(),
                row["grade"].as<std::string>()
                );

    // 4. 加入缓存
    _students.push_back(student);
    std::print("数据库查询到学生：{}\n", sid);
    return student;
}
bool StudentBroker::addStudent(std::shared_ptr<class Student>& student)
{
    if(isStudentExists(student->m_id)){
        print("学生已经存在，添加失败!\n");
        return false;
    }

    std::vector<std::string> cols = {"id", "name", "gender", "dept", "grade"};
    std::vector<std::string> vals = {
        student->m_id,
        student->m_name,
        student->m_gendar,
        student->m_dept,
        student->m_grade
    };

    bool success = insert("Student", cols, vals);
    if (success) {
        _students.push_back(student); // 同步更新内存缓存
        print("学生{}添加成功", student->m_name);
    } else {
        print("学生{}添加失败", student->m_name);
    }
    return success;
}
bool deleteStudent(const string& sid)
{
    if (!isStudentExists(sid)) {
        print("学生{}不存在，删除失败", sid);
        return false;
    }

    bool success = drop("Student", "id", sid);
    if (success) {
        for (auto it = _students.begin(); it != _students.end(); ++it) {
            if ((*it)->hasId(sid)) {
                _students.erase(it);
                break;
            }
        }
        print("学生{}删除成功", sid);
    } else {
        print("学生{}删除失败", sid);
    }
    return success;
}

bool StudentBroker::isStudentExists(const std::string& sid) {
    for (auto& stu : _students) {
        if (stu->hasId(sid)) {
            return true;
        }
    }
    std::string sql = std::format("SELECT * FROM Student WHERE id = '{}';",sid);
    pqxx::result res = query(sql);
    return !res.empty();
}
