//
// File: student.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.student;
import domain.person;
import std;

using std::print;
using std::string;
using std::vector;
using std::find;

export class Student : public Person
{
protected:
    double m_creditLimit;
    vector<string> m_enrolledCourses;
    vector<string> m_requiredCourses;
    string m_studentId;
    double m_enrolledCredits = 0.0;
public:
    Student(string id, string name, string gender, double creditLimit);
    void initCultivationPlan(const vector<string>& requiredCourses);
    void showRequiredCourses() const;
    bool isRequiredCourse(const string& courseId) const;
    bool enrollCourse(const string& courseId, double courseCredit);
    bool dropCourse(const string& courseId, double courseCredit);
};

Student::Student(string id, string name, string gender, double creditLimit)
    : Person(std::move(id), std::move(name), std::move(gender)),
      m_creditLimit(creditLimit), m_studentId(std::move(id)) {}

void Student::initCultivationPlan(const vector<string>& requiredCourses)
{
    m_requiredCourses = requiredCourses;
}

void Student::showRequiredCourses() const
{
    print("培养计划（必修课程）：");
    if (m_requiredCourses.empty())
    {
        print("无\n");
        return;
    }
    for (const auto& cid : m_requiredCourses)
        print("{} ", cid);
    print("\n");
}

bool Student::isRequiredCourse(const string& courseId) const
{
    auto it = find(m_requiredCourses.begin(), m_requiredCourses.end(), courseId);
    return it != m_requiredCourses.end();
}

bool Student::enrollCourse(const string& courseId, double courseCredit)
{
    if (m_enrolledCredits + courseCredit > m_creditLimit)
    {
        print("学生 {} 选课失败：学分超限！当前{:.1f}/上限{:.1f}\n", m_name, m_enrolledCredits, m_creditLimit);
        return false;
    }
    m_enrolledCourses.push_back(courseId);
    m_enrolledCredits += courseCredit;
    print("学生 {} 成功选修课程 {}（学分：{:.1f}）→ 当前总学分：{:.1f}\n", m_name, courseId, courseCredit, m_enrolledCredits);
    return true;
}

bool Student::dropCourse(const string& courseId, double courseCredit)
{
    auto it = find(m_enrolledCourses.begin(), m_enrolledCourses.end(), courseId);
    if (it == m_enrolledCourses.end())
    {
        print("学生 {} 退课失败：未选该课程\n", m_name);
        return false;
    }
    m_enrolledCourses.erase(it);
    m_enrolledCredits -= courseCredit;
    print("学生 {} 成功退选课程 {}（学分：{:.1f}）→ 当前总学分：{:.1f}\n", m_name, courseId, courseCredit, m_enrolledCredits);
    return true;
}
