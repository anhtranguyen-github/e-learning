import json
import random

def escape_sql(s):
    if s is None:
        return "NULL"
    return "'" + s.replace("'", "''") + "'"

def to_json_sql(data):
    return escape_sql(json.dumps(data))

users = []
lessons = []
exercises = []
exams = []

# Generate Users
for i in range(1, 6):
    users.append(f"INSERT INTO users (username, password_hash, full_name, role, level) VALUES ('user{i}', 'hash{i}', 'User {i}', 'student', 'beginner');")
users.append("INSERT INTO users (username, password_hash, full_name, role, level) VALUES ('teacher1', 'hash_teacher', 'Teacher One', 'teacher', 'advanced');")

# Generate Lessons
topics = ["Greetings", "Travel", "Food", "Business", "Technology"]
levels = ["beginner", "intermediate", "advanced"]

for i in range(1, 21):
    topic = random.choice(topics)
    level = random.choice(levels)
    title = f"{topic} Lesson {i}"
    
    vocab = [f"word{j}_{i}" for j in range(5)]
    grammar = [f"rule{j}_{i}" for j in range(3)]
    
    video_url = f"http://example.com/video_{i}.mp4" if random.random() > 0.3 else None
    audio_url = f"http://example.com/audio_{i}.mp3" if random.random() > 0.3 else None
    text_content = f"This is the reading text for lesson {i} about {topic}. It is very interesting."
    
    lessons.append(f"INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar, created_by) VALUES ({escape_sql(title)}, {escape_sql(topic)}, {escape_sql(level)}, {escape_sql(video_url)}, {escape_sql(audio_url)}, {escape_sql(text_content)}, {to_json_sql(vocab)}, {to_json_sql(grammar)}, 6);")

# Generate Exercises
exercise_types = ["rewrite_sentence", "essay", "speaking"]
for i in range(1, 31):
    lesson_id = random.randint(1, 20)
    ex_type = random.choice(exercise_types)
    title = f"Exercise {i} - {ex_type}"
    level = "beginner" # simplified
    
    question = ""
    options = None
    answer = ""
    
    if ex_type == "rewrite_sentence":
        question = "Rewrite the following sentence in past tense: 'I go to the market.'"
        answer = "I went to the market."
    elif ex_type == "essay":
        question = "Write a short paragraph about your daily routine."
        answer = None # Subjective
    elif ex_type == "speaking":
        question = "Describe the image shown below."
        answer = None # Subjective
        
    exercises.append(f"INSERT INTO exercises (lesson_id, title, type, level, options, question, answer, created_by) VALUES ({lesson_id}, {escape_sql(title)}, {escape_sql(ex_type)}, {escape_sql(level)}, {to_json_sql(options) if options else 'NULL'}, {escape_sql(question)}, {escape_sql(answer)}, 6);")

# Generate Exams
# We will embed JSON strings into the questions array to support rich types
exam_types = ["multiple_choice", "fill_blank", "order_sentence"]

for i in range(1, 11):
    lesson_id = random.randint(1, 20)
    ex_type = random.choice(exam_types)
    title = f"Exam {i} - {ex_type}"
    level = "intermediate"
    
    questions_data = []
    
    if ex_type == "multiple_choice":
        for q in range(3):
            q_obj = {
                "type": "multiple_choice",
                "text": f"Question {q+1}: What is the capital of France?",
                "options": ["Paris", "London", "Berlin", "Madrid"],
                "answer": "Paris"
            }
            questions_data.append(json.dumps(q_obj))
    elif ex_type == "fill_blank":
         for q in range(3):
            q_obj = {
                "type": "fill_blank",
                "text": f"Question {q+1}: The sky is _____.",
                "answer": "blue"
            }
            questions_data.append(json.dumps(q_obj))
    elif ex_type == "order_sentence":
         for q in range(3):
            q_obj = {
                "type": "order_sentence",
                "text": f"Question {q+1}: Arrange: is / name / My / John",
                "answer": "My name is John"
            }
            questions_data.append(json.dumps(q_obj))
            
    exams.append(f"INSERT INTO exams (lesson_id, title, type, level, question, created_by) VALUES ({lesson_id}, {escape_sql(title)}, {escape_sql(ex_type)}, {escape_sql(level)}, {to_json_sql(questions_data)}, 6);")

# Write to file
with open('database/seed_generated.sql', 'w') as f:
    f.write("-- Generated Seed Data\n")
    f.write("BEGIN;\n")
    f.write("TRUNCATE users, lessons, exercises, exams, results CASCADE;\n")
    for sql in users:
        f.write(sql + "\n")
    for sql in lessons:
        f.write(sql + "\n")
    for sql in exercises:
        f.write(sql + "\n")
    for sql in exams:
        f.write(sql + "\n")
    f.write("COMMIT;\n")

print("Generated database/seed_generated.sql")
