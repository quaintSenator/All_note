
# 1.C++ stl 顺序容器
顺序容器是c++三类容器中最重要的、更入门的一类容器，其应用场景最为广泛，也最为初级，在面试和开发中都有极其重要的地位。
## 顺序容器一览
| 顺序容器名称 | 特点                                                                 |
| ------------ | -------------------------------------------------------------------- |
| vector       | 可变大小数组。支持ra。在**尾部之外**插入删除可能**很慢**。           |
| deque        | 双端队列。支持ra，在**头尾**插入和删除都**很快**                     |
| list         | 双向链表。只能支持双向ra。在list的**任何位置**插入和删除**都很快**。 |
| forward_list | 单向链表。只支持单项ra。在链表**任何位置**插入和删除**都很快**。     |
| array        | 固定大小数组，支持快速ra，不能添加或删除元素。                       |
| string       | 与vector很像，在尾部快，其他地方慢。                                 |
（建议回答面试问题时直接说list和forward_list不支持随机访问，否则解释起来很麻烦）

特别讲一下list和vector的区别，这也是我一面碰到的问题。从选用什么容器的角度看，list的优点是增删快，vector的优点是读取快。具体选择哪个，应该看程序对于容器更主要的操作是读取还是增删。

所有C++容器都是模板类，必须声明容器中的单位元素是什么类型：`list<int>`

## 容器类型成员
容器类型成员是容器自行定义的几个类型。其中比较流行的是iterator，const_iterator,reference,size_type等。
```cpp
string s;
return s.size();
```
在上述代码中，return值可能被当做int去处理和使用。但事实上size()函数的返回值是size_type类型。其实按照其实际范围，size_type完全可以定义成unsigned int,但这不利于一些通过判断数值的类型产生逻辑的程序。

value_type是一个类型别名，它会被编译器解析成容器的元素类型值。

## 容器初始化
- 默认构造器。`C c;`
- 拷贝构造器。`C c1=c2;` `C c1(c2);`
- 列表初始化。`C c={a,b,c};` `C c{a,b,c};`
- 迭代器区间初始化。 `C c2(c1.begin(),c1.end());`
- 依据大小初始化。 `C seq(n);` `C seq(n,t);`

还有一个词汇叫容器元素**值初始化**，其含义是容器内每个值都不确定，但依然默认地生成了一些值。int元素会被生成成0，string会被生成成""。

使用另一个类的迭代器来初始化也是合法的，前提是这两个类的元素类型要么相同、要么有可用的类型转换。

```cpp
vector<const char*> article={"Milton","Sharkspeare","Austen"};
forward_list<string> words(article.begin(),article.end());
```

## 特殊的顺序容器：array
std::array 是一种固定大小的容器，和其他的几个顺序容器不同，其声明时就必须给出两个类型部件：1是元素类型，2是数组长度。
`array<int,42>` 
array与内置数组有这样一些差别:
- 内置数组不能接受另一个数组的拷贝和对象赋值，而array可以。
```cpp
int a[]={1,2,3};
int a2[]=a;//错误
array<int,10>ar={1,2,3,4,5,6,7,8,9,10};
array<int,10>copy=ar;
```
## 容器的=，assign,swap & 关系运算符
若将一个容器赋值给另一个容器，被覆盖的容器不但所有元素都被替换，而且容器大小也会改变，成为一个完全彻底的复制。

assign也是一个用于完成赋值操作的函数，类似一个带参数的=。
允许在不同类容器之间使用assign进行赋值。
```cpp
list<string> name;
vector<string> oldstyle;
name=oldstyle;//不允许，类型判断不通过且没有可用的类型转换
names.assign(oldstyle.cbegin(),oldstype.cend());//合法
```

`assign(10,"hello")`就像构造器一样赋值给容器。

不能在assign里填写调用者本身，因为本身的元素已经被替代了：
```cpp
vec.assign(vec.begin(),vec.end());//违法
```

swap能够置换两个容器的内容，且耗时O(1).这是因为swap并不是机械地逐个替换两个容器内的每个元素，而是直接置换两个容器的内部数据结构。
对于两个容器内任何元素求地址，会发现在swap前后地址不会改变，从而任何指向他们的iterator也不会改变。

对string调用swap会破坏其迭代器、指针和引用。

