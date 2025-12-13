# 🎮 Game Use Cases Specification

This document defines system-level use cases for three mini-games:

* Sentence Match
* Picture Match
* Word Match

The use cases are designed to be reusable, scalable, and backend-driven.

---

## 1. Common Use Cases (All Games)

### UC-00: View Game List

* **Actor:** Player
* **Description:** Player views all available games
* **Outcome:** Game catalog displayed

---

### UC-01: View Levels

* **Actor:** Player
* **Description:** Player views available levels of a selected game
* **Precondition:** Game exists
* **Outcome:** Level list with status (locked / unlocked / completed)

---

### UC-02: Select Level

* **Actor:** Player
* **Description:** Player selects a level to play
* **Precondition:** Level is unlocked
* **Outcome:** Level data is loaded

---

### UC-03: Play Level

* **Actor:** Player
* **Description:** Player interacts with game mechanics
* **Outcome:** Level completed or exited

---

### UC-04: Submit Result

* **Actor:** Player
* **Description:** Player submits gameplay result
* **Outcome:** Server validates data

---

### UC-05: View Result

* **Actor:** Player
* **Description:** Player receives score and feedback
* **Outcome:** Score displayed

---

### UC-06: Sync Progress

* **Actor:** System
* **Description:** System updates progress and unlocks new levels
* **Outcome:** Progress persisted

---

## 2. Sentence Match – Use Cases

### UC-S1: Load Sentence Level

* Load sentence fragments and choices

### UC-S2: Match Sentence Parts

* Player matches sentence segments correctly

### UC-S3: Validate Sentence Structure

* Server validates grammatical correctness

### UC-S4: Request Hint (Optional)

* Player requests hint with score penalty

### UC-S5: Sentence Scoring

* Score based on accuracy and time

---

## 3. Picture Match – Use Cases

### UC-P1: Load Picture Level

* Load images and matching labels

### UC-P2: Match Picture to Word

* Player matches image with correct word

### UC-P3: Time-Based Challenge

* Player completes matching within time limit

### UC-P4: Validate Image Matching

* Server validates image–word mapping

### UC-P5: Picture Scoring

* Score based on speed and accuracy

---

## 4. Word Match – Use Cases

### UC-W1: Load Word Level

* Load words and meanings

### UC-W2: Match Word Pairs

* Player matches word to meaning

### UC-W3: Difficulty Scaling

* Difficulty increases per level

### UC-W4: Limited Attempts

* Player has limited mistakes allowed

### UC-W5: Word Scoring

* Score based on correctness and attempts

---

## 5. Cross-Game System Use Cases

### UC-X1: Anti-Cheat Validation

* Server validates gameplay integrity

### UC-X2: Resume Game

* Player resumes unfinished session

### UC-X3: Leaderboard Update (Optional)

* System updates rankings

### UC-X4: Error Handling

* System handles invalid requests and timeouts

---

## 6. Summary

| Game           | Core Skill  | Focus               |
| -------------- | ----------- | ------------------- |
| Sentence Match | Grammar     | Structure & syntax  |
| Picture Match  | Recognition | Speed & association |
| Word Match     | Vocabulary  | Memory & accuracy   |

---
