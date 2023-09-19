# CS内存布局
## 值类型和引用类型
**值类型**继承自System.ValueType，所有的值类型都是隐式密封的。值类型包括int、bool、float、char等基元类型以及结构体和枚举。内存一搬分配在栈上，赋值或者传参的时候会发生复制。我们也可以定义自己的struct，也会是值类型，但是这样的值类型的拷贝将会造成比内置类型更大的开销，应该谨慎对待。

**引用类型**：包括类、string、委托、数组和接口，内存一般分配在堆上，赋值或传参的时候只复制引用（地址）。

## 堆和栈
C#的内存分区基本只分为堆和栈
1）栈：栈是一片连续的内存域，由系统自动分配和维护，大小固定（默认1M，可配置）
内存分配效率高（速度快），栈顶元素使用完毕，立马释放。
2）堆：堆内存是无序的，内存大小动态分配（有上限）。所有的对象都从托管堆分配，CLR维护了一个NextObjPtr指针，指向下一个对象在堆中的分配位置。
当程序需要更多的堆空间时，由GC（垃圾回收机制）帮助我们清理内存。内存分配效率和速度都较低。

## 装箱和拆箱
装箱和拆箱，箱子始终都指的是引用，也值得是变量对应的堆内存。
1）装箱：将值类型转换为引用类型的过程。装箱时，首先在托管堆中分配内存；然后将值类型的字段复制到新分配的堆内存；最后返回对象引用。
频繁装箱可能会触发GC，造成卡顿（GC会将所有线程挂起）
2）拆箱：将引用类型转换为值类型的过程。拆箱时，先获取已装箱对象中各个字段的地址；然后将字段包含的值从堆内存复制到栈上。

有的时候为了方便，可能会把值类型转变成object，或者反过来。转成object就是装箱，而object到值类型就是拆箱

## Finalize
Finalize方法 也就是C#析构函数。当没有在代码中显式调用时，编译器会自动调用此方法。

## 托管内存和非托管内存？

## IDisposable接口的作用？
我们在C#中有多种回收内存的方案，大多数时候我们什么都不做等待GC替我们回收。

IDisposable接口携带一个Dispose方法，用于执行资源清理和释放操作。我们需要手动进行回收的动机在于：
1. 确实应当立即回收，尽管该对象可能还被许多地方引用。这种情况下，我们大可以立刻进行强制的回收，也就是调用Dispose
2. 使用了一些非托管类型的数据，比如handle句柄。此时应该显式地对非托管数据的引用调用Dispose。由于操作系统给每个进程都设定了句柄上限，若不手动及时Dispose这些handle将存在发生句柄耗竭handles exhausted的风险
```cs
try
{
    // 创建FileStream对象
    fileStream = File.Open(filePath, FileMode.Open);

    // 使用fileStream进行文件读取或其他操作
    // 执行完文件操作后，显式调用Dispose()方法释放文件句柄
    fileStream.Dispose();
    fileStream = null;
    Console.WriteLine("File handle released.");
}
catch (Exception ex)
{
    Console.WriteLine("An error occurred: " + ex.Message);
}
```

此外，还有一种using写法，可以自动调用Dispose
```
using ( var obj = new SomeIDisposableObject ) 
{
    // ...
} 
```
一旦语句块结束，就能自动调用Dispose
# 权限和封装

# C#容器
## Array和List的区别？

# C#类型
var 和 dynamic

# 继承与多态
## 类和方法的访问权限
C#类和方法的访问级别限制
类：
类的访问修饰符有**public**、**internal**，缺省时为internal
internal限制类在本项目中访问，不能被其他项目访问
public使得类可以在其他项目中访问，但是先要添加对该类所在项目的引用，并且using该类的命名空间；

方法：
方法的访问修饰符有private、protected、internal、public、protected internal，缺省时为private
private限制方法在本类中访问，不能在本项目的其它类和外项目访问

protected限制方法在本类和其子类，包括外项目的子类中访问（直接调用，无需实例化）；不能在本项目和其他项目的无继承关系类，以及通过实例调用访问

internal限制方法在项目内部所有的类访问，不能被其他项目访问

public使得方法可以在其它项目中访问，但是先要添加对该方法所在类所在项目的引用，并且using该类的命名空间；

protected internal使得方法可以在本项目所有的类和外项目有继承关系的子类中访问，实际是取了两者限定范围的并集，注意不是交集

## C#字符串和Int的互转
1. string->int
   可以调用`int.Parse(string str)`, 输入一个string，返回一个int。如果string不是int合法的表示形式，抛出FormatException异常。`int.TryParse(string str, out number)`返回一个bool值来表示转换成功与否，也是一样的功能
2. int->string
   调用`int.ToString()`,返回一个字符串

## string和stringBuilder区别？
在C#中，string是不可变的，也就是说每次写`s = s + "hello"`实际上都是构造一个新的对象，而如果连续使用，中间每一步都会生成临时变量从而造成很大的开销
StringBuilder在内部维护了一个可变的字符串缓冲区，事实上更像C++string，其避免了频繁的字符串拷贝

重载和重写的区别，什么叫多态？

泛型委托方式有哪几种？

## 虚方法和抽象方法

## const和readonly的区别

## 有偏类partial和密封类sealed

## 依赖注入

## System.Array.CopyTo() 和 System.Array.Clone()的区别
使用 Clone() 方法，我们可以使用 CopyTo() 方法创建一个包含原始数组的所有元素的新数组对象。现有数组的所有项目都复制到另一个现有数组中。两种方式都会创建一个浅拷贝。


