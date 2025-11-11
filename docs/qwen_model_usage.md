# Qwen (千问) 模型使用说明

## 简介

QwenModel 是 LangChain C++ 框架中实现的千问模型接口，允许您通过阿里云 DashScope API 使用千问系列模型。

## 配置要求

1. 需要从阿里云获取 API 密钥
2. 需要设置环境变量 `QWEN_API_KEY`

## 使用方法

### 1. 设置环境变量

```bash
export QWEN_API_KEY=your_actual_api_key_here
```

### 2. 创建 Qwen 模型实例

```cpp
#include "langchain/langchain.h"

using namespace langchain;

// 创建 Qwen 模型实例
auto qwen_model = std::make_shared<QwenModel>(String(qwen_api_key), "qwen-turbo");

// 设置参数
qwen_model->set_temperature(0.7);
qwen_model->set_max_tokens(500);
```

### 3. 生成文本

```cpp
String prompt = "什么是人工智能？";
String response = qwen_model->generate(prompt);
std::cout << "Response: " << response << std::endl;
```

## 支持的模型

- qwen-turbo：快速推理模型
- qwen-plus：平衡性能与成本的模型
- qwen-max：效果最好的模型

## API 端点

千问模型使用以下 API 端点：
`https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation`

## 示例代码

完整的示例代码请参考 `examples/qwen_model_test.cpp` 文件。

## 注意事项

1. 请妥善保管您的 API 密钥，不要在代码中硬编码
2. 使用环境变量传递 API 密钥是推荐的做法
3. 注意 API 调用的配额和费用