// Module
// File: registrar.cppm   Version: 0.1.0   License: AGPLv3
// Created: 张雨欣2024051604045   3357714096@qq.com   2026-01-19 18:34:28
// Description:
//
export module registrar;
export import :ui;
export import :domain;
export import :dm;

import std;
using std::print;
using std::string;
using std::vector;
using std::cin;
using std::make_ptr;
using std::shared_ptr;
export class Registrar
{
private:
    void insertData();
    std::unique_ptr<EnrollInUi> ui;
    void handleStudentModule(string& studentId);
    void handleTeacherModule(string& teacherId);
    void handleSecretaryModule(string &SecretarId);
public:
    Registrar();
    ~Registrar() = default;
    int exec();

    Registrar(const Registrar&) = delete;
    Registrar& operator=(const Registrar&) = delete;

    bool studentEnrollCourse(string& studentId, string& courseId);
    vector<Enrollment*> courseRoster( string& courseId);
    bool removeStudentById(string& studentId);
};

Registrar::Registrar() {
    // 初始化UI对象
    ui = unique_ptr<EnrollInUi>(new EnrollInUi());
    // 初始化数据库表和测试数据
    insertData();
}


void Registrar::insertData()
{
    RelationalBroker::initConnection();
    auto& s=StudentBroker::singleton();
    s.createTable();
    s.initData();

    auto& c = CourseBroker::singleton();
    c.createTable();
    c.initData();

    auto& tea = TeacherBroker::singleton();
    tea.createTable();
    tea.initData();

    auto& sec = SecretaryBroker::singleton();
    sec.createTable();
    sec.initData();

    auto& room = ClassroomBroker::singleton();
    room.createTable();
    room.initData();

    auto& enroll = EnrollmentBroker::singleton();
    enroll.createTable();
    enroll.initData();
}
int Registrar::exec()
{
    int choice;

    do{
        //调用主菜单显示函数
        choice = ui->showMainMenu();
        switch(choice){
            case 1:{
                if(ui->studentLogIn()){
                    string studentId;
                    print("请再次输入学号确认：");
                    cin >> studentId;
                    handleStudentModule(studentId);
                }else{
                    print("学生登陆失败，请重试!\n");
                }
                break;
            }
            case 2:{
                if(ui->teacherLogIn()){
                    string teacherId;
                    print("请再次输入工号确定：");
                    cin >> teacherId;
                    handleTeacherModule(teacherId);
                }else{
                    print("教师登陆失败，请重试！\n");
                }
                break;
            }
            case 3:{
                if(ui->teachingsecretaryLogIn()){
                    string secretaryId;
                    print("请再次输入教务工号确认：\n");
                    cin >> secretaryId;
                    handleSecretaryModule(secretaryId);
                }else{
                    print("教务秘书登陆失败，请重试！\n");
                }
                break;
            }
            case 0:{
                print("感谢使用系统！goodbey\n");
                break;
            }
            default:{
                print("无效的选择，请输入0-3之间的数字！\n");
                break;
            }
        }
    }while(choice!=0);
    return 0;

}

void Registrar::handleStudentModule(string& studentId)
{
    while(true){
            print("\n======= 学生功能模块 =======\n");
            print("1. 选课操作\n");
            print("2. 退课操作\n");
            print("3. 查看课表\n");
            print("4. 查看成绩\n");
            print("0. 返回主菜单\n");
            print("===========================\n");
            print("请输入操作序号：");
            int ch1;
            cin >> ch1;
            auto& enrollBroker = EnrollmentBroker::singleton();
            auto& courseBroker = CourseBroker::singleton();

            switch (ch1) {
                case 1: { // 选课
                    string courseId;
                    print("请输入要选的课程ID：");
                    cin >> courseId;
                    bool success = studentEnrollCourse(studentId, courseId);
                    if (success) {
                        print("选课成功！\n");
                     } else {
                        print("选课失败！\n");
                        }
                    break;
                }
                case 2: { // 退课
                    string courseId;
                    print("请输入要退的课程ID：");
                    cin >> courseId;
                    // 调用EnrollmentBroker的退课接口
                    bool success = enrollBroker.dropEnroll(studentId, courseId);
                    if (success) {
                        print("退课成功！\n");
                    } else {
                        print("退课失败！\n");
                    }
                    break;
                }
                case 3: { // 查看课表
                    print("\n======= 你的课表 =======\n");
                    vector<string> enrolledCourses = enrollBroker.getEnrolledCourses(studentId);
                    if (enrolledCourses.empty()) {
                        print("暂无已选课程\n");
                    } else {
                        for (const auto& cid : enrolledCourses) {
                            Course* course = courseBroker.findById(cid);
                            if (course) {
                                print("课程ID：{} | 课程名：{} | 学分：{}\n",
                                    course->m_id, course->m_name, course->m_credit);
                            }
                        }
                    }
                    print("=======================\n");
                    cin.ignore();
                    cin.get();
                    break;
                }
                case 4: { // 查看成绩
                    print("\n======= 你的成绩 =======\n");
                    vector<string> enrolledCourses = enrollBroker.getEnrolledCourses(studentId);
                    if (enrolledCourses.empty()) {
                        print("暂无已选课程\n");
                    } else {
                        for (const auto& cid : enrolledCourses) {
                            double score = enrollBroker.getScore(studentId, cid);
                            string scoreStr = (score < 0) ? "未录入" : std::to_string(score);
                            Course* course = courseBroker.findById(cid);
                            print("课程ID：{} | 课程名：{} | 成绩：{}\n",
                            cid, course->m_name, score);
                            }
                    }
                    print("=======================\n");
                    print("按任意键继续...");
                    cin.ignore();
                    cin.get();
                    break;
                }
                case 0: // 返回主菜单
                    return;
                default:
                    print("无效的选择，请重新输入！\n");
                    break;
            }
        }
}

