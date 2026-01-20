//
// File: student.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module registrar:domain.student;
import :domain.person;
import :domain.enrollment;

import std;
using std::print;
using std::string;
using std::vector;
using std::shared_ptr;



// class StudentBroker;

class Course; // 前向声明

export class Student : public Person {
friend class StudentBroker;
private:
    int m_grade;
    std::string m_major;
    std::string m_studentclass;
    double m_creditLimit;
    std::vector<class Enrollment*> _enrollments;
    std::vector<class Course*> _courses;

public:
    Student(std::string id, std::string name, std::string gender, std::string dept,
             int grade, std::string major, std::string studentclass, double creditLimit);
    void enrollsIn(class Course* course);
    std::string schedule();
    bool dropCourse(string courseId, double courseCredit);
};

Student::Student(std::string id, std::string name, std::string gender, std::string dept,
                 int grade, std::string major, std::string studentclass, double creditLimit)
    : Person(std::move(id), std::move(name), std::move(gender), std::move(dept)),
      m_grade(grade), m_major(std::move(major)), m_studentclass(std::move(studentclass)), m_creditLimit(creditLimit) {}
