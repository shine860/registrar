// Module
// File: ui.cppm   Version: 0.1.0   License: AGPLv3
// Created: 张雨欣2024051604045   3357714096@qq.com   2026-01-20 14:36:04
// Description:
//
export module registrar:ui;
import registrar:domain.student;
import registrar:domain.teacher;
import registrar:domain.teachingsecretary;
import std;
using std::string;
using std::print;
using std::cin;
using std::cout;
export class EnrollInUi
{
public:
    EnrollInUi() = default;
    ~EnrollInUi() = default;
    int showMainMenu() const;
    bool studentLogIn();
    bool teacherLogIn();
    bool teachingSecretaryLogIn(); // 修正命名以匹配registrar.cppm中的调用
};
int EnrollInUi::showMainMenu() const
{
    int choice;
    // 先打印菜单，再获取输入
    cout << "========================================\n";
    cout << "        学生选课管理系统 V1.0\n";
    cout << "========================================\n";
    cout << "1.学生登陆\n";
    cout << "2.教师登陆\n";
    cout << "3.教学秘书登陆\n";
    cout << "0. 退出系统\n";
    cout << "========================================\n";
    do {
        cout << "请输入操作序号：";
        cin >> choice;
        if (choice < 0 || choice > 3) {
            print("输入无效！请输入0-3之间的数字。\n");
        }
    } while (choice < 0 || choice > 3);
    return choice;
}
bool EnrollInUi::studentLogIn()
{
    string id, name;
    print("=== 学生登录 ===\n");
    print("请输入学号: ");
    cin >> id;
    print("请输入姓名: ");
    cin >> name;
    auto& broker = StudentBroker::singleton();
    Student* student = broker.findById(id);
    // 增加对姓名的校验，假设姓名作为密码
    if (student && student->m_name == name) {
        print("登录成功！欢迎，学生 {}\n", id);
        return true;
    }
    print("登录失败：学号或姓名错误\n");
    return false;
}
bool EnrollInUi::teacherLogIn()
{
    string id, name;
    print("=== 教师登录 ===\n");
    print("请输入教师工号: ");
    cin >> id;
    print("请输入姓名: ");
    cin >> name;
    auto& broker = TeacherBroker::singleton();
    Teacher* teacher = broker.findById(id);
    if (teacher && teacher->m_name == name) {
        print("登录成功！欢迎，教师 {}\n", id);
        return true;
    }
    print("登录失败：工号或姓名错误\n");
    return false;
}
bool EnrollInUi::teachingSecretaryLogIn()
{
    string id, name;
    print("=== 教务登录 ===\n");
    print("请输入教务工号: ");
    cin >> id;
    print("请输入姓名: ");
    cin >> name;
    auto& broker = SecretaryBroker::singleton();
    TeachingSecretary* secretary = broker.findById(id);
    if (secretary && secretary->m_name == name) {
        print("登录成功！欢迎，教务 {}\n", id);
        return true;
    }
    print("登录失败：工号或姓名错误\n");
    return false;
}
