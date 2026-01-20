module;
#include "pqxx/pqxx"
export module registrar:dm.enrollmentbroker;

// 导入依赖模块
import std;
import :dm.base;
import :domain.enrollment;


export class EnrollmentBroker : public RelationalBroker {
public:
    static EnrollmentBroker& singleton();
    void createTable() override;
    void initData() override;

    // 1. 选课
    bool enroll(const std::string& sid, const std::string& cid, const std::string& time);

    // 2. 退课
    bool drop(const std::string& sid, const std::string& cid);

    // 3. 录入/更新成绩
    bool updateScore(const std::string& sid, const std::string& cid, double score);

    // === 查询功能 ===

    // 获取某学生选的所有课程ID
    std::vector<std::string> getCourseIdsByStudent(const std::string& sid);

    // 获取某课程下的所有学生ID
    std::vector<std::string> getStudentIdsByCourse(const std::string& cid);

    // 获取某学生某课程的成绩
    double getScore(const std::string& sid, const std::string& cid);

private:
    EnrollmentBroker();
    EnrollmentBroker(const EnrollmentBroker&) = delete;
    EnrollmentBroker& operator=(const EnrollmentBroker&) = delete;

    // 检查选课记录是否已存在
    bool isEnrollmentExists(const std::string& sid, const std::string& cid);
};

EnrollmentBroker::EnrollmentBroker() { initConnection(); }

EnrollmentBroker& EnrollmentBroker::singleton() {
    static EnrollmentBroker instance;
    return instance;
}

void EnrollmentBroker::createTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS Enrollment (
            student_id VARCHAR(20) NOT NULL,
            course_id VARCHAR(20) NOT NULL,
            score DOUBLE PRECISION DEFAULT -1.0,
            enroll_time VARCHAR(50) NOT NULL,
            PRIMARY KEY(student_id, course_id)
        )
    )";
    query(sql);
}

void EnrollmentBroker::initData() {
    query("DELETE FROM Enrollment;");

    try {
        pqxx::work tx(*m_conn);
        tx.exec(
            "INSERT INTO Enrollment(student_id, course_id, score, enroll_time) VALUES($1, $2, -1.0, $3)",
            pqxx::params{"2018001", "1001", "2023-09-01"}
        );
        tx.commit();
        std::cout << "Enrollment 数据初始化完成。" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "初始化失败: " << e.what() << std::endl;
    }
}

bool EnrollmentBroker::isEnrollmentExists(const std::string& sid, const std::string& cid) {
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec(
            "SELECT 1 FROM Enrollment WHERE student_id=$1 AND course_id=$2",
            pqxx::params{sid, cid}
        );
        tx.commit();
        return !res.empty();
    } catch (...) {
        return false;
    }
}


bool EnrollmentBroker::enroll(const std::string& sid, const std::string& cid, const std::string& time) {
    if (isEnrollmentExists(sid, cid)) {
        std::cout << "选课失败：已选该课程" << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec(
            "INSERT INTO Enrollment(student_id, course_id, score, enroll_time) VALUES($1, $2, -1.0, $3)",
            pqxx::params{sid, cid, time}
        );
        tx.commit();
        std::cout << ">> 选课成功" << std::endl;
        return true;
    } catch (const pqxx::unique_violation&) {
        std::cout << ">> 选课失败：数据库冲突" << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << ">> 选课失败: " << e.what() << std::endl;
        return false;
    }
}

bool EnrollmentBroker::drop(const std::string& sid, const std::string& cid) {
    double currentScore = getScore(sid, cid);
    if (currentScore >= 0) { // -1 是未录入，>=0 是有成绩
        std::cout << ">> 退课失败：已有成绩" << std::endl;
        return false;
    }

    // 2. 删除记录
    try {
        pqxx::work tx(*m_conn);
        tx.exec(
            "DELETE FROM Enrollment WHERE student_id=$1 AND course_id=$2",
            pqxx::params{sid, cid}
        );
        tx.commit();
        std::cout << ">> 退课成功" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << ">> 退课失败: " << e.what() << std::endl;
        return false;
    }
}

bool EnrollmentBroker::updateScore(const std::string& sid, const std::string& cid, double score) {
    if (score < 0 || score > 100) {
        std::cout << ">> 录入失败：分数必须在 0-100 之间" << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec(
            "UPDATE Enrollment SET score=$1 WHERE student_id=$2 AND course_id=$3",
            pqxx::params{score, sid, cid}
        );
        tx.commit();

        if (res.affected_rows() > 0) {
            std::cout << ">> 成绩录入成功：" << score << std::endl;
            return true;
        } else {
            std::cout << ">> 录入失败：未找到选课记录" << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << ">> 数据库错误: " << e.what() << std::endl;
        return false;
    }
}

// --- 查询功能 ---

std::vector<std::string> EnrollmentBroker::getCourseIdsByStudent(const std::string& sid) {
    std::vector<std::string> cids;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec(
            "SELECT course_id FROM Enrollment WHERE student_id=$1",
            pqxx::params{sid}
        );
        tx.commit();

        // === 注意：使用 const auto& ===
        for (const auto& r : res) {
            cids.push_back(r["course_id"].as<std::string>());
        }
    } catch (...) {}
    return cids;
}

std::vector<std::string> EnrollmentBroker::getStudentIdsByCourse(const std::string& cid) {
    std::vector<std::string> sids;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec(
            "SELECT student_id FROM Enrollment WHERE course_id=$1",
            pqxx::params{cid}
        );
        tx.commit();
        for (const auto& r : res) {
            sids.push_back(r["student_id"].as<std::string>());
        }
    } catch (...) {}
    return sids;
}

double EnrollmentBroker::getScore(const std::string& sid, const std::string& cid) {
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec(
            "SELECT score FROM Enrollment WHERE student_id=$1 AND course_id=$2",
            pqxx::params{sid, cid}
        );
        tx.commit();

        if (!res.empty()) {
            return res[0]["score"].as<double>();
        }
    } catch (...) {}
    return -2.0; // -2 表示未找到记录
}
