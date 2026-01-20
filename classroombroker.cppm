module;
#include <pqxx/pqxx>
#include <iostream>
#include <memory>
#include <vector>
#include <stdexcept>

export module registrar:dm.classroombroker;

import :dm.base;
import :domain.classroom;

export class ClassroomBroker : public RelationalBroker {
public:
    static ClassroomBroker& singleton();
    void createTable() override;
    void initData() override;

    std::shared_ptr<Classroom> findById(const std::string& rid);
    bool isClassroomExists(const std::string& rid) const;

    bool addClassroom(const Classroom& classroom);
    bool updateClassroomStatus(const std::string& rid, bool isAvailable);

    std::vector<std::shared_ptr<Classroom>> findAll();

    bool deleteClassroom(const std::string& rid);
    bool addCourseToSchedule(const std::string& rid, const std::string& cid, const std::string& time);
private:
    ClassroomBroker();
    ClassroomBroker(const ClassroomBroker&) = delete;
    ClassroomBroker& operator=(const ClassroomBroker&) = delete;

    std::vector<std::shared_ptr<Classroom>> _classrooms;
};

ClassroomBroker::ClassroomBroker() { initConnection(); }

ClassroomBroker& ClassroomBroker::singleton() {
    static ClassroomBroker instance;
    return instance;
}

void ClassroomBroker::createTable() {

    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS Classroom (
            id VARCHAR(20) PRIMARY KEY,
            building VARCHAR(50) NOT NULL,
            capacity INT NOT NULL,
            is_available BOOLEAN DEFAULT TRUE
        );
        CREATE TABLE IF NOT EXISTS ClassroomSchedule (
            classroom_id VARCHAR(20) NOT NULL,
            course_id VARCHAR(20) NOT NULL,
            time_slot VARCHAR(50) NOT NULL,
            PRIMARY KEY(classroom_id, time_slot)
        )
    )";
    query(sql);
}

void ClassroomBroker::initData() {
    query("DELETE FROM ClassroomSchedule; DELETE FROM Classroom;");
    _classrooms.clear();


    std::vector<Classroom> data = {
        {"A101", "一号教学楼", 50},
        {"B202", "二号教学楼", 30}
    };

    try {
        pqxx::work tx(*m_conn);
        for(auto& c : data) {

            tx.exec(
                "INSERT INTO Classroom(id, building, capacity, is_available) VALUES($1, $2, $3, TRUE)",
                pqxx::params{c.m_roomNum, c.m_building, c.m_capacity}
            );
        }
        tx.commit();

        for(auto& c : data) {
            _classrooms.push_back(std::make_shared<Classroom>(c));
        }
        std::cout << "[ClassroomBroker] 教室数据初始化完成。" << std::endl;
    } catch(const std::exception& e) {
        std::cerr << "初始化失败: " << e.what() << std::endl;
    }
}


bool ClassroomBroker::isClassroomExists(const std::string& rid) const {
    for(auto& c : _classrooms) {
        if(c->hasId(rid)) return true;
    }
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT 1 FROM Classroom WHERE id=$1", pqxx::params{rid});
        tx.commit();
        return !res.empty();
    } catch(...) {
        return false;
    }
}

std::shared_ptr<Classroom> ClassroomBroker::findById(const std::string& rid) {
    for(auto& c : _classrooms) {
        if(c->hasId(rid)) return c;
    }
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT * FROM Classroom WHERE id=$1", pqxx::params{rid});
        tx.commit();

        if(!res.empty()) {
            const auto& r = res[0];
            auto c = std::make_shared<Classroom>(
                r["id"].as<std::string>(),
                r["building"].as<std::string>(),
                r["capacity"].as<int>()
            );
            c->setAvailable(r["is_available"].as<bool>());
            _classrooms.push_back(c);
            return c;
        }
    } catch(const std::exception& e) {
        std::cerr << "查询教室失败: " << e.what() << std::endl;
    }
    return nullptr;
}

bool ClassroomBroker::addClassroom(const Classroom& classroom) {
    if(isClassroomExists(classroom.m_roomNum)) {
        std::cerr << "教室ID " << classroom.m_roomNum << " 已存在" << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec(
            "INSERT INTO Classroom(id, building, capacity, is_available) VALUES($1, $2, $3, TRUE)",
            pqxx::params{classroom.m_roomNum, classroom.m_building, classroom.m_capacity}
        );
        tx.commit();
        _classrooms.push_back(std::make_shared<Classroom>(classroom));
        std::cout << "教室 " << classroom.m_roomNum << " 添加成功" << std::endl;
        return true;
    } catch(const std::exception& e) {
        std::cerr << "添加教室失败: " << e.what() << std::endl;
        return false;
    }
}

bool ClassroomBroker::updateClassroomStatus(const std::string& rid, bool isAvailable) {
    if(!isClassroomExists(rid)) {
        std::cerr << "教室 " << rid << " 不存在" << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        // 使用 params
        tx.exec(
            "UPDATE Classroom SET is_available = $1 WHERE id = $2",
            pqxx::params{isAvailable, rid}
        );
        tx.commit();

        // 更新缓存
        for(auto& c : _classrooms) {
            if(c->hasId(rid)) {
                c->setAvailable(isAvailable);
                break;
            }
        }
        return true;
    } catch(const std::exception& e) {
        std::cerr << "更新状态失败: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::shared_ptr<Classroom>> ClassroomBroker::findAll() {
    std::vector<std::shared_ptr<Classroom>> result;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT * FROM Classroom ORDER BY id");
        tx.commit();

        for(const auto& r : res) { // 使用 const auto& r 避免拷贝和引用错误
            auto c = std::make_shared<Classroom>(
                r["id"].as<std::string>(),
                r["building"].as<std::string>(),
                r["capacity"].as<int>()
            );
            c->setAvailable(r["is_available"].as<bool>());
            result.push_back(c);
        }
    } catch(const std::exception& e) {
        std::cerr << "查询所有教室失败: " << e.what() << std::endl;
    }
    return result;
}

bool ClassroomBroker::deleteClassroom(const std::string& rid) {
    if(!isClassroomExists(rid)) {
        std::cerr << "教室 " << rid << " 不存在" << std::endl;
        return false;
    }
    try {
        pqxx::work tx(*m_conn);
        tx.exec("DELETE FROM Classroom WHERE id=$1", pqxx::params{rid});
        tx.commit();

        // 清理缓存
        _classrooms.erase(
            std::remove_if(_classrooms.begin(), _classrooms.end(), [&](const auto& c){ return c->hasId(rid); }),
            _classrooms.end()
        );
        return true;
    } catch(const std::exception& e) {
        std::cerr << "删除教室失败: " << e.what() << std::endl;
        return false;
    }
}
bool ClassroomBroker::addCourseToSchedule(const std::string& rid, const std::string& cid, const std::string& time) {
    try {
        pqxx::work tx(*m_conn);

        tx.exec(
            "INSERT INTO ClassroomSchedule(classroom_id, course_id, time_slot) VALUES($1, $2, $3)",
            pqxx::params{rid, cid, time}
        );
        tx.commit();

        std::cout << "排课成功: 教室 " << rid << " 在 " << time << " 授课 " << cid << std::endl;
        return true;
    } catch (const pqxx::unique_violation&) {
        std::cout << ">> 排课冲突：教室 " << rid << " 在 " << time << " 已被占用！" << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "数据库错误：" << e.what() << std::endl;
        return false;
    }
}
