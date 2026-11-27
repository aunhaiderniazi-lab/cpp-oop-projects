// Author: Aun Haider Niazi
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
using namespace std;

class Account {
protected:
    int id, pin;
    string name;
    double bal;
    static int count;
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

    virtual string type() const = 0;
    virtual void monthEnd() = 0;

    void show() const {
        cout << "\n[" << type() << "] #" << id << " " << name
             << " | Balance: Rs " << fixed << setprecision(2) << bal << "\n";
    }

    string save() const {
        return type() + " " + to_string(id) + " " + name + " " + to_string(pin) + " " + to_string(bal);
    }

    Account& operator+=(double a) {
        deposit(a);
        return *this;
    }

    static int total() { return count; }
};

int Account::count = 0;

class Savings : public Account {
public:
    using Account::Account;
    string type() const override { return "Savings"; }
    void monthEnd() override { bal += bal * 0.01; } // 1% monthly interest
};

class Current : public Account {
public:
    using Account::Account;
    string type() const override { return "Current"; }

    void withdraw(double a) override {
        if (a <= 0) throw string("Invalid amount");
        if (a > bal + 5000) throw string("Overdraft limit (Rs 5000) exceeded");
        bal -= a;
    }

    void monthEnd() override { bal -= 100; } // Rs 100 maintenance fee
};

class Bank {
    vector<Account*> v;

    Account* find(int id) {
        for (auto a : v) if (a->getId() == id) return a;
        return nullptr;
    }

    int readInt(const string& prompt) {
        int val;
        while (true) {
            cout << prompt;
            if (cin >> val) return val;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a valid number.\n";
        }
    }

    double readDouble(const string& prompt) {
        double val;
        while (true) {
            cout << prompt;
            if (cin >> val) return val;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a valid amount.\n";
        }
    }

public:
    Bank() {
        ifstream f("bank.txt");
        string t, n;
        int id, p;
        double b;
        while (f >> t >> id >> n >> p >> b) {
            if (t == "Savings") v.push_back(new Savings(id, n, p, b));
            else v.push_back(new Current(id, n, p, b));
        }
    }

    ~Bank() {
        ofstream f("bank.txt");
        for (auto a : v) {
            f << a->save() << "\n";
            delete a;
        }
    }

    void open() {
        string n, t;
        cout << "Name (one word): ";
        cin >> n;
        cout << "Type (1=Savings, 2=Current): ";
        cin >> t;
        int p = readInt("4-digit PIN: ");
        double b = readDouble("Opening deposit: ");

        int id = 1001 + static_cast<int>(v.size());
        if (t == "1") v.push_back(new Savings(id, n, p, b));
        else v.push_back(new Current(id, n, p, b));
        cout << "Account created! Your ID: " << id << "\n";
    }

    void login() {
        int id = readInt("Account ID: ");
        int p = readInt("PIN: ");
        Account* a = find(id);
        if (!a || !a->check(p)) {
            cout << "Wrong ID or PIN\n";
            return;
        }

        int c;
        do {
            c = readInt("\n1.Balance 2.Deposit 3.Withdraw 4.Transfer 5.Month-end 6.Logout\nChoice: ");
            try {
                if (c == 1) {
                    a->show();
                } else if (c == 2) {
                    double amt = readDouble("Amount: ");
                    *a += amt;
                    a->show();
                } else if (c == 3) {
                    double amt = readDouble("Amount: ");
                    a->withdraw(amt);
                    a->show();
                } else if (c == 4) {
                    int to = readInt("To ID: ");
                    double amt = readDouble("Amount: ");
                    Account* r = find(to);
                    if (!r || r == a) throw string("Invalid receiver");
                    a->withdraw(amt);
                    r->deposit(amt);
                    cout << "Transferred Rs " << amt << "\n";
                } else if (c == 5) {
                    a->monthEnd();
                    a->show();
                }
            } catch (const string& e) {
                cout << "Error: " << e << "\n";
            }
        } while (c != 6);
    }
};

int main() {
    cout << "Smart Bank - by Aun Haider Niazi\n";
    Bank b;
    int c;
    do {
        c = 0;
        cout << "\n=== SMART BANK ===\n1.Open Account 2.Login 3.Exit\nChoice: ";
        if (!(cin >> c)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a valid number.\n";
            continue;
        }
        if (c == 1) b.open();
        else if (c == 2) b.login();
    } while (c != 3);

    cout << "Total accounts: " << Account::total() << "\n";
    return 0;
}
