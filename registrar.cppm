// Module
// File: registrar.cppm   Version: 0.1.0   License: AGPLv3
// Created: 张雨欣2024051604045   3357714096@qq.com   2026-01-24 20:29:48
// Description:实现了业务逻辑交互
//
export module registrar;

export import :dm;
export import :domain;
export import :ui;
import std;

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::format;
using std::make_unique;
using std::vector;
using std::ctime;
using std::time_t;
using std::time;

// 控制层 Registrar 实现

export class Registrar {
private:
    std::unique_ptr<EnrollInUi> ui;
    void initData();

    void handleStudentModule(string& sid);
    void handleTeacherModule(const string& tid);
    void handleSecretaryModule(const string& sid);
public:
    Registrar();
    ~Registrar() = default;
    int exec();
};

Registrar::Registrar() {
    ui = std::make_unique<EnrollInUi>();
}

void Registrar::initData() {
    std::cout << "正在初始化数据..." << std::endl;
    auto& stu = StudentBroker::singleton();
    auto& cou = CourseBroker::singleton();
    auto& tea = TeacherBroker::singleton();
    auto& sec = TeachingSecretaryBroker::singleton();
    auto& room = ClassroomBroker::singleton();
    auto& enroll = EnrollmentBroker::singleton();
    stu.createTable();
    stu.initData();
    cou.createTable();
    cou.initData();
    tea.createTable();
    tea.initData();
    sec.createTable();
    sec.initData();
    room.createTable();
    room.initData();
    enroll.createTable();
    enroll.initData();
}

int Registrar::exec()
{
    initData();
    int choice;
    do {
        choice = ui->showMainMenu();
        string uid;
        switch (choice) {
            case 1:
                uid = ui->studentLogIn();
                if (!uid.empty()) handleStudentModule(uid);
                break;
            case 2:
                uid = ui->teacherLogIn();
                if (!uid.empty()) handleTeacherModule(uid);
                break;
            case 3:
                uid = ui->teachingsecretaryLogIn();
                if (!uid.empty()) handleSecretaryModule(uid);
                break;
            case 0:
                cout << "感谢使用！" << endl;
                break;
            default:
                cout << "无效输入，请重试！" << endl;
                break;
        }
    } while (choice != 0);
    return 0;
}

void Registrar::handleStudentModule(string& studentId) {
    while (true) {
        cout << "\n======= 学生功能模块 =======\n";
        cout << "1. 选课操作\n";
        cout << "2. 退课操作\n";
        cout << "3. 查看课表\n";
        cout << "4. 查看课程成绩\n";
        cout << "0. 返回主菜单\n";
        cout << "===========================\n";
        cout << "请输入操作序号：";
        int ch1;
        cin >> ch1;
        if (ch1 == 0) return;

        auto& eb = EnrollmentBroker::singleton();
        auto& cb = CourseBroker::singleton();

        switch (ch1) {
            case 1: { // 选课
                auto& cb = CourseBroker::singleton();
                auto allCourses = cb.findAll();

                // 打印课程列表
                cout << "\n======= 所有课程列表 =======\n";
                if (allCourses.empty()) {
                    cout << "暂无课程可选！" << endl;
                    break;
                }
                for (const auto& c : allCourses) {
                    print("{}\n",c->Info());
                }
                cout << "----------------------------------------\n";
                string cid;
                cout << "请输入课程ID：";
                cin >> cid;
                time_t now = time(0);
                char* tStr = ctime(&now);
                string timeStrCpp(tStr);
                //将最后的换行符删掉
                if (!timeStrCpp.empty() && timeStrCpp.back() == '\n') timeStrCpp.pop_back();

                if (eb.enroll(studentId, cid, timeStrCpp)) {
                    cout << "选课成功！" << endl;
                } else {
                    cout << "选课失败！（可能冲突或已选）" << endl;
                }
                break;
            }
            case 2: { // 退课
                string cid;
                cout << "请输入要退的课程ID：";
                cin >> cid;
                if (eb.drop(studentId, cid)) {
                    cout << "退课成功！" << endl;
                } else {
                    cout << "退课失败！（未选该课程或有成绩无法退课）" << endl;
                }
                break;
            }
            case 3: { // 查看课表
                cout << "\n======= 你的课表 =======\n";
                auto& sb = StudentBroker::singleton();
                if (sb.isStudentExists(studentId)) {
                    for (auto cid_ref : eb.getCourseIdsByStudent(studentId)) {
                        auto c = cb.findById(cid_ref);
                        if (c) {
                            double sc = eb.getScore(studentId, cid_ref);
                            string scoreStr = (sc < 0.0) ? "未录入" : std::to_string(sc);
                            cout << c->Info() << " 成绩: " << scoreStr << endl;
                        }
                    }
                } else {
                    cout << "学生 ID: " << studentId << " 不存在。" << endl;
                }
                cout << "按任意键继续...";
                cin.ignore();
                cin.get();
                break;
            }
            case 4:{ // 查看已经修过的课程成绩
                cout << "\n======= 你的成绩单 =======\n";
                auto &eb = EnrollmentBroker::singleton();
                auto &cb = CourseBroker::singleton();
                auto &sb = StudentBroker::singleton();
                bool hasScore = false;

                if (sb.isStudentExists(studentId)) {
                    for (auto cid_ref : eb.getCourseIdsByStudent(studentId)) {
                        auto c = cb.findById(cid_ref);
                        string courseName = c ? c->m_coursename : "未知课程";

                        double sc = eb.getScore(studentId, cid_ref);
                        string scoreStr;

                        if (sc < -1.0) {
                            scoreStr = "数据异常";
                        } else if (sc < 0.0) {
                            scoreStr = "未录入";
                        } else {
                            scoreStr = std::to_string(sc);
                        }

                        cout << "课程: " << courseName << "  成绩: " << scoreStr << endl;
                        hasScore = true;
                    }
                } else {
                    cout << "学生 ID: " << studentId << " 不存在。" << endl;
                }

                if (!hasScore && sb.isStudentExists(studentId)) {
                    cout << "暂无选课记录" << endl;
                }

                cout << "按任意键继续...";
                cin.ignore();
                cin.get();
                break;
            }
            default:
                cout << "无效的选择，请重新输入！" << endl;
                break;
        }
    }
}

