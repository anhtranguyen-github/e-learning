-- =====================================
-- SAMPLE LESSON DATA
-- =====================================
-- This file contains sample lesson data for testing the lesson management features

-- Note: Make sure the users table has at least one user before running this
-- The created_by field references users(user_id)

-- =====================================
-- Beginner Level Lessons
-- =====================================

INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar)
VALUES 
(
    'English Alphabet Basics',
    'basics',
    'beginner',
    'https://example.com/videos/alphabet.mp4',
    'https://example.com/audio/alphabet.mp3',
    'Welcome to English! The English alphabet has 26 letters: A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z. Practice saying each letter clearly.',
    '["alphabet", "letter", "vowel", "consonant", "pronunciation"]'::jsonb,
    '["There are 26 letters in English", "5 vowels: A, E, I, O, U", "21 consonants"]'::jsonb
),

(
    'Basic Greetings',
    'conversation',
    'beginner',
    'https://example.com/videos/greetings.mp4',
    'https://example.com/audio/greetings.mp3',
    'Learning how to greet people is essential. Common greetings include: Hello, Hi, Good morning, Good afternoon, Good evening. To ask how someone is doing, say "How are you?" and respond with "I am fine, thank you."',
    '["hello", "hi", "good morning", "good afternoon", "good evening", "how are you", "thank you"]'::jsonb,
    '["Use Good morning before noon", "Use Good afternoon from noon to 6pm", "Use Good evening after 6pm"]'::jsonb
),

(
    'Simple Present Tense',
    'grammar',
    'beginner',
    'https://example.com/videos/present-tense.mp4',
    'https://example.com/audio/present-tense.mp3',
    'The simple present tense describes habits, general facts, and routines. Form: Subject + Verb (base form). Examples: I eat breakfast. She reads books. They play soccer.',
    '["eat", "drink", "read", "write", "play", "study", "work", "sleep"]'::jsonb,
    '["Add -s or -es for he/she/it", "I/You/We/They + base verb", "He/She/It + verb-s", "Example: I play, He plays"]'::jsonb
),

(
    'Common Nouns',
    'vocabulary',
    'beginner',
    'https://example.com/videos/nouns.mp4',
    'https://example.com/audio/nouns.mp3',
    'Nouns are words that name people, places, things, or ideas. Common nouns include everyday objects you see around you.',
    '["apple", "book", "cat", "dog", "house", "car", "tree", "water", "school", "friend"]'::jsonb,
    '["Nouns name people, places, or things", "Capitalize proper nouns", "Use a/an before singular nouns"]'::jsonb
);

-- =====================================
-- Intermediate Level Lessons
-- =====================================

INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar)
VALUES 
(
    'Past Tense Verbs',
    'grammar',
    'intermediate',
    'https://example.com/videos/past-tense.mp4',
    'https://example.com/audio/past-tense.mp3',
    'The simple past tense describes completed actions in the past. Regular verbs add -ed: walk → walked, play → played. Irregular verbs have special forms: go → went, eat → ate, see → saw.',
    '["yesterday", "last week", "ago", "before", "then", "previously"]'::jsonb,
    '["Regular verbs: verb + ed", "Irregular verbs: special forms", "Time markers: yesterday, last week, ago"]'::jsonb
),

(
    'Making Suggestions',
    'conversation',
    'intermediate',
    'https://example.com/videos/suggestions.mp4',
    'https://example.com/audio/suggestions.mp3',
    'Learn how to make polite suggestions in English. Use phrases like: "Why don''t we...?", "How about...?", "Let''s...", "Should we...?", "What if we...?" Example: "Why don''t we go to the park?"',
    '["suggestion", "recommend", "propose", "consider", "perhaps", "maybe"]'::jsonb,
    '["Why don''t we + verb", "How about + verb-ing", "Let''s + verb", "Should we + verb"]'::jsonb
),

(
    'Business Vocabulary',
    'vocabulary',
    'intermediate',
    'https://example.com/videos/business.mp4',
    'https://example.com/audio/business.mp3',
    'Essential business vocabulary for professional settings. Learn words related to meetings, presentations, and workplace communication.',
    '["meeting", "presentation", "deadline", "project", "client", "colleague", "manager", "schedule", "report", "conference"]'::jsonb,
    '["Use formal language in business settings", "Common phrase: I am writing to...", "Professional greetings"]'::jsonb
),

