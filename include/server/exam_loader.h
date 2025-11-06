#ifndef EXAM_LOADER_H
#define EXAM_LOADER_H

#include "db_config.h"
#include <vector>
#include <string>

#include "server/database.h"
#include <memory>

namespace server
{
    struct Exam
    {
        int id;
        std::string title;
        std::string type;
        std::string level;
        std::string question;
    };

    class ExamLoader
    {
    public:
        ExamLoader() = default;
        ExamLoader(std::shared_ptr<Database> db);
        std::vector<Exam> loadAllExams();

    private:
        std::shared_ptr<Database> db;
    };
}
#endif // EXAM_LOADER_H
