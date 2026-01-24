// Module
// File: enrollbroker.cppm   Version: 0.1.0   License: AGPLv3
// Created: 苏茜（2024051604029）   3236863614@qq.com   2026-01-24 21:34:00
// Description:选课记录的代管者，继承基类
module;
#include "pqxx/pqxx"
export module registrar:dm.enrollmentbroker;

// 导入依赖模块
import std;
import :dm.base;
import :domain.enrollment;
using std::print;
using std::string;


export class EnrollmentBroker : public RelationalBroker {
public:
    static EnrollmentBroker& singleton();
    void createTable() override;
    void initData() override;

    //选课
    bool enroll(std::string sid, std::string cid, const std::string& time);
    //退课
    bool drop(std::string& sid,  std::string& cid);

    //录入/更新成绩
    bool updateScore( std::string& sid, std::string& cid, double score);

    // === 查询功能 ===

    // 获取某学生选的所有课程ID
    std::vector<std::string> getCourseIdsByStudent( std::string& sid);

    // 获取某课程下的所有学生ID
    std::vector<std::string> getStudentIdsByCourse(std::string& cid);

    // 获取某学生某课程的成绩
    double getScore(std::string& sid, std::string& cid);
    //去掉字符串首尾的“看不见”的空格、回车符或制表符
    std::string trim(std::string &s);

private:
    EnrollmentBroker();
    EnrollmentBroker( EnrollmentBroker&) = delete;
    EnrollmentBroker& operator=( EnrollmentBroker&) = delete;
    std::vector<std::shared_ptr<Enrollment>> _enrollments;

    // 检查选课记录是否已存在
    bool isEnrollmentExists( std::string& sid,  std::string& cid);
};
//构造函数
EnrollmentBroker::EnrollmentBroker()
{
    initConnection();
}
//单例的创建
EnrollmentBroker& EnrollmentBroker::singleton() {
    static EnrollmentBroker instance;
    return instance;
}
//创建数据表
void EnrollmentBroker::createTable() {
    // query("DROP TABLE IF EXISTS Enrollment;");
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS Enrollment (
            student_id VARCHAR(20) NOT NULL,
            course_id VARCHAR(20) NOT NULL,
            score NUMERIC(5,2) DEFAULT -1.0,
            enroll_time VARCHAR(50) NOT NULL,
            PRIMARY KEY(student_id, course_id)
        )
    )";
    query(sql);
}
//初始化数据表中的数据
void EnrollmentBroker::initData()
{
    //没有选课记录就是空的，选了课才有对应的记录
    _enrollments.clear();
    print("Enrollment table ready,no preset data.\n");
}
//判断选课记录是否存在
bool EnrollmentBroker::isEnrollmentExists (std::string& sid, std::string& cid) {
    sid = trim(sid);
    cid = trim(cid);
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT 1 FROM Enrollment WHERE student_id=$1 AND course_id=$2",
            pqxx::params{sid, cid});
        tx.commit();
        return !res.empty();
    } catch (...) {
        return false;
    }
}
//选课
bool EnrollmentBroker::enroll(std::string sid, std::string cid, const std::string& time) {
    sid = trim(sid);
    cid = trim(cid);

    if (isEnrollmentExists(sid, cid)) {
        //std::cout << ">> 选课失败：已选该课程" << std::endl;
        print("选课失败：已经选该课程\n");
        return false;
    }

    try {
        pqxx::work tx(*m_conn);

        //获取所有先修课
        auto prereqRes = tx.exec("SELECT pre_course_id FROM CoursePrerequisite WHERE course_id = $1",
            pqxx::params{cid});

        // 只要选过即可
        for (const auto& r : prereqRes) {
            string preId = r["pre_course_id"].as<std::string>();

            // 查询是否选过
            auto preRes = tx.exec("SELECT 1 FROM Enrollment WHERE student_id=$1 AND course_id=$2",
                pqxx::params{sid, preId});

            if (preRes.empty()) {
                // std::cout << ">> 选课失败：必须先修完课程 ID: " << preId << "（未发现选课记录）" << std::endl;
                print("选课失败：必须先修完课程 ID：{} （未发现选课记录)\n",preId);
                return false;
            }
        }
        tx.commit(); // 提交查询事务

        // 正式选课
        pqxx::work tx2(*m_conn);
        tx2.exec("INSERT INTO Enrollment(student_id, course_id, score, enroll_time) VALUES($1, $2, -1.0, $3)",
            pqxx::params{sid, cid, time});
        tx2.commit();
        //std::cout << "选课成功" << std::endl;
        print("选课成功\n");
        return true;

    } catch (const pqxx::unique_violation&) {
        // std::cout << "  选课失败：数据库冲突" << std::endl;
        print("  选课失败：数据库冲突\n");
        return false;
    } catch (const std::exception& e) {
        std::cerr << "  选课失败: " << e.what() << std::endl;
        return false;
    }
}

