#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <ctime>
#include <iomanip>

using namespace std;

const int MAX_MESSAGE_LEN = 500;
const int MAX_COMMENT_LEN = 55;

const string RESET = "\033[0m";
const string GREEN = "\033[92m";
const string YELLOW = "\033[93m";
const string CYAN = "\033[96m";
const string RED = "\033[91m";
const string BOLD = "\033[1m";

enum Role {
    USER = 0,
    ADMIN = 1,
    HEAD_ADMIN = 2
};

struct User {
    string login;
    string password_hash;
    Role role;
    string reg_time;
};

struct Comment {
    string author;
    string text;
}

struct Message {
    string author;
    string text;
    vector<Comment> comments;
};

struct FriendRequest {
    string from;
    string to;
};


map<string, User> users;
vector<Message> ghamlet;
vector<FriendRequest> friendRequests;
map<string, vector<string>> friends;

string CurrentTime() {
    time_t now = time(nullptr);
    tm t;
#if defined(_MSC_VER)
    localtime_s(&t, &now);
#else
    tm* tmp = localtime(&now);
    if (tmp) t = *tmp;
    else memset(&t, 0, sizeof(t));
#endif
    stringstream ss;
    ss << put_time(&t, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

string Escape(const string& s) {
    string r;
    for (char c : s) {
        if (c == '|') r += "\\|";
        else if (c == '\n') r += "\\n";
        else r += c;
    }
    return r;
}

string Unescape(const string& s) {
    string r;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            if (s[i + 1] == 'n') { r += '\n'; i++; }
            else if (s[i + 1] == '|') { r += '|'; i++; }
        }
        else r += s[i];
    }
    return r;
}

string HashPassword(const string& password) {
    const string SALT = "GHAMLET_SALT_2026";
    hash<string> h;
    string r = password + SALT;
    for (int i = 0; i < 5; i++)
        r = to_string(h(r));
    return r;
}

void SaveFriends() {
    ofstream f("friends.txt");
    for (auto& p : friends)
        for (auto& fr : p.second)
            f << p.first << "|" << fr << "\n";
}

void LoadFriends() {
    ifstream f("friends.txt");
    if (!f) return;

    string line;
    while (getline(f, line)) {
        auto p = line.find('|');
        if (p == string::npos) continue;
        string a = line.substr(0, p);
        string b = line.substr(p + 1);
        friends[a].push_back(b);
    }
}

void SaveFriendRequests() {
    ofstream f("requests.txt");
    for (auto& r : friendRequests)
        f << r.from << "|" << r.to << "\n";
}

void LoadFriendRequests() {
    ifstream f("requests.txt");
    if (!f) return;

    string line;
    while (getline(f, line)) {
        auto p = line.find('|');
        if (p == string::npos) continue;
        FriendRequest r;
        r.from = line.substr(0, p);
        r.to = line.substr(p + 1);
        friendRequests.push_back(r);
    }
}

void Pause() {
    cout << CYAN << "\nPress Enter..." << RESET;
    cin.get();
}

void SaveUsers() {
    ofstream f("users.txt");
    for (auto& p : users) {
        auto& u = p.second;
        f << u.login << "|"
            << u.password_hash << "|"
            << u.role << "|"
            << u.reg_time << "\n";
    }
}

void ManageUsers() {
    while (true) {
        system("cls");
        cout << CYAN << BOLD << "USER MANAGEMENT\n\n" << RESET;

        int i = 1;
        vector<string> logins;

        for (auto& p : users) {
            auto& u = p.second;

            cout << i << ") ";

            if (u.role == HEAD_ADMIN)
                cout << YELLOW << "[HEAD] ";
            else if (u.role == ADMIN)
                cout << GREEN << "[ADMIN] ";
            else
                cout << "[USER] ";

            cout << u.login << "\n";
            logins.push_back(u.login);
            i++;
        }

        cout << "\n0 - Back\n";
        cout << "Select user #: ";

        int c;
        cin >> c;

        if (c == 0)
            break;

        if (c < 1 || c >(int)logins.size())
            continue;

        string target = logins[c - 1];

        if (target == "roma1") {
            cout << RED << "\nCannot modify HEAD_ADMIN\n" << RESET;
            Pause();
            continue;
        }

        User& u = users[target];

        system("cls");
        cout << CYAN << "User: " << u.login << RESET << "\n\n";

        if (u.role == ADMIN)
            cout << "1 - Remove admin\n";
        else
            cout << "1 - Make admin\n";

        cout << "0 - Back\n> ";

        int a;
        cin >> a;

        if (a == 1) {
            if (u.role == ADMIN)
                u.role = USER;
            else
                u.role = ADMIN;

            SaveUsers();
            cout << GREEN << "Updated\n" << RESET;
            Pause();
        }
    }
}

