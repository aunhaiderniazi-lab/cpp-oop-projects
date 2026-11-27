// Author: Aun Haider Niazi
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
using namespace std;

struct EndOfInput {};   // signal graceful shutdown so destructors still save data

int readInt() {
    int v;
    while (!(cin >> v)) {
        if (cin.eof()) throw EndOfInput{};
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter a number: ";
    }
    return v;
}

string readWord() {
    string v;
    while (!(cin >> v)) {
        if (cin.eof()) throw EndOfInput{};
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter a value: ";
    }
    return v;
}

class Person {
protected:
    int id; string name;
public:
    Person(int i, string n) : id(i), name(n) {}
    virtual ~Person() {}
    int getId() const { return id; }
    string getName() const { return name; }
    virtual string role() const = 0;
};

class Course {
public:
    string code, name, prereq; int credits, cap, taken;
    Course(string c, string n, int cr, int cp, string p = "none", int t = 0)
        : code(c), name(n), prereq(p), credits(cr), cap(cp), taken(t) {}
    bool full() const { return taken >= cap; }
};

class Student : public Person {
public:
    static const int MAX = 18;
    vector<string> done, regd;
    Student(int i, string n) : Person(i, n) {}
    string role() const override { return "Student"; }
    bool has(const vector<string>& v, const string& c) const {
        return find(v.begin(), v.end(), c) != v.end();
    }
};

class University {
    vector<Course> cs; vector<Student> ss;
    Course* course(const string& c) { for (auto& x : cs) if (x.code == c) return &x; return nullptr; }
    Student* student(int id) { for (auto& s : ss) if (s.getId() == id) return &s; return nullptr; }
    int nextStudentId() const {
        int maxId = 2000;
        for (const auto& s : ss) if (s.getId() > maxId) maxId = s.getId();
        return maxId + 1;
    }
    // A stale or malformed students.txt must never cause a null dereference.
    int credits(const Student& s) {
        int t = 0;
        for (const auto& code : s.regd) {
            Course* x = course(code);
            if (x) t += x->credits;
        }
        return t;
    }
public:
    University() {
        ifstream f("courses.txt");
        string c, n, p; int cr, cp, t;
        while (f >> c >> n >> cr >> cp >> t >> p) {
            if (cr <= 0 || cp <= 0 || t < 0) continue;
            cs.emplace_back(c, n, cr, cp, p, t);
        }
        ifstream g("students.txt");
        int id, k; string sn;
        while (g >> id >> sn >> k) {
            if (id <= 0 || k < 0 || k > 1000) break;      // truncated/corrupt file
            Student s(id, sn);
            bool ok = true;
            for (int i = 0; i < k && ok; ++i) {
                string x; if (!(g >> x)) { ok = false; break; }
                s.done.push_back(x);
            }
            if (!ok) break;
            if (!(g >> k) || k < 0 || k > 1000) break;
            for (int i = 0; i < k && ok; ++i) {
                string x; if (!(g >> x)) { ok = false; break; }
                s.regd.push_back(x);
            }
            if (!ok) break;
            ss.push_back(s);
        }
        if (cs.empty()) {
            cs.emplace_back("CS101", "Programming", 3, 30);
            cs.emplace_back("CS102", "OOP", 3, 25, "CS101");
            cs.emplace_back("CS201", "DataStructures", 3, 25, "CS102");
            cs.emplace_back("CS301", "Database", 3, 20, "CS201");
            cs.emplace_back("MT101", "Calculus", 3, 40);
        }
    }

    ~University() {
        ofstream f("courses.txt", ios::trunc);
        if (!f) cerr << "Warning: could not write courses.txt; data was not saved.\n";
        for (auto& c : cs)
            if (f) f << c.code << " " << c.name << " " << c.credits << " "
                     << c.cap << " " << c.taken << " " << c.prereq << "\n";
        ofstream g("students.txt", ios::trunc);
        if (!g) cerr << "Warning: could not write students.txt; data was not saved.\n";
        for (auto& s : ss) {
            if (!g) break;
            g << s.getId() << " " << s.getName() << " " << s.done.size();
            for (auto& d : s.done) g << " " << d;
            g << " " << s.regd.size();
            for (auto& r : s.regd) g << " " << r;
            g << "\n";
        }
    }

    void list() {
        cout << "\n" << left << setw(8) << "Code" << setw(16) << "Name" << setw(5) << "Cr"
             << setw(10) << "Seats" << "Prereq\n";
        for (auto& c : cs)
            cout << setw(8) << c.code << setw(16) << c.name << setw(5) << c.credits
                 << setw(10) << (to_string(c.cap - c.taken) + "/" + to_string(c.cap)) << c.prereq << "\n";
    }