swap 两个array会机械地逐个替换两者的全部元素。

容器的**关系运算符**主要是`> < >= <=`,而无序容器不支持。

只能比较相同类型且相同元素类型的容器。比较的逻辑如下：
- 互为完全拷贝的容器相等，即长度相同，每个元素顺序都相同。
- 如果size不相同，但是较短的容器是较长容器的前缀，那么较短容器小于较大容器。
- 如果互不为前缀，那么必有第一个两侧不等的元素。返回这两个元素的比较效果，比如{1,3,4}<{2,1}

string也是顺序容器，因此也遵循上述的比较原则。

如果容器内的元素没有重载> <运算符，则容器也不能使用关系运算符。

## forward_list的特殊性
forward_list和其他顺序容器的删除行为不同。
正常的顺序容器`c.erase(begin,end)`，删去begin到end范围内的所有元素，返回一个迭代器，其指向被删除元素之后的第一个元素。如果被删除后已经没有更后的元素，则返回end()结果。`c.erase(it)`删除it指向的元素，并返回删除后的第一个元素。

如果模仿这个过程，forward_list将会造成非常多额外的开销。要想删除it位置的元素，对于单向链表，必须修改it位置的前一个元素，让前一个元素的后继指向it的后继。然而，拿到参数it，要想找到it的前驱要耗O(n)时间。

因此forward_list定义了insert_after和erase_after，拿到it后直接改写it即可。为了支持这些函数，forward_list还定义了before_begin迭代器，这样一来如果insert_after(before_begin)就相当于insert(begin)，也就是在首元素之前插入或删除。

## emplace()
是c11新特性。现在容器能够支持emplace_back()，一个push_back的替代。不同在于，emplace进行元素的构造，而不是元素的拷贝。

为了使用push_back(),我们常常需要首先在另一个内存位置创建一个新的对象，然后把这个对象拷贝进容器：
```cpp
myclass a();
avec.push_back(a);
```
很多时候，我们完全不会去在意这个a所占据的空间之后怎么样了——这事实上在庞大系统中增加了潜在内存问题。
`emplace()`就是这样的一个优化，其希望直接找到要插入的位置，在那个空间直接构造对象。

```cpp
class my
{
public:
    string name = "";
    mutable int a = 5;
    int b = 10;
    my(string n, int aa, int bb) { name = n, a = aa, b = bb; };
};
int main()
{
    vector<my> myvec;
    myvec.emplace_back("hello", 3, 5);
}
```
## resize()
array之外的顺序容器都支持resize()。resize更大size，会补充元素；更小size则会删掉超出size范围的元素。

内存管理的一个重点是：**避免iterator，引用和指针指向被销毁的元素。**

## vector的二倍扩容特性和容量管理
由于顺序容器，尤其是其中最流行的vector,往往要频繁地进行push_back()与pop_back()。因此，如果每次插入一个元素，就要重新进行一次内存分配或释放，无疑将造成很大的不必要开销。

为了加速提效，vector采用双倍扩容特性。
| 函数/容器支持情况 | vector | string | deque |
| ----------------- | ------ | ------ | ----- |
| shrink_to_fit()   | √      | √      | √     |
| capacity()        | √      | √      |       |
| reserve()         | √      | √      |       |

| 函数名          | 描述                                                                                             |
| --------------- | ------------------------------------------------------------------------------------------------ |
| shrink_to_fit() | 把capacity缩水到size大小                                                                         |
| capacity()      | 返回capacity，即在重新分配内存之前可以容纳的大小                                                 |
| reserve()       | 分配至少能容纳n个元素的内存空间。如果本身capacity>=n，不做任何事；如果capacity不足，才会进行扩容 |

事实上，这三个函数都是用于维护一个名为capacity的容量值。顺序容器总是在维护一个大于size的容量。resize仅仅修改size、删除或补足元素，而不直接影响capacity。

vector总是在size=capacity+1的时候，立刻将capacity翻倍来扩容。

