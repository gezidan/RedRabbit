#include <stdio.h>
#include "base/daemon_tool.h"
#include "base/arg_helper.h"
#include "base/strtool.h"
#include "base/smart_ptr.h"

#include "rpc/ffrpc.h"
#include "rpc/ffbroker.h"
#include "base/log.h"
#include "rpc/ffscene_python.h"
#include "rpc/ffgate.h"
#include "base/signal_helper.h"
#include "base/daemon_tool.h"
#include "base/performance_daemon.h"

using namespace ff;
//./app_redrabbit -gate gate@0 -broker tcp://127.0.0.1:10241 -gate_listen tcp://121.199.21.238:10242 -python_path ./ -scene scene@0

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        printf("./app_redrabbit -f default.config\n");
        return 0;
    }

    arg_helper_t arg_helper(argc, argv);
    arg_helper.load_from_file("default.config");

    if (arg_helper.is_enable_option("-d"))
    {
        daemon_tool_t::daemon();
    }
    
    //! 美丽的日志组件，shell输出是彩色滴！！
    if (arg_helper.is_enable_option("-log_path"))
    {
        LOG.start(arg_helper);
    }
    else
    {
        LOG.start("-log_path ./log -log_filename log -log_class DB_MGR,XX,BROKER,FFRPC,FFGATE,FFSCENE,FFSCENE_PYTHON,FFNET -log_print_screen true -log_print_file true -log_level 6");
    }
    string perf_path = "./perf";
    long perf_timeout = 30*60;//! second
    if (arg_helper.is_enable_option("-perf_path"))
    {
        perf_path = arg_helper.get_option_value("-perf_path");
    }
    if (arg_helper.is_enable_option("-perf_timeout"))
    {
        perf_timeout = ::atoi(arg_helper.get_option_value("-perf_timeout").c_str());
    }
    if (PERF_MONITOR.start(perf_path, perf_timeout))
    {
        return -1;
    }
    ffbroker_t ffbroker;
    ffgate_t ffgate;
    //ffscene_python_t ffscene_python;

    try
    {
        //! 启动broker，负责网络相关的操作，如消息转发，节点注册，重连等
        if (arg_helper.is_enable_option("-broker"))
        {
            if (ffbroker.open(arg_helper))
            {
                printf("broker open failed\n");
                return -1;
            }
        }
        if (arg_helper.is_enable_option("-gate"))
        {
            if (ffgate.open(arg_helper))
            {
                printf("gate open error!\n");
                return -1;
            }
        }
        if (arg_helper.is_enable_option("-scene"))
        {
            if (singleton_t<ffscene_python_t>::instance().open(arg_helper))
            {
                printf("scene open error!\n");
                return -1;
            }
        }
    }
    catch(exception& e_)
    {
        printf("exception=%s\n", e_.what());
        return -1;
    }

    signal_helper_t::wait();

    ffgate.close();
    singleton_t<ffscene_python_t>::instance().close();
    ffbroker.close();
    net_factory_t::stop();
    usleep(500);
    return 0;
}
