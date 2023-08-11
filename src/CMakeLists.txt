aux_source_directory(. SRC_LIST)
add_library(mprpc ${SRC_LIST})  #生成静态库
target_link_libraries(mprpc muduo_net muduo_base  pthread zookeeper_mt) #zookeeper_mt:多线程   zookeeper_st：单线程版本
