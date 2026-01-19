module;
#include "pqxx/pqxx"
export module registrar:dm.teacherbroker;

import std;
import :dm.base;
import registrar:domian.teacher;

export class TeacherBroker : public RelationalBroker {
public:
    static TeacherBroker& singleton();
    void createTable() override;
    void initData() override;
    std::shared_ptr<Teacher> findById(const string &tid);

    bool addTeacher(class Teacher& teacher);
    bool deleteTeacher(const string& id);

    std::vector<std::string> getTaughtCourses(const std::string& tid);
    std::vector<std::shared_ptr<Teacher>> findByDepartment(const std::string& dept);

    bool isTeacherExists(const std::string& tid);
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
                      "id VARCHAR(20) PRIMARY KEY,"          // 教师ID（主键，唯一）
                      "name VARCHAR(50) NOT NULL,"
                       "gender VARCHAR(8) NOT NULL,"
                      "department VARCHAR(50) NOT NULL,"     // 院系（非空）
                      "title VARCHAR(20) NOT NULL);";        // 职称（非空）
    try {
        query(sql);
        std::print("教师表创建成功\n");
    } catch (const std::exception& e) {
        std::cerr << "创建教师表失败：" << e.what() << std::endl;
        throw;
    }
}

void TeacherBroker::initData() {

    _teachers.clear();

    std::print("初始化教师测试数据...\n");

    std::vector<Teacher> testTeachers = {
        {"2001", "张三", "男","计算机学院", "教授"},
        {"2002", "李四", "男","计算机学院", "副教授"},
        {"2003", "王五", "女","电子工程学院", "讲师"},
        {"2004", "赵六", "女","数学学院", "教授"}
    };

    int successCount = 0;
    for (const auto& tea : testTeachers) {
        if (addTeacher(tea)) {
            successCount++;
        }
    }


    _teachers.clear();
    for (const auto& tea : testTeachers) {
        _teachers.push_back(std::make_shared<Teacher>(tea));
    }

    std::print("教师数据初始化完成，成功录入 {} 条记录\n", successCount);
}


std::shared_ptr<Teacher> TeacherBroker::findById(const std::string& tid) {

    for (const auto& tea : _teachers) {
        if (tea->hasId(tid)) {
            std::print("[缓存命中] 查询教师：{}（ID：{}）\n", tea->m_name, tid);
            return tea;
        }
    }


    std::print("[缓存未命中] 数据库查询教师：{}\n", tid);
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec_params(
                    "SELECT id, name,gender, department, title FROM Teacher WHERE id = $1;",tid);
        tx.commit();

        if (res.empty()) {
            std::cerr << "教师ID " << tid << " 不存在\n";
            return nullptr;
        }


        auto row = res[0];
        auto teacher = std::make_shared<Teacher>(
                    row["id"].as<std::string>(),
                    row["name"].as<std::string>(),
                    row["gender"].as<std::string>(),
                    row["department"].as<std::string>(),
                    row["title"].as<std::string>()
                    );
        // 加入缓存
        _teachers.push_back(teacher);
        return teacher;
    } catch (const std::exception& e) {
        std::cerr << "查询教师失败：" << e.what() << std::endl;
        return nullptr;
    }
}

bool TeacherBroker::addTeacher(const Teacher& teacher) {

    if (isTeacherExistsInternal(teacher.m_id)) {
        std::print("教师ID {} 已存在，新增失败\n", teacher.m_id);
        return false;
    }


    if (teacher.m_name.empty() || teacher.m_department.empty() || teacher.m_title.empty()) {
        std::print("教师姓名/院系/职称不能为空，新增失败（ID：{}）\n", teacher.m_id);
        return false;
    }


    try {
        pqxx::work tx(*m_conn);
        tx.exec_params(
            "INSERT INTO Teacher(id, name, department, title) VALUES ($1, $2, $3, $4);",
            teacher.m_id, teacher.m_name, teacher.m_department, teacher.m_title
        );
        tx.commit();


        _teachers.push_back(std::make_shared<Teacher>(teacher));
        std::print("新增教师成功：{}（ID：{}，院系：{}）\n", teacher.m_name, teacher.m_id, teacher.m_department);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "新增教师失败（ID：" << teacher.m_id << "）：" << e.what() << std::endl;
        return false;
    }
}

// 删除教师（校验授课记录）
bool TeacherBroker::deleteTeacher(const std::string& tid) {
    if (!isTeacherExistsInternal(tid)) {
        std::print("教师ID {} 不存在，删除失败\n", tid);
        return false;
    }

    try {
        pqxx::work txCheck(*m_conn);
        auto res = txCheck.exec_params("SELECT 1 FROM Course WHERE teacher_id = $1;", tid);
        txCheck.commit();
        if (!res.empty()) {
            std::print("教师ID {} 有授课记录，禁止删除\n", tid);
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "校验教师授课记录失败：" << e.what() << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec_params("DELETE FROM Teacher WHERE id = $1;", tid);
        tx.commit();

        for (auto it = _teachers.begin(); it != _teachers.end(); ++it) {
            if ((*it)->hasId(tid)) {
                _teachers.erase(it);
                break;
            }
        }

        std::print("删除教师成功（ID：{}）\n", tid);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "删除教师失败（ID：" << tid << "）：" << e.what() << std::endl;
        return false;
    }
}
bool TeacherBroker::isTeacherExistsInternal(const std::string& tid) {
    for (const auto& tea : _teachers) {
        if (tea->hasId(tid)) return true;
    }

    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec_params("SELECT 1 FROM Teacher WHERE id = $1;", tid);
        tx.commit();
        return !res.empty();
    } catch (const std::exception& e) {
        std::cerr << "检查教师存在性失败：" << e.what() << std::endl;
        return false;
    }
}
