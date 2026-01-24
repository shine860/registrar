// Module
// File: teachsecretary.cppm   Version: 0.1.0   License: AGPLv3
// Created: 苏茜（2024051604029）   3236863614@qq.com   2026-01-24 21:36:02
// Description:教学秘书的实体类代管者，继承基类
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
using std::print;

export class TeachingSecretaryBroker : public RelationalBroker {
public:
    static TeachingSecretaryBroker& singleton();
    void createTable() override;
    void initData() override;

    bool addTeacher(const Teacher& teacher);
    bool addCourse(const Course& course);
    //建立教学班
    bool assignCourseToTeacher(const std::string& cid, const std::string& tid);
    std::shared_ptr<TeachingSecretary> findById(const std::string& sid);
    // 排课
    bool arrangement(const std::string& courseId, const std::string& roomId,const std::string&teaId, const std::string& timeSlot);
    bool createCourseClass(const std::string& cid,const std::string &tid,const std::string& studentClass, const std::string& classId);

private:
    //检查教师是否忙绿
    bool isTeacherBusy(const std::string& tid, const std::string& timeSlot);
    TeachingSecretaryBroker();
    TeachingSecretaryBroker(const TeachingSecretaryBroker&) = delete;
    TeachingSecretaryBroker& operator=(const TeachingSecretaryBroker&) = delete;

    std::vector<std::shared_ptr<TeachingSecretary>> _secretaries;
};
//构造函数
TeachingSecretaryBroker::TeachingSecretaryBroker()
{
    initConnection();
}
TeachingSecretaryBroker& TeachingSecretaryBroker::singleton() {
    static TeachingSecretaryBroker instance;
    return instance;
}
//创建表
void TeachingSecretaryBroker::createTable() {
    query("CREATE TABLE IF NOT EXISTS Secretary (id VARCHAR(20) PRIMARY KEY, name VARCHAR(50) NOT NULL, gender VARCHAR(10) NOT NULL,dept VARCHAR(50) NOT NULL);");
    //新增教学班表：一门课+一个老师+一个班级
    std::string sql = R"(
          CREATE TABLE IF NOT EXISTS CourseClass (
              id VARCHAR(20) PRIMARY KEY,       -- 教学班ID，如 "CC-001"
              course_id VARCHAR(20) NOT NULL,   -- 课程ID，关联 Course 表
              teacher_id VARCHAR(20) NOT NULL,  -- 教师ID，关联 Teacher 表
              student_class VARCHAR(50),         -- 行政班级，如 "计科1801"
              FOREIGN KEY (course_id) REFERENCES Course(id) ON DELETE CASCADE,
              FOREIGN KEY (teacher_id) REFERENCES Teacher(id) ON DELETE CASCADE
          )
      )";
    query(sql);

    std::string sqlSchedule = R"(
         CREATE TABLE IF NOT EXISTS CourseSchedule (
             classroom_id VARCHAR(20) NOT NULL,
             course_id VARCHAR(20) NOT NULL,
             teacher_id VARCHAR(20) NOT NULL,
             time_slot VARCHAR(50) NOT NULL,
             PRIMARY KEY(classroom_id, time_slot)
         )
     )";
     query(sqlSchedule);
}
//初始化数据
void TeachingSecretaryBroker::initData() {
    // query("DELETE FROM Secretary;");
    _secretaries.clear();
    std::vector<TeachingSecretary> data = { {"3001", "张秘书","女", "计算机学院"},
                                            {"3002","李秘书","男","数学学院"}
                                          };
    try {
        pqxx::work tx(*m_conn);
        for(auto& s : data) tx.exec("INSERT INTO Secretary(id, name,gender,dept) VALUES($1, $2, $3,$4) ON CONFLICT(id) DO NOTHING",
                                    pqxx::params{s.m_id, s.m_name,s.m_gender, s.m_dept});
        tx.commit();
        for(auto& s : data) _secretaries.push_back(std::make_shared<TeachingSecretary>(s));
        // std::cout << "多学院教学秘书初始化完成." << std::endl;
        print("多学院教学秘书初始化完成.\n");
    } catch(const std::exception& e) {
        std::cerr << "教务初始化失败" <<e.what()<< std::endl;
    }
}
//根据id查找教学秘书
std::shared_ptr<TeachingSecretary> TeachingSecretaryBroker::findById(const std::string& sid)
{
    for(auto& s : _secretaries) {
        if(s->hasId(sid)) return s;
    }
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT * FROM Secretary WHERE id=$1",
                           pqxx::params{sid});
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
//添加老师
bool TeachingSecretaryBroker::addTeacher(const Teacher& teacher) {
    auto& teaBroker = TeacherBroker::singleton();
    if(teaBroker.isTeacherExists(teacher.m_id)) return false;
    try {
        pqxx::work tx(*m_conn);
        tx.exec("INSERT INTO Teacher(id, name, gender, dept, title) VALUES($1, $2, $3, $4, $5)",
                pqxx::params{teacher.m_id, teacher.m_name, teacher.m_gender, teacher.m_dept, teacher.m_title});
        tx.commit();
        teaBroker.addTeacherToCache(teacher);
        return true;
    } catch(std::exception& e) {
        std::cerr << "添加教师失败。" << e.what() <<std::endl;
        return false;
    }
}
//添加课程
bool TeachingSecretaryBroker::addCourse(const Course& course) {
    auto& couBroker = CourseBroker::singleton();

    // 检查课程是否存在
    if(couBroker.isCourseExists(course.m_courseId)) {
        return false;
    }

    //插入数据库
    try {
        pqxx::work tx(*m_conn);

        tx.exec("INSERT INTO Course(id, name, major, grade, credit, syllabus,teacherid) VALUES($1, $2, $3, $4, $5, $6,$7)",
                pqxx::params{course.m_courseId, course.m_coursename, course.m_major, course.m_grade, std::to_string(course.m_credit), course.m_syllabus, course.m_teacherId});
        tx.commit();

        //调用 CourseBroker 的添加方法
        couBroker.addCourse(course);

        std::cout << "课程 " << course.m_coursename << " 添加成功！" << std::endl;
        return true;
    } catch(const std::exception& e) {
        std::cerr << "添加课程失败: " << e.what() << std::endl;
        return false;
    }
}
//分配课程给老师
bool TeachingSecretaryBroker::assignCourseToTeacher(const std::string& cid, const std::string& tid) {
    auto& cou = CourseBroker::singleton();
    return cou.updateTeacher(cid, tid);
}