// 教师功能子模块实现
void Registrar::handleTeacherModule(string& teacherId) {
    while (true) {
        print("\n======= 教师功能模块 =======\n");
        print("1. 查看对应学生表\n");
        print("2. 给学生打分\n");
        print("0. 返回主菜单\n");
        print("===========================\n");
        print("请输入操作序号：");

        int ch1;
        cin >> ch1;
        auto& courseBroker = CourseBroker::singleton();
        auto& enrollBroker = EnrollmentBroker::singleton();

        switch (ch1) {
            case 1: { // 查看课程学生表
                string courseId;
                print("请输入要查询的课程ID：");
                cin >> courseId;
                print("\n======= 课程 {} 选课学生 =======\n", courseId);
                vector<string> studentIds = enrollBroker.getEnrolledStudents(courseId);
                if (studentIds.empty()) {
                    print("暂无学生选课\n");
                } else {
                    auto& studentBroker = StudentBroker::singleton();
                    for (const auto& sid : studentIds) {
                        Student* student = studentBroker.findById(sid);
                        print("学生ID：{} | 姓名：{}\n", sid, student->m_name);
                    }
                }
                print("==============================\n");
                print("按任意键继续...");
                cin.ignore();
                cin.get();
                break;
            }
            case 2: { // 给学生打分
                string courseId, studentId;
                double score;
                print("请输入课程ID：");
                cin >> courseId;
                print("请输入学生ID：");
                cin >> studentId;
                print("请输入成绩（0-100）：");
                cin >> score;

                // 校验成绩范围
                if (score < 0 || score > 100) {
                    print("❌ 成绩需在0-100之间！\n");
                    break;
                }

                // 调用打分接口
                bool success = enrollBroker.updateStudentCourseScore(studentId, courseId, score);
                if (success) {
                    print("✅ 成绩录入成功！\n");
                } else {
                    print("❌ 成绩录入失败（学生未选课/课程不存在）！\n");
                }
                break;
            }
            case 0: // 返回主菜单
                return;
            default:
                print("❌ 无效的选择，请重新输入！\n");
                break;
        }
    }
}

//教学秘书操作
void Registrar::handleSecretaryModule(const string& secretaryId) {
    while (true) {
        print("\n======= 教务秘书功能模块 =======\n");
        print("1. 检测排课冲突\n");
        print("2. 分配课程给教师\n");
        print("0. 返回主菜单\n");
        print("===============================\n");
        print("请输入操作序号：");

        int ch1;
        cin >> ch1;
        auto& secretaryBroker = SecretaryBroker::singleton();
        auto& courseBroker = CourseBroker::singleton();
        auto& teacherBroker = TeacherBroker::singleton();

        switch (ch1) {
            case 1: { // 检测排课冲突
                print("\n======= 排课冲突检测 =======\n");
                // 调用教务秘书的排课冲突检测接口
                vector<string> conflictCourses = secretaryBroker.detectCourseScheduleConflict();
                if (conflictCourses.empty()) {
                    print("暂无排课冲突\n");
                } else {
                    for (const auto& cid : conflictCourses) {
                        Course* course = courseBroker.findById(cid);
                        print("冲突课程ID：{} | 课程名：{}（时间/教室冲突）\n",
                              cid, course->getName());
                    }
                }
                print("==============================\n");
                print("按任意键继续...");
                cin.ignore();
                cin.get();
                break;
            }
            case 2: { // 分配课程给教师
                string courseId, teacherId;
                print("请输入课程ID：");
                cin >> courseId;
                print("请输入教师ID：");
                cin >> teacherId;

                // 校验课程和教师是否存在
                if (!courseBroker.isCourseExist(courseId)) {
                    print("❌ 课程ID不存在！\n");
                    break;
                }
                if (!teacherBroker.isTeacherExist(teacherId)) {
                    print("❌ 教师ID不存在！\n");
                    break;
                }
                // 调用分配接口
                bool success = secretaryBroker.assignCourseToTeacher(courseId, teacherId);
                if (success) {
                    print("✅ 课程分配成功！\n");
                } else {
                    print("❌ 课程分配失败（已分配/权限不足）！\n");
                }
                break;
            }
            case 0: // 返回主菜单
                return;
            default:
                print("❌ 无效的选择，请重新输入！\n");
                break;
        }
    }
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

    if (!student->enrollsIn(courseId)) {
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
    vector<shared_ptr<Enrollment>> roster;
    auto& enrollBroker = EnrollmentBroker::singleton();

    // 从EnrollmentBroker获取选课学生ID列表
    vector<string> studentIds = enrollBroker.getEnrolledStudents(courseId);
    for (const auto& sid : studentIds) {
        // 构造Enrollment对象并加入名单
        std::shared_ptr<Enrollment> enrollment = std::make_shared<Enrollment>(sid, courseId);
        roster.push_back(enrollment);
    }

    return roster;
}

bool Registrar::removeStudentById(const string& studentId)
{
    auto& studentBroker = StudentBroker::singleton();
    return studentBroker.deleteStudent(studentId);
}
