export module registrar:domain.course;
import std;
import :domian.enrollment;

using std::print;
using std::string;
using std::vector;

export class Course
{
private:
    string name;
    string id;
    double credit;
    // string syllabus;
    vector<Enrollment> enrolls;

public:
    Course(string name, string id, double credit);//, string syllabus);
    bool acceptEnrollment(const string& studentId);
    string roster() const;
};

Course::Course(string name, string id, double credit, string syllabus)
    : name(std::move(name)), id(std::move(id)), credit(credit), syllabus(std::move(syllabus)) {}

bool Course::acceptEnrollment(const string& studentId)
{
    this->enrolledStudents.push_back(studentId);
    return true;
}

string Course::roster() const
{
    string result = "Enrolled Students: ";
    for (const auto& sid : this->enrolledStudents)
        result += sid + ", ";
    return result.empty() ? "No students enrolled" : result;
}
