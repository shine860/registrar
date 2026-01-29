//
// File: classroom.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module registrar:domain.classroom;
import std;
using std::print;
using std::string;
using std::vector;

class Course;

class TeachingSecretary;
export class Classroom
{
friend class ClassroomBroker;
friend class TeachingSecretary;
friend class Registrar;
public:
    Classroom(string roomNum, string building,int capacity);
    bool checkAvailability();
    bool occupyClassroom(const string& timeSlot,Course* course);
    bool releaseClassroom();
    void showClassroomInfo();
    bool removeCourseFromSchedule(Course* course);
    void updateAvailable(bool isAvailable);
    bool hasId(string id);
private:
    string m_roomNum;//教室编号
    string m_building;//教学楼名称
    int m_capacity;//容量
    bool m_isOccupied;//是否被占用
    vector<Course*> _courseSchedule;//教室的课程排课表
};

Classroom::Classroom(string roomNum,string building, int capacity)
    : m_roomNum(roomNum),m_building(building), m_capacity(capacity), m_isOccupied(false) {}

bool Classroom::checkAvailability()
{
    return !this->m_isOccupied;
}



bool Classroom::releaseClassroom()
{
    if (!this->m_isOccupied) return false;
    this->m_isOccupied = false;
    print("教室{}已释放", this->m_roomNum);
    return true;
}

void Classroom::showClassroomInfo()
{
    print("教室编号：{}，容量：{}，状态：{}", this->m_roomNum, this->m_capacity, this->m_isOccupied ? "已占用" : "空闲");
    print("排课数量：{}\n",_courseSchedule.size());
}
bool Classroom::removeCourseFromSchedule(Course* course)
{
    for(auto it = _courseSchedule.begin();it!=_courseSchedule.end();){
        if(*it == course){
            _courseSchedule.erase(it);
            print("课程已经从教室{}的排课表中移除!\n",m_roomNum);
            return true;
        }
    }
    print("错误：教室{}的排课表中没有找到该课程！\n",m_roomNum);
    return false;
}
void Classroom::updateAvailable(bool isAvailable) {
    this->m_isOccupied = !isAvailable;
}
 bool Classroom::hasId(string id)
 {
    return id==m_roomNum;
}