void LoadUsers() {
    ifstream f("users.txt");
    if (!f) return;

    string line;
    while (getline(f, line)) {
        stringstream ss(line);
        vector<string> p;
        string x;
        while (getline(ss, x, '|')) p.push_back(x);
        if (p.size() != 4) continue;

        User u;
        u.login = p[0];
        u.password_hash = p[1];
        u.role = (Role)stoi(p[2]);
        u.reg_time = p[3];

        if (u.login == "roma1")
            u.role = HEAD_ADMIN;

        if (u.login == "dima1") {
            u.role = ADMIN;
        }

        users[u.login] = u;
    }
}

void SaveGhamlet() {
    ofstream f("ghamlet.txt");
    for (auto& m : ghamlet) {
        f << "M|" << Escape(m.author) << "|" << Escape(m.text) << "\n";
        for (auto& c : m.comments)
            f << "C|" << Escape(c.author) << "|" << Escape(c.text) << "\n";
        f << "E\n";
    }
}

void LoadGhamlet() {
    ifstream f("ghamlet.txt");
    if (!f) return;
    ghamlet.clear();

    string line;
    Message m;
    while (getline(f, line)) {
        if (line == "E") {
            ghamlet.push_back(m);
            m = Message();
        }
        else if (line[0] == 'M') {
            auto p = line.find('|', 2);
            m.author = Unescape(line.substr(2, p - 2));
            m.text = Unescape(line.substr(p + 1));
        }
        else if (line[0] == 'C') {
            auto p = line.find('|', 2);
            Comment c;
            c.author = Unescape(line.substr(2, p - 2));
            c.text = Unescape(line.substr(p + 1));
            m.comments.push_back(c);
        }
    }
    if (!m.author.empty())
        ghamlet.push_back(m);
}

int CountUserPosts(const string& login) {
    int c = 0;
    for (auto& m : ghamlet)
        if (m.author == login)
            c++;
    return c;
}

int CountUserComments(const string& login) {
    int c = 0;
    for (auto& m : ghamlet)
        for (auto& cm : m.comments)
            if (cm.author == login)
                c++;
    return c;
}

void ProfileMenu(const string& login) {
    while (true) {
        system("cls");

        auto& u = users[login];

        cout << CYAN << BOLD << "PROFILE\n\n" << RESET;

        cout << "Login: " << u.login << "\n";

        if (u.role == HEAD_ADMIN)
            cout << "Role: HEAD_ADMIN\n";
        else if (u.role == ADMIN)
            cout << "Role: ADMIN\n";
        else
            cout << "Role: USER\n";

        cout << "Registered: " << u.reg_time << "\n";
        cout << "Posts: " << CountUserPosts(login) << "\n";
        cout << "Comments: " << CountUserComments(login) << "\n\n";

        cout << "1 - Change login\n";
        cout << "0 - Back\n> ";

        int c;
        cin >> c;
        cin.ignore();

        if (c == 0)
            break;

        if (c == 1) {
            string newLogin;
            cout << "New login: ";
            cin >> newLogin;

            if (users.count(newLogin)) {
                cout << RED << "Login already exists\n" << RESET;
                Pause();
                continue;
            }

            User uCopy = users[login];
            users.erase(login);
            uCopy.login = newLogin;
            users[newLogin] = uCopy;

            for (auto& m : ghamlet) {
                if (m.author == login)
                    m.author = newLogin;
                for (auto& cmt : m.comments)
                    if (cmt.author == login)
                        cmt.author = newLogin;
            }

            SaveUsers();
            SaveGhamlet();

            cout << GREEN << "Login changed\n" << RESET;
            Pause();
            break;
        }
    }
}

void ShowUsers() {
    system("cls");
    cout << CYAN << BOLD << "USERS\n\n" << RESET;

    for (auto& p : users) {
        auto& u = p.second;

        if (u.role == HEAD_ADMIN)
            cout << YELLOW << "[Head Admin] ";
        else if (u.role == ADMIN)
            cout << GREEN << "[ADMIN] ";
        else
            cout << "[USER] ";

        cout << u.login << " | " << u.reg_time << "\n";
    }
    ManageUsers();
}

