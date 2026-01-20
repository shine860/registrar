//
// File: grade.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module registrar:domain.grade;
import std;
using std::print;
using std::vector;
using std::string;
using std::domain_error;
using std::ranges::stable_sort;

export class Grade
{
private:
    double m_midterm;
    double m_final;
    vector<double> homework;
    class Enrollment *m_enrollment;
public:
    Grade(double midterm, double final, vector<double> hw);
    double score();
    double median();
};

Grade::Grade(double midterm, double final, vector<double> hw)
    : m_midterm(midterm), m_final(final), homework(hw)
{}

double Grade::median()
{
    auto size = homework.size();
    if (size == 0)
        throw domain_error("empty vec.");

    stable_sort(homework);

    auto mid = size / 2;
    if (size % 2 == 0){
        return (homework[mid] + homework[mid - 1]) / 2;

    }else{
      return  homework[mid];

        }

}
double Grade::score(){
    return 0.2*m_midterm+0.4*m_final+0.4*median();
}
