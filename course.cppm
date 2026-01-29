//
// File: course.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module registrar:domain.course;
import std;
import :domain.enrollment;

using std::print;
using std::string;
using std::vector;
using std::weak_ptr;
// class CourseBroker;
class Student;//仅仅前置申明
export class Course
{
friend class CourseBroker;
friend class TeachingSecretaryBroker;
friend class ClassroomBroker;
friend class Registrar;

public:
    Course(string id,string name, string major,int grade,double credit,string syllabus);
    bool acceptEnrollment(class Student* student);
    string roster();
    bool assignTeacher(class Teacher* teacher);
    string Info();
    bool hasId(string id);
    void addPrerequisiteId(const std::string& pid);
    bool assignTeacherId(const std::string& tid);
    // void setTeacherId(const string& tid);
private:
    string m_courseId;//课程号
    string m_coursename;//课程名字
    string m_major;//所属专业
    int m_grade;//对应年级
    double m_credit; //学分
    string m_syllabus;//新增培养方案必修课
    string m_teacherId;
    vector<class Enrollment*> _enrollments; //选该课程的选课记录
    vector<class Student*> _students;
    vector<class Teacher*> _teachers;
    vector<string> _prerequisiteIds;
};

Course::Course(string id,string name, string major,int grade,double credit,string syllabus)
    :m_courseId(id),m_coursename(name),m_major(major), m_grade(grade),m_credit(credit), m_syllabus(syllabus)
{}

bool Course::acceptEnrollment(Student *student){
    if(_students.size() < 80){  // 假定某个课程的最大人数为80
        _students.push_back(student);
        print("\"{}\" 选课成功！目前选择该课程的人数: {}\n",
              m_coursename, _students.size());
        return true;
    }
    return false;
}

bool Course::assignTeacherId(const std::string& tid) {
       this->m_teacherId = tid;
       return true;
}
bool Course::assignTeacher(class Teacher* teacher)
{
    if(!teacher){
        print("该教师不存在！\n");
        return false;
    }
    _teachers.push_back(teacher);
    print("分配教师成功！\n");
    return true;
}

string Course::Info()
{
    return std::format("课程：{}    {}    {}     {} \n",m_courseId,m_coursename,m_credit,m_syllabus);
}

bool Course::hasId(string id){
    return id == m_courseId;
}
void Course::addPrerequisiteId(const std::string& pid) {
       _prerequisiteIds.push_back(pid);
}
// void Course::setTeacherId(const string& tid) {
//        m_teacherId = tid;
// }
