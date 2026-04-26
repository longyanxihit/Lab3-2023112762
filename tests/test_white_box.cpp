#include <gtest/gtest.h>
#include "../TextGraph.cpp"

/**
 * 白盒测试夹具：提供三个不同的图用于覆盖最短路径算法的各种分支。
 * - buildWeightedGraph : 带权图（A->B 权重3, B->C 1, A->D 2, D->C 2, A->C 5）
 *   A->C的最短路径有两条：A->B->C 和 A->D->C，长度均为4。
 * - buildSimpleGraph   : 简单图（M->N, N->P, M->P），所有边权重1。
 *   M->P的最短路径为直接边M->P（长度1）。
 * - buildUnreachableGraph: 不可达图（X->Y->Z 连通，Isolated 孤立自环）
 *   从X无法到达Isolated。
 */
class ShortestPathWhiteBoxTest : public ::testing::Test {
protected:
    Graph buildWeightedGraph() {
        Graph g;
        for (int i = 0; i < 3; ++i) g.addEdge("a", "b");
        g.addEdge("b", "c");
        for (int i = 0; i < 5; ++i) g.addEdge("a", "c");
        for (int i = 0; i < 2; ++i) g.addEdge("a", "d");
        for (int i = 0; i < 2; ++i) g.addEdge("d", "c");
        return g;
    }

    Graph buildSimpleGraph() {
        Graph g;
        g.addEdge("m", "n");
        g.addEdge("n", "p");
        g.addEdge("m", "p");
        return g;
    }

    Graph buildUnreachableGraph() {
        Graph g;
        g.addEdge("x", "y");
        g.addEdge("y", "z");
        g.addEdge("isolated", "isolated");
        return g;
    }
};

// ========== calcShortestPath 测试 ==========

// 基本路径1：两个单词都不在图中
// 输入：word1="no1", word2="no2"
// 预期：返回 "No \"no1\" and \"no2\" in the graph!"
TEST_F(ShortestPathWhiteBoxTest, BothWordsNotInGraph) {
    Graph g = buildSimpleGraph();
    std::string result = g.calcShortestPath("no1", "no2");
    EXPECT_EQ(result, "No \"no1\" and \"no2\" in the graph!");
}

// 基本路径2：第一个单词不在图中
// 输入：word1="no1", word2="m"（图中存在）
// 预期：返回 "No \"no1\" in the graph!"
TEST_F(ShortestPathWhiteBoxTest, FirstWordNotInGraph) {
    Graph g = buildSimpleGraph();
    std::string result = g.calcShortestPath("no1", "m");
    EXPECT_EQ(result, "No \"no1\" in the graph!");
}

// 基本路径3：第二个单词不在图中
// 输入：word1="m", word2="no1"
// 预期：返回 "No \"no1\" in the graph!"
TEST_F(ShortestPathWhiteBoxTest, SecondWordNotInGraph) {
    Graph g = buildSimpleGraph();
    std::string result = g.calcShortestPath("m", "no1");
    EXPECT_EQ(result, "No \"no1\" in the graph!");
}

// 基本路径4：两个单词都在图中但无路径
// 输入：word1="x", word2="isolated"
// 预期：返回 "No path from \"x\" to \"isolated\"!"
TEST_F(ShortestPathWhiteBoxTest, NoPathBetweenWords) {
    Graph g = buildUnreachableGraph();
    std::string result = g.calcShortestPath("x", "isolated");
    EXPECT_EQ(result, "No path from \"x\" to \"isolated\"!");
}

// 基本路径5：存在唯一最短路径
// 输入：word1="m", word2="p"，图中 m->p 直接边权重1，m->n->p 权重2
// 预期：最短路径长度为1，路径为 "m -> p"
TEST_F(ShortestPathWhiteBoxTest, UniqueShortestPath) {
    Graph g = buildSimpleGraph();
    std::string result = g.calcShortestPath("m", "p");
    EXPECT_NE(result.find("Shortest path length (sum of weights): 1"), std::string::npos);
    EXPECT_NE(result.find("m -> p"), std::string::npos);
}

