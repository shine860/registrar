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
private:
    double creditLimit;
    vector<string> enrolledCourses;
    vector<string> requiredCourses;
    string studentId;

public:
    Student(string id, string name, string gender, double creditLimit);
    void initCultivationPlan(const vector<string>& requiredCourses);
    void showRequiredCourses() const;
    bool isRequiredCourse(const string& courseId) const;
    bool enrollCourse(const string& courseId, double courseCredit);
    bool dropCourse(const string& courseId);
    double getCurrentCredits() const;
    bool authenticate(const std::string& password) override;
};

Student::Student(string id, string name, string gender, double creditLimit)
    : Person(std::move(id), std::move(name), std::move(gender)),
      creditLimit(creditLimit), studentId(std::move(id)) {}

void Student::initCultivationPlan(const vector<string>& requiredCourses)
{
    this->requiredCourses = requiredCourses;
}

void Student::showRequiredCourses() const
{
    print("培养计划（必修课程）：");
    if (this->requiredCourses.empty()) {
        print("无\n");
        return;
    }
    for (const auto& cid : this->requiredCourses) {
        print("{} ", cid);
    }
    print("\n");
}

bool Student::isRequiredCourse(const string& courseId) const
{
    auto it = find(this->requiredCourses.begin(), this->requiredCourses.end(), courseId);
    return it != this->requiredCourses.end();
}

bool Student::enrollCourse(const string& courseId, double courseCredit)
{
    if (this->getCurrentCredits() + courseCredit > this->creditLimit)
        return false;
    this->enrolledCourses.push_back(courseId);
    return true;
}

bool Student::dropCourse(const string& courseId)
{
    auto it = find(this->enrolledCourses.begin(), this->enrolledCourses.end(), courseId);
    if (it != this->enrolledCourses.end()) {
        this->enrolledCourses.erase(it);
        return true;
    }
    return false;
}

double Student::getCurrentCredits() const
{
    return this->enrolledCourses.size() * 3.0;
}

bool Student::authenticate(const std::string& password)
{
    return password == "student_" + this->studentId;
}
