#ifndef SERVER_MODEL_QUESTION_H
#define SERVER_MODEL_QUESTION_H

#include <string>
#include <vector>
#include <json/json.h>

namespace server {

class Question {
private:
    std::string text;
    std::vector<std::string> options;
    std::string answer;
    std::string explanation;
    std::string type; // For mixed types in exams

public:
    Question() = default;
    Question(const std::string& t, const std::vector<std::string>& o, const std::string& a, const std::string& e = "", const std::string& ty = "")
        : text(t), options(o), answer(a), explanation(e), type(ty) {}

    // Getters
    std::string getText() const { return text; }
    std::vector<std::string> getOptions() const { return options; }
    std::string getAnswer() const { return answer; }
    std::string getExplanation() const { return explanation; }
    std::string getType() const { return type; }

    // Setters
    void setText(const std::string& t) { text = t; }
    void setOptions(const std::vector<std::string>& o) { options = o; }
    void setAnswer(const std::string& a) { answer = a; }
    void setExplanation(const std::string& e) { explanation = e; }
    void setType(const std::string& t) { type = t; }

    // Serialization helper (to JSON string)
    std::string toJsonString() const {
        Json::Value root;
        root["text"] = text;
        root["type"] = type;
        root["answer"] = answer;
        root["explanation"] = explanation;
        
        Json::Value opts(Json::arrayValue);
        for (const auto& opt : options) {
            opts.append(opt);
        }
        root["options"] = opts;

        Json::FastWriter writer;
        return writer.write(root);
    }

    // Deserialization helper
    static Question fromJson(const Json::Value& val) {
        Question q;
        q.setText(val.get("text", "").asString());
        // Support "question" key as fallback
        if (q.getText().empty()) q.setText(val.get("question", "").asString());
        
        q.setType(val.get("type", "").asString());
        q.setAnswer(val.get("answer", "").asString());
        q.setExplanation(val.get("explanation", "").asString());
        
        if (val.isMember("options") && val["options"].isArray()) {
            std::vector<std::string> opts;
            for (const auto& opt : val["options"]) {
                opts.push_back(opt.asString());
            }
            q.setOptions(opts);
        }
        return q;
    }
};

} // namespace server

#endif // SERVER_MODEL_QUESTION_H
