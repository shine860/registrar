//
// File: enrollment.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.enrollment;
import std;
using std::print;
using std::string;

export class Enrollment
{
private:
    string studentId;
    string courseId;

public:
    Enrollment(string studentId, string courseId);
    bool hasId(const string& sid, const string& cid) const;
};

Enrollment::Enrollment(string studentId, string courseId)
    : studentId(std::move(studentId)), courseId(std::move(courseId)) {}

bool Enrollment::hasId(const string& sid, const string& cid) const
{
    return this->studentId == sid && this->courseId == cid;
}
