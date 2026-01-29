export module registrar:domain;

export import :domain.student;
export import :domain.course;
export import :domain.teacher;
export import :domain.classroom;
export import :domain.enrollment;
export import :domain.grade;
export import :domain.person;
export import :domain.teachingsecretary;
import std;
//选课类循环依赖
bool Enrollment::hasTId(string sid, string cid)
{
    return m_student->hasId(sid) && m_course->hasId(cid);
}

//学生类中的循环依赖
void Student::enrollsIn(Course *course){
    if(course->acceptEnrollment(this))
        _courses.push_back(course);
}

string Student::schedule()
{
    auto s = format("{}'s schedule:\n", m_name);
    for(auto &c: _courses){
        s += c->Info();
    }
    return s;
}
bool Student::dropCourse(string courseId, double courseCredit)
{
   for(auto c=_enrollments.begin();c!=_enrollments.end();){
    if(!(*c) -> hasTId(this->m_id,courseId))
    {
        print("学生 {} 退课失败：未选该课程\n", m_name);
        return false;
    }else{
       c =  _enrollments.erase(c);
    }
    }
    print("学生 {} 成功退选课程 {}（学分：{:.1f}\n", m_name, courseId, courseCredit);
    return true;
}



//教师类中的循环依赖
bool Teacher::assignCourse(Course* course)
{
    if(!course) return false;
    for(auto &c:_teachingCourses){
        if(course == c){
            print("教师已经分配该课程！\n");
            return false;
        }
    }
    _teachingCourses.push_back(course);
    course->assignTeacher(this);
    print("教师{}成功分配该课程！\n",m_name);
    return true;
}
void Teacher::showTeachCourse()
{
    for(auto c:_teachingCourses){
        print("{}   教授课程列表:{}\n",m_name,c->Info());
    }
}
void Teacher::submitGrade(const string studentId, const std::string courseId, double midterm, double final, const std::vector<double>& homeworks) {
    Grade grade(midterm, final, homeworks);
    double finalScore = grade.score();

    std::cout << "教师 " << m_name << " 正在为学生 " << studentId
    << " 的课程 " << courseId << " 录入成绩..." << std::endl;
    std::cout << "期中: " << midterm << " 期末: " << final
    << ", 作业数: " << homeworks.size() << std::endl;
    std::cout << " -> 计算最终得分: " << finalScore << std::endl;
}

//课程循环依赖
string Course::roster(){
    auto rst = format("{} selected by the students:\n", m_coursename);
    for (auto s : _students) {
        rst += s->Info(); // 课程对象委托学生对象自己输出相关信息
    }
    return rst;
}
bool Classroom::occupyClassroom(const string& timeSlot,Course* course)
{
    if (this->m_isOccupied){
        print("错误：教室已经被占用，无法预约！\n",m_roomNum);
        return false;
    }
    this->m_isOccupied = true;
    if(course != nullptr){
        _courseSchedule.push_back(course);
        print("教室{} 已经被课程{}占用（时间段：{},已经加入排课表！\n",m_roomNum,course->Info(),timeSlot);
    }
    return true;
}
//教学秘书循环依赖
bool TeachingSecretary::arrangement(Course* course, Classroom* classroom,Teacher* teacher,const string& timeslot)
{
if (!course || !classroom || !teacher) {
       print("排课失败\n");
       return false;
   }
   string roomId = classroom->m_roomNum;
   string key = roomId + ":" + timeslot;

   for (const auto& slot : this->_roomTimeSlots)
   {
       if (slot == key)
       {
           print("排课失败：教室 {} 在时间段 {} 已经被占用！", roomId, timeslot);
           return false;
       }
   }
   this->_roomTimeSlots.push_back(key);

   classroom->occupyClassroom(timeslot, course);
   print("教务秘书 {} 排课成功 (教室: {}, 时间: {})", this->m_name, roomId, timeslot);
   return true;
}
