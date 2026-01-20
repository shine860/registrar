export module registrar:ui;
import :domain.student;
import :domain.teacher;
import :domain.teachingsecretary;
import :dm.studentbroker;
import :dm.coursebroker;
import :dm.teacherbroker;
import :dm.teachsecretarybroker;


import std;
using std::string;
using std::print;
using std::vector;

export class EnrollInUi
{
public:
    EnrollInUi() = default;
    ~EnrollInUi() = default;
    int showMainMenu() const;
    bool studentLogIn();
    bool teacherLogIn();
    bool teachingsecretaryLogIn();
    void showStudents(const vector<std::shared_ptr<Student>>& students);
};

int EnrollInUi::showMainMenu() const
{
    int choice;
    std::cin >> choice;
    std::cout << "========================================\n";
    std::cout << "        学生选课管理系统 V1.0\n";
    std::cout << "========================================\n";
    std::cout << "1.学生登陆\n";
    std::cout << "2.教师登陆\n";
    std::cout << "3.教学秘书登陆\n";
    std::cout << "0. 退出系统\n";
    std::cout << "========================================\n";
    std::cout << "请输入操作序号：";
    while(choice < 0 || choice > 3){
        print("输入无效！请输入0-3之间的数字。");
        print("请重新输入: ");
    }
    return choice;
}
bool EnrollInUi::studentLogIn()
{
    string id, name;
    print("=== 学生登录 ===\n");
    print("请输入学号: ");
    std::cin >> id;
    print("请输入姓名: ");
    std::cin >> name;
    std::shared_ptr<Student> student = StudentBroker::singleton().findById(id);

    if (student && student->hasId(id)) {
        print("登录成功！欢迎，学生 {}\n", id);
        return true;
    }
    print("登录失败：学号或密码错误\n");
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

    std::shared_ptr<Teacher> teacher = TeacherBroker::singleton().findById(id);

    if (teacher && teacher->hasId(id)) {
        print("登录成功！欢迎，教师 {}\n", id);
        return true;
    }

    print("登录失败：工号或密码错误\n");
    return false;
}

bool EnrollInUi::teachingsecretaryLogIn()
{
    string id, name;
    print("=== 教务登录 ===\n");
    print("请输入教务工号: ");
    std::cin >> id;
    print("请输入姓名: ");
    std::cin >> name;

   std::shared_ptr<TeachingSecretary> secretary =TeachingSecretaryBroker::singleton().findById(id);

    if (secretary) {
        print("登录成功！欢迎，教务 {}\n", id);
        return true;
    }

    print("登录失败：工号或密码错误\n");
    return false;
}