    void reg(Student& s) {
        list();
        cout << "Course code: ";
        string c = readWord();
        Course* x = course(c);
        if (!x) throw string("Course not found");
        if (s.has(s.regd, c)) throw string("Already registered");
        if (s.has(s.done, c)) throw string("Already completed");
        if (x->full()) throw string("Course is full");
        if (x->prereq != "none" && !s.has(s.done, x->prereq)) throw string("Prerequisite needed: " + x->prereq);
        if (credits(s) + x->credits > Student::MAX) throw string("Credit limit exceeded (max 18)");
        s.regd.push_back(c); x->taken++;
        cout << "Registered in " << x->name << "\n";
    }

    void drop(Student& s) {
        cout << "Course code to drop: ";
        string c = readWord();
        Course* x = course(c);
        if (!x) throw string("Course not found");
        auto it = find(s.regd.begin(), s.regd.end(), c);
        if (it == s.regd.end()) throw string("You are not registered in that course");
        s.regd.erase(it); x->taken--;
        cout << "Dropped " << c << "\n";
    }

    void schedule(Student& s) {
        cout << "\nStudent: " << s.getName() << " (#" << s.getId() << ")\nRegistered:";
        for (auto& c : s.regd) cout << " " << c;
        cout << "\nCompleted:";
        for (auto& c : s.done) cout << " " << c;
        cout << "\nCredits: " << credits(s) << "/" << Student::MAX << "\n";
    }

    void studentMenu(Student& s) {
        int c;
        do {
            cout << "\n1.Courses 2.Register 3.Drop 4.My schedule 5.Back\nChoice: ";
            c = readInt();
            try {
                if (c == 1) list();
                else if (c == 2) reg(s);
                else if (c == 3) drop(s);
                else if (c == 4) schedule(s);
            } catch (string& e) { cout << "Error: " << e << "\n"; }
        } while (c != 5);
    }

    void newStudent() {
        cout << "Name (one word): ";
        string n = readWord();
        int id = nextStudentId();
        ss.push_back(Student(id, n));
        cout << "Account created! Your ID: " << id << "\n";
    }

    void login() {
        cout << "Student ID: ";
        Student* s = student(readInt());
        if (!s) cout << "Student not found\n"; else studentMenu(*s);
    }

    void admin() {
        const char* expected = getenv("ADMIN_PASSWORD");
        if (!expected || !*expected) {
            cout << "Admin access is disabled. Set the ADMIN_PASSWORD environment variable.\n";
            return;
        }
        cout << "Admin password: ";
        string pw = readWord();
        if (pw != expected) { cout << "Wrong password\n"; return; }
        int c;
        do {
            cout << "\n1.Add course 2.Mark completed 3.View courses 4.Back\nChoice: ";
            c = readInt();
            try {
                if (c == 1) {
                    cout << "Code Name(one word) Credits Seats Prereq(or none): ";
                    string code = readWord(), n = readWord(), p = readWord();
                    int cr = readInt(), cp = readInt();
                    if (cr <= 0 || cp <= 0) throw string("Credits and seats must be positive");
                    if (course(code)) throw string("Course already exists");
                    cs.emplace_back(code, n, cr, cp, p);
                    cout << "Course added\n";
                } else if (c == 2) {
                    cout << "Student ID: ";
                    int id = readInt();
                    cout << "Course code: ";
                    string code = readWord();
                    Student* s = student(id);
                    if (!s) throw string("Student not found");
                    Course* x = course(code);
                    if (!x) throw string("Course not found");
                    auto it = find(s->regd.begin(), s->regd.end(), code);
                    if (it == s->regd.end()) throw string("Student is not registered in that course");
                    s->regd.erase(it); s->done.push_back(code); x->taken--;
                    cout << "Marked completed\n";
                } else if (c == 3) list();
            } catch (string& e) { cout << "Error: " << e << "\n"; }
        } while (c != 4);
    }
};

int main() {
    cout << "Course Registration System - by Aun Haider Niazi\n";
    try {
        University u;
        int c;
        do {
            cout << "\n=== COURSE REGISTRATION ===\n1.Student login 2.New student 3.Admin 4.Exit\nChoice: ";
            c = readInt();
            if (c == 1) u.login(); else if (c == 2) u.newStudent(); else if (c == 3) u.admin();
        } while (c != 4);
    } catch (const EndOfInput&) {
        cout << "\nEnd of input. Data saved.\n";
    }
    return 0;
}
