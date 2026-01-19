module;
#include "pqxx/pqxx"
export module registrar:dm.teachsecretarybroker;

import std;
import :dm.base;
import registrar:domian.teachingSecretary;

export class TeachingSecretaryBroker : public RelationalBroker {
public:
    static SecretaryBroker& singleton();
    void createTable() override;
    void initData() override;
    bool isSecretaryExistsInternal(const std::string& sid);
    // 添加教学秘书
    bool addTeachingSecretary(const TeachingSecretary& secretary);
    // 删除秘书
    bool deleteTeachingSecretary(const string& id);

    bool addTeacherBySecretary(const std::string& secretaryId, const Teacher& teacher);
    bool deleteTeacherBySecretary(const std::string& secretaryId, const std::string& teacherId);
    //添加课程
    bool addCourseBySecretary(const std::string& secretaryId, const Course& course);
    //分配课程给教师
    bool assignCourseToTeacher(const std::string& secretaryId, const std::string& courseId, const std::string& teacherId);
    bool deleteCourseBySecretary(const std::string& secretaryId, const std::string& courseId);
private:
    TeachingSecretaryBroker();
    TeachingSecretaryBroker(const SecretaryBroker&) = delete;
    TeachingSecretaryBroker& operator=(const SecretaryBroker&) = delete;

    // 秘书缓存
    vector<shared_ptr<TeachingSecretary>> _secretarys;
};


SecretaryBroker::SecretaryBroker()
{
    initConnection();
}

SecretaryBroker& SecretaryBroker::singleton() {
    static SecretaryBroker instance;
    return instance;
}

void SecretaryBroker::createTable() {
    std::cout << "创建教学秘书表..." << std::endl;
    std::string sql = "CREATE TABLE IF NOT EXISTS Secretary("
                      "id VARCHAR(20) PRIMARY KEY,"          // 主键
                      "name VARCHAR(50) NOT NULL,"           // 姓名非空
                      "department VARCHAR(50) NOT NULL);";   // 负责院系非空
    try {
        query(sql);
        std::cout << "教学秘书表创建成功" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "创建教学秘书表失败：" << e.what() << std::endl;
    }
}

void SecretaryBroker::initData() {

    _secretaries.clear();

    std::cout << "初始化教学秘书测试数据..." << std::endl;
    // 仅保留1条测试数据（计算机学院秘书）
    std::vector<Secretary> testSecretaries = {
        {"3001", "张秘书", "计算机学院"},
        {"3002","李秘书","数学学院"}
    };


    int success = 0;
    for (const auto& sec : testSecretaries) {
        if (addSecretary(sec)) success++;
    }


    for (const auto& sec : testSecretaries) {
        _secretaries.push_back(std::make_shared<Secretary>(sec));
    }

    std::cout << "教学秘书数据初始化完成，成功录入 " << success << " 条" << std::endl;
}

bool SecretaryBroker::isSecretaryExistsInternal(const std::string& sid) {

    for (const auto& sec : _secretaries) {
        if (sec->hasId(sid)) return true;
    }


    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec_params("SELECT 1 FROM Secretary WHERE id = $1;", sid);
        tx.commit();
        return !res.empty();
    } catch (const std::exception& e) {
        std::cerr << "检查教学秘书存在性失败：" << e.what() << std::endl;
        return false;
    }
}


std::shared_ptr<Secretary> SecretaryBroker::findById(const std::string& sid) {

    for (const auto& sec : _secretaries) {
        if (sec->hasId(sid)) {
            std::cout << "查询到教学秘书：" << sec->m_name << "（ID：" << sid << "）" << std::endl;
            return sec;
        }
    }

    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec_params(
            "SELECT id, name, department FROM Secretary WHERE id = $1;",
            sid
        );
        tx.commit();

        if (res.empty()) {
            std::cerr << "教学秘书ID " << sid << " 不存在" << std::endl;
            return nullptr;
        }

        auto row = res[0];
        auto secretary = std::make_shared<Secretary>(
            row["id"].as<std::string>(),
            row["name"].as<std::string>(),
            row["department"].as<std::string>()
        );
        _secretaries.push_back(secretary);

        std::cout << "查询到教学秘书：" << secretary->m_name << "（ID：" << sid << "）" << std::endl;
        return secretary;
    } catch (const std::exception& e) {
        std::cerr << "查询教学秘书失败：" << e.what() << std::endl;
        return nullptr;
    }
}
bool SecretaryBroker::addSecretary(const Secretary& secretary) {

    if (isSecretaryExistsInternal(secretary.m_id)) {
        std::cerr << "教学秘书ID " << secretary.m_id << " 已存在，新增失败" << std::endl;
        return false;
    }


    if (secretary.m_name.empty() || secretary.m_department.empty()) {
        std::cerr << "秘书姓名/负责院系不能为空，新增失败（ID：" << secretary.m_id << "）" << std::endl;
        return false;
    }


    try {
        pqxx::work tx(*m_conn);
        tx.exec_params(
            "INSERT INTO Secretary(id, name, department) VALUES ($1, $2, $3);",
            secretary.m_id, secretary.m_name, secretary.m_department
        );
        tx.commit();


        _secretaries.push_back(std::make_shared<Secretary>(secretary));
        std::cout << "新增教学秘书成功：" << secretary.m_name << "（ID：" << secretary.m_id << "）" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "新增教学秘书失败：" << e.what() << std::endl;
        return false;
    }
}

