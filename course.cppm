//
// File: course.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.course;
import std;
using std::string;
using std::vector;

export class Course
{
protected:
    string m_name;
    string m_id;
    double m_credit;
    string m_syllabus;
    vector<string> _enrolledStudents;
public:
    Course(string name, string id, double credit, string syllabus);
    bool acceptEnrollment(const string& studentId);
    string roster() const;
};

Course::Course(string name, string id, double credit, string syllabus)
    : m_name(std::move(name)), m_id(std::move(id)), m_credit(credit), m_syllabus(std::move(syllabus)) {}

bool Course::acceptEnrollment(const string& studentId)
{
    this->_enrolledStudents.push_back(studentId);
    return true;
}

string Course::roster() const
{
    string result = "选课学生：";
    for (const auto& sid : this->_enrolledStudents)
        result += sid + ", ";
    return result.empty() ? "暂无学生选课" : result;
}