bool TeachingSecretaryBroker::createCourseClass(const std::string& cid,const std::string &tid,const std::string& studentClass, const std::string& classId)
{
    // 判断是否存在
    auto& cou = CourseBroker::singleton();
    auto& tea = TeacherBroker::singleton();
    if (!cou.isCourseExists(cid) || !tea.isTeacherExists(tid)) {
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        // 插入教学班记录
        tx.exec("INSERT INTO CourseClass(id, course_id, teacher_id, student_class) VALUES($1, $2, $3, $4) ON CONFLICT(id) DO NOTHING",
                pqxx::params{classId, cid, tid, studentClass});
        tx.exec("UPDATE Course SET teacher_id = $1 WHERE id = $2",
                pqxx::params{tid, cid});
        tx.commit();
        cou.reloadCourse(cid);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "建立教学班失败: " << e.what() << std::endl;
        return false;
    }
}
//教师是否有冲突
bool TeachingSecretaryBroker::isTeacherBusy(const std::string& tid, const std::string& timeSlot) {
    try {
        pqxx::work tx(*m_conn);
        // 查询：该教师在这个时间段是否有排课记录
        auto res = tx.exec("SELECT 1 FROM CourseSchedule WHERE teacher_id=$1 AND time_slot=$2",
                           pqxx::params{tid, timeSlot});
        tx.commit();
        return !res.empty(); // 有记录 = 忙碌 = true
    } catch (...) {
        return false;
    }
}
//管理
bool TeachingSecretaryBroker::arrangement(const std::string& courseId, const std::string& roomId, const std::string& teaId, const std::string& timeSlot) {
    auto& roomBroker = ClassroomBroker::singleton();
    auto& couBroker = CourseBroker::singleton();
    auto& teaBroker = TeacherBroker::singleton();

    // 基础存在性检查
    if(!couBroker.isCourseExists(courseId) || !roomBroker.isClassroomExists(roomId) ||!teaBroker.isTeacherExists(teaId)) {
        // std::cout << "排课失败：资源不存在！" << std::endl;
        print("排课失败：资源不存在!\n");
        return false;
    }

    // std::cout << "正在检测排课冲突..." << std::endl;
    print("正在检测排课冲突...\n");

    // 教室冲突
    if (!roomBroker.isRoomAvailable(roomId, timeSlot)) {
        // std::cout << "   [教室冲突] " << roomId << " 在 " << timeSlot << " 已被占用！" << std::endl;
        print("教室冲突：{} 在 {}已经被占用!\n",roomId,timeSlot);
        return false;
    }
    // std::cout << "   [教室] 通过" << std::endl;
    print("教室检测通过\n");

    // 教师冲突
    if (isTeacherBusy(teaId, timeSlot)) {
        // std::cout << "   [教师冲突] " << teaId << " 在 " << timeSlot << " 已有课！" << std::endl;
        print("教师冲突：{} 在 {}已经有课程\n",teaId,timeSlot);
        return false;
    }
    // std::cout << "   [教师] 通过" << std::endl;
    print("教室检测通过\n");

    // 通过检测，写入数据库
    try {
        pqxx::work tx(*m_conn);

        // 写入排课记录
        tx.exec("INSERT INTO CourseSchedule(classroom_id, course_id, teacher_id, time_slot) VALUES($1, $2, $3, $4) ON CONFLICT DO NOTHING",
                pqxx::params{roomId, courseId, teaId, timeSlot}
                );

        // 绑定教师到课程
        tx.exec("UPDATE Course SET teacher_id=$1 WHERE id=$2", pqxx::params{teaId, courseId});

        tx.commit();

        // 刷新 Course 缓存
        couBroker.reloadCourse(courseId);

        // std::cout << ">> 排课成功！" << std::endl;
        print("排课成功\n");
        return true;
    } catch(const std::exception& e) {
        std::cerr << "排课异常: " << e.what() << std::endl;
        return false;
    }
}
