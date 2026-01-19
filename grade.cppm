//
// File: grade.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.grade;
import std;
using std::vector;

export class Grade
{
private:
    double m_midterm;
    double m_final;
    vector<double> _homework;
public:
    Grade(double midterm, double final, vector<double> homework);
    double score() const;
};

Grade::Grade(double midterm, double final, vector<double> homework)
    : m_midterm(midterm), m_final(final), _homework(std::move(homework)) {}

double Grade::score() const
{
    double hwAvg = 0.0;
    if (!this->_homework.empty())
    {
        for (double hw : this->_homework)
            hwAvg += hw;
        hwAvg /= this->_homework.size();
    }
    return 0.3 * this->m_midterm + 0.5 * this->m_final + 0.2 * hwAvg;
}