void ShowGhamlet() {
    if (ghamlet.empty()) {
        cout << RED << "No posts\n" << RESET;
        return;
    }

    for (size_t i = 0; i < ghamlet.size(); i++) {
        auto& m = ghamlet[i];
        Role r = users.count(m.author) ? users[m.author].role : USER;

        cout << CYAN << "==============================\n" << RESET;
        cout << "[" << i + 1 << "] ";

        if (r == HEAD_ADMIN)
            cout << BOLD << YELLOW << m.author << " [HEAD]\n" << RESET;
        else if (r == ADMIN)
            cout << BOLD << GREEN << m.author << " [ADMIN]\n" << RESET;
        else
            cout << m.author << "\n";

        cout << m.text << "\n";

        for (auto& c : m.comments)
            cout << "  -> " << c.author << ": " << c.text << "\n";
    }
}

void RegisterUser() {
    string login, pass;
    cout << "Login: ";
    cin >> login;

    if (users.count(login)) {
        cout << RED << "User exists\n" << RESET;
        Pause();
        return;
    }

    cout << "Password: ";
    cin >> pass;

    User u;
    u.login = login;
    u.password_hash = HashPassword(pass);
    u.role = USER;
    u.reg_time = CurrentTime();

    users[login] = u;
    SaveUsers();

    cout << GREEN << "Registered\n" << RESET;
    Pause();
}

void GhamletMenu(const string& login) {
    LoadGhamlet();

    while (true) {
        system("cls");
        cout << BOLD << CYAN << "G H A M L E T\n\n" << RESET;

        if (users[login].role == HEAD_ADMIN)
            cout << YELLOW << login << " [HEAD ADMIN]\n\n" << RESET;
        else if (users[login].role == ADMIN)
            cout << GREEN << login << " [ADMIN]\n\n" << RESET;
        else
            cout << login << "\n\n";

        cout << "1 - Upload post\n";
        cout << "2 - Comment\n";
        cout << "3 - Show posts\n";
        cout << "4 - Profile\n";
        if (users[login].role >= ADMIN)
            cout << "5 - Delete post\n";
        if (users[login].role == HEAD_ADMIN)
            cout << "6 - Show users\n";
        cout << "0 - Exit\n> ";

        int c;
        cin >> c;
        cin.ignore();

        if (c == 0) break;

        if (c == 1) {
            Message m;
            m.author = login;
            getline(cin >> ws, m.text);
            if (!m.text.empty() && m.text.size() <= MAX_MESSAGE_LEN) {
                ghamlet.push_back(m);
                SaveGhamlet();
            }
            Pause();
        }
        else if (c == 2) {
            ShowGhamlet();
            int n;
            cout << "\nPost #: ";
            cin >> n;
            cin.ignore();

            if (n > 0 && n <= (int)ghamlet.size()) {
                Comment cmt;
                cout << "Comment: ";
                getline(cin >> ws, cmt.text);
                if (!cmt.text.empty() && cmt.text.size() <= MAX_COMMENT_LEN) {
                    cmt.author = users[login].role >= ADMIN ? login : "anonymous";
                    ghamlet[n - 1].comments.push_back(cmt);
                    SaveGhamlet();
                }
            }
            Pause();
        }
        else if (c == 3) {
            ShowGhamlet();
            Pause();
        }
        else if (c == 5 && users[login].role >= ADMIN) {
            ShowGhamlet();
            int n;
            cin >> n;
            if (n > 0 && n <= (int)ghamlet.size()) {
                ghamlet.erase(ghamlet.begin() + n - 1);
                SaveGhamlet();
            }
        }
        else if (c == 6 && users[login].role == HEAD_ADMIN) {
            ShowUsers();
        }
        else if (c == 4) {
            ProfileMenu(login);
        }

    }
}

void LoginUser() {
    string login, pass;
    cout << "Login: ";
    cin >> login;
    cout << "Password: ";
    cin >> pass;

    if (users.count(login) &&
        users[login].password_hash == HashPassword(pass)) {
        GhamletMenu(login);
    }
    else {
        cout << RED << "Wrong credentials\n" << RESET;
        Pause();
    }
}

int main() {
    LoadUsers();

    while (true) {
        system("cls");
        cout << BOLD << CYAN << "G H A M L E T\n\n" << RESET;
        cout << "1 - Register\n2 - Login\n0 - Exit\n> ";

        int c;
        cin >> c;

        if (c == 0) break;
        if (c == 1) RegisterUser();
        if (c == 2) LoginUser();
    }
}