(
    'Conditional Sentences',
    'grammar',
    'intermediate',
    'https://example.com/videos/conditionals.mp4',
    'https://example.com/audio/conditionals.mp3',
    'Conditional sentences express situations and their results. First conditional: If + present simple, will + verb. Example: If it rains, I will stay home. Second conditional: If + past simple, would + verb.',
    '["if", "when", "unless", "provided", "suppose", "condition"]'::jsonb,
    '["First conditional: real possibilities", "If + present, will + verb", "Second conditional: imaginary situations", "If + past, would + verb"]'::jsonb
);

-- =====================================
-- Advanced Level Lessons
-- =====================================

INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar)
VALUES 
(
    'Advanced Academic Writing',
    'writing',
    'advanced',
    'https://example.com/videos/academic-writing.mp4',
    'https://example.com/audio/academic-writing.mp3',
    'Academic writing requires formal language, clear structure, and evidence-based arguments. Key elements include: thesis statement, topic sentences, supporting evidence, citation of sources, and a strong conclusion.',
    '["thesis", "argument", "evidence", "citation", "methodology", "hypothesis", "conclusion", "analysis", "critique", "discourse"]'::jsonb,
    '["Use passive voice for objectivity", "Avoid contractions", "Third person perspective", "Formal vocabulary choices"]'::jsonb
),

(
    'Idiomatic Expressions',
    'vocabulary',
    'advanced',
    'https://example.com/videos/idioms.mp4',
    'https://example.com/audio/idioms.mp3',
    'Idioms are expressions that have meanings different from their literal words. Understanding idioms is crucial for natural English conversation.',
    '["piece of cake", "break the ice", "hit the nail on the head", "cost an arm and a leg", "blessing in disguise", "call it a day"]'::jsonb,
    '["Idioms cannot be translated word-by-word", "Context determines meaning", "Common in informal speech"]'::jsonb
),

(
    'Perfect Tenses and Aspects',
    'grammar',
    'advanced',
    'https://example.com/videos/perfect-tenses.mp4',
    'https://example.com/audio/perfect-tenses.mp3',
    'Master the perfect tenses: Present Perfect (have/has + past participle) for actions continuing to present or with present relevance. Past Perfect (had + past participle) for actions before another past action. Future Perfect (will have + past participle) for actions that will be completed by a future time.',
    '["already", "yet", "just", "ever", "never", "recently", "lately", "so far"]'::jsonb,
    '["Present Perfect: have/has + past participle", "Past Perfect: had + past participle", "Future Perfect: will have + past participle", "Use with time expressions"]'::jsonb
),

(
    'Debate and Persuasion',
    'speaking',
    'advanced',
    'https://example.com/videos/debate.mp4',
    'https://example.com/audio/debate.mp3',
    'Learn techniques for effective debating and persuasive speaking. Structure arguments logically, support claims with evidence, anticipate counterarguments, and use rhetorical devices effectively.',
    '["argument", "rebuttal", "evidence", "counterargument", "persuade", "convince", "rhetoric", "logic", "fallacy", "concession"]'::jsonb,
    '["State your position clearly", "Support with evidence", "Address counterarguments", "Use rhetorical questions", "Conclude with a strong statement"]'::jsonb
);

-- =====================================
-- Mixed Topics
-- =====================================

INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar)
VALUES 
(
    'Travel English',
    'practical',
    'intermediate',
    'https://example.com/videos/travel.mp4',
    'https://example.com/audio/travel.mp3',
    'Essential English for traveling. Learn how to ask for directions, order food, book hotels, and handle common travel situations.',
    '["airport", "hotel", "reservation", "ticket", "luggage", "customs", "passport", "departure", "arrival", "destination"]'::jsonb,
    '["Polite requests: Could you...?", "Modal verbs: can, could, would", "Question formation"]'::jsonb
),

(
    'Food and Cooking Vocabulary',
    'vocabulary',
    'beginner',
    'https://example.com/videos/food.mp4',
    'https://example.com/audio/food.mp3',
    'Learn vocabulary related to food, cooking, and dining. This lesson covers common foods, cooking methods, and restaurant phrases.',
    '["bread", "rice", "vegetables", "fruit", "meat", "fish", "milk", "cheese", "breakfast", "lunch", "dinner"]'::jsonb,
    '["Countable vs uncountable nouns", "Use some/any with food", "How much/How many"]'::jsonb
);

-- =====================================
-- Verification Query
-- =====================================

-- Run this to verify lessons were inserted:
SELECT 
    lesson_id,
    title,
    topic,
    level,
    CASE 
        WHEN vocabulary IS NOT NULL THEN jsonb_array_length(vocabulary)
        ELSE 0 
    END as vocab_count,
    CASE 
        WHEN grammar IS NOT NULL THEN jsonb_array_length(grammar)
        ELSE 0 
    END as grammar_count
FROM lessons
ORDER BY level, lesson_id;
