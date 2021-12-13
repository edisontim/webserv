#include "webserv.hpp"
#include "Request.hpp"

void    php_fill_env(Request & request, std::string path, char *env[13])
{
    std::string env_vars[13];
    int         i;
    std::string filename;

    filename = path.substr(path.rfind("/") + 1);
    env_vars[0] = "CONTENT_LENGTH=" + request.headers["Content-Length"];
    env_vars[1] = "CONTENT_TYPE=" + request.headers["Content-Type"];
    env_vars[2] = "REQUEST_METHOD=POST";
    env_vars[3] = "REDIRECT_STATUS=200";
    env_vars[4] = "GATEWAY_INTERFACE=CGI/1.1";
    env_vars[5] = "SCRIPT_NAME=" + request.uri;
    env_vars[6] = "PATH_INFO=" + path;
    env_vars[7] = "PATH_TRANSLATED=" + path;
    env_vars[8] = "REMOTE_ADDR=" + request.headers["Origini"];
    env_vars[9] = "AUTH_TYPE=" + request.headers["Authorization"];
    env_vars[10] = "QUERY_STRING="; // set only for get requests
    env_vars[11] = "REMOTE_USER=" + request.headers["Authorization"];

    i = 0;
    while (i < 12) {
        env[i] = new char[env_vars[i].size() + 1];
        strcpy(env[i], env_vars[i].c_str());
        ++i;
    }
    env[i] = NULL;
}

void    php_cgi(Request & request, std::string server_directory, std::string path)
{
    char                *cgi_args[3];
    int                 tubes[2];
    int                 fd, cgi_pid;
    std::string         cgi_output_path;
    char                *env[13];
    std::stringstream   ss_content_length(request.headers["Content-Length"]);
    unsigned int        content_length = 0;
    std::string         cgi_path;

    // change !! if value is empty, then error (cgi disabled)
    cgi_path = "/Users/amilis/Documents/homebrew/Cellar/php@7.4/7.4.26_1/bin/php-cgi";

    cgi_args[0] = new char[cgi_path.size() + 1];
    strcpy(cgi_args[0], cgi_path.c_str());
    cgi_args[1] = new char[path.size() + 1];
    strcpy(cgi_args[1], path.c_str());
    cgi_args[2] = NULL;

    cgi_output_path = server_directory + "cgi_output.html";
    fd = open(cgi_output_path.c_str(), O_RDWR | O_CREAT, 0777);

    pipe(tubes);
    ss_content_length >> content_length;

    write(tubes[1], request.data.c_str(), content_length);
    php_fill_env(request, path, env);

    cgi_pid = fork();
    if (cgi_pid == 0)
    {
        close(tubes[1]);
        dup2(tubes[0], 0);
        dup2(fd, 1);
        execve(cgi_args[0], cgi_args, env);
    }
    close(tubes[0]);
    close(tubes[1]);
    delete cgi_args[0];
    delete cgi_args[1];
    for (int i = 0; i < 13 ; i++)
        delete env[i];
}
