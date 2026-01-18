#pragma once
#include "relationalbroker.h"
#include <vector>
#include <string>
#include <pqxx/pqxx>
import registrar:domian.enrollment
class EnrollmentBroker:public RelationalBroker
{
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
    // 私有构造
    EnrollmentBroker() = default;
    // 禁用拷贝/赋值
    EnrollmentBroker(const EnrollmentBroker&) = delete;
    EnrollmentBroker& operator=(const EnrollmentBroker&) = delete;

    // 内存缓存
    std::vector<std::shared_ptr<Enrollment>> _enrollments;
};
};

