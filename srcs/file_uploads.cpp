#include "Server.hpp"
#include "webserv.hpp"

std::pair<bool, std::string>    forbidden_page()
{
    std::string page_message = "403 Forbidden";
    return (std::make_pair(false, generate_error_page(page_message, page_message)));
}

std::pair<bool, std::string>    request_entity_too_large()
{
    std::string page_message = "413 Request Entity Too Large";
    return (std::make_pair(false, generate_error_page(page_message, page_message)));
}

unsigned int    get_max_body_size(Location & location)
{
    std::string         str_body_size = location.location_map["client_max_body_size"];
    std::stringstream   ss(str_body_size);
    unsigned int        max_body_size = 0;
    std::string         units;

    ss >> max_body_size;
    ss >> units;
    transform(units.begin(), units.end(), units.begin(), tolower);
    if (str_body_size.empty())
        return (0);
    if (units == "ko")
        return (max_body_size * 1000);
    if (units == "mo")
        return (max_body_size * 1000000);
    return (max_body_size);
}

bool    directory_exists(std::string directory_path)
{
    struct stat info;

    if (stat(directory_path.c_str(), &info) != 0)
        return (false);
    else if (info.st_mode & S_IFDIR)
        return (true);
    else
        return (false);
}

std::string gen_random_string(const int len)
{
    static const char   alphanum[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
    std::string rand_str;
    
    rand_str.reserve(len);
    for (int i = 0; i < len; ++i) {
        rand_str += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return (rand_str);
}

std::pair<bool, std::string>    upload_file(Request & request, std::string upload_path)
{
    if (!directory_exists(upload_path)) {
        if (mkdir(upload_path.c_str(), 0644) == -1)
            return (internal_server_error());
    }
    if (upload_path[upload_path.size() - 1] != '/')
        upload_path += '/';
    std::string file_name = gen_random_string(10);
    std::cout << "random file name: " << file_name << std::endl;
    std::string full_path = upload_path + file_name;
    struct stat buffer;
    while (stat(full_path.c_str(), &buffer) == 0)   // file already exists, we need a new one
        file_name = gen_random_string(10);
    // create file with full path
    std::ofstream   outfile(full_path);
    // write data to new file
    outfile.write(request.data.c_str(), request.data.size());
    return std::make_pair(true, "ok");
}

std::pair<bool, std::string>    check_upload_file(Request & request, Location & location)
{
    std::string         upload_path = location.location_map["upload_path"];
    std::stringstream   ss_content_len(request.headers["Content-Length"]);
    unsigned int        content_len = 0;
    unsigned int        max_body_size;

    ss_content_len >> content_len;
    max_body_size = get_max_body_size(location);

    if (upload_path.empty()) {
        return (forbidden_page());
    }
    else if (content_len > max_body_size) {
        return (request_entity_too_large());
    }
    else {
        return (upload_file(request, upload_path));
    }
}
