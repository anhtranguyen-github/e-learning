# Model Class Method Grouping

This document categorizes the methods of each model class into **DAO (Data Access/Accessors)**, **DTO (Data Transfer/Serialization)**, and **Utils (Utilities)**.

## ChatMessage

### DAO / Accessors
*   `getId()`, `setId(int)`
*   `getSenderId()`, `setSenderId(int)`
*   `getReceiverId()`, `setReceiverId(int)`
*   `getContent()`, `setContent(string)`
*   `getMessageType()`, `setMessageType(string)`
*   `getTimestamp()`, `setTimestamp(string)`
*   `getIsRead()`, `setIsRead(bool)`

### DTO / Serialization
*   *(None explicit - relies on external serialization or simple struct usage)*

### Utils
*   *(None)*

---

## Question

### DAO / Accessors
*   `getText()`, `setText(string)`
*   `getOptions()`, `setOptions(vector<string>)`
*   `getAnswer()`, `setAnswer(string)`
*   `getExplanation()`, `setExplanation(string)`
*   `getType()`, `setType(string)`

### DTO / Serialization
*   `toJsonString()`
*   `fromJson(Json::Value)`

### Utils
*   *(None)*

---

## Exam

### DAO / Accessors
*   `getExamId()`, `setExamId(int)`
*   `getLessonId()`, `setLessonId(int)`
*   `getTitle()`, `setTitle(string)`
*   `getType()`, `setType(string)`
*   `getLevel()`, `setLevel(string)`
*   `getQuestions()`, `setQuestions(vector<Question>)`

### DTO / Serialization
*   `serializeForNetwork(ExamType)`
*   `serializeMetadata()`
*   `toDTO()`
*   `toMetadataDTO()`

### Utils
*   *(None)*

---

## ExamList

### DAO / Accessors
*   `addExam(Exam)`
*   `getExams()`
*   `count()`
*   `clear()`

### DTO / Serialization
*   `serializeForNetwork()`

### Utils
*   `filter(string, string, int)`

---

## Exercise

### DAO / Accessors
*   `getExerciseId()`, `setExerciseId(int)`
*   `getLessonId()`, `setLessonId(int)`
*   `getTitle()`, `setTitle(string)`
*   `getType()`, `setType(string)`
*   `getLevel()`, `setLevel(string)`
*   `getQuestions()`, `setQuestions(vector<Question>)`
*   **Proxy Accessors:**
    *   `getQuestion()`, `setQuestion(string)`
    *   `getOptions()`, `setOptions(vector<string>)`
    *   `getAnswer()`, `setAnswer(string)`
    *   `getExplanation()`, `setExplanation(string)`

### DTO / Serialization
*   `serializeForNetwork(ExerciseType)`
*   `serializeMetadata()`
*   `toDTO()`
*   `toMetadataDTO()`

### Utils
*   *(None)*

---

## ExerciseList

### DAO / Accessors
*   `addExercise(Exercise)`
*   `getExercises()`
*   `count()`
*   `clear()`

### DTO / Serialization
*   `serializeForNetwork()`

### Utils
*   `filter(string, string, int)`

---

## Lesson

### DAO / Accessors
*   `getLessonId()`, `setLessonId(int)`
*   `getTitle()`, `setTitle(string)`
*   `getTopic()`, `setTopic(string)`
*   `getLevel()`, `setLevel(string)`
*   `getVideoUrl()`, `setVideoUrl(string)`
*   `getAudioUrl()`, `setAudioUrl(string)`
*   `getTextContent()`, `setTextContent(string)`
*   `getVocabulary()`, `setVocabulary(vector<string>)`
*   `getGrammar()`, `setGrammar(vector<string>)`

### DTO / Serialization
*   `serializeForNetwork(LessonType)`
*   `serializeMetadata()`
*   `toDTO()`
*   `toMetadataDTO()`

### Utils
*   *(None)*

---

## LessonList

### DAO / Accessors
*   `addLesson(Lesson)`
*   `getLessons()`
*   `count()`
*   `clear()`

### DTO / Serialization
*   `serializeForNetwork()`

### Utils
*   `filter(string, string)`

---

## User

### DAO / Accessors
*   `getId()`, `setId(int)`
*   `getUsername()`, `setUsername(string)`
*   `getPasswordHash()`, `setPasswordHash(string)`
*   `getFullName()`, `setFullName(string)`
*   `getRole()`, `setRole(string)`
*   `getLevel()`, `setLevel(string)`

### DTO / Serialization
*   *(None explicit)*

### Utils
*   *(None)*
