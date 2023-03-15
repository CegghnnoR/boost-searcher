#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "util.hpp"
#include "log.hpp"

namespace ns_index {
    struct DocInfo {
        std::string title;
        std::string content;
        std::string url;
        uint64_t doc_id;
    };

    struct InvertedElem {
        uint64_t doc_id;
        std::string word;
        int weight;
    };

    class Index {
    private:
        // 正排索引，通过文档id（数组下标）找文档内容
        std::vector<DocInfo> forward_index;
        // 倒排索引，通过关键字找文档id等信息
        std::unordered_map<std::string, std::vector<InvertedElem>> inverted_index;

        // 单例模式
        Index() {}
        Index(const Index&) = delete;
        Index& operator=(const Index&) = delete;
        static Index* instance;
        static std::mutex mtx;
    public:
        ~Index() {}

        static Index* GetInstance() {
            if (nullptr == instance) {
                mtx.lock();
                if (nullptr == instance) {
                    instance = new Index();
                }
                mtx.unlock();
            }
            return instance;
        }

        // 根据doc_id找到文档内容
        DocInfo* GetForwardIndex(uint64_t doc_id) {
            if (doc_id >= forward_index.size()) {
                std::cerr << "doc_id out range, error!" << std::endl;
                return nullptr;
            }
            return &forward_index[doc_id];
        }
        // 根据关键字string，获得倒排拉链
        std::vector<InvertedElem>* GetInvertedList(const std::string& word) {
            auto iter = inverted_index.find(word);
            if (iter == inverted_index.end()) {
                std::cerr << word << " have no InvertedList" << std::endl;
                return nullptr;
            }
            return &iter->second;
        }

        // 根据去标签，格式化之后的文档，构建正排和倒排索引
        bool BuildIndex(const std::string& input) {
            std::ifstream in(input, std::ios::in | std::ios::binary);
            if (!in.is_open()) {
                std::cerr << "sorry, " << input << " open error" << std::endl;
                return false;
            }

            std::string line;
            int count = 0;
            while (std::getline(in, line)) {
                DocInfo* doc = BuildForwardIndex(line);
                if (nullptr == doc) {
                    std::cerr << "build " << line << " error" << std::endl;
                    continue;
                }

                BuildInvertedIndex(*doc);
                ++count;
                if (count % 50 == 0) {
                    // std::cout << "当前已经建立的索引文档：" << count << std::endl;
                    LOG(NORMAL, "当前已经建立的索引文档：" + std::to_string(count));
                }
            }
            return true;
        }
    private:
        DocInfo* BuildForwardIndex(const std::string& line) {
            // 1.解析line，字符串切分
            std::vector<std::string> results;
            const std::string sep("\3");
            ns_util::StringUtil::Split(line, results, sep);
            if (results.size() != 3) {
                return nullptr;
            }
            // 2.字符串填充到DocInfo
            DocInfo doc;
            doc.title = results[0];
            doc.content = results[1];
            doc.url = results[2];
            doc.doc_id = forward_index.size();
            // 3.插入到正排索引的vector
            forward_index.push_back(std::move(doc));

            return &forward_index.back();
        }

        bool BuildInvertedIndex(const DocInfo& doc) {
            struct word_cnt {
                int title_cnt;
                int content_cnt;
                word_cnt() : title_cnt(0), content_cnt(0) {}
            };
            std::unordered_map<std::string, word_cnt> word_map;

            // 对标题进行分词
            std::vector<std::string> title_words;
            ns_util::JiebaUtil::CutString(doc.title, title_words);
            // 对标题进行词频统计
            for (std::string s : title_words) {
                boost::to_lower(s); // 将分词统一转化成小写
                ++word_map[s].title_cnt;
            }

            // 对内容进行分词
            std::vector<std::string> content_words;
            ns_util::JiebaUtil::CutString(doc.content, content_words);
            // 对内容进行词频统计
            for (std::string s : content_words) {
                boost::to_lower(s); // 将分词统一转化成小写
                ++word_map[s].content_cnt;
            }

#define X 10
#define Y 1
            for (auto& word_pair : word_map) {
                InvertedElem item;
                item.doc_id = doc.doc_id;
                item.word = word_pair.first;
                item.weight = X * word_pair.second.title_cnt + Y * word_pair.second.content_cnt;
                inverted_index[word_pair.first].push_back(std::move(item));
            }
            return true;
        }
    };
    Index* Index::instance = nullptr;
    std::mutex Index::mtx;
}
