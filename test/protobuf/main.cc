#include"test.pb.h"
#include<iostream>
#include<string>
using namespace fixbug;
int main(){
    // LoginResponse raq;
    // Resultcode *rc=raq.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("login error!");
    // std::string send_str;
    // if(raq.SerializeToString(&send_str)){
    //     std::cout<<send_str<<std::endl;
    // }
    //反序列化一个loginrequest对象数据
    GetFriendListsResponse rsp;
    Resultcode *rc=rsp.mutable_result();
    rc->set_errcode(0);

    User *user1=rsp.add_frined_list();//使用这个函数，自动在列表中添加一个好友指针，然后返回指针，这时候就可以通过指针这是该好友的变量
    user1->set_name("zhangsan");
    user1->set_age(11);
    user1->set_sex(User::MAN);

    User *user2=rsp.add_frined_list();
    user2->set_name("刘欢");
    user2->set_age(25);
    user2->set_sex(User::MAN);

    std::cout<<rsp.frined_list_size()<<std::endl;//获取好友个数

    int number=rsp.frined_list_size();
    std::string send_str;
    if(rsp.SerializeToString(&send_str)){//序列化
        std::cout<<"sueecss!"<<std::endl;
    }

    //反序列化
    GetFriendListsResponse reqB;
    if(reqB.ParseFromString(send_str)){
        for(int i=0;i<number;i++){
            User user=reqB.frined_list(i); //调用变量名直接返回该变量的值，可以加索引
            std::cout<<user.name()<<std::endl;
        }
    }
    return 0;
}



int main1(){
    //封装了loginrequest对象
    LoginRequest req;
    req.set_name("liuhuan");
    req.set_pwd("123456");
    //序列化loginrequest对象数据
    std::string send_str;
    if(req.SerializeToString(&send_str)){//序列化
        std::cout<<send_str.c_str()<<std::endl;
    }
    //反序列化一个loginrequest对象数据
    LoginRequest reqB;
    if(reqB.ParseFromString(send_str)){
        std::cout<<reqB.name()<<std::endl;
        std::cout<<reqB.pwd()<<std::endl;
    }

    return 0;
}