//退课
bool EnrollmentBroker::drop(std::string& sid,  std::string& cid) {
    sid = trim(sid);
    cid = trim(cid);

    double currentScore = getScore(sid, cid);
    if (currentScore >= 0) { // -1 是未录入，>=0 是有成绩
        // std::cout << " 退课失败：已有成绩" << std::endl;
        print("退课失败：已有成绩\n");
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec("DELETE FROM Enrollment WHERE student_id=$1 AND course_id=$2",
            pqxx::params{sid, cid});
        tx.commit();
        // std::cout << " 退课成功" << std::endl;
        print("退课成功\n");
        return true;
    } catch (const std::exception& e) {
        // std::cerr << " 退课失败: " << e.what() << std::endl;
        print("退课失败\n");
        return false;
    }
}
//更新/录入分数
bool EnrollmentBroker::updateScore(std::string& sid, std::string& cid, double score) {
    if (score < 0 || score > 100) {
        // std::cout << " 录入失败：分数必须在 0-100 之间" << std::endl;
        print("录入失败：分数必须在0-100之间\n");
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("UPDATE Enrollment SET score=$1 WHERE student_id=$2 AND course_id=$3",
            pqxx::params{std::to_string(score), sid, cid});
        tx.commit();

        if (res.affected_rows() > 0) {
            // std::cout << " 成绩录入成功：" << score << std::endl;
            print("成绩录入成功： {}\n",score);
            return true;
        } else {
            // std::cout << " 录入失败：未找到选课记录" << std::endl;
            print("录入失败：未找到选课记录\n");
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "数据库错误: " << e.what() << std::endl;
        return false;
    }
}

// --- 查询功能 ---

std::vector<std::string> EnrollmentBroker::getCourseIdsByStudent( std::string& sid) {
    std::vector<std::string> cids;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec(
            "SELECT course_id FROM Enrollment WHERE student_id=$1",
            pqxx::params{sid});
        tx.commit();

        for (const auto& r : res) {
            cids.push_back(r["course_id"].as<std::string>());
        }
    } catch (...) {
        print("未找到相关记录");
    }
    return cids;
}

std::vector<std::string> EnrollmentBroker::getStudentIdsByCourse(std::string& cid) {
    std::vector<std::string> sids;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT student_id FROM Enrollment WHERE course_id=$1",
            pqxx::params{cid});
        tx.commit();
        for (const auto& r : res) {
            sids.push_back(r["student_id"].as<std::string>());
        }
    } catch (...) {
        print("未找到相关记录\n");
    }
    return sids;
}

double EnrollmentBroker::getScore(std::string& sid, std::string& cid) {
    sid = trim(sid);
    cid = trim(cid);

    pqxx::work tx(*m_conn);
    auto res = tx.exec("SELECT score FROM Enrollment WHERE student_id=$1 AND course_id=$2",
                pqxx::params{sid, cid});
    tx.commit();

    if (!res.empty()) {
        return res[0]["score"].as<double>();
    }

    return -2.0; // -2 表示未找到记录
}
//将字符窜首尾的所有空白字符切掉
std::string EnrollmentBroker::trim( std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}
