#include <gtest/gtest.h>
#include "../TextGraph.cpp"

/**
 * 黑盒测试夹具：构建一个固定的有向图用于桥接词查询测试。
 * 图结构（所有单词以小写存储）：
 *   new -> hello, hi, good
 *   hello -> world
 *   hi -> world
 *   good -> world
 *   world -> end
 *   loop -> loop（自环）
 *   isolated -> isolated（孤立自环）
 */
class BridgeWordsBlackBoxTest : public ::testing::Test {
protected:
    Graph graph;

    void SetUp() override {
        graph.addEdge("new", "hello");
        graph.addEdge("new", "hi");
        graph.addEdge("new", "good");
        graph.addEdge("hello", "world");
        graph.addEdge("hi", "world");
        graph.addEdge("good", "world");
        graph.addEdge("world", "end");
        graph.addEdge("loop", "loop");
        graph.addEdge("isolated", "isolated");
    }
};

// 测试目标：两个单词都存在且存在多个桥接词（new -> world 有 hello,hi,good）
// 输入：word1="new", word2="world"
// 预期：输出包含三个桥接词，并使用复数形式 "bridge words" 和 "are"
TEST_F(BridgeWordsBlackBoxTest, BothExistWithMultipleBridges) {
    std::string result = graph.queryBridgeWords("new", "world");
    EXPECT_NE(result.find("hello"), std::string::npos);
    EXPECT_NE(result.find("hi"), std::string::npos);
    EXPECT_NE(result.find("good"), std::string::npos);
    EXPECT_NE(result.find("bridge words"), std::string::npos);
    EXPECT_NE(result.find("are"), std::string::npos);
}

// 测试目标：两个单词都存在且只有一个桥接词（new -> direct -> end）
// 输入：word1="new", word2="end"，临时添加 direct 作为唯一桥接词
// 预期：输出包含 "direct"，使用单数 "bridge word" 和 "is"
TEST_F(BridgeWordsBlackBoxTest, BothExistWithSingleBridge) {
    Graph g = graph;
    g.addEdge("new", "direct");
    g.addEdge("direct", "end");
    std::string result = g.queryBridgeWords("new", "end");
    EXPECT_NE(result.find("direct"), std::string::npos);
    EXPECT_EQ(result.find("bridge words"), std::string::npos);  // 复数不应出现
    EXPECT_NE(result.find("bridge word"), std::string::npos);
    EXPECT_NE(result.find("is"), std::string::npos);
}

// 测试目标：两个单词都存在但无桥接词（hello -> world 是直接边，没有中间节点）
// 输入：word1="hello", word2="world"
// 预期：返回 "No bridge words from \"hello\" to \"world\"!"
TEST_F(BridgeWordsBlackBoxTest, BothExistNoBridge) {
    std::string result = graph.queryBridgeWords("hello", "world");
    EXPECT_EQ(result, "No bridge words from \"hello\" to \"world\"!");
}

// 测试目标：第一个单词不在图中
// 输入：word1="nonexist", word2="world"
// 预期：返回 "No \"nonexist\" in the graph!"
TEST_F(BridgeWordsBlackBoxTest, FirstWordNotExist) {
    std::string result = graph.queryBridgeWords("nonexist", "world");
    EXPECT_EQ(result, "No \"nonexist\" in the graph!");
}

// 测试目标：第二个单词不在图中
// 输入：word1="new", word2="nonexist"
// 预期：返回 "No \"nonexist\" in the graph!"
TEST_F(BridgeWordsBlackBoxTest, SecondWordNotExist) {
    std::string result = graph.queryBridgeWords("new", "nonexist");
    EXPECT_EQ(result, "No \"nonexist\" in the graph!");
}

// 测试目标：两个单词都不在图中
// 输入：word1="aaa", word2="bbb"
// 预期：返回 "No \"aaa\" and \"bbb\" in the graph!"
TEST_F(BridgeWordsBlackBoxTest, NeitherExist) {
    std::string result = graph.queryBridgeWords("aaa", "bbb");
    EXPECT_EQ(result, "No \"aaa\" and \"bbb\" in the graph!");
}

// 边界测试：空字符串作为单词
// 输入：word1="", word2="world"
// 预期：空串当作单词，不在图中，返回 "No \"\" in the graph!"
TEST_F(BridgeWordsBlackBoxTest, EmptyWord) {
    std::string result = graph.queryBridgeWords("", "world");
    EXPECT_EQ(result, "No \"\" in the graph!");
}

// 边界测试：超长单词（1000个字符）
// 输入：word1=1000个'x', word2="world"
// 预期：超长单词不在图中，返回相应错误信息
TEST_F(BridgeWordsBlackBoxTest, VeryLongWord) {
    std::string longWord(1000, 'x');
    std::string result = graph.queryBridgeWords(longWord, "world");
    EXPECT_EQ(result, "No \"" + longWord + "\" in the graph!");
}

// 测试目标：大小写不敏感
// 输入：word1="NEW", word2="WORLD"（大写）
// 预期：应该找到小写存储的桥接词 "hello"
TEST_F(BridgeWordsBlackBoxTest, CaseInsensitive) {
    std::string result = graph.queryBridgeWords("NEW", "WORLD");
    EXPECT_NE(result.find("hello"), std::string::npos);
}

// 测试目标：自环节点作为桥接词（需要添加 new->loop 和 loop->world）
// 输入：word1="new", word2="world"，临时添加 loop 作为中间节点
// 预期：输出包含 "loop"
TEST_F(BridgeWordsBlackBoxTest, SelfLoopAsBridge) {
    Graph g = graph;
    g.addEdge("new", "loop");
    g.addEdge("loop", "world");
    std::string result = g.queryBridgeWords("new", "world");
    EXPECT_NE(result.find("loop"), std::string::npos);
}

// 测试目标：孤立节点（只有自环）作为起点，没有出边到目标
// 输入：word1="isolated", word2="world"
// 预期：无桥接词，返回 "No bridge words ..."
TEST_F(BridgeWordsBlackBoxTest, IsolatedNodeAsStart) {
    std::string result = graph.queryBridgeWords("isolated", "world");
    EXPECT_EQ(result, "No bridge words from \"isolated\" to \"world\"!");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}