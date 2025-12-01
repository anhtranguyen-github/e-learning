# Naming Convention Unification Plan

## Objective
Standardize method names across all model classes to ensure consistency, readability, and predictability. This plan outlines the proposed conventions and the specific changes required for each class.

## 1. Proposed Conventions

### Accessors (DAO)
*   **Getters**: `get<Property>()` (CamelCase)
*   **Setters**: `set<Property>(<Type> value)`
*   **Boolean Accessors**: `is<Property>()` (e.g., `isRead()`) instead of `getIs<Property>()`.

### Serialization / DTOs
*   **To DTO**: `toDto()` (CamelCase, 'Dto' instead of 'DTO').
*   **To Metadata**: `toMetadataDto()`.
*   **JSON Conversion**: `toJson()` (instead of `toJsonString`).
*   **Network Serialization**: `serialize()` (if default) or `toNetworkString()` (instead of `serializeForNetwork`).
*   **From JSON**: `fromJson(Json::Value)` (Static factory or method).

### Collection / List Classes
*   **Add Item**: `add(<ItemType> item)` (Generic `add` is preferred if the container type is explicit, e.g., `ExamList::add`).
*   **Get All Items**: `getAll()` (Standardized accessor for the main list).
*   **Count**: `count()` (Keep as is).
*   **Clear**: `clear()` (Keep as is).
*   **Filter**: `filter(...)` (Keep as is).

---

## 2. Proposed Changes by Class

### ChatMessage
*   **Rename**: `getIsRead()` $\rightarrow$ `isRead()`
*   **Add**: `toDto()` (Missing serialization)
*   **Add**: `toJson()` (Missing serialization)

### Question
*   **Rename**: `toJsonString()` $\rightarrow$ `toJson()`

### Exam / Exercise / Lesson
*   **Rename**: `serializeForNetwork(Type)` $\rightarrow$ `serialize(Type)`
*   **Rename**: `toDTO()` $\rightarrow$ `toDto()`
*   **Rename**: `toMetadataDTO()` $\rightarrow$ `toMetadataDto()`

### ExamList / ExerciseList / LessonList
*   **Rename**: `addExam(Exam)` $\rightarrow$ `add(Exam)`
*   **Rename**: `addExercise(Exercise)` $\rightarrow$ `add(Exercise)`
*   **Rename**: `addLesson(Lesson)` $\rightarrow$ `add(Lesson)`
*   **Rename**: `getExams()` $\rightarrow$ `getAll()`
*   **Rename**: `getExercises()` $\rightarrow$ `getAll()`
*   **Rename**: `getLessons()` $\rightarrow$ `getAll()`
*   **Rename**: `serializeForNetwork()` $\rightarrow$ `serialize()`

### User
*   **Add**: `toDto()` (Missing serialization)
*   **Add**: `toJson()` (Missing serialization)

---

## 3. Summary of Impact
*   **Consistency**: All boolean getters will use `is...`. All serialization will use `to...` or `serialize`.
*   **Polymorphism**: List classes will share a common interface (`add`, `getAll`, `count`, `clear`), making it easier to template or refactor later.
*   **Readability**: Shorter, cleaner names (`toJson` vs `toJsonString`).