// 基本路径6：多条等长最短路径（权重影响）
// 输入：word1="a", word2="c"，图中有 a->b->c (3+1=4) 和 a->d->c (2+2=4)
// 预期：最短路径长度为4，输出两条路径（a->b->c 和 a->d->c），不输出 a->c（权重5）
TEST_F(ShortestPathWhiteBoxTest, MultipleShortestPathsWithWeights) {
    Graph g = buildWeightedGraph();
    std::string result = g.calcShortestPath("a", "c");
    EXPECT_NE(result.find("Shortest path length (sum of weights): 4"), std::string::npos);
    EXPECT_NE(result.find("a -> b -> c"), std::string::npos);
    EXPECT_NE(result.find("a -> d -> c"), std::string::npos);
    EXPECT_EQ(result.find("a -> c"), std::string::npos);
}

// 额外场景：起点等于终点
// 输入：word1="m", word2="m"
// 预期：距离为0，路径只包含 "m"
TEST_F(ShortestPathWhiteBoxTest, SameWord) {
    Graph g = buildSimpleGraph();
    std::string result = g.calcShortestPath("m", "m");
    EXPECT_NE(result.find("Shortest path length (sum of weights): 0"), std::string::npos);
    EXPECT_NE(result.find("m"), std::string::npos);
}

// ========== calcShortestPathsFromWord 测试 ==========

// 单源最短路径：起点不在图中
// 输入：word="nosource"
// 预期：返回 "No \"nosource\" in the graph!"
TEST_F(ShortestPathWhiteBoxTest, SingleSourceSourceNotInGraph) {
    Graph g = buildSimpleGraph();
    std::string result = g.calcShortestPathsFromWord("nosource");
    EXPECT_EQ(result, "No \"nosource\" in the graph!");
}

// 单源最短路径：起点存在，且所有节点可达
// 输入：word="m"（简单图）
// 预期：输出到 n 和 p 的距离和路径（均为1）
TEST_F(ShortestPathWhiteBoxTest, SingleSourceToAllReachable) {
    Graph g = buildSimpleGraph();
    std::string result = g.calcShortestPathsFromWord("m");
    EXPECT_NE(result.find("To \"n\": distance = 1, path: m -> n"), std::string::npos);
    EXPECT_NE(result.find("To \"p\": distance = 1, path: m -> p"), std::string::npos);
}

// 单源最短路径：起点存在，包含不可达节点
// 输入：word="x"，图中 isolated 不可达
// 预期：输出到 y、z 的路径，isolated 标记为 unreachable
TEST_F(ShortestPathWhiteBoxTest, SingleSourceWithUnreachable) {
    Graph g = buildUnreachableGraph();
    std::string result = g.calcShortestPathsFromWord("x");
    EXPECT_NE(result.find("To \"y\": distance = 1, path: x -> y"), std::string::npos);
    EXPECT_NE(result.find("To \"z\": distance = 2, path: x -> y -> z"), std::string::npos);
    EXPECT_NE(result.find("To \"isolated\": unreachable"), std::string::npos);
}

// 单源最短路径：带权图中验证各节点最短距离和路径
// 输入：word="a"（加权图）
// 预期：到 b 距离3，到 d 距离2，到 c 距离4（路径可能有多条）
TEST_F(ShortestPathWhiteBoxTest, SingleSourceWeightedGraph) {
    Graph g = buildWeightedGraph();
    std::string result = g.calcShortestPathsFromWord("a");
    EXPECT_NE(result.find("To \"b\": distance = 3, path: a -> b"), std::string::npos);
    EXPECT_NE(result.find("To \"c\": distance = 4, path:"), std::string::npos);
    EXPECT_NE(result.find("To \"d\": distance = 2, path: a -> d"), std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}