#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "util.hpp"

// 是一个目录，下面放的是所有html网页
const std::string src_path = "data/input";
const std::string output = "data/raw_html/raw.txt";

struct DocInfo {
    std::string title; // 文档标题
    std::string content; // 文档内容
    std::string url; // 该文档在官网中的url
};

bool EnumFile(const std::string& src_path, std::vector<std::string>& files_list);
bool ParseHtml(const std::vector<std::string>& files_list, std::vector<DocInfo>& results);
bool SaveHtml(const std::vector<DocInfo>& results, const std::string& output);

int main() {
    std::vector<std::string> file_list;
    // 1.递归式地把每个html文件名带路径，保存到files_list中，方便后期对一个个文件进行读取
    if (!EnumFile(src_path, file_list)) {
        std::cerr << "enum file name error!" << std::endl;
        return 1;
    }
    // 2.按照files_list读取每个文件的内容，并进行解析
    std::vector<DocInfo> results;
    if (!ParseHtml(file_list, results)) {
        std::cerr << "parse html error!" << std::endl;
        return 2;
    }
    // 3.把解析完毕的文件内容，写入到output中，按照'\3'作为每个文档的分割符
    if (!SaveHtml(results, output)) {
        std::cerr << "save html error!" << std::endl;
        return 3;
    }
    return 0;
}

bool EnumFile(const std::string& src_path, std::vector<std::string>& files_list) {
    boost::filesystem::path root_path(src_path); // 路径对象
    // 判断路径是否存在，如果不存在就return false
    if (!boost::filesystem::exists(root_path)) {
        std::cerr << src_path << " not exists" << std::endl;
        return false;
    }
    // 定义一个空的迭代器，用来进行判断递归结束
    boost::filesystem::recursive_directory_iterator end;
    for (boost::filesystem::recursive_directory_iterator iter(root_path); iter != end; ++iter) { // 遍历文件
        if (!boost::filesystem::is_regular_file(*iter)) { // 如果不是普通文件，就跳过
            continue;
        }
        if (iter->path().extension() != ".html") { // 判断后缀
            continue;
        }
        files_list.push_back(iter->path().string());
    }

    return true;
}

static bool ParseTitle(const std::string& file, std::string& title) {
    std::size_t begin = file.find("<title>");
    if (begin == std::string::npos) {
        return false;
    }
    std::size_t end = file.find("</title>");
    if (end == std::string::npos) {
        return false;
    }
    begin += std::string("<title>").size();
    if (begin > end) {
        return false;
    }
    title = file.substr(begin, end - begin);
    return true;
}

static bool ParseContent(const std::string& file, std::string& content) {
    // 去标签，基于一个简易的状态机
    // 枚举两个状态，分别表示标签和content
    enum status {
        LABLE,
        CONTENT
    };
    status s = LABLE;
    for (char c : file) {
        if (s == LABLE) {
            if (c == '>') s = CONTENT;
        } else if (s == CONTENT) {
            if (c == '<') s = LABLE;
            else {
                if (c == '\n') c = ' ';
                content.push_back(c);
            }
        }
    }
    return true;
}

static bool ParseUrl(const std::string& file_path, std::string& url) {
    std::string url_head = "https://www.boost.org/doc/libs/1_81_0/doc/html";
    std::string url_tail = file_path.substr(src_path.size());
    url = url_head + url_tail;
    return true;
}

// void ShowDoc(const DocInfo& doc) {
//     std::cout << "title: " << doc.title << std::endl;
//     std::cout << "content: " << doc.content << std::endl;
//     std::cout << "url: " << doc.url << std::endl;
// }

bool ParseHtml(const std::vector<std::string>& files_list, std::vector<DocInfo>& results) {
    for (const std::string& file : files_list) {
        // 1.读取文件
        std::string result;
        if (!ns_util::FileUtil::ReadFile(file, result)) {
            continue;
        }
        // 2.解析文件，提取title
        DocInfo doc;
        if(!ParseTitle(result, doc.title)) {
            continue;
        }
        // 3.解析文件，提取content
        if (!ParseContent(result, doc.content)) {
            continue;
        }
        // 4.解析文件路径，构建url
        if (!ParseUrl(file, doc.url)) {
            continue;
        }
        results.push_back(std::move(doc));
    }
    return true;
}

bool SaveHtml(const std::vector<DocInfo>& results, const std::string& output) {
#define SEP '\3'
    std::ofstream out(output, std::ios::out | std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "open " << output << " failed!" << std::endl;
        return false;
    }

    for (auto& item : results) {
        std::string out_string;
        out_string = item.title;
        out_string += SEP;
        out_string += item.content;
        out_string += SEP;
        out_string += item.url;
        out_string += '\n';

        out.write(out_string.c_str(), out_string.size());
    }

    out.close();
    return true;
}
