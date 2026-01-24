// Module
// File: coursebroker.cppm   Version: 0.1.0   License: AGPLv3
// Created: 苏茜（2024051604029）   3236863614@qq.com   2026-01-24 21:33:08
// Description:课程实体的代管者类，继承基类
//
module;
#include "pqxx/pqxx"
export module registrar:dm.coursebroker;

import :dm.base;
import :dm.teacherbroker;
import :domain.course;

import std;

using std::print;

export class CourseBroker : public RelationalBroker {
public:
    static CourseBroker& singleton();
    void createTable() override;
    void initData() override;

    // 查询
    std::shared_ptr<Course> findById(const std::string& cid);
    bool isCourseExists(const std::string& cid) const;
    bool updateTeacher(const std::string& cid, const std::string& tid);
    // 课程管理
    bool addCourse(const Course& course);
    bool deleteCourse(const std::string& cid);

    // 先修课管理
    bool addPrerequisite(const std::string& cid, const std::string& pid);
    bool removePrerequisite(const std::string& cid, const std::string& pid);

    // 教师分配
    bool assignTeacher(const std::string& cid, const std::string& tid);
    void reloadCourse(const std::string& cid);
    //方便学生查看
    std::vector<std::shared_ptr<Course>> findAll();
private:
    CourseBroker();
    CourseBroker(const CourseBroker&) = delete;
    CourseBroker& operator=(const CourseBroker&) = delete;
     std::vector<std::shared_ptr<Course>> _courses;

    std::vector<std::string> getPrerequisiteIds(const std::string& cid);
};
//构造函数
CourseBroker::CourseBroker()
{
    initConnection();
}
//单例创建
CourseBroker& CourseBroker::singleton() {
    static CourseBroker instance;
    return instance;
}
//创建数据表
void CourseBroker::createTable() {
    // 创建课程表和先修课关联表
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS Course (
            id VARCHAR(20) PRIMARY KEY,
            name VARCHAR(50) NOT NULL,
            major VARCHAR(50),
            grade INT,
            credit DOUBLE PRECISION,
            syllabus VARCHAR(100),
            teacher_id VARCHAR(20)
        );
        CREATE TABLE IF NOT EXISTS CoursePrerequisite (
            course_id VARCHAR(20) NOT NULL,
            pre_course_id VARCHAR(20) NOT NULL,
            PRIMARY KEY(course_id, pre_course_id),
            FOREIGN KEY(course_id) REFERENCES Course(id) ON DELETE CASCADE,
            FOREIGN KEY(pre_course_id) REFERENCES Course(id) ON DELETE CASCADE
        )
    )";
    query(sql);
}
//初始化数据
void CourseBroker::initData() {
    // query("DELETE FROM CoursePrerequisite; DELETE FROM Course;");
    _courses.clear();

    // 注意：必须显式构造对象，参数顺序：id, name, major, grade, credit, syllabus
    std::vector<Course> testCourses = {
        {"1001", "C++程序设计", "计算机", 1, 2.0, "必修课"},
        {"1002", "数据结构",     "计算机", 1, 3.0, "必修课"},
        {"1003", "操作系统",     "计算机", 2, 3.5, "必修课"},
        {"1004", "Python编程",   "计算机", 1, 2.0, "选修课"}
    };

    try {
        pqxx::work tx(*m_conn);
        for(auto& c : testCourses) {
            tx.exec(
                "INSERT INTO Course(id, name, major, grade, credit, syllabus) VALUES($1, $2, $3, $4, $5, $6) ON CONFLICT(id) DO NOTHING",
                pqxx::params{c.m_courseId, c.m_coursename, c.m_major, c.m_grade, std::to_string(c.m_credit), c.m_syllabus}
            );
        }
        tx.commit();

        // 加载先修课数据
        addPrerequisite("1002", "1001"); // 数据结构需要C++基础
        addPrerequisite("1003", "1002"); // 操作系统需要数据结构


        for(auto& c : testCourses) {
            auto ptr = std::make_shared<Course>(c);

            auto pids = getPrerequisiteIds(c.m_courseId);
            for(auto& pid : pids) {
                ptr->addPrerequisiteId(pid);
            }
            _courses.push_back(ptr);
        }

    } catch(const std::exception& e) {
        std::cerr << "Init Data Error: " << e.what() << std::endl;
    }
}

//判断课程是否存在
bool CourseBroker::isCourseExists(const std::string& cid) const {

    for(auto& c : _courses) {
        if(c->hasId(cid)) return true;
    }

    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT 1 FROM Course WHERE id=$1",
            pqxx::params{cid});
        tx.commit();
        return !res.empty();
    } catch(...) {
        return false;
    }
}

