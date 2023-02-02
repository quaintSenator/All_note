#include <iostream>

class Base{
protected: double p = 5.0;
public:
       virtual double price(int n) const{
              return n*p;
       }

};
class Deprive:public Base{
public:
       double price(int n)const override;
private:
       int min_discount_num = 3;
       double discount = 0.7;
};
double Deprive::price(int n)const{
              if(n>=min_discount_num)return discount*n*p;
              else return n*p;
       }
int main(){
       Base* basep=new Deprive();//基类指针动态绑定派生类对象
       std::cout<<basep->price(4)<<std::endl;  //输出14.00
       std::cout<<basep->Base::price(4)<<std::endl; //输出20.00
}