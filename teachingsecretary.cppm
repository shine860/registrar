
export module registrar:domain.teachingsecretary;
import :domain.person;
import std;
using std::print;
using std::string;
using std::vector;
// class SecretaryBroker;
class Course;
class Classroom;
class Teacher;
class Student;

export class TeachingSecretary : public Person
{
friend class TeachingSecretaryBroker;

public:
    TeachingSecretary(string id, string name, string gender,string dept);
    bool arrangement(Course* course, Classroom* classroom,Teacher* teacher,const string& timeslot);
private:
    vector<string> _roomTimeSlots;
    vector<Course*> _courses;
    vector<Classroom*> _rooms;
};

TeachingSecretary::TeachingSecretary(string id, string name, string gender,string dept)
    : Person(id,name,gender,dept)
{}

