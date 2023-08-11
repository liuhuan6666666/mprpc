#include <mprpcconfig.h>
#include <iostream>
#include <string>
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (pf == nullptr)
    {
        std::cout << config_file << " is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 1.去掉注释  2.正确的配置项  3.去掉开头的多余空格
    while (!feof(pf))  //feof()是检测流上的文件结束符的函数，如果文件结束，则返回非0值，否则返回0
    {
        char buf[512];
        fgets(buf, 512, pf); // 读取一行

        std::string read_buf(buf);
        Trim(read_buf);
        // 判断#的注释
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }
        // 解析配置项
        int idx = read_buf.find('=');
        if (idx == -1)
        {
            continue;
        }
        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        Trim(read_buf);
        int enidx=read_buf.find('\n',idx);
        value = read_buf.substr(idx + 1, enidx-idx-1);
        Trim(read_buf);
        m_configMap.insert({key, value});
    }
}
// 去掉空格
void MprpcConfig::Trim(std::string &src_buf)
{

    int idx = src_buf.find_first_not_of(' '); // 寻找第一个不为空格的index
    if (idx != -1)
    {
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    // 去掉字符串后面的空格
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        src_buf = src_buf.substr(0, idx + 1);
    }
}
// 查询配置项信息
std::string MprpcConfig::Load(std::string key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}