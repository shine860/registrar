#pragma once
#include "relationalbroker.h"
#include <memory>
#include <vector>
#include <print>
import registrar:domian.student;
using std::vector;
using std::print;

import registrar:domian.student;

class StudentBroker:public RelationalBroker
{
public:
    static StudentBroker& singleton();
    std::shared_ptr<class Student> findById(const string& sid);
    // 1. 添加学生（调用基类insert方法，直接拼接SQL）
    void createTable() override;
    void initData() override;
    bool addStudent(std::shared_ptr<class Student>& student);
    bool deleteStudent(const string& sid);
    bool isStudentExists(const string& sid);
private:
    StudentBroker();
    StudentBroker(const StudentBroker&) = delete;
    StudentBroker& operator=(const StudentBroker&) = delete;
    std::vector<std::shared_ptr<class Student>> _students;
};

