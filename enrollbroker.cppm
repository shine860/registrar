module;
#include "pqxx/pqxx"
export module registrar:dm.enrollmentbroker;

// 导入依赖模块
import std;
import :dm.base;
import registrar:domian.enrollment;

export class EnrollmentBroker : public RelationalBroker {
public:
    // 单例模式
    static EnrollmentBroker& singleton();
    void createTable() override;
    void initData() override;
    // 核心接口（选课/退课/查询）
    bool enrollcourse(const std::string& studentId, const std::string& courseId, const std::string& enrollTime);
    bool dropEnroll(const std::string& studentId, const std::string& courseId);
    std::vector<std::shared_ptr<Enrollment>> findByStudentId(const std::string& studentId);
    bool isEnrolled(const std::string& studentId, const std::string& courseId); // 保留（复合主键专用）
    bool isEnrollmentExistsInternal(const std::string& stuId, const std::string& couId);
    bool EnrollmentBroker::checkCourseCapacity(const std::string& couId);
    bool updateScore(const std::string& stuId, const std::string& couId, double newScore);

    std::vector<std::string> getEnrolledCourses(const std::string& stuId);
    std::vector<std::string> getEnrolledStudents(const std::string& couId);
    double getScore(const std::string& stuId, const std::string& couId);
private:

    EnrollmentBroker();

    EnrollmentBroker(const EnrollmentBroker&) = delete;
    EnrollmentBroker& operator=(const EnrollmentBroker&) = delete;


    std::vector<std::shared_ptr<Enrollment>> _enrollments;
};
EnrollmentBroker::EnrollmentBroker()
{}

EnrollmentBroker::EnrollmentBroker() {
    initConnection(); // 继承自RelationalBroker的数据库连接
}

EnrollmentBroker& EnrollmentBroker::singleton() {
    static EnrollmentBroker instance;
    return instance;
}

void EnrollmentBroker::createTable() {
    std::cout << "创建选课记录表（Enrollment）..." << std::endl;
    std::string sql = "CREATE TABLE IF NOT EXISTS Enrollment("
                      "student_id VARCHAR(20) NOT NULL,"
                      "course_id VARCHAR(20) NOT NULL,"
                      "score DOUBLE PRECISION DEFAULT -1.0 CHECK(score = -1.0 OR (score >= 0.0 AND score <= 100.0)),"
                      "enroll_time VARCHAR(20) NOT NULL,"
                      "PRIMARY KEY(student_id, course_id)," // 复合主键：一个学生一门课只能选一次
                      "FOREIGN KEY(student_id) REFERENCES Student(id) ON DELETE CASCADE,"
                      "FOREIGN KEY(course_id) REFERENCES Course(id) ON DELETE CASCADE);";
    try {
        query(sql);
        std::cout << "选课记录表创建成功" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "创建选课记录表失败：" << e.what() << std::endl;
    }
}


void EnrollmentBroker::initData() {
    // 清空数据库和缓存
    query("DELETE FROM Enrollment;");
    _enrollments.clear();

    std::cout << "初始化选课测试数据..." << std::endl;
    // 测试数据：学生1001选了课程1001、1002
    std::vector<Enrollment> testEnrollments = {
        {"1001", "1001", -1.0, "2026-01-01"}, // 未录入成绩
        {"1001", "1002", 85.5, "2026-01-01"}  // 已录入成绩
    };

    int success = 0;
    for (const auto& enr : testEnrollments) {
        if (isEnrollmentExistsInternal(enr.getStudentId(), enr.getCourseId())) continue;
        // 插入数据库
        try {
            pqxx::work tx(*m_conn);
            tx.exec_params(
                "INSERT INTO Enrollment(student_id, course_id, score, enroll_time) VALUES ($1, $2, $3, $4);",
                enr.getStudentId(), enr.getCourseId(), enr.getScore(), enr.getEnrollTime()
            );
            tx.commit();
            _enrollments.push_back(std::make_shared<Enrollment>(enr));
            success++;
        } catch (const std::exception& e) {
            std::cerr << "初始化选课记录失败：" << e.what() << std::endl;
        }
    }

    std::cout << "选课数据初始化完成，成功录入 " << success << " 条" << std::endl;
}

