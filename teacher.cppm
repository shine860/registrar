//
// File: teacher.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.teacher;
import domain.person;
import std;
using std::print;
using std::string;
using std::vector;

export class Teacher : public Person
{
private:
    int maxTeachingHours;
    vector<string> assignedCourses;
    string teacherId;

public:
    Teacher(string id, string name, string gender, int maxTeachingHours);
    bool assignCourse(const string& courseId, int courseHours);
    bool submitGrade(const string& studentId, const string& courseId, double grade);
    int getCurrentTeachingHours() const;
    bool authenticate(const std::string& password) override;
};

Teacher::Teacher(string id, string name, string gender, int maxTeachingHours)
    : Person(std::move(id), std::move(name), std::move(gender)),
      maxTeachingHours(maxTeachingHours), teacherId(std::move(id)) {}

bool Teacher::assignCourse(const string& courseId, int courseHours)
{
    if (this->getCurrentTeachingHours() + courseHours > this->maxTeachingHours)
        return false;
    this->assignedCourses.push_back(courseId);
    return true;
}

bool Teacher::submitGrade(const string& studentId, const string& courseId, double grade)
{
    return true;
}

int Teacher::getCurrentTeachingHours() const
{
    return this->assignedCourses.size() * 3;
}

bool Teacher::authenticate(const std::string& password)
{
    return password == "teacher_" + this->teacherId;
}