## vector 扩容同时还发生了什么
一个问题：既然vector可能多次扩容，那么vector后面的下一个变量应该存放在什么位置？如果vector长到即将占用下一个变量的位置，该怎么办？
```cpp
int main()
{
    vector<int> v1;
    vector<int> v2;

    std::cout << "&v1=" << &v1 << std::endl;
    std::cout << "&v2=" << &v2 << std::endl;
    std::cout << "&v1[0]=" << &v1[0] << std::endl;

    v1 = vector<int>(32, 1);
    v2.push_back(1);
    std::cout << "&v1=" << &v1 << std::endl;
    std::cout << "&v2=" << &v2 << std::endl;
    std::cout << "&v1[0]=" << &v1[0] << std::endl;
    std::cout << "&v2[0]=" << &v2[0] << std::endl;
}
```
輸出結果：
```
&v1=0x61fdc0
&v2=0x61fda0
&v1[0]=0
&v1=0x61fdc0
&v2=0x61fda0
&v1[0]=0xd834a0
&v2[0]=0xec6fa0
&v1[16]=0xd834e0
```
首先，不存在vector本身地址增长，&v1和&v1[0]是两个不相关的地址，只是我们可以从v1的底层成员查找到v1[0]的位置，且这个查找的过程被vector本身封装。

其次，v1[0]和v2[0]在开始时都是nullptr，直到容器中有数据了才会为其分配一个地址。可以看到尽管程序上v1和v2是紧邻的，但是空间上他们相差了超过16^4(64K)个字节位置，如果要用int去填满v1[0]和v2[0]之间的空隙，则不得不为v1填入这么多个整数才行。

vector的内存分配遵循动态内存管理。

我又做了一个实验，实验的结果非常的离谱。我试着算出两个vector初始指针位置的差值，比如是一个六十多万的量，之后我给这个v1插入这么多的数，尝试让v1的end覆盖掉v2.
```cpp
int main()
{
    vector<int> v1(1, 1);
    vector<int> v2(1, 1);

    int len = &v2[0] - &v1[0];
    std::cout << len << std::endl;
    std::cout << "&v1[0]=" << &v1[0] << std::endl;
    std::cout << "&v2[0]=" << &v2[0] << std::endl;

    if (len > 0)
    {
        // v2地址大，v2在后
        for (int i = 0; i < len; i++)
        {
            v1.push_back(1);
        }
    }
    else
    {
        len = -len;
        // v1地址大，v1在后
        for (int i = 0; i < len; i++)
        {
            v2.push_back(1); // v2倒追v1
        }
    }

    v1.push_back(3);
    std::cout << "&v1[0]=" << &v1[0] << std::endl;
    std::cout << "&v1[v1.size()-1]=" << &v1[v1.size() - 1] << std::endl;
    std::cout << "&v2[0]=" << &v2[0] << std::endl;
    std::cout << "&v2[v2.size()-1]=" << &v2[v2.size() - 1] << std::endl;
}
```
输出
```
-28
&v1[0]=0x6b6fa0
&v2[0]=0x6b6f30
&v1[0]=0x6d34a0
&v1[v1.size()-1]=0x6d34a4
&v2[0]=0x6d34f0
&v2[v2.size()-1]=0x6d3560
```
v1和v2的初始地址竟然都移动了，最终完美化解了这个问题。最一开始的时候，`&v1[0]=0x6b6fa0`,`&v2[0]=0x6b6f30`,如果按照我的设计，那么v2会补充28个元素，从而追上v1.结果，现在我们看到v2移动到了一个安全的位置，移动到了v1的后面。


1.vector不是谁先声明谁拿v[0]、谁拿的v[0]小。有时候后声明的vector地址反而小。
2.push_back不仅可能需要扩容，有时候还需要移动。

我还注意到，有的时候v2转移后，其地址还会多出来一位
```
&v1[0]=0x776fa0
&v2[0]=0x776f30

&v1[0]=0x776fa0
&v1[v1.size()-1]=0x776fa0
&v2[0]=0x26a34f0
&v2[v2.size()-1]=0x26a3560
```

# 2.C++ stl 关联容器
关联容器指stl下的set, multiset, map, multimap; unordered_set, unordered_map;
前4个被称为有序容器，而后两个被称为无序容器。
有序容器，不论是map还是set，都必须有**可比较**的key。以set为例，其可以使用int, float, string作为key，这是因为他们都有compare函数。

可以使用一个自定义的类作为有序容器的key，但必须自己定义好类的compare函数，并在声明时指出。

# 3.C++ stl 无序容器

# 面试问题集

