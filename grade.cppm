//
// File: grade.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.grade;
import std;
using std::print;
using std::vector;

export class Grade
{
private:
    double midterm;
    double final;
    vector<double> homework;

public:
    Grade(double midterm, double final, vector<double> homework);
    double score() const;
};

Grade::Grade(double midterm, double final, vector<double> homework)
    : midterm(midterm), final(final), homework(std::move(homework)) {}

double Grade::score() const
{
    double hwAvg = 0.0;
    if (!this->homework.empty()) {
        for (double hw : this->homework)
            hwAvg += hw;
        hwAvg /= this->homework.size();
    }
    return 0.3 * this->midterm + 0.5 * this->final + 0.2 * hwAvg;
}
