// Copyright 2026 <lyx2023112762>
/**
 * @file WordGraphProcessor.cpp
 * @brief 软件工程lab1：基于文本的有向图构建与操作（交互增强版）
 * @version 3.5
 * 
 * 改进：
 *   - 初始菜单增加退出选项
 *   - 打开新文件菜单增加退出选项
 *   - 文件加载失败时重新显示菜单，而不是退出
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <algorithm>
#include <cctype>
#include <random>
#include <cmath>
#include <functional>
#include <chrono>
#include <thread>
#include <limits>
#include <cstdio>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

bool kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0) perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
    return buf;
}
#endif

// ==================== 路径辅助函数 ====================
std::string getParentDir(const std::string& filepath) {
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    size_t pos = filepath.find_last_of(sep);
    if (pos == std::string::npos) return "";
    return filepath.substr(0, pos + 1);
}

std::string getFileStem(const std::string& filepath) {
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    size_t start = filepath.find_last_of(sep);
    if (start == std::string::npos) start = 0;
    else start++;
    size_t dot = filepath.find_last_of('.');
    if (dot == std::string::npos || dot < start) {
        return filepath.substr(start);
    }
    return filepath.substr(start, dot - start);
}

bool fileExists(const std::string& filepath) {
    std::ifstream f(filepath.c_str());
    return f.good();
}

// ==================== 特殊指令判断 ====================
static bool isBackCommand(const std::string& s) {
    std::string lower;
    lower.reserve(s.size());
    std::transform(s.begin(), s.end(), std::back_inserter(lower),
                   [](unsigned char c) { return std::tolower(c); });
    return lower == "back";
}

static bool isExitCommand(const std::string& s) {
    std::string lower;
    lower.reserve(s.size());
    std::transform(s.begin(), s.end(), std::back_inserter(lower),
                   [](unsigned char c) { return std::tolower(c); });
    return lower == "exit" || lower == "quit";
}

// ==================== 图类 ====================
class Graph {
 private:
    std::vector<std::string> nodes;
    std::unordered_map<std::string, int> wordToIndex;
    std::vector<std::unordered_map<int, int>> adjList;
    mutable int m_randomWalkStart;
    mutable bool m_randomWalkFixedLength;
    mutable int m_randomWalkLength;

    int getOrAddNode(const std::string& word) {
        auto it = wordToIndex.find(word);
        if (it != wordToIndex.end()) return it->second;
        int idx = nodes.size();
        nodes.push_back(word);
        wordToIndex[word] = idx;
        adjList.emplace_back();
        return idx;
    }

 public:
    Graph() : m_randomWalkStart(-1), m_randomWalkFixedLength(false),
              m_randomWalkLength(0) {}

    static std::string toLower(const std::string& s) {
        std::string res;
        res.reserve(s.size());
        std::transform(s.begin(), s.end(), std::back_inserter(res),
                       [](unsigned char c) { return std::tolower(c); });
        return res;
    }

    void addEdge(const std::string& word1, const std::string& word2) {
        int u = getOrAddNode(word1);
        int v = getOrAddNode(word2);
        adjList[u][v]++;
    }

    void buildFromWordList(const std::vector<std::string>& words) {
        for (size_t i = 0; i + 1 < words.size(); ++i) {
            addEdge(words[i], words[i + 1]);
        }
    }

    bool loadFromDot(const std::string& dotFile) {
        std::ifstream file(dotFile);
        if (!file.is_open()) return false;
        nodes.clear();
        wordToIndex.clear();
        adjList.clear();
        std::string line;
        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            if (line.empty() || line[0] == '#' || line.substr(0, 2) == "//") {
                continue;
            }
            if (line.find("->") == std::string::npos) continue;
            size_t pos1 = line.find('"');
            if (pos1 == std::string::npos) continue;
            size_t pos2 = line.find('"', pos1 + 1);
            if (pos2 == std::string::npos) continue;
            std::string node1 = line.substr(pos1 + 1, pos2 - pos1 - 1);
            size_t posArrow = line.find("->", pos2);
            if (posArrow == std::string::npos) continue;
            size_t pos3 = line.find('"', posArrow + 2);
            if (pos3 == std::string::npos) continue;
            size_t pos4 = line.find('"', pos3 + 1);
            if (pos4 == std::string::npos) continue;
            std::string node2 = line.substr(pos3 + 1, pos4 - pos3 - 1);
            size_t posLabel = line.find("label=", pos4);
            if (posLabel == std::string::npos) continue;
            posLabel += 6;
            int weight = 1;
            if (line[posLabel] == '"') {
                size_t posLabelEnd = line.find('"', posLabel + 1);
                if (posLabelEnd != std::string::npos) {
                    std::string wstr = line.substr(posLabel + 1,
                                                    posLabelEnd - posLabel - 1);
                    weight = std::stoi(wstr);
                }
            } else {
                size_t posEnd = line.find_first_of(" \t;]", posLabel);
                if (posEnd != std::string::npos) {
                    std::string wstr = line.substr(posLabel, posEnd - posLabel);
                    weight = std::stoi(wstr);
                }
            }
            int u = getOrAddNode(node1);
            int v = getOrAddNode(node2);
            adjList[u][v] = weight;
        }
        return true;
    }

    bool hasWord(const std::string& word) const {
        return wordToIndex.find(toLower(word)) != wordToIndex.end();
    }

    int getIndex(const std::string& word) const {
        auto it = wordToIndex.find(toLower(word));
        return (it != wordToIndex.end()) ? it->second : -1;
    }

    int nodeCount() const { return nodes.size(); }

    int outDegreeSum(int idx) const {
        int sum = 0;
        for (const auto& p : adjList[idx]) sum += p.second;
        return sum;
    }

    void setRandomWalkRandomStart() { m_randomWalkStart = -1; }

    void setRandomWalkStart(const std::string& word) {
        int idx = getIndex(word);
        m_randomWalkStart = (idx != -1) ? idx : -1;
    }

    void setRandomWalkFixedLength(int len) {
        m_randomWalkFixedLength = true;
        m_randomWalkLength = len;
    }

    void setRandomWalkInfinite() { m_randomWalkFixedLength = false; }

    void showDirectedGraph(const std::string& inputFilePath) const {
        std::cout << "\n===== 有向图信息 =====\n";
        std::cout << "节点总数: " << nodes.size() << "\n";
        for (size_t i = 0; i < nodes.size(); ++i) {
            std::cout << "节点 \"" << nodes[i] << "\" 的出边: ";
            if (adjList[i].empty()) {
                std::cout << "无";
            } else {
                for (const auto& [v, w] : adjList[i]) {
                    std::cout << "\"" << nodes[v] << "\"(" << w << ") ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "========================\n";

        std::string ext = inputFilePath.substr(
            inputFilePath.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        bool isDot = (ext == "dot");
        std::string dir = getParentDir(inputFilePath);
        std::string stem = getFileStem(inputFilePath);
        std::string dotPath = isDot ? inputFilePath : dir + stem + ".dot";
        std::string picPath = dir + stem + "pic.png";

        if (!isDot) {
            std::ofstream dotFile(dotPath);
            if (!dotFile) {
                std::cerr << "无法创建dot文件: " << dotPath << std::endl;
                return;
            }
            dotFile << "digraph G {\n  node [shape=box];\n";
            for (size_t u = 0; u < nodes.size(); ++u) {
                for (const auto& [v, w] : adjList[u]) {
                    dotFile << "  \"" << nodes[u] << "\" -> \""
                            << nodes[v] << "\" [label=\"" << w << "\"];\n";
                }
            }
            dotFile << "}\n";
            dotFile.close();
        }

        std::string cmd = "dot -Tpng \"" + dotPath + "\" -o \"" + picPath + "\"";
        int ret = std::system(cmd.c_str());
        if (ret == 0) {
            std::cout << "有向图已保存至: " << picPath << std::endl;
        } else {
            std::cerr << "调用dot命令失败，请确保Graphviz已安装且dot在PATH中。\n";
        }
    }

    std::string queryBridgeWords(const std::string& word1,
                                 const std::string& word2) const {
        std::string w1 = toLower(word1), w2 = toLower(word2);
        int u = getIndex(w1), v = getIndex(w2);
        if (u == -1 && v == -1) {
            return "No \"" + w1 + "\" and \"" + w2 + "\" in the graph!";
        }
        if (u == -1) return "No \"" + w1 + "\" in the graph!";
        if (v == -1) return "No \"" + w2 + "\" in the graph!";

        std::vector<std::string> bridges;
        for (size_t k = 0; k < nodes.size(); ++k) {
            auto it1 = adjList[u].find(k);
            auto it2 = adjList[k].find(v);
            if (it1 != adjList[u].end() && it2 != adjList[k].end()) {
                bridges.push_back(nodes[k]);
            }
        }
        if (bridges.empty()) {
            return "No bridge words from \"" + w1 + "\" to \"" + w2 + "\"!";
        }

        std::ostringstream oss;
        oss << "The bridge word" << (bridges.size() > 1 ? "s" : "")
            << " from \"" << w1 << "\" to \"" << w2 << "\" "
            << (bridges.size() > 1 ? "are" : "is") << ": ";
        for (size_t i = 0; i < bridges.size(); ++i) {
            if (i > 0) oss << (i == bridges.size() - 1 ? " and " : ", ");
            oss << "\"" << bridges[i] << "\"";
        }
        oss << ".";
        return oss.str();
    }

    std::string generateNewText(const std::string& inputText) const {
        std::vector<std::string> words;
        std::string current;
        for (char ch : inputText) {
            if (std::isalpha(static_cast<unsigned char>(ch))) {
                current.push_back(std::tolower(static_cast<unsigned char>(ch)));
            } else {
                if (!current.empty()) {
                    words.push_back(current);
                    current.clear();
                }
            }
        }
        if (!current.empty()) words.push_back(current);
        if (words.empty()) return "";

        static std::mt19937 rng(
            std::chrono::steady_clock::now().time_since_epoch().count());
        std::vector<std::string> newWords;
        newWords.push_back(words[0]);
        for (size_t i = 0; i + 1 < words.size(); ++i) {
            const std::string& w1 = words[i];
            const std::string& w2 = words[i + 1];
            int u = getIndex(w1), v = getIndex(w2);
            std::vector<std::string> bridges;
            if (u != -1 && v != -1) {
                for (size_t k = 0; k < nodes.size(); ++k) {
                    if (adjList[u].count(k) && adjList[k].count(v)) {
                        bridges.push_back(nodes[k]);
                    }
                }
            }
            if (!bridges.empty()) {
                std::uniform_int_distribution<size_t> dist(0,
                                                           bridges.size() - 1);
                newWords.push_back(bridges[dist(rng)]);
            }
            newWords.push_back(w2);
        }
        std::ostringstream oss;
        for (size_t i = 0; i < newWords.size(); ++i) {
            if (i > 0) oss << ' ';
            oss << newWords[i];
        }
        return oss.str();
    }

    std::string calcShortestPath(const std::string& word1,
                                 const std::string& word2) const {
        std::string w1 = toLower(word1), w2 = toLower(word2);
        int src = getIndex(w1), dst = getIndex(w2);
        if (src == -1 && dst == -1) {
            return "No \"" + w1 + "\" and \"" + w2 + "\" in the graph!";
        }
        if (src == -1) return "No \"" + w1 + "\" in the graph!";
        if (dst == -1) return "No \"" + w2 + "\" in the graph!";

        const int INF = 1e9;
        int n = nodes.size();
        std::vector<int> dist(n, INF);
        std::vector<bool> visited(n, false);
        dist[src] = 0;
        using P = std::pair<int, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        pq.emplace(0, src);
        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();
            if (visited[u]) continue;
            visited[u] = true;
            if (u == dst) break;
            for (const auto& [v, w] : adjList[u]) {
                if (d + w < dist[v]) {
                    dist[v] = d + w;
                    pq.emplace(dist[v], v);
                }
            }
        }
        if (dist[dst] == INF) {
            return "No path from \"" + w1 + "\" to \"" + w2 + "\"!";
        }

        std::vector<std::vector<int>> paths;
        std::vector<int> currentPath = {src};
        std::function<void(int)> dfs = [&](int u) {
            if (u == dst) {
                paths.push_back(currentPath);
                return;
            }
            for (const auto& [v, w] : adjList[u]) {
                if (dist[u] + w == dist[v]) {
                    currentPath.push_back(v);
                    dfs(v);
                    currentPath.pop_back();
                    if (paths.size() >= 3) return;
                }
            }
        };
        dfs(src);
        std::ostringstream oss;
        oss << "Shortest path length (sum of weights): " << dist[dst] << "\n";
        oss << "Path" << (paths.size() > 1 ? "s" : "") << ":\n";
        for (size_t i = 0; i < paths.size(); ++i) {
            oss << "  ";
            for (size_t j = 0; j < paths[i].size(); ++j) {
                if (j > 0) oss << " -> ";
                oss << nodes[paths[i][j]];
            }
            oss << "\n";
        }
        return oss.str();
    }

    std::string calcShortestPathsFromWord(const std::string& word) const {
        std::string w = toLower(word);
        int src = getIndex(w);
        if (src == -1) return "No \"" + w + "\" in the graph!";

        int n = nodes.size();
        const int INF = 1e9;
        std::vector<int> dist(n, INF);
        std::vector<int> prev(n, -1);
        dist[src] = 0;
        using P = std::pair<int, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        pq.emplace(0, src);
        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();
            if (d > dist[u]) continue;
            for (const auto& [v, w] : adjList[u]) {
                if (d + w < dist[v]) {
                    dist[v] = d + w;
                    prev[v] = u;
                    pq.emplace(dist[v], v);
                }
            }
        }
        std::ostringstream oss;
        oss << "Shortest paths from \"" << w << "\" to all other nodes:\n";
        for (int i = 0; i < n; ++i) {
            if (i == src) continue;
            oss << "To \"" << nodes[i] << "\": ";
            if (dist[i] == INF) {
                oss << "unreachable\n";
            } else {
                oss << "distance = " << dist[i] << ", path: ";
                std::vector<int> path;
                for (int v = i; v != -1; v = prev[v]) path.push_back(v);
                std::reverse(path.begin(), path.end());
                for (size_t j = 0; j < path.size(); ++j) {
                    if (j > 0) oss << " -> ";
                    oss << nodes[path[j]];
                }
                oss << "\n";
            }
        }
        return oss.str();
    }

    double calPageRank(const std::string& word) const {
        std::string w = toLower(word);
        int idx = getIndex(w);
        if (idx == -1) return -1.0;
        int n = nodes.size();
        if (n == 0) return 0.0;
        const double d = 0.85;
        const double eps = 1e-8;
        const int maxIter = 200;
        std::vector<double> pr(n, 1.0 / n);
        for (int iter = 0; iter < maxIter; ++iter) {
            std::vector<double> newPr(n, (1.0 - d) / n);
            double danglingSum = 0.0;
            for (int u = 0; u < n; ++u) {
                if (adjList[u].empty()) danglingSum += pr[u];
            }
            double addToAll = d * danglingSum / n;
            for (int u = 0; u < n; ++u) {
                if (adjList[u].empty()) continue;
                double outSum = outDegreeSum(u);
                for (const auto& [v, weight] : adjList[u]) {
                    newPr[v] += d * pr[u] * (weight / outSum);
                }
            }
            for (int v = 0; v < n; ++v) newPr[v] += addToAll;
            double diff = 0.0;
            for (int i = 0; i < n; ++i) diff += std::abs(newPr[i] - pr[i]);
            pr = std::move(newPr);
            if (diff < eps) break;
        }
        return pr[idx];
    }

    std::string randomWalk() const {
        if (nodes.empty()) return "";
        static std::mt19937 rng(
            std::chrono::steady_clock::now().time_since_epoch().count());
        int current;
        if (m_randomWalkStart == -1) {
            std::uniform_int_distribution<int> nodeDist(0, nodes.size() - 1);
            current = nodeDist(rng);
        } else {
            current = m_randomWalkStart;
        }
        std::vector<std::string> path;
        path.push_back(nodes[current]);
        std::set<std::pair<int, int>> visitedEdges;
        std::cout << "\n随机游走开始";
        if (m_randomWalkFixedLength) {
            std::cout << "，目标词数: " << m_randomWalkLength;
        }
        std::cout << "\n当前路径: " << nodes[current];

        if (!m_randomWalkFixedLength) {
            while (true) {
                if (kbhit()) {
                    char ch = getch();
                    if (ch == 'q' || ch == 'Q') {
                        path.push_back("INTERRUPT");
                        break;
                    }
                }
                if (adjList[current].empty()) break;
                std::vector<int> targets, weights;
                for (const auto& [v, w] : adjList[current]) {
                    targets.push_back(v);
                    weights.push_back(w);
                }
                std::discrete_distribution<int> dist(weights.begin(),
                                                     weights.end());
                int next = targets[dist(rng)];
                auto edge = std::make_pair(current, next);
                if (visitedEdges.count(edge)) break;
                visitedEdges.insert(edge);
                current = next;
                path.push_back(nodes[current]);
                std::cout << " -> " << nodes[current];
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } else {
            while (path.size() < static_cast<size_t>(m_randomWalkLength)) {
                if (adjList[current].empty()) break;
                std::vector<int> targets, weights;
                for (const auto& [v, w] : adjList[current]) {
                    targets.push_back(v);
                    weights.push_back(w);
                }
                std::discrete_distribution<int> dist(weights.begin(),
                                                     weights.end());
                int next = targets[dist(rng)];
                auto edge = std::make_pair(current, next);
                if (visitedEdges.count(edge)) break;
                visitedEdges.insert(edge);
                current = next;
                path.push_back(nodes[current]);
                std::cout << " -> " << nodes[current];
            }
            if (path.size() == static_cast<size_t>(m_randomWalkLength) &&
                !adjList[current].empty()) {
                bool hasUnvisited = false;
                for (const auto& [v, w] : adjList[current]) {
                    if (visitedEdges.count({current, v}) == 0) {
                        hasUnvisited = true;
                        break;
                    }
                }
                if (hasUnvisited) path.push_back("INTERRUPT");
            }
        }
        std::ostringstream oss;
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) oss << ' ';
            oss << path[i];
        }
        return oss.str();
    }
};

// 应用状态结构体，避免全局变量
struct AppState {
    Graph graph;
    std::string currentFilePath;
    std::string currentFileName;
};

// 读取单词辅助函数
std::vector<std::string> readWordsFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) throw std::runtime_error("无法打开文件: " + filePath);
    std::vector<std::string> words;
    std::string line, currentWord;
    auto flushWord = [&]() {
        if (!currentWord.empty()) {
            words.push_back(Graph::toLower(currentWord));
            currentWord.clear();
        }
    };
    while (std::getline(file, line)) {
        for (char ch : line) {
            if (std::isalpha(static_cast<unsigned char>(ch))) {
                currentWord.push_back(ch);
            } else {
                flushWord();
            }
        }
        flushWord();
    }
    return words;
}

// 提取第一个单词
std::string extractFirstWord(const std::string& s) {
    std::string word;
    bool inWord = false;
    for (char ch : s) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            word.push_back(ch);
            inWord = true;
        } else if (inWord) {
            break;
        }
    }
    return word;
}

// 加载文件（增加格式校验）
bool loadFile(const std::string& path, bool isDot, AppState& state) {
    if (!fileExists(path)) {
        std::cerr << "文件不存在或无法打开: " << path << std::endl;
        return false;
    }
    try {
        if (isDot) {
            if (!state.graph.loadFromDot(path)) {
                std::cerr << "加载.dot文件失败（解析错误或格式不符）。\n";
                return false;
            }
            if (state.graph.nodeCount() == 0) {
                std::cerr << "警告：从文件中未解析出任何节点，"
                          << "文件可能不是有效的.dot文件，或文件为空。\n";
                std::cout << "是否仍要使用该文件？（y/n）: ";
                char confirm;
                std::cin >> confirm;
                std::cin.ignore();
                if (confirm != 'y' && confirm != 'Y') {
                    std::cout << "已取消加载。\n";
                    return false;
                }
            }
            std::cout << "成功从.dot文件加载有向图。\n";
        } else {
            auto words = readWordsFromFile(path);
            if (words.empty()) {
                std::cerr << "警告：文件中未读取到任何有效单词，"
                          << "文件可能不是纯文本文件或内容为空。\n";
                std::cout << "是否仍要使用该文件？（y/n）: ";
                char confirm;
                std::cin >> confirm;
                std::cin.ignore();
                if (confirm != 'y' && confirm != 'Y') {
                    std::cout << "已取消加载。\n";
                    return false;
                }
            }
            state.graph.buildFromWordList(words);
            std::cout << "成功读取 " << words.size()
                      << " 个单词，构建有向图完成。\n";
        }
        state.currentFilePath = path;
        state.currentFileName = getFileStem(path);
        state.graph.showDirectedGraph(path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return false;
    }
}

#ifndef TESTING

// ==================== 主函数 ====================
int main(int argc, const char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "========== 软件工程lab1：基于文本的有向图处理 ==========\n";

    AppState state;

    // 如果提供了命令行参数，直接加载，失败则退出
    if (argc > 1) {
        std::string path = argv[1];
        std::string ext = path.substr(path.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        bool isDot = (ext == "dot");
        if (!loadFile(path, isDot, state)) return 1;
    } else {
        // 循环直到成功加载文件或用户选择退出
        bool loaded = false;
        while (!loaded) {
            std::cout << "\n请选择操作模式:\n";
            std::cout << "1. 使用已有数据（读取.dot文件）\n";
            std::cout << "2. 新建数据（读取文本文件）\n";
            std::cout << "0. 退出程序\n";
            std::cout << "请输入选择: ";
            int mode;
            std::cin >> mode;
            std::cin.ignore();

            if (mode == 0) {
                std::cout << "程序退出。\n";
                return 0;
            }

            std::string path;
            if (mode == 1) {
                std::cout << "请输入.dot文件路径: ";
                std::getline(std::cin, path);
                loaded = loadFile(path, true, state);
            } else if (mode == 2) {
                std::cout << "请输入文本文件路径: ";
                std::getline(std::cin, path);
                loaded = loadFile(path, false, state);
            } else {
                std::cout << "无效选择，请重新输入。\n";
                continue;
            }

            if (!loaded) {
                std::cout << "加载失败，请重新选择。\n";
            }
        }
    }

    // 主循环
    while (true) {
        std::cout << "\n当前文件: " << state.currentFileName << std::endl;
        std::cout << "========== 功能菜单 ==========\n";
        std::cout << "1. 查询桥接词\n";
        std::cout << "2. 根据桥接词生成新文本\n";
        std::cout << "3. 计算两个单词的最短路径\n";
        std::cout << "4. 计算单个单词到所有其他单词的最短路径（进阶）\n";
        std::cout << "5. 计算单词的PageRank值\n";
        std::cout << "6. 随机游走\n";
        std::cout << "7. 打开新文件\n";
        std::cout << "0. 退出\n";
        std::cout << "请选择: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，请输入数字0-7。\n";
            continue;
        }

        if (choice == 0) break;

        bool backToMenu = false;

        switch (choice) {
            case 1:
                while (!backToMenu) {
                    std::cout << "\n[桥接词查询] 请输入两个单词"
                              << "（输入 'back' 返回菜单，"
                              << "输入 'exit' 或 'quit' 退出程序）: ";
                    std::string line;
                    std::getline(std::cin, line);
                    if (isExitCommand(line)) return 0;
                    if (isBackCommand(line)) {
                        backToMenu = true;
                        break;
                    }
                    if (line.empty()) continue;

                    std::vector<std::string> words;
                    std::string cur;
                    for (char ch : line) {
                        if (std::isalpha(static_cast<unsigned char>(ch))) {
                            cur.push_back(ch);
                        } else {
                            if (!cur.empty()) {
                                words.push_back(cur);
                                cur.clear();
                            }
                        }
                    }
                    if (!cur.empty()) words.push_back(cur);
                    if (words.size() == 1) {
                        std::cout << "请输入第二个单词: ";
                        std::string w2;
                        std::getline(std::cin, w2);
                        if (isExitCommand(w2)) return 0;
                        if (isBackCommand(w2)) {
                            backToMenu = true;
                            break;
                        }
                        std::string w2clean = extractFirstWord(w2);
                        if (w2clean.empty()) {
                            std::cout << "输入无效，请重新开始。\n";
                            continue;
                        }
                        words.push_back(w2clean);
                    } else if (words.size() != 2) {
                        std::cout << "输入单词数量不对，请重新输入。\n";
                        continue;
                    }
                    std::cout << state.graph.queryBridgeWords(words[0],
                                                              words[1])
                              << std::endl;
                }
                break;
            case 2:
                while (!backToMenu) {
                    std::cout << "\n[生成新文本] 请输入一行文本"
                              << "（输入 'back' 返回菜单，"
                              << "输入 'exit' 或 'quit' 退出程序）: ";
                    std::string text;
                    std::getline(std::cin, text);
                    if (isExitCommand(text)) return 0;
                    if (isBackCommand(text)) {
                        backToMenu = true;
                        break;
                    }
                    if (text.empty()) continue;
                    std::cout << "生成的新文本: "
                              << state.graph.generateNewText(text) << std::endl;
                }
                break;
            case 3:
                while (!backToMenu) {
                    std::cout << "\n[最短路径] 请输入两个单词"
                              << "（输入 'back' 返回菜单，"
                              << "输入 'exit' 或 'quit' 退出程序）: ";
                    std::string line;
                    std::getline(std::cin, line);
                    if (isExitCommand(line)) return 0;
                    if (isBackCommand(line)) {
                        backToMenu = true;
                        break;
                    }
                    if (line.empty()) continue;

                    std::vector<std::string> words;
                    std::string cur;
                    for (char ch : line) {
                        if (std::isalpha(static_cast<unsigned char>(ch))) {
                            cur.push_back(ch);
                        } else {
                            if (!cur.empty()) {
                                words.push_back(cur);
                                cur.clear();
                            }
                        }
                    }
                    if (!cur.empty()) words.push_back(cur);
                    if (words.size() == 1) {
                        std::cout << "请输入第二个单词: ";
                        std::string w2;
                        std::getline(std::cin, w2);
                        if (isExitCommand(w2)) return 0;
                        if (isBackCommand(w2)) {
                            backToMenu = true;
                            break;
                        }
                        std::string w2clean = extractFirstWord(w2);
                        if (w2clean.empty()) {
                            std::cout << "输入无效，请重新输入。\n";
                            continue;
                        }
                        words.push_back(w2clean);
                    } else if (words.size() != 2) {
                        std::cout << "输入单词数量不对，请重新输入。\n";
                        continue;
                    }
                    std::cout << state.graph.calcShortestPath(words[0],
                                                              words[1])
                              << std::endl;
                }
                break;
            case 4:
                while (!backToMenu) {
                    std::cout << "\n[单源最短路径] 请输入一个单词"
                              << "（输入 'back' 返回菜单，"
                              << "输入 'exit' 或 'quit' 退出程序）: ";
                    std::string word;
                    std::getline(std::cin, word);
                    if (isExitCommand(word)) return 0;
                    if (isBackCommand(word)) {
                        backToMenu = true;
                        break;
                    }
                    if (word.empty()) continue;
                    std::string w = extractFirstWord(word);
                    if (w.empty()) {
                        std::cout << "输入无效，请重新输入。\n";
                        continue;
                    }
                    std::cout << state.graph.calcShortestPathsFromWord(w)
                              << std::endl;
                }
                break;
            case 5:
                while (!backToMenu) {
                    std::cout << "\n[PageRank] 请输入一个单词"
                              << "（输入 'back' 返回菜单，"
                              << "输入 'exit' 或 'quit' 退出程序）: ";
                    std::string word;
                    std::getline(std::cin, word);
                    if (isExitCommand(word)) return 0;
                    if (isBackCommand(word)) {
                        backToMenu = true;
                        break;
                    }
                    if (word.empty()) continue;
                    std::string w = extractFirstWord(word);
                    if (w.empty()) {
                        std::cout << "输入无效，请重新输入。\n";
                        continue;
                    }
                    double pr = state.graph.calPageRank(w);
                    if (pr < 0) {
                        std::cout << "单词 \"" << w << "\" 不在图中。\n";
                    } else {
                        std::cout << "PageRank值 of \"" << w
                                  << "\": " << pr << std::endl;
                    }
                }
                break;
            case 6:
                while (!backToMenu) {
                    std::cout << "\n[随机游走] 输入 'back' 返回菜单，"
                              << "输入 'exit' 或 'quit' 退出程序，"
                              << "其他键开始设置...\n";
                    std::string input;
                    std::getline(std::cin, input);
                    if (isExitCommand(input)) return 0;
                    if (isBackCommand(input)) {
                        backToMenu = true;
                        break;
                    }

                    std::cout << "1. 随机选择起始词\n";
                    std::cout << "2. 指定起始词\n";
                    int startChoice;
                    std::cin >> startChoice;
                    std::cin.ignore();
                    if (startChoice == 1) {
                        state.graph.setRandomWalkRandomStart();
                    } else if (startChoice == 2) {
                        std::cout << "请输入起始词: ";
                        std::string word;
                        std::getline(std::cin, word);
                        std::string w = extractFirstWord(word);
                        if (w.empty() || !state.graph.hasWord(w)) {
                            std::cout << "单词无效或不在图中，"
                                      << "将随机选择起始词。\n";
                            state.graph.setRandomWalkRandomStart();
                        } else {
                            state.graph.setRandomWalkStart(w);
                        }
                    } else {
                        std::cout << "无效选择，将随机选择起始词。\n";
                        state.graph.setRandomWalkRandomStart();
                    }

                    std::cout << "请选择游走模式:\n";
                    std::cout << "1. 指定生成词数\n";
                    std::cout << "2. 不定词数（自动每0.1秒生成，按q停止）\n";
                    int lenChoice;
                    std::cin >> lenChoice;
                    std::cin.ignore();
                    if (lenChoice == 1) {
                        std::cout << "请输入目标词数: ";
                        int n;
                        std::cin >> n;
                        std::cin.ignore();
                        if (n <= 0) {
                            n = 10;
                            std::cout << "词数必须为正数，使用默认值10。\n";
                        }
                        state.graph.setRandomWalkFixedLength(n);
                    } else if (lenChoice == 2) {
                        state.graph.setRandomWalkInfinite();
                    } else {
                        std::cout << "无效选择，使用不定词数模式。\n";
                        state.graph.setRandomWalkInfinite();
                    }

                    std::string walk = state.graph.randomWalk();
                    std::cout << "\n随机游走路径: " << walk << std::endl;

                    std::string dir = getParentDir(state.currentFilePath);
                    std::string stem = getFileStem(state.currentFilePath);
                    std::string outPath = dir + stem + "random.txt";
                    std::ofstream ofs(outPath);
                    if (ofs) {
                        ofs << walk;
                        std::cout << "随机游走文本已保存至: "
                                  << outPath << std::endl;
                    } else {
                        std::cerr << "无法保存文件。" << std::endl;
                    }
                }
                break;
            case 7: {
                std::cout << "\n=== 打开新文件 ===\n";
                std::cout << "请选择操作模式:\n";
                std::cout << "1. 使用已有数据（读取.dot文件）\n";
                std::cout << "2. 新建数据（读取文本文件）\n";
                std::cout << "3. 放弃并返回菜单\n";
                std::cout << "0. 退出程序\n";
                std::cout << "请输入选择: ";
                int mode;
                std::cin >> mode;
                std::cin.ignore();
                if (mode == 0) return 0;
                if (mode == 3) break;
                std::string path;
                bool loaded = false;
                if (mode == 1) {
                    std::cout << "请输入.dot文件路径: ";
                    std::getline(std::cin, path);
                    loaded = loadFile(path, true, state);
                } else if (mode == 2) {
                    std::cout << "请输入文本文件路径: ";
                    std::getline(std::cin, path);
                    loaded = loadFile(path, false, state);
                } else {
                    std::cout << "无效选择，返回菜单。\n";
                    break;
                }
                if (loaded) {
                    std::cout << "已切换到新文件。\n";
                } else {
                    std::cout << "加载失败，保留原文件。\n";
                }
                break;
            }
            default:
                std::cout << "无效选择，请重新输入。\n";
        }
    }

    std::cout << "程序结束。\n";
    return 0;
}
#endif
