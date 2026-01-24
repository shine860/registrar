//
// File: enrollment.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module registrar:domain.enrollment;
import std;
import :domain.grade;
using std::print;
using std::string;

export class Enrollment
{
friend class EnrollmentBroker;

public:
    Enrollment(string studentId, string courseId);
    bool hasTId(string sid,string cid);
    void enrollInGrade(class Grade grade);
private:
    string m_studentId;
    string m_courseId;
    class Student* m_student;
    class Course* m_course;
    std::vector<Grade> _grade_0_2;
};

Enrollment::Enrollment(string studentId, string courseId)
    : m_studentId(studentId), m_courseId(courseId)
{}


void Enrollment::enrollInGrade(Grade grade)
{
    if(_grade_0_2.size() >=2){
        print("error: 已经添加期末成绩+补考成绩，无法再添加！\n");
    }
    if(_grade_0_2.empty()){
        if(grade.score() >= 60)
        {
            _grade_0_2.push_back(grade);
            print("成绩及格，期末通过！\n");
        }else{
            _grade_0_2.push_back(grade);
            print("成绩不合格，需要参加补考！\n");
        }
        return;
    }
    if(_grade_0_2.size()==1){
        if(_grade_0_2[0].score() >=60){
            print("期末成绩已经合格，不需要补考!\n");
            return;
        }else{
            _grade_0_2.push_back(grade);
            if(grade.score() >= 60){
                print("补考及格，课程通过！\n");
            }else{
                print("补考不及格，课程为通过，需要重修！\n");
            }
        }
    }
}
