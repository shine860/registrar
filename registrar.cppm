export module registrar;
export import :domain;
export import :ui;
export import :dm;

using std::cout;
std::cin;
std::endl;
std::string;
std::vector;
std::make_unique;

export class Registrar {
private:
    void insertData();
    std::unique_ptr<EnrollInUi> ui;
    void handleStudentModule(const string& sid);
    void handleTeacherModule(const string& tid);
    void handleSecretaryModule(const string& sid);
public:
    Registrar();
    int exec();
};

Registrar::Registrar() { ui = make_unique<EnrollInUi>(); insertData(); }

void Registrar::insertData() {
    RelationalBroker::initConnection();
    StudentBroker::singleton().createTable();
    StudentBroker::singleton().initData();
    CourseBroker::singleton().createTable();
    CourseBroker::singleton().initData();
    TeacherBroker::singleton().createTable();
    TeacherBroker::singleton().initData();
    TeachingSecretaryBroker::singleton().createTable();
    TeachingSecretaryBroker::singleton().initData();
    ClassroomBroker::singleton().createTable();
    ClassroomBroker::singleton().initData();
    EnrollmentBroker::singleton().createTable();
    EnrollmentBroker::singleton().initData();
}

int Registrar::exec() {
    int choice;
    do {
        choice = ui->showMainMenu();
        string userId;
        switch (choice) {
            case 1: userId = ui->studentLogIn();
                if(!userId.empty()) handleStudentModule(userId);
                break;
            case 2: userId = ui->teacherLogIn();
                 if(!userId.empty()) handleTeacherModule(userId);
                break;
            case 3: userId = ui->teachingsecretaryLogIn();
                if(!userId.empty()) handleSecretaryModule(userId);
                break;
            case 0:
               print("感谢使用！\n");
               break;
            default:
                print("无效输入!\n");
        }
    } while (choice != 0);
    return 0;
}

void Registrar::handleStudentModule(const string& sid) {
    while(true) {
        cout << "\n1.选课 2.退课 3.课表 4.成绩 0.返回\n>> ";
        int ch; cin >> ch; if(ch==0) return;
        auto& eb = EnrollmentBroker::singleton();
        auto& cb = CourseBroker::singleton();
        if(ch==1) {
            string cid; cout<<"CID:";
            cin>>cid; time_t t=time(0);
            string s=ctime(&s);
            s.pop_back();
        if(eb.enroll(sid,cid,s))
            cout<<"OK\n";
        else cout<<"FAIL\n";
        }else if(ch==2) {
            string cid;
            cout<<"CID:";
            cin>>cid;
            if(eb.drop(sid, cid))
                cout<<"OK\n";
            else cout<<"FAIL\n";
        }else if(ch==3) {
            for(auto& c:eb.getCourseIdsByStudent(sid)) {
                auto cs=cb.findById(c);
                if(cs) cout<<cs->m_courseId<<" "<<cs->m_coursename<<endl;
                }
        }else if(ch==4) { for(auto& c:eb.getCourseIdsByStudent(sid)) {
            cout<<c<<" "<<eb.getScore(sid, c)<<endl;
            }
            }
    }
}

void Registrar::handleTeacherModule(const string& tid) {
    while(true) {
        cout << "\n1.查看学生 2.录入成绩 0.返回\n>> ";
        int ch; cin >> ch; if(ch==0) return;
        auto& eb = EnrollmentBroker::singleton();
        auto& sb = StudentBroker::singleton();
        if(ch==1) {
            string cid; cout<<"CID:";
            cin>>cid;
            for(auto& s:eb.getStudentIdsByCourse(cid)) {
                auto st=sb.findById(s);
                if(st) cout<<st->m_id<<" "<<st->m_name<<endl;
            } }
        else if(ch==2) {
            string cid, sid;
            double sc; cout<<"CID SID Score:";
            cin>>cid>>sid>>sc;
            if(eb.updateScore(sid,cid,sc))
                cout<<"OK\n";
            else
                cout<<"FAIL\n"; }
    }
}

void Registrar::handleSecretaryModule(const string& sid) {
    while(true) {
        cout << "\n1.添加教师 2.添加课程 3.分配教师 4.排课 0.返回\n>> ";
        int ch;
        cin >> ch;
        if(ch==0) return;
        auto& sec = TeachingSecretaryBroker::singleton();
        if(ch==1) {
            string id,n,g,d,t;
            cout<<"ID Name Gen Dept Title:";
            cin>>id>>n>>g>>d>>t;
            if(sec.addTeacher(Teacher(id,n,g,d,t)))
            cout<<"OK\n"; else cout<<"FAIL\n"; }
        else if(ch==2) {
            string id,n,m,s,tid;
            int gr; double cr;
            cout<<"ID Name Maj Gra Cre Syl TID:";
            cin>>id>>n>>m>>gr>>cr>>s>>tid;
            Course c(id,n,m,gr,cr,s);
            c.m_teacherId=tid;
            if(sec.addCourse(c)) cout<<"OK\n"; else cout<<"FAIL\n";
            }
        else if(ch==3) { string cid,tid;
            cout<<"CID TID:";
            cin>>cid>>tid;
            if(sec.assignCourseToTeacher(cid,tid))
                cout<<"OK\n";
            else cout<<"FAIL\n"; }
        else if(ch==4) {
            string cid,rid,t;
            cout<<"CID RID Time:";
            cin>>cid>>rid>>t;
            if(sec.arrangement(cid,rid,t))
            cout<<"OK\n"; else cout<<"FAIL\n"; }
    }
}
