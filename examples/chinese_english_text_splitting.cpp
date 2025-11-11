#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ 中英文语义文本分块测试\n";
    std::cout << "=====================================\n\n";

    // 测试英文文本
    std::cout << "测试1: 英文文本分块\n";
    String english_text =
        "Artificial Intelligence (AI) is a branch of computer science that aims to create software or machines that exhibit human-like intelligence. "
        "This can include learning from experience, understanding natural language, solving problems, and recognizing patterns. "
        "Machine learning is a subset of AI that focuses on algorithms that can learn and improve from data without being explicitly programmed. "
        "Deep learning is a further subset of machine learning that uses neural networks with multiple layers to model complex patterns in data. "
        "Natural Language Processing (NLP) is another important area of AI that deals with the interaction between computers and humans using natural language. "
        "AI has many applications including image recognition, speech recognition, autonomous vehicles, and recommendation systems. "
        "The development of AI raises important ethical questions about privacy, bias, and the impact on employment. "
        "Researchers continue to work on making AI systems more transparent, fair, and beneficial to society.";

    std::cout << "原始英文文本长度: " << english_text.length() << " 字符\n";

    auto english_splitter = std::make_shared<TextSplitter>(200, 50); // 小块大小便于测试
    auto english_chunks = english_splitter->split_text(english_text);

    std::cout << "分块数量: " << english_chunks.size() << std::endl;
    for (size_t i = 0; i < english_chunks.size() && i < 3; ++i) {
        std::cout << "块 " << (i + 1) << " 长度: " << english_chunks[i].length() << " 字符\n";
        std::cout << "内容预览: " << english_chunks[i].substr(0, 100) << "...\n\n";
    }

    // 测试中文文本
    std::cout << "测试2: 中文文本分块\n";
    String chinese_text =
        "人工智能是计算机科学的一个分支，它企图了解智能的实质，并生产出一种新的能以人类智能相似的方式做出反应的智能机器。"
        "人工智能研究包括机器人、语言识别、图像识别、自然语言处理和专家系统等。"
        "机器学习是人工智能的一个重要分支，它使计算机能够从数据中学习并做出预测或决策，而无需明确编程。"
        "深度学习是机器学习的一个子集，它使用多层神经网络来模拟复杂的数据模式。"
        "自然语言处理是人工智能的另一个重要领域，它处理计算机与人类使用自然语言进行交互的问题。"
        "人工智能有许多应用，包括图像识别、语音识别、自动驾驶汽车和推荐系统。"
        "人工智能的发展引发了关于隐私、偏见和就业影响的重要伦理问题。"
        "研究人员继续努力使人工智能系统更加透明、公平并对社会有益。";

    std::cout << "原始中文文本长度: " << chinese_text.length() << " 字符\n";

    auto chinese_splitter = std::make_shared<TextSplitter>(100, 30); // 小块大小便于测试
    auto chinese_chunks = chinese_splitter->split_text(chinese_text);

    std::cout << "分块数量: " << chinese_chunks.size() << std::endl;
    for (size_t i = 0; i < chinese_chunks.size() && i < 3; ++i) {
        std::cout << "块 " << (i + 1) << " 长度: " << chinese_chunks[i].length() << " 字符\n";
        std::cout << "内容预览: " << chinese_chunks[i].substr(0, 50) << "...\n\n";
    }

    // 测试中英文混合文本
    std::cout << "测试3: 中英文混合文本分块\n";
    String mixed_text =
        "人工智能 (Artificial Intelligence, AI) 是计算机科学的一个重要分支。它致力于创造能够展现类人智能的软件或机器。"
        "This includes learning from experience, understanding natural language, solving problems, and recognizing patterns. "
        "机器学习是人工智能的一个子集，专注于能够从数据中学习和改进的算法。"
        "Machine learning is a subset of AI that focuses on algorithms that can learn and improve from data. "
        "深度学习使用多层神经网络来建模复杂的数据模式。"
        "Deep learning uses neural networks with multiple layers to model complex patterns in data. "
        "自然语言处理是人工智能的另一个重要领域。"
        "Natural Language Processing (NLP) is another important area of AI. "
        "人工智能的发展引发了许多伦理问题。"
        "The development of AI raises important ethical questions. ";

    std::cout << "原始混合文本长度: " << mixed_text.length() << " 字符\n";

    auto mixed_splitter = std::make_shared<TextSplitter>(150, 40);
    auto mixed_chunks = mixed_splitter->split_text(mixed_text);

    std::cout << "分块数量: " << mixed_chunks.size() << std::endl;
    for (size_t i = 0; i < mixed_chunks.size() && i < 3; ++i) {
        std::cout << "块 " << (i + 1) << " 长度: " << mixed_chunks[i].length() << " 字符\n";
        std::cout << "内容预览: " << mixed_chunks[i].substr(0, 80) << "...\n\n";
    }

    // 测试不同配置
    std::cout << "测试4: 不同配置的文本分块\n";
    String test_text =
        "这是第一个句子。这是第二个句子！这是第三个句子？这是第四个句子；这是第五个句子。"
        "This is the first sentence. This is the second sentence! This is the third sentence? This is the fourth sentence; This is the fifth sentence.";

    std::cout << "测试文本长度: " << test_text.length() << " 字符\n";

    // 大块大小
    auto large_splitter = std::make_shared<TextSplitter>(100, 20);
    auto large_chunks = large_splitter->split_text(test_text);
    std::cout << "大块配置 (100, 20) 分块数量: " << large_chunks.size() << std::endl;

    // 小块大小
    auto small_splitter = std::make_shared<TextSplitter>(50, 10);
    auto small_chunks = small_splitter->split_text(test_text);
    std::cout << "小块配置 (50, 10) 分块数量: " << small_chunks.size() << std::endl;

    // 验证语义完整性
    std::cout << "\n验证语义完整性:\n";
    if (!large_chunks.empty()) {
        std::cout << "大块第一个分块: " << large_chunks[0] << std::endl;
    }
    if (!small_chunks.empty()) {
        std::cout << "小块第一个分块: " << small_chunks[0] << std::endl;
    }

    std::cout << "\n中英文语义文本分块测试完成！\n";

    return 0;
}