bool SecretaryBroker::deleteSecretary(const std::string& sid) {

    if (!isSecretaryExistsInternal(sid)) {
        std::cerr << "教学秘书ID " << sid << " 不存在，删除失败" << std::endl;
        return false;
    }


    try {
        pqxx::work tx(*m_conn);
        tx.exec_params("DELETE FROM Secretary WHERE id = $1;", sid);
        tx.commit();


        for (auto it = _secretaries.begin(); it != _secretaries.end(); ++it) {
            if ((*it)->hasId(sid)) {
                _secretaries.erase(it);
                break;
            }
        }

        std::cout << "删除教学秘书成功（ID：" << sid << "）" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "删除教学秘书失败：" << e.what() << std::endl;
        return false;
    }
}
bool SecretaryBroker::addTeacherBySecretary(const std::string& secretaryId, const Teacher& teacher) {

    if (!isSecretaryExistsInternal(secretaryId)) {
        std::cerr << "教学秘书ID " << secretaryId << " 不存在，录入教师失败" << std::endl;
        return false;
    }


    auto& teaBroker = TeacherBroker::singleton();
    return teaBroker.addTeacher(teacher);
}

bool SecretaryBroker::deleteTeacherBySecretary(const std::string& secretaryId, const std::string& teacherId) {

    if (!isSecretaryExistsInternal(secretaryId)) {
        std::cerr << "教学秘书ID " << secretaryId << " 不存在，删除教师失败" << std::endl;
        return false;
    }


    auto& teaBroker = TeacherBroker::singleton();
    return teaBroker.deleteTeacher(teacherId);
}

// 教学秘书录入课程（关联教师，先校验秘书身份）
bool SecretaryBroker::addCourseBySecretary(const std::string& secretaryId, const Course& course) {

    if (!isSecretaryExistsInternal(secretaryId)) {
        std::cerr << "教学秘书ID " << secretaryId << " 不存在，录入课程失败" << std::endl;
        return false;
    }


    auto& couBroker = CourseBroker::singleton();
    return couBroker.addCourse(course);
}

// 教学秘书分配课程给教师（核心操作）
bool SecretaryBroker::assignCourseToTeacher(const std::string& secretaryId, const std::string& courseId, const std::string& teacherId) {

    if (!isSecretaryExistsInternal(secretaryId)) {
        std::cerr << "教学秘书ID " << secretaryId << " 不存在，分配课程失败" << std::endl;
        return false;
    }


    auto& couBroker = CourseBroker::singleton();
    auto& teaBroker = TeacherBroker::singleton();
    if (!couBroker.isCourseExists(courseId)) {
        std::cerr << "课程ID " << courseId << " 不存在，分配失败" << std::endl;
        return false;
    }
    if (!teaBroker.isTeacherExists(teacherId)) {
        std::cerr << "教师ID " << teacherId << " 不存在，分配失败" << std::endl;
        return false;
    }


    return couBroker.updateCourseTeacher(courseId, teacherId);
}


bool SecretaryBroker::deleteCourseBySecretary(const std::string& secretaryId, const std::string& courseId) {

    if (!isSecretaryExistsInternal(secretaryId)) {
        std::cerr << "教学秘书ID " << secretaryId << " 不存在，删除课程失败" << std::endl;
        return false;
    }


    auto& couBroker = CourseBroker::singleton();
    return couBroker.deleteCourse(courseId);
}
