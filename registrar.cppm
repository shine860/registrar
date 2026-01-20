// Module
// File: registrar.cppm   Version: 0.1.0   License: AGPLv3
// Created: 张雨欣2024051604045   3357714096@qq.com   2026-01-20 14:24:34
// Description:
//
export module registrar;
export import :ui;
export import :domain;
export import :dm;
import std;
export class Registrar
{
private:
    void insertData();
    std::unique_ptr<EnrollInUi> ui;
    void handleStudentModule(std::string& studentId);
    void handleTeacherModule(std::string& teacherId);
    void handleSecretaryModule(std::string& secretaryId);
public:
    Registrar();
    ~Registrar() = default;
    int exec();
    Registrar(const Registrar&) = delete;
    Registrar& operator=(const Registrar&) = delete;
    bool studentEnrollCourse(std::string& studentId, std::string& courseId);
    std::vector<Enrollment*> courseRoster(std::string& courseId);
    bool removeStudentById(std::string& studentId);
};
Registrar::Registrar() {
    // 初始化UI对象
    ui = std::make_unique<EnrollInUi>();
    // 初始化数据库表和测试数据
    insertData();
}
void Registrar::insertData()
{
    RelationalBroker::initConnection();
    auto& s = StudentBroker::singleton();
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
    do {
        // 调用主菜单显示函数
        choice = ui->showMainMenu();
        switch(choice) {
            case 1: {
                if(ui->studentLogIn()) {
                    std::string studentId;
                    std::print("请再次输入学号确认：");
                    std::cin >> studentId;
                    handleStudentModule(studentId);
                } else {
                    std::print("学生登陆失败，请重试!\n");
                }
                break;
            }
            case 2: {
                if(ui->teacherLogIn()) {
                    std::string teacherId;
                    std::print("请再次输入工号确定：");
                    std::cin >> teacherId;
                    handleTeacherModule(teacherId);
                } else {
                    std::print("教师登陆失败，请重试！\n");
                }
                break;
            }
            case 3: {
                if(ui->teachingSecretaryLogIn()) {
                    std::string secretaryId;
                    std::print("请再次输入教务工号确认：\n");
                    std::cin >> secretaryId;
                    handleSecretaryModule(secretaryId);
                } else {
                    std::print("教务秘书登陆失败，请重试！\n");
                }
                break;
            }
            case 0: {
                std::print("感谢使用系统！goodbye\n");
                break;
            }
            default: {
                std::print("无效的选择，请输入0-3之间的数字！\n");
                break;
            }
        }
    } while(choice != 0);
    return 0;
}
void Registrar::handleStudentModule(std::string& studentId)
{
    while(true) {
        std::print("\n======= 学生功能模块 =======\n");
        std::print("1. 选课操作\n");
        std::print("2. 退课操作\n");
        std::print("3. 查看课表\n");
        std::print("4. 查看成绩\n");
        std::print("0. 返回主菜单\n");
        std::print("===========================\n");
        std::print("请输入操作序号：");
        int ch1;
        std::cin >> ch1;
        auto& enrollBroker = EnrollmentBroker::singleton();
        auto& courseBroker = CourseBroker::singleton();
        switch (ch1) {
            case 1: { // 选课
                std::string courseId;
                std::print("请输入要选的课程ID：");
                std::cin >> courseId;
                bool success = studentEnrollCourse(studentId, courseId);
                if (success) {
                    std::print("选课成功！\n");
                } else {
                    std::print("选课失败！\n");
                }
                break;
            }
            case 2: { // 退课
                std::string courseId;
                std::print("请输入要退的课程ID：");
                std::cin >> courseId;
                // 调用EnrollmentBroker的退课接口
                bool success = enrollBroker.dropEnroll(studentId, courseId);
                if (success) {
                    std::print("退课成功！\n");
                } else {
                    std::print("退课失败！\n");
                }
                break;
            }
            case 3: { // 查看课表
                std::print("\n======= 你的课表 =======\n");
                std::vector<std::string> enrolledCourses = enrollBroker.getEnrolledCourses(studentId);
                if (enrolledCourses.empty()) {
                    std::print("暂无已选课程\n");
                } else {
                    for (const auto& cid : enrolledCourses) {
                        Course* course = courseBroker.findById(cid);
                        if (course) {
                            std::print("课程ID：{} | 课程名：{} | 学分：{}\n",
                                course->m_id, course->m_name, course->m_credit);
                        }
                    }
                }
                std::print("=======================\n");
                std::cin.ignore();
                std::cin.get();
                break;
            }
            case 4: { // 查看成绩
                std::print("\n======= 你的成绩 =======\n");
                std::vector<std::string> enrolledCourses = enrollBroker.getEnrolledCourses(studentId);
                if (enrolledCourses.empty()) {
                    std::print("暂无已选课程\n");
                } else {
                    for (const auto& cid : enrolledCourses) {
                        double score = enrollBroker.getScore(studentId, cid);
                        std::string scoreStr = (score < 0) ? "未录入" : std::to_string(score);
                        Course* course = courseBroker.findById(cid);
                        std::print("课程ID：{} | 课程名：{} | 成绩：{}\n",
                        cid, course->m_name, score);
                    }
                }
                std::print("=======================\n");
                std::print("按任意键继续...");
                std::cin.ignore();
                std::cin.get();
                break;
            }
            case 0: // 返回主菜单
                return;
            default:
                std::print("无效的选择，请重新输入！\n");
                break;
        }
    }
}
void Registrar::handleTeacherModule(std::string& teacherId) {
    while (true) {
        std::print("\n======= 教师功能模块 =======\n");
        std::print("1. 查看对应学生表\n");
        std::print("2. 给学生打分\n");
        std::print("0. 返回主菜单\n");
        std::print("===========================\n");
        std::print("请输入操作序号：");
        int ch1;
        std::cin >> ch1;
        auto& courseBroker = CourseBroker::singleton();
        auto& enrollBroker = EnrollmentBroker::singleton();
        switch (ch1) {
            case 1: { // 查看课程学生表
                std::string courseId;
                std::print("请输入要查询的课程ID：");
                std::cin >> courseId;
                std::print("\n======= 课程 {} 选课学生 =======\n", courseId);
                std::vector<std::string> studentIds = enrollBroker.getEnrolledStudents(courseId);
                if (studentIds.empty()) {
                    std::print("暂无学生选课\n");
                } else {
                    auto& studentBroker = StudentBroker::singleton();
                    for (const auto& sid : studentIds) {
                        Student* student = studentBroker.findById(sid);
                        std::print("学生ID：{} | 姓名：{}\n", sid, student->m_name);
                    }
                }
                std::print("==============================\n");
                std::print("按任意键继续...");
                std::cin.ignore();
                std::cin.get();
                break;
            }
            case 2: { // 给学生打分
                std::string courseId, studentId;
                double score;
                std::print("请输入课程ID：");
                std::cin >> courseId;
                std::print("请输入学生ID：");
                std::cin >> studentId;
                std::print("请输入成绩（0-100）：");
                std::cin >> score;
                // 校验成绩范围
                if (score < 0 || score > 100) {
                    std::print("❌ 成绩需在0-100之间！\n");
                    break;
                }
                // 调用打分接口
                bool success = enrollBroker.updateStudentCourseScore(studentId, courseId, score);
                if (success) {
                    std::print("✅ 成绩录入成功！\n");
                } else {
                    std::print("❌ 成绩录入失败（学生未选课/课程不存在）！\n");
                }
                break;
            }
            case 0: // 返回主菜单
                return;
            default:
                std::print("❌ 无效的选择，请重新输入！\n");
                break;
        }
    }
}
void Registrar::handleSecretaryModule(std::string& secretaryId) {
    while (true) {
        std::print("\n======= 教务秘书功能模块 =======\n");
        std::print("1. 检测排课冲突\n");
        std::print("2. 分配课程给教师\n");
        std::print("0. 返回主菜单\n");
        std::print("===============================\n");
        std::print("请输入操作序号：");
        int ch1;
        std::cin >> ch1;
        auto& secretaryBroker = SecretaryBroker::singleton();
        auto& courseBroker = CourseBroker::singleton();
        auto& teacherBroker = TeacherBroker::singleton();
        switch (ch1) {
            case 1: { // 检测排课冲突
                std::print("\n======= 排课冲突检测 =======\n");
                // 调用教务秘书的排课冲突检测接口
                std::vector<std::string> conflictCourses = secretaryBroker.detectCourseScheduleConflict();
                if (conflictCourses.empty()) {
                    std::print("暂无排课冲突\n");
                } else {
                    for (const auto& cid : conflictCourses) {
                        Course* course = courseBroker.findById(cid);
                        std::print("冲突课程ID：{} | 课程名：{}（时间/教室冲突）\n",
                              cid, course->getName());
                    }
                }
                std::print("==============================\n");
                std::print("按任意键继续...");
                std::cin.ignore();
                std::cin.get();
                break;
            }
            case 2: { // 分配课程给教师
                std::string courseId, teacherId;
                std::print("请输入课程ID：");
                std::cin >> courseId;
                std::print("请输入教师ID：");
                std::cin >> teacherId;
                // 校验课程和教师是否存在
                if (!courseBroker.isCourseExist(courseId)) {
                    std::print("❌ 课程ID不存在！\n");
                    break;
                }
                if (!teacherBroker.isTeacherExist(teacherId)) {
                    std::print("❌ 教师ID不存在！\n");
                    break;
                }
                // 调用分配接口
                bool success = secretaryBroker.assignCourseToTeacher(courseId, teacherId);
                if (success) {
                    std::print("✅ 课程分配成功！\n");
                } else {
                    std::print("❌ 课程分配失败（已分配/权限不足）！\n");
                }
                break;
            }
            case 0: // 返回主菜单
                return;
            default:
                std::print("❌ 无效的选择，请重新输入！\n");
                break;
        }
    }
}
bool Registrar::studentEnrollCourse(std::string& studentId, std::string& courseId)
{
    auto& studentBroker = StudentBroker::singleton();
    auto& courseBroker = CourseBroker::singleton();
    auto& enrollBroker = EnrollmentBroker::singleton();
    Student* student = studentBroker.findById(studentId);
    if (!student) {
        std::print("Student not found: {}\n", studentId);
        return false;
    }
    Course* course = courseBroker.findById(courseId);
    if (!course) {
        std::print("Course not found: {}\n", courseId);
        return false;
    }
    if (!student->enrollsIn(courseId)) {
        std::print("Failed to enroll student {} in course {}\n", studentId, courseId);
        return false;
    }
    if (!course->acceptEnrollment(studentId)) {
        std::print("Course {} rejected enrollment for student {}\n", courseId, studentId);
        return false;
    }
    auto enrollment = std::make_shared<Enrollment>(studentId, courseId);
    enrollBroker.insert(enrollment);
    std::print("Student {} successfully enrolled in course {}\n", studentId, courseId);
    return true;
}
std::vector<std::shared_ptr<Enrollment>> Registrar::courseRoster(std::string& courseId)
{
    std::vector<std::shared_ptr<Enrollment>> roster;
    auto& enrollBroker = EnrollmentBroker::singleton();
    // 从EnrollmentBroker获取选课学生ID列表
    std::vector<std::string> studentIds = enrollBroker.getEnrolledStudents(courseId);
    for (const auto& sid : studentIds) {
        // 构造Enrollment对象并加入名单
        auto enrollment = std::make_shared<Enrollment>(sid, courseId);
        roster.push_back(enrollment);
    }
    return roster;
}
bool Registrar::removeStudentById(std::string& studentId)
{
    auto& studentBroker = StudentBroker::singleton();
    return studentBroker.deleteStudent(studentId);
}