void Registrar::handleTeacherModule(const string& teacherId) {
    while (true) {
        cout << "\n======= 教师功能模块 =======\n";
        cout << "1. 查看课程学生表\n";
        cout << "2. 给学生打分\n";
        cout << "3. 查看我教授的课程\n";
        cout << "0. 返回主菜单\n";
        cout << "===========================\n";
        cout << "请输入操作序号：";
        int ch1;
        cin >> ch1;
        if (ch1 == 0) return;

        auto& eb = EnrollmentBroker::singleton();
        auto& cb = CourseBroker::singleton();
        auto& sb = StudentBroker::singleton();

        switch (ch1) {
            case 1: { // 查看课程学生表
                string cid;
                cout << "请输入要查询的课程ID：";
                cin >> cid;
                auto c = cb.findById(cid);

                // 权限检查
                if (!c || c->m_teacherId.empty()) {
                    cout << "错误：课程不存在或未分配教师！" << endl;
                } else if (c->m_teacherId != teacherId) {
                    cout << "  权限拒绝：您不是该课程的主讲教师！" << endl;
                    cout << "  现任教师ID: " << c->m_teacherId << endl;
                } else {
                    cout << "\n======= 课程 " << cid << " 选课学生名单 =======\n";
                    auto studentIds = eb.getStudentIdsByCourse(cid);
                    if (studentIds.empty()) {
                        cout << "暂无学生选课" << endl;
                    } else {
                        for (const auto& sid : studentIds) {
                            auto s = sb.findById(sid);
                            if (s) {
                                cout << "学生ID：" << s->get_id() << " | 姓名：" << s->get_name() << endl;
                            }
                        }
                    }
                    cout << "=============================" << endl;
                }
                cout << "按任意键继续...";
                cin.ignore();
                cin.get();
                break;

            }
            case 2: { // 给学生打分
                string cid, sid;
                double mid, fin;
                cout << "请输入课程ID："; cin >> cid;
                cout << "请输入学生ID："; cin >> sid;
                cout << "请输入期中成绩(0-100)："; cin >> mid;
                cout << "请输入期末成绩(0-100)："; cin >> fin;
                std::print("请输入有几个家庭作业成绩:");
                int hwCount;
                cin >> hwCount;
                if(hwCount<=0) hwCount = 0;//防止死循环

                double hwAvg = 0;
                for(int i = 0;i<hwCount;++i){
                    double s;
                    cout<<"请输入第 " << i+1 << "个家庭作业的成绩:";
                    cin >> s;
                    hwAvg += s;//累加
                }

                double totalScore = mid * 0.4+fin*0.4+0.2*hwAvg/hwCount;
                auto c = cb.findById(cid);
                if(c){
                    if (mid < 0 || mid > 100 || fin < 0 || fin > 100|| hwAvg<0 || hwAvg>100) {
                        cout << "成绩需在 0-100之间！" << endl;
                    } else {
                        if (eb.updateScore(sid, cid, totalScore)) {
                            cout << ">> 成绩录入成功！" << endl;
                        } else {
                            cout << "成绩录入失败（或学生未选该课）" << endl;
                        }
                    }
                }
                break;
            }
            case 3: { // 查看我所教授的课程
            cout << "\n======= 您负责的课程 =======\n";
            bool found = false;

            // cc = CourseClass (教学班表), c = Course (课程表)
            string sql = "SELECT c.id, c.name, c.credit, cc.student_class "
                        "FROM CourseClass cc "
                        "JOIN Course c ON cc.course_id = c.id " //通过课程ID把两张表连起来
                        "WHERE cc.teacher_id = '" + teacherId + "'";

                try {
                    // 执行查询
                    auto res = CourseBroker::singleton().query(sql);

                    if (res.empty()) {
                        cout << "您暂未分配任何课程。" << endl;
                    } else {
                        // 循环内修改：读取正确的列名
                        for (const auto& r : res) {
                            // 注意：这里读取的列名必须和上面 SELECT 后面的列名完全对应
                            cout << "课程ID: " << r["id"].as<std::string>()
                                 << "    课程名: " << r["name"].as<std::string>()
                                 << "    学分: " << r["credit"].as<double>()
                                 << "    教学班: " << r["student_class"].as<std::string>()
                                 << endl;
                            found = true;
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "查询出错: " << e.what() << endl;
                }

                cout << "============================" << endl;
                cout << "按任意键继续...";
                cin.ignore();
                cin.get();
                break;
            }
            default:
                cout << "无效的选择，请重新输入！" << endl;
                break;
        }
    }
}

void Registrar::handleSecretaryModule(const string& secretaryId)
{
    while (true) {
        cout << "\n======= 教务秘书功能模块 =======\n";
        cout << "1. 添加教师\n";
        cout << "2. 建立教学班 (分配老师+班级)\n";
        cout << "3. 添加课程\n";
        cout << "4. 智能排课 (检测冲突)\n";
        cout << "0. 返回主菜单\n";
        cout << "==============================" << endl;
        cout << "请输入操作序号：";

        int ch1;
        cin >> ch1;
        if (ch1 == 0) return;

        auto& sec = TeachingSecretaryBroker::singleton();
        auto& courseBroker = CourseBroker::singleton();
        auto& teacherBroker = TeacherBroker::singleton();
        auto& classroomBroker = ClassroomBroker::singleton();

        switch (ch1) {
            case 1: { // 添加教师
                string id, name, gender, dept, title;
                cout << "ID   Name   Gen   Dept   Title: ";
                cin >> id >> name >> gender >> dept >> title;

                Teacher teacher(id, name, gender, dept, title);

                if (sec.addTeacher(teacher)) {
                    cout << "教师 " << name << " 添加成功！" << endl;
                } else {
                    cout << "教师ID " << id << " 已存在，添加失败！" << endl;
                }
                break;
            }
            case 2: { // 建立教学班
                string cid, tid, sClass;
                cout << "请输入课程ID: ";
                cin >> cid;
                cout << "请输入教师ID: ";
                cin >> tid;
                cout << "请输入教学班级 (如 计科1801): ";
                cin >> sClass;

                auto c = courseBroker.findById(cid);
                auto te = teacherBroker.findById(tid);

                if (!c || !te) {
                    cout << ">> 失败：资源不存在！" << endl;
                } else {
                    string classId = cid + "-" + sClass;
                    if (sec.createCourseClass(cid, tid, sClass, classId)) {
                        cout << " 分配成功！教学班ID: " << classId << endl;
                    } else {
                        cout << " 分配失败（可能已存在）！" << endl;
                    }
                }
                break;
            }
            case 3: { // 添加课程
                string id, name, major, syllubus, tid;
                int gr;
                double cr;
                cout << "ID Name Maj Gra Cre Syl TID: ";
                cin >> id >> name >> major >> gr >> cr >> syllubus >> tid;

                Course c(id, name, major, gr, cr, syllubus);
                c.assignTeacherId(tid);

                if (courseBroker.isCourseExists(id)) {
                    cout << "课程ID " << id << " 已存在，添加失败！" << endl;
                } else {
                    if (sec.addCourse(c)) {
                        cout << "课程 " << name << " 添加成功！" << endl;
                    } else {
                        cout << "课程添加失败！" << endl;
                    }
                }
                break;
            }
            case 4: { // 智能排课
                cout << "\n======= 当前可用教室列表 =======\n";
                auto classrooms = ClassroomBroker::singleton().findAll();
                if (classrooms.empty()) {
                    cout << "暂无教室数据，无法排课。" << endl;
                    cout << "按任意键继续...";
                    cin.ignore();
                    cin.get();
                    break; // 没教室就退出
                }
                for (const auto& r : classrooms) {
                    cout << "教室ID: " << r->m_roomNum
                        << "   楼宇: " << r->m_building
                        << "   容量: " << r->m_capacity << endl;
                }
                cout << "==================================\n";
                string cid, rid,tid, t;
                cout << "courseId roomID teacherID Time: ";
                cin >> cid >> rid >>tid >> t;

                auto c = courseBroker.findById(cid);
                auto r = classroomBroker.findById(rid);
                auto te = teacherBroker.findById(tid);

                if (!c || !r || !te) {
                    cout << "排课失败：资源（课程/教室/教师）不存在！" << endl;
                    break;
                }

                // 清理缓冲区，防止残留字符导致死循环
                cin.ignore(10000, '\n');
                auto& tea = TeachingSecretaryBroker::singleton();
                if (tea.arrangement(c->m_courseId, r->m_roomNum, te->get_id(), t)) {
                    cout << "排课成功！" << endl;
                } else {
                    cout << "排课失败（冲突或资源不存在）！" << endl;
                }
                break;
            }
            default:
                cout << "❌ 无效的选择，请重新输入！" << endl;
                break;
        }
    }
}