bool EnrollmentBroker::isEnrollmentExistsInternal(const std::string& stuId, const std::string& couId) {

    for (const auto& enr : _enrollments) {
        if (enr->isMatch(stuId, couId)) return true;
    }


    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec_params(
            "SELECT 1 FROM Enrollment WHERE student_id = $1 AND course_id = $2;",
            stuId, couId
        );
        tx.commit();
        return !res.empty();
    } catch (const std::exception& e) {
        std::cerr << "检查选课记录失败：" << e.what() << std::endl;
        return false;
    }
}


bool EnrollmentBroker::checkCourseCapacity(const std::string& couId) {
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec_params(
            "SELECT COUNT(*) FROM Enrollment WHERE course_id = $1;",
            couId
        );
        tx.commit();
        int count = res[0][0].as<int>();
        if (count >= 30) {
            std::cerr << "课程 " << couId << " 已选满（30人），无法选课" << std::endl;
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "校验课程容量失败：" << e.what() << std::endl;
        return false;
    }
}


bool EnrollmentBroker::enrollCourse(const std::string& stuId, const std::string& couId, const std::string& enrollTime) {

    Enrollment enrollment(stuId, couId, -1.0, enrollTime);
    if (!enrollment.isValid()) {
        std::cerr << "学生ID/课程ID为空，选课失败" << std::endl;
        return false;
    }


    auto& stuBroker = StudentBroker::singleton();
    auto& couBroker = CourseBroker::singleton();
    if (!stuBroker.isStudentExists(stuId)) {
        std::cerr << "学生ID " << stuId << " 不存在，选课失败" << std::endl;
        return false;
    }
    if (!couBroker.isCourseExists(couId)) {
        std::cerr << "课程ID " << couId << " 不存在，选课失败" << std::endl;
        return false;
    }


    if (isEnrollmentExistsInternal(stuId, couId)) {
        std::cerr << "学生 " << stuId << " 已选课程 " << couId << "，无需重复选课" << std::endl;
        return false;
    }


    if (!couBroker.checkPrerequisiteForStudent(stuId, couId)) {
        std::cerr << "学生 " << stuId << " 未满足课程 " << couId << " 的先修课条件，选课失败" << std::endl;
        return false;
    }


    if (!checkCourseCapacity(couId)) {
        return false;
    }


    try {
        pqxx::work tx(*m_conn);
        tx.exec_params(
            "INSERT INTO Enrollment(student_id, course_id, score, enroll_time) VALUES ($1, $2, $3, $4);",
            stuId, couId, -1.0, enrollTime
        );
        tx.commit();

        // 加入缓存
        _enrollments.push_back(std::make_shared<Enrollment>(enrollment));
        std::cout << "学生 " << stuId << " 选课 " << couId << " 成功" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "学生选课失败：" << e.what() << std::endl;
        return false;
    }
}


bool EnrollmentBroker::dropCourse(const std::string& stuId, const std::string& couId) {
    // 校验选课记录存在
    if (!isEnrollmentExistsInternal(stuId, couId)) {
        std::cerr << "学生 " << stuId << " 未选课程 " << couId << "，退课失败" << std::endl;
        return false;
    }

    // 校验是否已录入成绩（调用Domain层逻辑）
    double score = getScore(stuId, couId);
    Enrollment temp(stuId, couId, score);
    if (temp.hasScore()) {
        std::cerr << "课程 " << couId << " 已录入成绩（" << score << "），禁止退课" << std::endl;
        return false;
    }

    // 3. 删除数据库记录
    try {
        pqxx::work tx(*m_conn);
        tx.exec_params(
            "DELETE FROM Enrollment WHERE student_id = $1 AND course_id = $2;",
            stuId, couId
        );
        tx.commit();

        // 清理缓存
        for (auto it = _enrollments.begin(); it != _enrollments.end(); ++it) {
            if ((*it)->isMatch(stuId, couId)) {
                _enrollments.erase(it);
                break;
            }
        }

        std::cout << "学生 " << stuId << " 退课 " << couId << " 成功" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "学生退课失败：" << e.what() << std::endl;
        return false;
    }
}


