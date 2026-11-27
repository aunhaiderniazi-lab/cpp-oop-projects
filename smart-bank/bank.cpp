// Author: Aun Haider Niazi
#include <cctype>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
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

double readDbl() {
    double v;
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

class Account {
protected:
    int id;
    uint64_t pinSalt;
    string pinHash;
    string name;
    double bal;
    int lastMonthYear = 0, lastMonth = 0;
    static int count;

public:
    // FNV-1a over salt + PIN. Not a substitute for a password KDF, but it keeps
    // cleartext PINs out of bank.txt. Configure a strong PIN policy separately.
    static string hashPin(int pin, uint64_t salt) {
        uint64_t h = 14695981039346656037ull;
        const uint64_t values[2] = {salt, static_cast<uint64_t>(pin)};
        for (uint64_t value : values) {
            for (int i = 0; i < 8; ++i) {
                h ^= (value >> (i * 8)) & 0xff;
                h *= 1099511628211ull;
            }
        }
        return to_string(h);
    }

    static uint64_t makeSalt() {
        static mt19937_64 rng(random_device{}());
        return rng();
    }

    static bool sameHash(const string& a, const string& b) {
        if (a.size() != b.size()) return false;
        unsigned char diff = 0;
        for (size_t i = 0; i < a.size(); ++i)
            diff |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
        return diff == 0;
    }

    Account(int i, string n, int p, double b)
        : id(i), pinSalt(makeSalt()), pinHash(hashPin(p, pinSalt)), name(n), bal(b) { count++; }

    // Loader constructor: restores a previously salted hash (or a migrated legacy PIN).
    Account(int i, string n, uint64_t salt, string hash, double b, int y = 0, int m = 0)
        : id(i), pinSalt(salt), pinHash(hash), name(n), bal(b), lastMonthYear(y), lastMonth(m) { count++; }

    virtual ~Account() {}
    int getId() const { return id; }
    bool check(int p) const { return sameHash(hashPin(p, pinSalt), pinHash); }

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
    virtual void applyMonthEnd() = 0;

    // Month-end is allowed at most once per calendar month, even across restarts.
    void monthEnd() {
        time_t now = time(nullptr);
        tm* lt = localtime(&now);
        if (!lt) throw string("Could not determine current date");
        int y = lt->tm_year + 1900, m = lt->tm_mon + 1;
        if (y == lastMonthYear && m == lastMonth)
            throw string("Month-end has already been applied this month");
        applyMonthEnd();
        lastMonthYear = y;
        lastMonth = m;
    }

    void show() const {
        cout << "\n[" << type() << "] #" << id << " " << name
             << " | Balance: Rs " << fixed << setprecision(2) << bal << "\n";
    }

    string save() const {
        return type() + " " + to_string(id) + " " + name + " H " + to_string(pinSalt)
             + " " + pinHash + " " + to_string(bal) + " " + to_string(lastMonthYear)
             + " " + to_string(lastMonth);
    }

    Account& operator+=(double a) { deposit(a); return *this; }
    static int total() { return count; }
};
int Account::count = 0;

class Savings : public Account {
public:
    using Account::Account;
    string type() const override { return "Savings"; }
    void applyMonthEnd() override { bal += bal * 0.01; }               // 1% interest
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
    void applyMonthEnd() override { bal -= 100; }                      // Rs 100 fee
};

class Bank {
    vector<Account*> v;
    Account* find(int id) { for (auto a : v) if (a->getId() == id) return a; return nullptr; }
    int nextId() const {
        int maxId = 1000;
        for (auto a : v) if (a->getId() > maxId) maxId = a->getId();
        return maxId + 1;
    }
public:
    Bank() {
        ifstream f("bank.txt");
        string t, n, marker;
        int id;
        while (f >> t >> id >> n >> marker) {
            uint64_t salt;
            string hash;
            double b;
            int y = 0, m = 0;
            if (marker == "H") {
                if (!(f >> salt >> hash >> b >> y >> m)) break;
            } else {
                // Migrate legacy records that stored the PIN in cleartext.
                int legacyPin = 0;
                try { legacyPin = stoi(marker); } catch (...) { continue; }
                salt = Account::makeSalt();
                hash = Account::hashPin(legacyPin, salt);
                if (!(f >> b)) break;
            }
            if (b < 0) b = 0;
            v.push_back(t == "Savings"
                ? static_cast<Account*>(new Savings(id, n, salt, hash, b, y, m))
                : new Current(id, n, salt, hash, b, y, m));
        }
    }

    ~Bank() {
        ofstream f("bank.txt", ios::trunc);
        if (!f) cerr << "Warning: could not write bank.txt; data was not saved.\n";
        for (auto a : v) {
            if (f) f << a->save() << '\n';
            delete a;
        }
        v.clear();
    }

    void open() {
        string n = readWord();
        int t = readInt();
        if (t != 1 && t != 2) { cout << "Invalid type\n"; return; }
        cout << "4-digit PIN: ";
        int p = readInt();
        if (p < 1000 || p > 9999) { cout << "PIN must be 4 digits\n"; return; }
        cout << "Opening deposit: ";
        double b = readDbl();
        if (b < 0) { cout << "Deposit cannot be negative\n"; return; }
        int id = nextId();
        v.push_back(t == 1 ? static_cast<Account*>(new Savings(id, n, p, b))
                           : new Current(id, n, p, b));
        cout << "Account created! Your ID: " << id << "\n";
    }

    void login() {
        cout << "Account ID: ";
        int id = readInt();
        cout << "PIN: ";
        int p = readInt();
        Account* a = find(id);
        if (!a || !a->check(p)) { cout << "Wrong ID or PIN\n"; return; }
        int c;
        do {
            cout << "\n1.Balance 2.Deposit 3.Withdraw 4.Transfer 5.Month-end 6.Logout\nChoice: ";
            c = readInt();
            try {
                double amt;
                if (c == 1) a->show();
                else if (c == 2) { cout << "Amount: "; amt = readDbl(); *a += amt; a->show(); }
                else if (c == 3) { cout << "Amount: "; amt = readDbl(); a->withdraw(amt); a->show(); }
                else if (c == 4) {
                    cout << "To ID: ";
                    int to = readInt();
                    cout << "Amount: ";
                    amt = readDbl();
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
    try {
        Bank b;
        int c;
        do {
            cout << "\n=== SMART BANK ===\n1.Open Account 2.Login 3.Exit\nChoice: ";
            c = readInt();
            if (c == 1) b.open(); else if (c == 2) b.login();
        } while (c != 3);
        cout << "Total accounts: " << Account::total() << "\n";
    } catch (const EndOfInput&) {
        cout << "\nEnd of input. Data saved.\n";
    }
    return 0;
}
