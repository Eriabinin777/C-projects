#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <dirent.h>
#include <limits.h>
#include <linux/rtc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <cstdlib>

using namespace std;

void pwd()// название текущей директории
{
    string s(get_current_dir_name());
    cout << s << '\n';
}

void cd(string path)// смена рабочей директории
{
    if (path == "")
    {
        string s = getenv("HOME");
        chdir(s.c_str());
    }
    else
    {
        if(chdir(path.c_str()) == -1)
        {
            perror("Wrong path");
        }
    }
}

sig_atomic_t flag = 0;
void sighd(int signal){
    if(signal == 2)
    {
        flag = 1;
    }
}

int main()
{
    while(1)// главный цикл работы программы
    {
        flag = 0;
        signal(SIGINT, sighd);
        if(flag) continue;
        string cur(get_current_dir_name());
        if (geteuid() == 0)
            cout << cur <<  "~$ ";
        else
            cout << cur <<  "~> ";
        string input;
        getline(cin, input);
        if(input.size() == 0)
            continue;
         // режем строку на вектор-вектор строк по |, потом по пробелу
        vector<string> split;
        string division(" | ");
        size_t prev = 0;
        size_t next;
        size_t step = division.length();
        while((next = input.find(division, prev)) != string::npos )
        {
            string tmp = input.substr(prev, next-prev);
            split.push_back( input.substr(prev, next-prev));
            prev = next + step;
        }
        string tmp = input.substr(prev);
        split.push_back(input.substr(prev));
        vector<vector <string>> splitnew;
        splitnew.resize(split.size());
        for(size_t i = 0; i < split.size(); i++)
        {
            string s1;
            stringstream s(split[i]);
            while (s >> s1) splitnew[i].push_back(s1);
        }
        // конец нарезки

        if(splitnew[0][0] == "cd")// уходим
        {
            if (splitnew[0].size() == 1)
            {
                cd("");
            }
            else
            {
                cd(splitnew[0][1]);
            }
        }
        else if(splitnew[0][0] == "pwd")
        {
            pwd();
        }
        else if(splitnew[0][0] == "echo")// выводим введенное
        {
            for(size_t i = 1; i < splitnew[0].size(); i++){
                cout << splitnew[0][i] << ' ';
            }
            cout << '\n';
        }
        else if(splitnew[0][0] == "set")// настраиваем
        {
            extern char ** locality;
            for (int i = 0; locality[i] != NULL; i++)
            {
                cout << locality[i] << '\n';
            }
        }
        else // запускаем без нашего конвеера
        {
            size_t nk = 0;
            struct timeval start,stop;
            if(splitnew[0][0] == "time")
            {
                nk = 1;
                gettimeofday(&start,NULL);
            }
            if(splitnew.size() == 1)
            {
                // переключение
                int original_stdin = dup(STDIN_FILENO); // тут изначальные fd
                int original_stdout = dup(STDOUT_FILENO);
                size_t size_splitnew = splitnew[0].size();
                auto l_than = find(splitnew[0].begin(),splitnew[0].end(), "<");
                auto m_then = find(splitnew[0].begin(),splitnew[0].end(), ">");
                if(l_than != splitnew[0].end())
                {
                    string l_file = *(l_than + 1);
                    int fd_in = open(l_file.c_str(), O_RDONLY);
                    if(fd_in == -1)
                    {
                        perror("input failed");
                        continue;
                    }
                    dup2(fd_in, STDIN_FILENO);
                    size_splitnew = distance(splitnew[0].begin(), l_than);
                }
                if(m_then != splitnew[0].end())
                {
                    string m_file = *(m_then + 1);
                    int fd_out = open(m_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
                    if(fd_out == -1)
                    {
                        perror("output failed");
                        continue;
                    }
                    dup2(fd_out, STDOUT_FILENO);
                    size_splitnew = distance(splitnew[0].begin(), m_then);
                }
                pid_t ch = fork();
                if (ch != 0)   //предки
                {
                    wait(0);
                }
                else    //потомки
                {
                    vector<char *> v;
                    for (size_t i = nk; i < size_splitnew; i++)
                    {
                        v.push_back((char *)splitnew[0][i].c_str());
                    }
                    v.push_back(NULL);
                    prctl(PR_SET_PDEATHSIG, SIGINT);
                    execvp(v[0], &v[0]);
                    perror(v[0]);
                    exit(1);
                }
                if(l_than != splitnew[0].end()){
                    dup2(original_stdin, STDIN_FILENO);
                    close(original_stdin);

                }
                if(m_then != splitnew[0].end()){
                    dup2(original_stdout, STDOUT_FILENO);
                    close(original_stdout);
                }
            }
            else// запускаем с конвеером
            {
                int original_stdin = dup(STDIN_FILENO); // тут изначальные fd
                int original_stdout = dup(STDOUT_FILENO);
                int pipes[splitnew.size()-1][2];
                int run_fd = 0;
                int prev_fd = -1;
                for(size_t i = 0; i < splitnew.size()-1; i++)
                    if(pipe2(pipes[i], O_CLOEXEC))
                        perror("pipe\n");
                for (size_t i = 0; i < splitnew.size(); i++)
                {
                    size_t size_splitnew = splitnew[i].size();
                    auto l_than = find(splitnew[i].begin(),splitnew[i].end(), "<");
                    auto m_then = find(splitnew[i].begin(),splitnew[i].end(), ">");
                    if(l_than != splitnew[i].end())
                    {
                        string l_file = *(l_than + 1);
                        int fd_in = open(l_file.c_str(), O_RDONLY);
                        if(fd_in == -1)
                        {
                            perror("input failed");
                            continue;
                        }
                        dup2(fd_in, STDIN_FILENO);
                        size_splitnew = distance(splitnew[i].begin(), l_than);
                    }
                    if(m_then != splitnew[i].end())
                    {
                        string m_file = *(m_then + 1);
                        int fd_out = open(m_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
                        if(fd_out == -1)
                        {
                            perror("output failed");
                            continue;
                        }
                        dup2(fd_out, STDOUT_FILENO);
                        size_splitnew = distance(splitnew[i].begin(), m_then);
                    }
                    pid_t pid = fork();
                    if (!pid)
                    {
                        // для первой команды переключаем stdout
                        if (!i)
                        {
                            dup2(pipes[0][1], STDOUT_FILENO);
                            close(pipes[0][0]);
                        }
                        // для следующей и неконечной команды переключаем stdin и stdout
                        if (i && i < splitnew.size() - 1)
                        {
                            dup2(pipes[prev_fd][0], STDIN_FILENO);
                            close(pipes[prev_fd][1]);
                            dup2(pipes[run_fd][1], STDOUT_FILENO);
                            close(pipes[run_fd][0]);
                        }
                        // для последней команды переключаем stdin
                        if(i == splitnew.size() - 1)
                        {
                            dup2(pipes[prev_fd][0], STDIN_FILENO);
                            close(pipes[prev_fd][1]);
                        }
                        // запускаем
                        vector<char *> vec;
                        for (size_t j = 0; j < size_splitnew; j++)
                        {
                            vec.push_back((char *)splitnew[i][j].c_str());
                        }
                        vec.push_back(NULL);
                        prctl(PR_SET_PDEATHSIG, SIGINT);
                        execvp(vec[0], &vec[0]);
                        perror(vec[0]);
                        exit(1);
                    }//
                    else
                    {
                        if(i == splitnew.size() - 1)
                        {
                            for(size_t j = 0; j < splitnew.size() - 1; j++)
                            {
                                close(pipes[j][0]);
                                close(pipes[j][1]);
                            }
                            for(size_t j = 0; j < splitnew.size() - 1; j++)
                                wait(0);
                        }
                    }
                    run_fd++;
                    prev_fd++;
                }
            dup2(original_stdin, STDIN_FILENO);
            dup2(original_stdout, STDOUT_FILENO);
            close(original_stdin);
            close(original_stdout);
            }
            if(nk)// вывод времени работы дочерних процессов
            {
                gettimeofday(&stop,NULL);
                struct rusage usage;
                getrusage(RUSAGE_CHILDREN, &usage);
                cout << "\nreal     " << stop.tv_sec - start.tv_sec << '.' << (stop.tv_usec - start.tv_usec)/1000 << "s\nuser     "
                     << usage.ru_utime.tv_sec << '.' << (usage.ru_utime.tv_usec)/1000 << "s\nsystem   "
                     <<  usage.ru_stime.tv_sec << '.' << (usage.ru_stime.tv_usec)/1000 << "s\n";
            }
        }
    }
    return 0;
}
