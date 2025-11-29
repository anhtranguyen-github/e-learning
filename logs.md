Timestamp,Direction,Protocol ID,Content / Payload
14:00:01,[RX],LOGIN_REQUEST (100),user=student1; pass=12345
14:00:02,[TX],LOGIN_SUCCESS (101),token=ABC-XYZ; role=student
14:00:10,[RX],LESSON_LIST_REQUEST (120),level=A1
14:00:11,[TX],LESSON_LIST_SUCCESS (121),"{""id"":1, ""title"":""Hello""}"
14:00:15,[RX],SUBMIT_ANSWER_REQUEST (250),q_id=50; answer=B



Timestamp,Level,Class / Source,Message
14:00:00,[INFO],[ServerMain],Server started on port 8080.
14:00:01,[INFO],[AuthController],Received login request from 192.168.1.5
14:00:01,[DBUG],[UserRepo],Query: SELECT * FROM users WHERE name='student1'
14:00:01,[INFO],[AuthController],Login successful. Sending token.
14:00:15,[ERR ],[ExamController],Invalid Question ID: 50 not found in DB.



Timestamp,Level,Component / Screen,Event / Message
14:00:00,[INFO],[MainWindow],App initialized.
14:00:05,[INFO],[LoginScreen],"User clicked ""Login"" button."
14:00:05,[INFO],[TcpSocket],Connecting to server... Connected.
14:00:10,[INFO],[LessonScreen],"User selected ""Level A1"" from dropdown."
14:00:15,[INFO],[ExamScreen],User finished timer. Submitting answer.



Timestamp,Protocol ID,Client Info,Status / RTT
14:00:05,HEARTBEAT (900),192.168.1.5,Alive (5ms)
14:00:10,HEARTBEAT (900),192.168.1.5,Alive (4ms)
14:00:15,HEARTBEAT (900),192.168.1.5,Alive (6ms)
14:05:00,DISCONNECT (901),192.168.1.5,Graceful Shutdown