//根据id找课程
std::shared_ptr<Course> CourseBroker::findById(const std::string& cid) {
    for(auto& c : _courses) {
        if(c->hasId(cid)) return c;
    }
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT * FROM Course WHERE id=$1",pqxx::params{cid});
        tx.commit();

        if(!res.empty()) {
            auto r = res[0];
            auto c = std::make_shared<Course>(
                r["id"].as<std::string>(),
                r["name"].as<std::string>(),
                r["major"].as<std::string>(),
                r["grade"].as<int>(),
                r["credit"].as<double>(),
                r["syllabus"].as<std::string>()
            );
            if(!r["teacher_id"].is_null()) c->assignTeacherId(r["teacher_id"].as<std::string>());


            auto pids = getPrerequisiteIds(cid);
            for(auto& pid : pids) {
                c->addPrerequisiteId(pid);
            }

            _courses.push_back(c);
            return c;
        }
    } catch(...) {
        print("没有找到对应的课程.\n");
    }
    return nullptr;
}
//添加课程
bool CourseBroker::addCourse(const Course& course) {
    if(isCourseExists(course.m_courseId)) {
        std::cout << "课程ID " << course.m_courseId << " 已存在" << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec("INSERT INTO Course(id, name, major, grade, credit, syllabus) VALUES($1, $2, $3, $4, $5, $6)",
            pqxx::params{course.m_courseId, course.m_coursename, course.m_major, course.m_grade, std::to_string(course.m_credit), course.m_syllabus});
        tx.commit();
        _courses.push_back(std::make_shared<Course>(course));
        std::cout << "课程添加成功" << std::endl;
        return true;
    } catch(const std::exception& e) {
        std::cerr << "添加课程失败: " << e.what() << std::endl;
        return false;
    }
}
//删除课程
bool CourseBroker::deleteCourse(const std::string& cid) {
    if(!isCourseExists(cid)) {
        std::cout << "课程不存在" << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec("DELETE FROM Course WHERE id=$1", pqxx::params{cid});
        tx.commit();

        // 清理缓存
        _courses.erase(std::remove_if(_courses.begin(), _courses.end(), [&](const auto& c){ return c->hasId(cid);}), _courses.end());
        return true;
    } catch(...) { return false; }
}
//添加先修课
bool CourseBroker::addPrerequisite(const std::string& cid, const std::string& pid) {
    if(!isCourseExists(cid) || !isCourseExists(pid)) {
        print("课程或先修课不存在\n");
        return false;
    }
    if(cid == pid) {
        print("课程不能作为自己的先修课\n");
        return false;
    }
    try {
        pqxx::work tx(*m_conn);
        tx.exec("INSERT INTO CoursePrerequisite(course_id, pre_course_id) VALUES($1, $2)",
            pqxx::params{cid, pid});
        tx.commit();
        // 同步缓存
        for(auto& c : _courses) {
            if(c->hasId(cid)) c->addPrerequisiteId(pid);
        }
        return true;
    } catch(const pqxx::unique_violation&) {
        print("先修课关系已存在\n");
        return false;
    } catch(...) {
        return false;
    }
}
//移除先修课
bool CourseBroker::removePrerequisite(const std::string& cid, const std::string& pid) {
    if(!isCourseExists(cid) || !isCourseExists(pid)) return false;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("DELETE FROM CoursePrerequisite WHERE course_id=$1 AND pre_course_id=$2", pqxx::params{cid, pid});
        tx.commit();
        return true;
    } catch(...) {
        return false;
    }
}
//分配教师
bool CourseBroker::assignTeacher(const std::string& cid, const std::string& tid) {
    if(!isCourseExists(cid) || !TeacherBroker::singleton().isTeacherExists(tid)) return false;
    return updateTeacher(cid, tid);
}
//得到先修课的id
std::vector<std::string> CourseBroker::getPrerequisiteIds(const std::string& cid) {
    std::vector<std::string> pids;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT pre_course_id FROM CoursePrerequisite WHERE course_id=$1", pqxx::params{cid});
        tx.commit();
        for(const auto& r : res) {
            pids.push_back(r["pre_course_id"].as<std::string>());
        }
    } catch(...) {
        print("没有查询到对应的先修课记录.\n");
    }
    return pids;
}
bool CourseBroker::updateTeacher(const std::string& cid, const std::string& tid) {
    if (!isCourseExists(cid)) {
        std::cerr << "课程 " << cid << " 不存在！" << std::endl;
        return false;
    }
    auto& tea = TeacherBroker::singleton();

    if (!tea.isTeacherExists(tid)) {
        std::cerr << "教师 " << tid << " 不存在！" << std::endl;
        return false;
    }


    try {
        pqxx::work tx(*m_conn);

        tx.exec("UPDATE Course SET teacher_id = $1 WHERE id = $2",
                pqxx::params{tid, cid});
        tx.commit();

        for (auto& c : _courses) {
            if (c->hasId(cid)) {
                c->assignTeacherId(tid);
                //std::cout << ">> 教师 " << tid << " 已分配给课程 " << cid << std::endl;
                print("教师 {} 已经分配给课程 {}\n",tid,cid);
                return true;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "更新教师失败：" << e.what() << std::endl;
        return false;
    }
    return true;
}
void CourseBroker::reloadCourse(const std::string& cid) {
    _courses.erase(std::remove_if(_courses.begin(), _courses.end(),[&](const auto& c){ return c->hasId(cid); }),_courses.end());
}

//找到所有course的信息
std::vector<std::shared_ptr<Course>> CourseBroker::findAll() {
    std::vector<std::shared_ptr<Course>> result;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT * FROM Course ORDER BY id"); // 查询所有课程
        tx.commit();

        for(const auto& r : res) {
            auto c = std::make_shared<Course>(
                r["id"].as<std::string>(),
                r["name"].as<std::string>(),
                r["major"].as<std::string>(),
                r["grade"].as<int>(),
                r["credit"].as<double>(),
                r["syllabus"].as<std::string>()
            );
            if (!r["teacher_id"].is_null()) {
                c->assignTeacherId(r["teacher_id"].as<std::string>());
            }

            result.push_back(c);
        }
    } catch (...) {
        std::cerr << "查询所有课程失败: " << std::endl;
    }
    return result;
}
