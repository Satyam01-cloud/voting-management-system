# Voting Management System

A secure, lightweight web-based Voting Management System built with a **C++ backend** (using `httplib`), a **flat-file database** architecture, and a clean HTML/CSS/JavaScript frontend.

## 🚀 Features
* **User Authentication**: Secure voter registration and login system backed by flat-file storage (`voters.txt`).
* **Election & Candidate Management**: Dynamic loading of active elections and candidate profiles.
* **Vote Casting & Validation**: Prevents double-voting and safely logs votes with timestamps (`votes.txt`).
* **High Performance C++ Backend**: Powered by `cpp-httplib` with prioritized API route handling and static asset serving.

## 🛠️ Tech Stack
* **Backend**: C++17, `cpp-httplib`, Winsock2
* **Frontend**: HTML5, CSS3, JavaScript (Fetch API)
* **Storage**: Flat-file text database (`.txt`)

## 📂 Project Structure
```text
VOTING-MANAGEMENT-SYSTEM/
│
├── backend/
│   ├── main.cpp         # C++ server & API route controllers
│   └── httplib.h        # C++ HTTP library header
│
├── database/
│   ├── database.h       # Database handling logic & models
│   ├── candidates.txt   # Candidate records
│   ├── elections.txt    # Election details
│   └── votes.txt        # Cast vote logs
│
└── frontend/
    ├── index.html       # Login & Registration page
    ├── dashboard.html   # Voter dashboard
    ├── script.js        # Frontend API integration
    └── style.css        # UI styling
