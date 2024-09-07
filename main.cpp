#include <iostream>
#include <fstream>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<string>
#include<cstring>
#include<vector>
#include<map>
#include <set>

using namespace std;
string wpath,usrname,cmkd;
vector<string> fls;
map<string, long long> hsmap;
const string WPCONF = "wpath.txt",HSCONF = "hash.txt",USRCONF = "username.txt",CRASHINFO = "Please Input Watching Path:";

string getWatchingPath() {
    string s = "none";
    ifstream ifs;
    ifs.open(WPCONF, ios::in);
    if (!ifs.is_open()) {
        cout << "Please Enter the Path to Watch:" << endl;
        cin >> s;
        ofstream ofs;
        ofs.open(WPCONF);
        if (!ofs.is_open()) {
            cout << CRASHINFO << endl;
            return "none";
        }
        ofs << s;
        ofs.close();
        return s;
    }
    ifs >> s;
    ifs.close();
    return s;
}
string getUsrName() {
    string s = "john_smith";
    ifstream ifs;
    ifs.open(USRCONF, ios::in);
    if (!ifs.is_open()) {
        cout << "Please Enter the User Name:" << endl;
        cin >> s;
        ofstream ofs;
        ofs.open(USRCONF);
        if (!ofs.is_open()) {
            cout << CRASHINFO << endl;
            return "john_smith";
        }
        ofs << s;
        ofs.close();
        return s;
    }
    ifs >> s;
    ifs.close();
    return s;
}
map<string,long long> calcHSMap() {
    map<string,long long> mp;
    for (int i = 0;i < fls.size(); i++) {
        ifstream ifs;
        ifs.open(fls[i]);
        if(!ifs.is_open()) {
            cout << CRASHINFO << endl;
            continue;
        }
        istreambuf_iterator<char> beg(ifs),end;
        string source(beg,end);
        ifs.close();
        hash<string> hasher;
        long long hs_source = hasher(source);
        string name = fls[i];
        mp[name] = hs_source;
        cout << "file:" << fls[i] << " hash value:" << hs_source << endl;
    }
    return mp;
}
void saveHSMapToFile(map<string, long long> mp) {
    ofstream ofs;
    ofs.open(HSCONF);
    if(!ofs.is_open()) {
        cout << CRASHINFO << endl;
        return;
    }
    ofs << fls.size() * 2 << endl;
    for (int i = 0;i < fls.size();i++) {
        hash<string> hasher;
        string name = fls[i];
        ofs << name << "n" << endl;
        ofs << mp[name] << "s"  << endl;
    }
    ofs.close();
}
map<string, long long> loadHSMapFromFile() {
    map<string, long long> mp;
    ifstream ifs;
    ifs.open(HSCONF);
    if(!ifs.is_open()) {
        cout << CRASHINFO << endl;
        return mp;
    }
    int sz = 0;
    ifs >> sz;
    string name;
    long long hs_source = 0;
    for (int i = 0;i < sz;i++) {
        string x;
        ifs >> x;
//        if (i == 28) {
//            cout << "28!" << endl;
//        }
        if(x[x.length() - 1] == 'n') {
            x.pop_back();
            name = x;
        } else {
            x.pop_back();
            hs_source = atoll(x.c_str());
        }
        if((!name.empty()) && hs_source != 0) {
            mp[name] = hs_source;
            name.clear();
            hs_source = 0;
        }
    }
    ifs.close();
    return mp;
}
vector<string> getChangedList(map<string,long long> x, map<string,long long> y) {
    vector<string> ret;
    for (int i = 0;i < fls.size(); i++) {
        if (x[fls[i]] != y[fls[i]]) {
            string tmp = fls[i];
            ret.push_back(tmp.substr(wpath.size(),fls[i].size()-1));
        }
    }
    return ret;
}
void addCommiter(vector<string> chg) {
    cout << "Adding Commiter..." << endl;
    string ph = "commiters.list";
    ifstream ifs;
    ifs.open(ph);
    if(!ifs.is_open()) {
        cout << "Commiters list file not found.Creating..." << endl;
        ofstream ofs;
        ofs.open(ph);
        string lst;
        for(int i = 0;i < chg.size();i++) {
            lst += chg[i] + ":" + usrname + "|\n";
        }
        ofs << lst;
        ofs.close();
    } else {
        string ln;
        map<string,set<string>> t;
        int sz = 0;
        while(getline(ifs,ln)) {
            string flname,cm;
            int s = 0;
            for(int i = 0;i< ln.size();i++) {
                if(ln[i] == ':') {
                    s=i;
                    break;
                }
                flname.push_back(ln[i]);
            }
            for(int i = s+1;i<ln.size();i++) {
                if(ln[i] == '|') {
                    t[flname].insert(cm);
                    cm.clear();
                    continue;
                }
                cm.push_back(ln[i]);
            }
            ++sz;
        }
        ifs.close();
        for(int i = 0;i < chg.size();i++) {
            t[chg[i]].insert(usrname);
        }
        ofstream ofs;
        ofs.open(ph,ios::trunc);
        ofstream ofs2;
        ofs2.open(cmkd,ios::trunc);
        string to_output,md="# 贡献者列表\n这里是贡献者列表，由CommitRecorder生成\n";
        for(auto iter = t.begin(); iter != t.end(); ++iter) {
            string fr = iter->first;
            to_output += fr + ":";
            md += "## " + fr + "\n";
            set<string> r = iter->second;
            for(auto iter2 = r.begin(); iter2 != r.end(); ++iter2) {
                to_output += *iter2 + "|";
                md += "- " + *iter2 + "\n";
            }
            to_output += "\n";
            md += "\n";
        }
        ofs << to_output;
        ofs2 << md;
        ofs.close();
        ofs2.close();
    }
}
void getHSMap() {
    ifstream ifs;
    ifs.open(HSCONF, ios::in);
    if (!ifs.is_open()) {
        cout << "You are using it for the first time.So Create Hashmap for You!" << endl;
        hsmap = calcHSMap();
        saveHSMapToFile(hsmap);
        return;
    }
    ifs.close();
    cout << "Getting Old HashMap..." << endl;
    hsmap = loadHSMapFromFile();
    cout << "done." << endl;
    cout << "Calculating New one..." << endl;
    map<string, long long> hsmap2 = calcHSMap();
    if(hsmap == hsmap2) {
        cout << "Nothing Changed." << endl;
    } else {
        vector<string> chg = getChangedList(hsmap, hsmap2);
        cout << "Changed List:" << endl;
        for (int i = 0;i < chg.size();i++) {
            cout << chg[i] << endl;
        }
        addCommiter(chg);
        saveHSMapToFile(calcHSMap());
    }
}
int getAbsoluteFiles(string directory, vector<string> &filesAbsolutePath) //参数1[in]要变量的目录  参数2[out]存储文件名
{
    DIR *dir = opendir(directory.c_str()); //打开目录   DIR-->类似目录句柄的东西
    if (dir == NULL) {
        cout << directory << " is not a directory or not exist!" << endl;
        return -1;
    }
    struct dirent *d_ent = NULL;       //dirent-->会存储文件的各种属性
    char fullpath[128] = {0};
    char dot[3] = ".";                //linux每个下面都有一个 .  和 ..  要把这两个都去掉
    char dotdot[6] = "..";
    while ((d_ent = readdir(dir)) != NULL)    //一行一行的读目录下的东西,这个东西的属性放到dirent的变量中
    {
        if ((strcmp(d_ent->d_name, dot) != 0)
            && (strcmp(d_ent->d_name, dotdot) != 0))   //忽略 . 和 ..
        {
            if (d_ent->d_type == DT_DIR) //d_type可以看到当前的东西的类型,DT_DIR代表当前都到的是目录,在usr/include/dirent.h中定义的
            {

                string newDirectory = directory + string("/") + string(d_ent->d_name); //d_name中存储了子目录的名字
                if (directory[directory.length() - 1] == '/') {
                    newDirectory = directory + string(d_ent->d_name);
                }

                if (-1 == getAbsoluteFiles(newDirectory, filesAbsolutePath))  //递归子目录
                {
                    return -1;
                }
            } else   //如果不是目录
            {
                string absolutePath = directory + string("/") + string(d_ent->d_name);  //构建绝对路径
                if (directory[directory.length() - 1] == '/')  //如果传入的目录最后是/--> 例如a/b/  那么后面直接链接文件名
                {
                    absolutePath = directory + string(d_ent->d_name); // /a/b/1.txt
                }
                filesAbsolutePath.push_back(absolutePath);
            }
        }
    }

    closedir(dir);
    return 0;
}

int main() {
    cout << "Welcome to use CommitRecord!" << endl;
    wpath = getWatchingPath();
    usrname = getUsrName();
    cmkd = wpath + "/commiter_record.md";
    cout << "WPath is " << wpath << endl;
    cout << "And Username is " << usrname << endl;
    cout << "Getting Files..." << endl;
    getAbsoluteFiles(wpath, fls);
    for(auto iter = fls.begin();iter != fls.end();++iter) {
        if((*iter) == cmkd) {
            fls.erase(iter);
            break;
        }
    }
    cout << "done." << endl;
    getHSMap();
    return 0;
}