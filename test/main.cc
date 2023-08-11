#include "iostream"
using namespace std;


class base{    //定义一个简单的父类
public:
void print(){    //简单的打印函数
cout << "base print" << endl;
}
};


class child:public base {
public:
void print(){  //重写父类print函数
cout << "child print" << endl;
}

void print(int a){     //重载print函数
cout << "child print a=" << a << endl;
}
};




int main()
{

child c1;

c1.print();

c1.print(3);
//其实是可以重载父类中的函数，编译器并没有报错

system("pause");
return 0;
}