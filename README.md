# Smart Bus Conductor Attendance and Automated Gate Management System

TO GET THE ID OF THE RFID CARD USE THE GET.ino FILE IN CODE FOLDER 
IN 43 AROUND THIS LINE CHANGE WITH YOUR ID OF CARD IN CONDUCTORS








The Smart Bus Conductor Attendance and Automated Gate Management System is an IoT-based solution designed to modernize attendance tracking and entry management in bus depots and transport facilities. The system combines RFID technology, wireless networking, automated gate control, and digital record management to create a reliable, efficient, and user-friendly attendance platform.

Traditional attendance systems often rely on manual registers, signatures, or verbal reporting, which can lead to inaccuracies, delays, manipulation of records, and additional administrative workload. This project eliminates these challenges by automatically identifying conductors through RFID cards, recording attendance in real time, calculating late-reporting penalties, and providing supervisors with a convenient web-based dashboard.

At the heart of the system is an ESP8266 NodeMCU microcontroller that coordinates all hardware and software operations. Each conductor is issued a unique RFID card that acts as a digital identity. When the conductor arrives at the depot, the RFID card is placed near the RC522 RFID reader. The system instantly reads the card's unique UID (Unique Identification Number) and compares it with the database of registered conductors stored in memory.

If the card matches an authorized conductor, the system records the attendance and determines whether the conductor has reported on time. A predefined reporting time is set by the supervisor through a wireless web interface. The system also allows a configurable grace period. If the conductor arrives after the allowed reporting window, the attendance status is marked as "Late" and a fine is automatically added to the conductor's record. If the conductor arrives within the permitted time, the attendance status is marked as "On Time."

Once a valid attendance entry is recorded, a servo motor operates an automated gate mechanism, allowing the conductor to enter. Simultaneously, an OLED display presents important information such as the conductor's name, assigned bus number, arrival time, and attendance status. This provides immediate visual confirmation and improves transparency during the attendance process.

The ESP8266 creates its own Wi-Fi hotspot, allowing supervisors to connect using any smartphone, tablet, or laptop without requiring an internet connection. Through the built-in web dashboard, administrators can set reporting times, monitor attendance records, view accumulated fines, manually open the gate when required, and download attendance reports in CSV format for further analysis and record keeping.

The system also includes security measures to prevent unauthorized access. If an unregistered RFID card is scanned, the system immediately identifies it as invalid, displays an error message, and denies gate access. This ensures that only authorized personnel can enter the facility.

A key advantage of this project is its ability to maintain digital attendance records automatically. The generated reports can be opened in spreadsheet software such as Microsoft Excel or Google Sheets, enabling supervisors to review attendance history, identify recurring late arrivals, and simplify administrative reporting.

The project demonstrates the practical application of embedded systems, RFID technology, wireless communication, automation, and web-based monitoring in a real-world transportation environment. Its modular design allows it to be expanded in the future with features such as cloud storage, SMS notifications, biometric verification, GPS integration, real-time clock modules, or centralized fleet management systems.

Overall, the Smart Bus Conductor Attendance and Automated Gate Management System provides a cost-effective, efficient, and scalable solution for attendance management, access control, punctuality monitoring, and digital record keeping. By reducing manual intervention and automating routine processes, the system improves operational efficiency, enhances accountability, and contributes to the digital transformation of public and private transport management.
