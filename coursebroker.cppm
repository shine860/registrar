module;
#include "pqxx/pqxx"
export module registrar:dm.coursebroker;
import std;
import :dm.base;
import registrar:domain.course;

export class CourseBroker : public RelationalBroker {
public:
    static CourseBroker& singleton();
    std::shared_ptr<Course> findById(const std::string& cno);
    bool addCourse(const std::shared_ptr<Course>& course);
    bool deleteCourse(const std::string& cno);

    void createTable() override;
    void initData() override;
    //添加先修课
    bool addPrecourse(const std::string& courseId,const std::string& preCourseId);
    //移除课程的某个先修课
    bool removePrecourse(const std::string& courseId,const std::string& preCourseI);
    // 获取课程的所有先修课
    std::vector<std::shared_ptr<Course>> getPrerequisites(const std::string& courseId);
    // 校验学生是否满足该课程的先修课条件（选课前置检查）
    bool checkPrerequisiteForStudent(const std::string& studentId, const std::string& courseId);
    bool updateCourseTeacher(const std::string& courseId, const std::string& newTeacherId);
private:
    CourseBroker();
    CourseBroker(const CourseBroker&) = delete;
    CourseBroker& operator=(const CourseBroker&) = delete;
    std::vector<std::shared_ptr<Course>> _courses;
    bool isCourseExits(const string &cid);
};


CourseBroker::CourseBroker()
{
    initConnection();
}
//获取单例
CourseBroker& CourseBroker::singleton() {
    static CourseBroker instance;
    return instance;
}

void CourseBroker::createTable() {
    std::print("创建课程表和先修课关联表...\n");

    // 1. 创建课程主表
    std::string sqlCourse = "CREATE TABLE IF NOT EXISTS Course("
                            "id VARCHAR(20) PRIMARY KEY,"
                            "name VARCHAR(50) NOT NULL,"
                            "credit FLOAT NOT NULL,"
                            "teacher_id VARCHAR(20) NOT NULL,"
                            "FOREIGN KEY(teacher_id) REFERENCES Teacher(id));";
    query(sqlCourse);

    // 2. 创建先修课关联表（多对多）
    std::string sqlPre = "CREATE TABLE IF NOT EXISTS CoursePrerequisite("
                          "course_id VARCHAR(20) NOT NULL,"
                          "pre_course_id VARCHAR(20) NOT NULL,"
                          "PRIMARY KEY(course_id, pre_course_id),"
                          "FOREIGN KEY(course_id) REFERENCES Course(id) ON DELETE CASCADE,"
                          "FOREIGN KEY(pre_course_id) REFERENCES Course(id) ON DELETE CASCADE);";
    query(sqlPre);

    std::print("课程表 + 先修课关联表创建完成...\n");
}

void CourseBroker::initData() {
    // 先清空数据库和缓存
    _courses.clear();

    std::print("插入课程初始化数据...\n");
    std::vector<Course> testCourses = {
        {"1001", "C++程序设计", 2.0, "2001"},
        {"1002", "数据结构", 3.0, "2001"},
        {"1003", "操作系统", 3.5, "2001"},
        {"1004", "Python编程", 2.0, "2002"}
    };

    int successCount = 0;
    for (auto& cou : testCourses) {
        if (addCourse(cou)) {
            successCount++;
        }
    }
    addPrerequisite("1002", "1001");  // 数据结构 → C++程序设计
    addPrerequisite("1003", "1002");  // 操作系统 → 数据结构


    _courses.clear();
    for (auto& cou : testCourses) {
        auto coursePtr = std::make_shared<Course>(cou);
        // 加载先修课ID到缓存对象
        auto pres = getPrerequisites(cou.m_id);
        for (auto& pre : pres) {
            coursePtr->addPrerequisiteId(pre->m_id);
        }
        _courses.push_back(coursePtr);
    }

    std::print("课程数据初始化完成，成功插入{} 条数据\n", successCount);
}

bool CourseBroker::isCourseExists(const std::string& cid) {

    for (auto& cou : _courses) {
        if (cou->hasId(cid)) {
            return true;
        }
    }
}


