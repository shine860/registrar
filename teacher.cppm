//
// File: teacher.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.teacher;
import std;
import domain.person;
import domain.course;

using std::print;
using std::string;
using std::vector;

export class Teacher : public Person
{
private:
    int m_maxTeachingHours;
    vector<string> _assignedCourses;
    string m_teacherId;
public:
    Teacher(string id, string name, string gender, int maxTeachingHours);
    bool assignCourse(Course* course);
    bool submitGrade();
};

Teacher::Teacher(string id, string name, string gender, int maxTeachingHours)
    : Person(std::move(id), std::move(name), std::move(gender)),
      m_maxTeachingHours(maxTeachingHours), m_teacherId(std::move(id))
{}

bool Teacher::assignCourse(Course* course)
{
    this->_assignedCourses.push_back("courseId");
    print("教师 {} 已接收课程安排", this->m_name);
    return true;
}

bool Teacher::submitGrade()
{
    print("教师 {} 提交成绩成功", this->m_name);
    return true;
}
