// Author: Aun Haider Niazi
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdlib>
using namespace std;

int readInt() {                                  // safe number input
    int v;
    while (!(cin >> v)) {
        if (cin.eof()) exit(0);
        cin.clear(); cin.ignore(10000, '\n');
        cout << "Enter a number: ";
    }
    return v;
}
double readDbl() {
    double v;
    while (!(cin >> v)) {
        if (cin.eof()) exit(0);
        cin.clear(); cin.ignore(10000, '\n');
        cout << "Enter a number: ";
    }
    return v;
}

class Account {
protected:
    int id, pin; string name; double bal;
    static int count;                       // static member
public:
    Account(int i, string n, int p, double b) : id(i), pin(p), name(n), bal(b) { count++; }
    virtual ~Account() {}
    int getId() const { return id; }
    bool check(int p) const { return pin == p; }
    void deposit(double a) {
        if (a <= 0) throw string("Invalid amount");
        bal += a;
    }
    virtual void withdraw(double a) {
        if (a <= 0) throw string("Invalid amount");
        if (a > bal) throw string("Insufficient funds");
        bal -= a;
    }
    virtual string type() const = 0;        // abstract class
    virtual void monthEnd() = 0;
    void show() const {
        cout << "\n[" << type() << "] #" << id << " " << name
             << " | Balance: Rs " << fixed << setprecision(2) << bal << "\n";
    }
    string save() const {
        return type() + " " + to_string(id) + " " + name + " " + to_string(pin) + " " + to_string(bal);
    }
    Account& operator+=(double a) { deposit(a); return *this; }   // operator overloading
    static int total() { return count; }
};
int Account::count = 0;

class Savings : public Account {            // inheritance
public:
    using Account::Account;
    string type() const override { return "Savings"; }
    void monthEnd() override { bal += bal * 0.01; }               // 1% interest
};

class Current : public Account {
public:
    using Account::Account;
    string type() const override { return "Current"; }
    void withdraw(double a) override {                            // polymorphism
        if (a <= 0) throw string("Invalid amount");
        if (a > bal + 5000) throw string("Overdraft limit (Rs 5000) exceeded");
        bal -= a;
    }
    void monthEnd() override { bal -= 100; }                      // Rs 100 fee
};

class Bank {
    vector<Account*> v;
    Account* find(int id) { for (auto a : v) if (a->getId() == id) return a; return nullptr; }
public:
    Bank() {                                                      // file handling
        ifstream f("bank.txt");
        string t, n; int id, p; double b;
        while (f >> t >> id >> n >> p >> b)
            v.push_back(t == "Savings" ? (Account*)new Savings(id, n, p, b) : new Current(id, n, p, b));
    }
    ~Bank() {
        ofstream f("bank.txt");
        for (auto a : v) { f << a->save() << "\n"; delete a; }
    }
    void open() {
        string n; int t, p; double b;
        cout << "Name (one word): "; cin >> n;
        cout << "Type (1=Savings, 2=Current): "; t = readInt();
        if (t != 1 && t != 2) { cout << "Invalid type\n"; return; }
        cout << "4-digit PIN: "; p = readInt();
        if (p < 1000 || p > 9999) { cout << "PIN must be 4 digits\n"; return; }
        cout << "Opening deposit: "; b = readDbl();
        if (b < 0) { cout << "Deposit cannot be negative\n"; return; }
        int id = 1001 + v.size();
        v.push_back(t == 1 ? (Account*)new Savings(id, n, p, b) : new Current(id, n, p, b));
        cout << "Account created! Your ID: " << id << "\n";
    }
    void login() {
        int id, p, c;
        cout << "Account ID: "; id = readInt(); cout << "PIN: "; p = readInt();
        Account* a = find(id);
        if (!a || !a->check(p)) { cout << "Wrong ID or PIN\n"; return; }
        do {
            cout << "\n1.Balance 2.Deposit 3.Withdraw 4.Transfer 5.Month-end 6.Logout\nChoice: ";
            c = readInt();
            try {                                                 // exception handling
                double amt;
                if (c == 1) a->show();
                else if (c == 2) { cout << "Amount: "; amt = readDbl(); *a += amt; a->show(); }
                else if (c == 3) { cout << "Amount: "; amt = readDbl(); a->withdraw(amt); a->show(); }
                else if (c == 4) {
                    int to; cout << "To ID: "; to = readInt(); cout << "Amount: "; amt = readDbl();
                    Account* r = find(to);
                    if (!r || r == a) throw string("Invalid receiver");
                    a->withdraw(amt); r->deposit(amt);
                    cout << "Transferred Rs " << amt << "\n";
                }
                else if (c == 5) { a->monthEnd(); a->show(); }
            } catch (string& e) { cout << "Error: " << e << "\n"; }
        } while (c != 6);
    }
};

int main() {
    cout << "Smart Bank - by Aun Haider Niazi\n";
    Bank b; int c;
    do {
        cout << "\n=== SMART BANK ===\n1.Open Account 2.Login 3.Exit\nChoice: ";
        c = readInt();
        if (c == 1) b.open(); else if (c == 2) b.login();
    } while (c != 3);
    cout << "Total accounts: " << Account::total() << "\n";
}
