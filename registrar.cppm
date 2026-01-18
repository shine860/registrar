// Module
// File: registrar.cppm   Version: 0.1.0   License: AGPLv3
// Created: 张雨欣2024051604045   3357714096@qq.com   2026-01-17 16:01:51
// Description:
//
export module control.registrar;
import domain.student;
import domain.course;
import domain.enrollment;

import std;
using std::string;
using std::vector;
using std::print;

export class Registrar
{
private:
    static Registrar* instance;

    Registrar();
    ~Registrar() = default;

public:
    static Registrar& getInstance();
    Registrar(const Registrar&) = delete;
    Registrar& operator=(const Registrar&) = delete;

    bool studentEnrollCourse(const string& studentId, const string& courseId);
    vector<Enrollment*> courseRoster(const string& courseId);
    bool removeStudentById(const string& studentId);
};

Registrar* Registrar::instance = nullptr;

Registrar::Registrar() {}

Registrar& Registrar::getInstance()
{
    if (!instance) {
        instance = new Registrar();
    }
    return *instance;
}

bool Registrar::studentEnrollCourse(const string& studentId, const string& courseId)
{
    auto& studentBroker = StudentBroker::singleton();
    auto& courseBroker = CourseBroker::singleton();
    auto& enrollBroker = EnrollBroker::singleton();

    Student* student = studentBroker.findById(studentId);
    if (!student) {
        print("Student not found: {}\n", studentId);
        return false;
    }

    Course* course = courseBroker.findById(courseId);
    if (!course) {
        print("Course not found: {}\n", courseId);
        return false;
    }

    if (!student->enrollCourse(courseId, 3.0)) {
        print("Failed to enroll student {} in course {}\n", studentId, courseId);
        return false;
    }

    if (!course->acceptEnrollment(studentId)) {
        print("Course {} rejected enrollment for student {}\n", courseId, studentId);
        return false;
    }

    Enrollment* enrollment = new Enrollment(studentId, courseId);
    enrollBroker.insert(enrollment);

    print("Student {} successfully enrolled in course {}\n", studentId, courseId);
    return true;
}

vector<Enrollment*> Registrar::courseRoster(const string& courseId)
{
    auto& enrollBroker = EnrollBroker::singleton();
    vector<Enrollment*> allEnrollments = enrollBroker.query("");
    vector<Enrollment*> result;

    for (auto* enrollment : allEnrollments) {
        if (enrollment->hasId("", courseId)) {
            result.push_back(enrollment);
        }
    }

    return result;
}

bool Registrar::removeStudentById(const string& studentId)
{
    auto& studentBroker = StudentBroker::singleton();
    return studentBroker.drop(studentId);
}