bool EnrollmentBroker::updateScore(const std::string& stuId, const std::string& couId, double newScore) {

    Enrollment enrollment(stuId, couId, newScore);
    if (!enrollment.isScoreValid()) {
        return false; // Domain层已输出错误提示
    }

    if (!isEnrollmentExistsInternal(stuId, couId)) {
        std::cerr << "学生 " << stuId << " 未选课程 " << couId << "，无法录入成绩" << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec_params(
            "UPDATE Enrollment SET score = $1 WHERE student_id = $2 AND course_id = $3;",
            newScore, stuId, couId
        );
        tx.commit();

        // 更新缓存
        for (auto& enr : _enrollments) {
            if (enr->isMatch(stuId, couId)) {
                enr->setScore(newScore); // 调用Domain层的Setter
                break;
            }
        }

        std::cout << "学生 " << stuId << " 课程 " << couId << " 成绩更新为：" << newScore << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "更新成绩失败：" << e.what() << std::endl;
        return false;
    }
}

// 查询学生已选所有课程ID
std::vector<std::string> EnrollmentBroker::getEnrolledCourses(const std::string& stuId) {
    std::vector<std::string> courseIds;

    // 校验学生存在
    auto& stuBroker = StudentBroker::singleton();
    if (!stuBroker.isStudentExists(stuId)) {
        std::cerr << "学生ID " << stuId << " 不存在，查询选课失败" << std::endl;
        return courseIds;
    }

    // 查询数据库
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec_params(
            "SELECT course_id FROM Enrollment WHERE student_id = $1;",
            stuId
        );
        tx.commit();

        for (const auto& row : res) {
            courseIds.push_back(row["course_id"].as<std::string>());
        }
        std::cout << "学生 " << stuId << " 已选 " << courseIds.size() << " 门课程" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "查询学生选课失败：" << e.what() << std::endl;
    }

    return courseIds;
}

// 查询课程的所有选课学生ID
std::vector<std::string> EnrollmentBroker::getEnrolledStudents(const std::string& couId) {
    std::vector<std::string> studentIds;

    // 校验课程存在
    auto& couBroker = CourseBroker::singleton();
    if (!couBroker.isCourseExists(couId)) {
        std::cerr << "课程ID " << couId << " 不存在，查询选课名单失败" << std::endl;
        return studentIds;
    }

    // 查询数据库
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec_params(
            "SELECT student_id FROM Enrollment WHERE course_id = $1;",
            couId
        );
        tx.commit();

        for (const auto& row : res) {
            studentIds.push_back(row["student_id"].as<std::string>());
        }
        std::cout << "课程 " << couId << " 共有 " << studentIds.size() << " 名学生选课" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "查询课程选课名单失败：" << e.what() << std::endl;
    }

    return studentIds;
}

// 查询学生某门课程的成绩
double EnrollmentBroker::getScore(const std::string& stuId, const std::string& couId) {
    // 1. 查缓存
    for (const auto& enr : _enrollments) {
        if (enr->isMatch(stuId, couId)) {
            return enr->getScore();
        }
    }

    // 2. 查数据库
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec_params(
            "SELECT score FROM Enrollment WHERE student_id = $1 AND course_id = $2;",
            stuId, couId
        );
        tx.commit();

        if (res.empty()) {
            std::cerr << "学生 " << stuId << " 未选课程 " << couId << std::endl;
            return -2.0; // 用-2表示未选课（区别于-1未录入成绩）
        }

        double score = res[0]["score"].as<double>();
        // 加入缓存
        _enrollments.push_back(std::make_shared<Enrollment>(stuId, couId, score));
        return score;
    } catch (const std::exception& e) {
        std::cerr << "查询成绩失败：" << e.what() << std::endl;
        return -2.0;
    }
}


