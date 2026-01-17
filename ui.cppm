// Module
// File: ui.cppm   Version: 0.1.0   License: AGPLv3
// Created: 张雨欣2024051604045   3357714096@qq.com   2026-01-17 16:00:42
// Description:
//
export module ui.ui;
import domain.student;
import domain.teacher;
import domain.teachingsecretary;
import dm.studentbroker;
import dm.teacherbroker;
import dm.teachingsecretarybroker;
import std;
using std::string;
using std::print;
using std::getline;

export class EnrollInUi
{
public:
    EnrollInUi() = default;
    ~EnrollInUi() = default;

    bool studentLogIn();
    bool teacherLogIn();
    bool teachingsecretaryLogIn();
    void studentSchedule(const string& studentId);
    void gradeReport(const string& studentId);
    void operation();
};

bool EnrollInUi::studentLogIn()
{
    string id, password;
    print("=== 学生登录 ===\n");
    print("请输入学号: ");
    getline(std::cin, id);
    print("请输入密码: ");
    getline(std::cin, password);

    auto& broker = StudentBroker::singleton();
    Student* student = broker.findById(id);

    if (student && student->authenticate(password)) {
        print("登录成功！欢迎，学生 {}\n", id);
        return true;
    }

    print("登录失败：学号或密码错误\n");
    return false;
}

bool EnrollInUi::teacherLogIn()
{
    string id, password;
    print("=== 教师登录 ===\n");
    print("请输入教师工号: ");
    getline(std::cin, id);
    print("请输入密码: ");
    getline(std::cin, password);

    auto& broker = TeacherBroker::singleton();
    Teacher* teacher = broker.findById(id);

    if (teacher && teacher->authenticate(password)) {
        print("登录成功！欢迎，教师 {}\n", id);
        return true;
    }

    print("登录失败：工号或密码错误\n");
    return false;
}

bool EnrollInUi::teachingsecretaryLogIn()
{
    string id, password;
    print("=== 教务登录 ===\n");
    print("请输入教务工号: ");
    getline(std::cin, id);
    print("请输入密码: ");
    getline(std::cin, password);

    auto& broker = TeachingSecretaryBroker::singleton();
    TeachingSecretary* secretary = broker.findById(id);

    if (secretary && secretary->authenticate(password)) {
        print("登录成功！欢迎，教务 {}\n", id);
        return true;
    }

    print("登录失败：工号或密码错误\n");
    return false;
}

void EnrollInUi::studentSchedule(const string& studentId)
{
    print("=== 学生课程表 ===\n");
    print("学生 {} 的课程安排：\n", studentId);
    print("（课程表功能待实现）\n");
}

void EnrollInUi::gradeReport(const string& studentId)
{
    print("=== 学生成绩单 ===\n");
    print("学生 {} 的成绩：\n", studentId);
    print("（成绩单功能待实现）\n");
}

void EnrollInUi::operation()
{
    while (true) {
        print("\n========================================\n");
        print("       教务管理系统 - 主菜单\n");
        print("========================================\n");
        print("1. 学生登录\n");
        print("2. 教师登录\n");
        print("3. 教务登录\n");
        print("0. 退出系统\n");
        print("========================================\n");
        print("请选择操作 (0-3): ");

        string choice;
        getline(std::cin, choice);

        if (choice == "0") {
            print("感谢使用，再见！\n");
            break;
        } else if (choice == "1") {
            if (studentLogIn()) {
                print("进入学生功能模块...\n");
            }
        } else if (choice == "2") {
            if (teacherLogIn()) {
                print("进入教师功能模块...\n");
            }
        } else if (choice == "3") {
            if (teachingsecretaryLogIn()) {
                print("进入教务功能模块...\n");
            }
        } else {
            print("无效的选择，请重新输入\n");
        }
    }
}
