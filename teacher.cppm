//
// File: teacher.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module registrar:domain.teacher;
import :domain.person;
import std;
using std::print;
using std::string;
using std::vector;

export class Teacher : public Person
{
friend class TeacherBroker;
friend class TeachingSecretaryBroker;
private:
    string m_title;//职称
    vector<class Course*> _teachingCourses;//教授的课程

public:
    Teacher(string id, string name, string gender, string dept,string title);
    bool assignCourse(class Course *course);
    void submitGrade(const string studentId, const std::string courseId, double midterm, double final, const std::vector<double>& homeworks);
    void showTeachCourse();
};

Teacher::Teacher(string id, string name, string gender, string dept,string title)
    : Person(id, name, gender,dept),m_title(title)
{}