std::shared_ptr<Course> CourseBroker::findById(const std::string& cid) {

        for (auto& cou : _courses) {
            if (cou->hasId(cid)) {
                std::print("查询到课程：{}\n", cid);
                return cou;
            }
        }


        std::print("[缓存未命中] 去数据库查询课程：{}\n", cid);
        pqxx::work tx(*m_conn);
        pqxx::result res = tx.exec_params(
                    "SELECT id, name, credit, teacher_id FROM Course WHERE id = {};",cid);
        tx.commit();

        if (res.empty()) {
            std::cerr << "未找到课程：" << cid << "\n";
            return nullptr;
        }


        auto row = res[0];
        auto course = std::make_shared<Course>(
                    row["id"].as<std::string>(),
                    row["name"].as<std::string>(),
                    row["credit"].as<double>(),
                    row["teacher_id"].as<std::string>()
                    );

        auto pres = getPrerequisites(cid);
        for (auto& pre : pres) {
            course->addPrerequisiteId(pre->m_id);
        }


        _courses.push_back(course);
        return course;
}
bool CourseBroker::isCourseExists(const std::string& cid) {

    for (auto& cou : _courses) {
        if (cou->hasId(cid)) return true;
    }

    pqxx::work tx(*m_conn);
    auto res = tx.exec_params("SELECT 1 FROM Course WHERE id = $1;", cid);
    tx.commit();
    return !res.empty();
}
bool CourseBroker::addCourse(const Course& course) {
    if (isCourseExists(course.m_id)) {
        std::print("课程{}已经存在，添加失败!\n", course.m_id);
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec_params(
                    "INSERT INTO Course(id, name, credit, teacher_id) VALUES ($1, $2, $3, $4);",
                    course.m_id,
                    course.m_name,
                    course.m_credit,
                    course.m_teacherId
                    );
        tx.commit();

        // 4. 加入缓存
        _courses.push_back(std::make_shared<Course>(course));
        std::print("课程{}添加成功\n", course.m_name);
        return true;
    } catch (const std::exception& e) {
        std::print("课程{}添加失败：{}\n", course.m_name, e.what());
        return false;
    }
}
bool CourseBroker::deleteCourse(const std::string& cid) {

    if (!isCourseExists(cid)) {
        std::print("课程 {} 不存在，删除失败\n", cid);
        return false;
    }


    pqxx::work txCheck(*m_conn);
    auto res = txCheck.exec_params("SELECT 1 FROM Enrollment WHERE course_id = $1;", cid);
    txCheck.commit();
    if (!res.empty()) {
        std::print("课程 {} 已有选课记录，禁止删除\n", cid);
        return false;
    }


    try {
        pqxx::work tx(*m_conn);
        tx.exec_params("DELETE FROM Course WHERE id = $1;", cid);
        tx.commit();

        // 4. 清理缓存
        for (auto it = _courses.begin(); it != _courses.end(); ++it) {
            if ((*it)->hasId(cid)) {
                _courses.erase(it);
                break;
            }
        }

        std::print("课程 {} 删除成功\n", cid);
        return true;
    } catch (const std::exception& e) {
        std::print("课程 {} 删除失败：{}\n", cid, e.what());
        return false;
    }
}


//教学秘书分配/更换秘书分配的教师必须有效
bool CourseBroker::updateCourseTeacher(const std::string& courseId, const std::string& newTeacherId) {
    // 1. 校验课程存在
    if (!isCourseExists(courseId)) {
        std::print("课程 {} 不存在，更换教师失败\n", courseId);
        return false;
    }

    // 2. 校验新教师存在（教学秘书分配的教师必须有效）
    if (!TeacherBroker::singleton().isTeacherExists(newTeacherId)) {
        std::print("教师 {} 不存在，课程 {} 更换教师失败\n", newTeacherId, courseId);
        return false;
    }

    // 3. 校验是否和原教师一致
    auto course = findById(courseId);
    if (course->m_teacherId == newTeacherId) {
        std::print("课程 {} 原教师已是 {}，无需更换\n", courseId, newTeacherId);
        return true;
    }

    // 4. 更新数据库
    try {
        pqxx::work tx(*m_conn);
        tx.exec_params(
                    "UPDATE Course SET teacher_id = $1 WHERE id = $2;",
                    newTeacherId, courseId
                    );
        tx.commit();

        // 5. 更新缓存（核心：保证缓存和数据库一致）
        for (auto& cou : _courses) {
            if (cou->hasId(courseId)) {
                cou->setTeacherId(newTeacherId);
                break;
            }
        }
        std::print("课程 {} 成功更换教师：{} → {}\n", courseId, course->m_teacherId, newTeacherId);
        return true;
    } catch (const std::exception& e) {
        std::print("课程 {} 更换教师失败：{}\n", courseId, e.what());
        return false;
    }
}

