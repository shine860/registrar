module;
#include "pqxx/pqxx"
export module registrar:dm.teachsecretarybroker;

import std;
import :dm.base;
import :domain.teachingsecretary;
import :domain.teacher;
import :domain.course;
import :dm.teacherbroker;
import :dm.coursebroker;
import :dm.classroombroker;


export class TeachingSecretaryBroker : public RelationalBroker {
public:
    static TeachingSecretaryBroker& singleton();
    void createTable() override;
    void initData() override;

    bool addTeacher(const Teacher& teacher);
    bool addCourse(const Course& course);
    bool assignCourseToTeacher(const std::string& cid, const std::string& tid);
    std::shared_ptr<TeachingSecretary> findById(const std::string& sid);
    // 排课
    bool arrangement(const std::string& courseId, const std::string& roomId, const std::string& timeSlot);

private:
    TeachingSecretaryBroker();
    std::vector<std::shared_ptr<TeachingSecretary>> _secretaries;
};

TeachingSecretaryBroker::TeachingSecretaryBroker()
{
    initConnection();
}
TeachingSecretaryBroker& TeachingSecretaryBroker::singleton() { static TeachingSecretaryBroker instance; return instance; }

void TeachingSecretaryBroker::createTable() {
    query("CREATE TABLE IF NOT EXISTS Secretary (id VARCHAR(20) PRIMARY KEY, name VARCHAR(50) NOT NULL, dept VARCHAR(50) NOT NULL);");
}

void TeachingSecretaryBroker::initData() {
    query("DELETE FROM Secretary;");
    _secretaries.clear();
    std::vector<TeachingSecretary> data = { {"3001", "张秘书","女", "计算机学院"} };
    try {
        pqxx::work tx(*m_conn);
        for(auto& s : data) tx.exec("INSERT INTO Secretary(id, name, dept) VALUES($1, $2, $3)", pqxx::params{s.m_id, s.m_name, s.m_dept});
        tx.commit();
        for(auto& s : data) _secretaries.push_back(std::make_shared<TeachingSecretary>(s));
    } catch(...) {}
}
std::shared_ptr<TeachingSecretary> TeachingSecretaryBroker::findById(const std::string& sid) {

    for(auto& s : _secretaries) {
        if(s->hasId(sid)) return s;
    }


    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec(
            "SELECT * FROM Secretary WHERE id=$1",
            pqxx::params{sid}
        );
        tx.commit();

        if(!res.empty()) {
            auto r = res[0];
            auto s = std::make_shared<TeachingSecretary>(
                r["id"].as<std::string>(),
                r["name"].as<std::string>(),
                r["gender"].as<std::string>(),
                r["dept"].as<std::string>()
            );
            _secretaries.push_back(s);
            return s;
        }
    } catch(const std::exception& e) {
        std::cerr << "查询教学秘书失败: " << e.what() << std::endl;
    }
    return nullptr;
}
bool TeachingSecretaryBroker::addTeacher(const Teacher& teacher) {
    auto& teaBroker = TeacherBroker::singleton();
    if(teaBroker.isTeacherExists(teacher.m_id)) return false;
    try {
        pqxx::work tx(*m_conn);
        tx.exec("INSERT INTO Teacher(id, name, gender, dept, title) VALUES($1, $2, $3, $4, $5)",
            pqxx::params{teacher.m_id, teacher.m_name, teacher.m_gender, teacher.m_dept, teacher.m_title});
        tx.commit();
        teaBroker._teachers.push_back(std::make_shared<Teacher>(teacher));
        return true;
    } catch(...) { return false; }
}

bool TeachingSecretaryBroker::addCourse(const Course& course) {
    auto& couBroker = CourseBroker::singleton();
    if(couBroker.isCourseExists(course.m_courseId)) return false;
    try {
        pqxx::work tx(*m_conn);
        tx.exec("INSERT INTO Course(id, name, major, grade, credit, syllabus, teacher_id) VALUES($1, $2, $3, $4, $5, $6, $7)",
            pqxx::params{course.m_courseId, course.m_coursename, course.m_major, course.m_grade, course.m_credit, course.m_syllabus, course.m_teacherId});
        tx.commit();
        auto c = std::make_shared<Course>(course);
        couBroker._courses.push_back(c);
        return true;
    } catch(...) { return false; }
}

bool TeachingSecretaryBroker::assignCourseToTeacher(const std::string& cid, const std::string& tid) {
    return CourseBroker::singleton().updateTeacher(cid, tid);
}

bool TeachingSecretaryBroker::arrangement(const std::string& courseId, const std::string& roomId, const std::string& timeSlot) {
    auto& roomBroker = ClassroomBroker::singleton();
    auto& couBroker = CourseBroker::singleton();

    if(!couBroker.isCourseExists(courseId) || !roomBroker.isClassroomExists(roomId)) return false;

    bool roomOk = roomBroker.addCourseToSchedule(roomId, courseId, timeSlot);
    if(!roomOk) { std::cout << "排课冲突" << std::endl; return false; }

    std::cout << "排课成功" << std::endl;
    return true;
}
