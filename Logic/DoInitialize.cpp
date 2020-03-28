#include "Logic.hpp"
#include <stdexcept>

AlohaIO::TfcConfigCodec LibConf;

void DoInitialize(const char *conf_file)
{
    puts("Parsing business library config file...");
    int ret = LibConf.ParseFile(conf_file);
    if (ret != 0)
    {
        throw std::runtime_error(string("Parse config file failed: ").append(conf_file));
    }

}