// 添加先修课关联
bool CourseBroker::addPrerequisite(const std::string& courseId, const std::string& preCourseId) {
    if (!isCourseExists(courseId) || !isCourseExists(preCourseId)) {
        std::print("课程 {} 或先修课 {} 不存在\n", courseId, preCourseId);
        return false;
    }
    if (courseId == preCourseId) {
        std::print("课程 {} 不能关联自身为先修课\n", courseId);
        return false;
    }

    // 校验关联已存在
    pqxx::work txCheck(*m_conn);
    auto res = txCheck.exec_params(
        "SELECT 1 FROM CoursePrerequisite WHERE course_id = $1 AND pre_course_id = $2;",
        courseId, preCourseId
    );
    txCheck.commit();
    if (!res.empty()) {
        std::print("课程 {} 已关联先修课 {}，无需重复添加\n", courseId, preCourseId);
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec_params(
            "INSERT INTO CoursePrerequisite(course_id, pre_course_id) VALUES ($1, $2);",
            courseId, preCourseId
        );
        tx.commit();

        for (auto& cou : _courses) {
            if (cou->hasId(courseId)) {
                cou->addPrerequisiteId(preCourseId);
                break;
            }
        }

        std::print("课程 {} 新增先修课 {} 成功\n", courseId, preCourseId);
        return true;
    } catch (const std::exception& e) {
        std::print("添加先修课关联失败：{}\n", e.what());
        return false;
    }
}
// 移除先修课关联
bool CourseBroker::removePrerequisite(const std::string& courseId, const std::string& preCourseId) {
    if (!isCourseExists(courseId) || !isCourseExists(preCourseId)) {
        std::print("课程 {} 或先修课 {} 不存在\n", courseId, preCourseId);
        return false;
    }

    // 校验关联存在
    pqxx::work txCheck(*m_conn);
    auto res = txCheck.exec_params(
        "SELECT 1 FROM CoursePrerequisite WHERE course_id = $1 AND pre_course_id = $2;",
        courseId, preCourseId
    );
    txCheck.commit();
    if (res.empty()) {
        std::print("课程 {} 未关联先修课 {}，移除失败\n", courseId, preCourseId);
        return false;
    }

    // 删除关联
    try {
        pqxx::work tx(*m_conn);
        tx.exec_params(
            "DELETE FROM CoursePrerequisite WHERE course_id = $1 AND pre_course_id = $2;",
            courseId, preCourseId
        );
        tx.commit();

        // 更新缓存
        for (auto& cou : _courses) {
            if (cou->hasId(courseId)) {
                auto& preIds = cou->m_prerequisiteIds;
                for (auto it = preIds.begin(); it != preIds.end(); ++it) {
                    if (*it == preCourseId) {
                        preIds.erase(it);
                        break;
                    }
                }
                break;
            }
        }

        std::print("课程 {} 移除先修课 {} 成功\n", courseId, preCourseId);
        return true;
    } catch (const std::exception& e) {
        std::print("移除先修课关联失败：{}\n", e.what());
        return false;
    }
}
// 获取课程的所有先修课
std::vector<std::shared_ptr<Course>> CourseBroker::getPrerequisites(const std::string& courseId) {
    std::vector<std::shared_ptr<Course>> pres;
    if (!isCourseExists(courseId)) return pres;

    pqxx::work tx(*m_conn);
    auto res = tx.exec_params(
        "SELECT pre_course_id FROM CoursePrerequisite WHERE course_id = $1;",
        courseId
    );
    tx.commit();

    for (auto& row : res) {
        std::string preId = row["pre_course_id"].as<std::string>();
        auto preCourse = findById(preId);
        if (preCourse) pres.push_back(preCourse);
    }

    return pres;
}
// 校验学生是否满足先修课条件
bool CourseBroker::checkPrerequisiteForStudent(const std::string& studentId, const std::string& courseId) {
    auto course = findById(courseId);
    if (!course) return false;

    // 无先修课直接通过
    auto pres = getPrerequisites(courseId);
    if (pres.empty()) return true;

    // 查询学生已修及格课程（成绩>0）
    pqxx::work tx(*m_conn);
    auto res = tx.exec_params(
        "SELECT course_id FROM Enrollment WHERE student_id = $1 AND score > 0;",
        studentId
    );
    tx.commit();

    // 整理已修课程ID
    std::vector<std::string> finished;
    for (auto& row : res) finished.push_back(row["course_id"].as<std::string>());

    // 校验所有先修课已修
    for (auto& pre : pres) {
        bool found = false;
        for (auto& id : finished) {
            if (pre->m_id == id) {
                found = true;
                break;
            }
        }
        if (!found) {
            std::print("学生 {} 未修先修课 {}，无法选课程 {}\n", studentId, pre->m_name, course->m_name);
            return false;
        }
    }

    return true;
}

