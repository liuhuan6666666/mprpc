该项目是基于zookeeper分布式器,protobuf库和muduo库实现的分布式通信框架。一个工程可以在多个服务器上实现。在该项目中，主要使用zookeeper的注册中心服务来保存各个服务器的ip和port，客户端通过该注册中心来访问各个服务的函数。利用zookeeper自身API的心跳机制对各个服务进行维护。protobuf是用于对数据的序列化与反序列化，调用方与被调用方需要使用一种消息类型，RPC接收和发送需要一个信息类型。muduo主要是用来实现该RPC框架的高并发，对于muduo库连接操作以及消息操作函数进行了重写。
主要流程：
    被调用方：首先对框架初始化，主要是将自己服务器ip和port放在具有单列模式的哈希map中，然后将服务和函数放在给rpc节点的map表中，然后创建muduo库并将rpc节点的服务注册在zookeeper上，对于zookeeper注册，服务创建为永久性节点，函数被创建为临时性节点，主要是为了防止该服务器函数的增删。zookeeper创建节点的细节，要一层一层的创建才行，
