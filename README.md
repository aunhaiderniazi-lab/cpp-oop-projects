# C++ OOP Projects

Two real-world object-oriented projects in C++, with data saved to files so records stay after the program closes.

| Project | Folder | What it does |
|---|---|---|
| Smart Bank | `smart-bank/bank.cpp` | Accounts with PIN login, deposit, withdraw, transfer and month-end |
| Course Registration System | `course-registration/reg.cpp` | Students register for courses with prerequisite, seat and credit rules |

## Smart Bank

- Savings accounts earn 1% interest at month-end.
- Current accounts allow an Rs 5000 overdraft and charge a Rs 100 fee.
- Transfers between accounts and PIN-protected login.
- Data saved in `bank.txt`.

## Course Registration System

- Student and admin roles (admin password: `admin123`).
- Rules: prerequisites, seat limits and an 18-credit cap.
- Admin can add courses and mark courses completed.
- Data saved in `courses.txt` and `students.txt`.

## OOP Concepts Used

Abstract classes, inheritance, virtual functions and polymorphism, encapsulation, operator overloading, static members, exception handling, file streams and STL vectors.

## How to Run

```
g++ -std=c++11 smart-bank/bank.cpp -o bank && ./bank
g++ -std=c++11 course-registration/reg.cpp -o reg && ./reg
```

Use one-word names (use `_` for spaces) and enter numbers only at the menus.

## Author

Aun Haider Niazi - BS Information Engineering Technology, Foundation University Islamabad
