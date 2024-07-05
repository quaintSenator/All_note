[toc]
# Learn Lua
## Lua元表
Lua元表是Lua非常重要且强大的特性。在Lua实战中，几乎所有概念都会最终变成Table，比如用Table实现元类，从而在Lua中使用面向对象特性进行开发。Table仅仅是键值对的顺序表，而缺乏一种规定表的行为的方法。为了弥补这点不足，为Lua编程增添灵活性，引入了Lua元表。

元表归根到底也是表。我们将元表附加到表上，如下：
```lua
aMetaTable = {}
aTable = {}

aTable = setmetatable(aTable, aMetaTable)
```

元表内有一些保留键，这些保留键用来说明当附带该元表的实际表出现key miss时的行为。这种保留键被称为**元方法**，总是以双下划线为名字起始。

### 元方法 __index
__index是键值缺失后的查询逻辑元方法。
```lua
local t = {qa = 1, be = 2}

local oneMetaTable = {
    __index = {
        other = 4
    }
}
t = setmetatable(t, oneMetaTable)

for k, v in pairs(t) do 
    print(k.." "..v)
end
print(t.other)
print(t["other"])
```
输出：
```
qa 1
be 2
4
4
```
像这样，我们就可以设计一类*第二级的键值对*，这些键值对在迭代器遍历中并不会显示，但是精准键访问却能命中。

__index更为强大的一个设计在于我称之为**查询转发**的特性。如果对于table t 的key的查询miss，转进t的元表，而__index又是一张表（在上面的例子中__index是函数），将会进入一个循环：查询t的元表mt是否具有key，有则返回；如果没有，而mt的元表又有__index，那么将会进而查询mt的元表mmt是否有key，如此循环。这个特性非常强大，我们在后面Lua面向对象中就会用到。

__index可以是一个表，那么被转发后将会查询这张表；也可以是一个函数，两个参数分别是调用table和查询目标key。比如，我们希望完成这样一个功能：

顶层程序员可以通过`mgrs.window`来使用名为WindowManager的类，由`mgrs.event`来使用名为EventManager的类。但我们又不希望直接设计一个Managers.Lua，其中没有任何类对象，直接堆放mgrs的所有成员：
```lua
    mgrs = {}
    mgrs.window = require("app.managers.WindowManager")
    mgrs.event = require("app.managers.EventManager")
```
这个代码至少有这样一些严重问题：
1. 在项目启动时，mgrs的所有Manager类就必须被装入了，启动时会花费更长的时间；
2. 如果Managers越来越多，体量越来越大，Manager的所有行为和数据都要被装入内存，且不论项目如何运行都要一直维持这些内存不得释放。mgrs变成一个巨大的全局表。

于是我们设计一层路由，为mgrs设计一个元表Managers，并规定如何由mgrs.xx访问到具体的Manager类
```lua
local paths = {
    "app.EventManager",
    "app.WindowManager",
    "app.FoodManager",
    "app.module.InnerManager"
}
-- 随便写的切割字符串工具，细节不到位
function split(str, targetChar)
    local tbl = {}
    local l = 1
    local r = 1
    if #str <= 2 then
        return tbl
    end
    for r = 2, #str do
        if string.sub(str, r, r) == targetChar then
            local subs = string.sub(str, l, r - 1)
            tbl[#tbl + 1] = subs
            l = r + 1
        end
    end
    r = #str
    tbl[#tbl + 1] = string.sub(str, l, r)
    return tbl
end
local class2Path = {}
for _, path in ipairs(paths) do
    local split_list = split(path, ".")
    local cls = split_list[#split_list]
    class2Path[cls] = path
end
-- InnerManager->app.module.InnerManager   for example

local Managers = {
    __index = function(table, index)
        local mgr = rawget(table, index) -- 如果能查到，直接把mgrs[index]返回
        if not mgr then
            --改大写
            local class = string.gsub(index, "^[a-z]", string.upper)
            local path = class2Path[class]
            if not path then 
                path = class2Path[class.."Manager"]
            end
            -- mgr = require(path).new()
            -- 这里为了快速展示效果，直接print
            print("requiring " .. path)
            rawset(table, index, mgr)
        end
        return mgr
    end
}

mgrs = {} -- _G 全局量
setmetatable(mgrs, Managers)
------
-- 使用mgrs的其他区代码
local h = mgrs.window
```
输出：
```
requiring app.WindowManager
```
在这个过程中，mgrs.window未能命中，触发了元表的Managers.__index(table, index)元方法，table = mgrs，index = "window"；在这之后，mgrs中就新增了一项记录`["window"->WindowManager Instance]`,从而在未来mgrs.window访问时，不再需要触发元表的__index元方法。时间上来看，这套方法是即用即取而不是在启动时洪泛装载，开销均匀地分布在程序运行全周期里；从空间上来看，mgrs占用了全局空间，但是未使用到的pairs不会出现在内存里，也比之前的方法更实惠。

这个功能实现后，我们就能以mgrs.xx 直接访问到一个创建好的Manager类，而且从Manager模式的设计初衷来看，Manager应当是全局单例的。

### 元方法 __newindex
另一个元方法是`__newindex`，与`__index`类似，但是只在改写值的调用miss时才会执行。事实上，元表在更底层具有更高的执行优先级，**当发生table赋值访问miss，先查询元表__newindex，依然miss才会执行插入**。这就带来了一个潜在的编程错误，我们不能在__newindex函数体内用下标直接改写新值，像这样：
```lua
local oneMetaTable = {
    __index = {
        other = 4
    },
    __newindex = function(table, key, value)
        print("executing __newindex function")
        --注意，这里的写法错误
        table[key] = value
        --注意，这里的写法错误
    end
}
```
这个程序将不断打印executing __newindex function，直至stack overflow。Lua是一种解释型语言，天然缺乏一种编译时的错误发现逻辑，请务必注意。

为了解决这个问题，我们应该用更底层的内存写入命令来对表内特定位置写值。
```lua
local oneMetaTable = {
    __index = {
        other = 4
    },
    __newindex = function(table, key, value)
        print("executing __newindex function")
        rawset(table, key, value)
    end
}
```
### 类似运算符重载的元方法
下面这段程序展现了对应+运算符的元方法__add。在这当中，体现出几个重要的原则：
1. 当两个table被执行a+b,会检测双方的元表情况，首先检查+的左值a，如果发现a元表具备__add行为，那么直接执行；如果未能发现a.__add，就去搜寻b.__add。

2. __add行为中必须返回一个table

3. table.insert依然会触发__newindex

```lua
local t = {"important", "significant", "critical", "mattered"}

local oneMetaTable = {
    __index = {
        other = 4
    },
    __newindex = function(table, key, value)
        print("executing __newindex function")
        rawset(table, key, value)
    end,
    __add = function(lhs, rhs)
        print("executing __add function")
        if(type(lhs) ~= "table" or type(rhs) ~= "table")
        then print("bad add")
            return
        end
        for i = 1, #rhs do 
            table.insert(lhs, #lhs + 1, rhs[i])
        end
        return lhs
    end
}
t = setmetatable(t, oneMetaTable)
a = {"q","u","i"}
a = setmetatable(a, {
    __add = function(lhs, rhs)
        print("__add function hazard!! a.__add wins")
        return lhs
    end
})
t = t + a
a = a + t

for k, v in pairs(a) do 
    print(k.." "..v)
end

for k, v in pairs(t) do 
    print(k.." "..v)
end
```
输出如下：
```
executing __add function
executing __newindex function       
executing __newindex function       
executing __newindex function       
__add function hazard!! a.__add wins
1 q
2 u
3 i
1 important
2 significant
3 critical
4 mattered
5 q
6 u
7 i
```
类似与add这样可以通过元方法重载的运算符都是双目运算符，它们包括：
|元方法|对应算符|
|--|--|
|__add|+|
|__sub|-|
|__mul|*|
|__div|/|
|__mod|+|
|__unm|+|
|__concat|..|
|__eq|==|
|__lt|<|
|__le|<=|

### 元方法 __call
与call 函数调用的含义相似，用括号方式执行函数调用功能。

```lua
    
```
## Lua模块
Lua从5.1开始引入了模块，

## Lua面向对象
*Lua中本没有类；用的人多了，便有了类*

在Unity+Lua的开发流程中，开发者们终于不再满足于过程式的开发，分别实现了Lua的一套面向对象机制。

Lua中，函数是第一等公民，而Lua Table又可以接纳Function作为表项的值，用Table可以非常轻松地写出一个简单的代码例子：
```lua
-- bad code 1
Rectangle = {fieldA = 1, fieldB = 2}
function Rectangle:print()
    print(self.fieldA .. self.fieldB)
end
function Rectangle:new(A, B)
    local obj = {}
    obj.fieldA = A
    obj.fieldB = B
    return obj
end

local r = Rectangle:new(1, 5)
r:print()
```
这段程序是无法执行的，我们可以想见，在构造器部分，obj变成了有两个键值对的表格，但并没有一个名为print的key对应print函数的执行逻辑，因此r:print() 会报：`attempt to call a nil value (method 'print')`

为了解决这个问题，有一个刻舟求剑的做法：

```lua
-- bad code 2
Rectangle = {}
function Rectangle:new(A, B)
    local obj = {}
    obj.fieldA = A
    obj.fieldB = B
    obj.print = function()
        print(obj.fieldA .. obj.fieldB)
    end
    return obj
end

local r = Rectangle:new(1, 5)
r.print()
```

这样一来面向对象是无从谈起的：我们不可能放弃编写函数，转而在构造器里写大量的function并就地绑定进表格,即便忽略编码的不方便，更大的问题是，函数不应当在内存中被反复拷贝，如此的类Rectangle的print函数在每次new都拷贝一片print代码，这简直是最坏的面向对象设计。

Lua的元表机制是Lua面向对象拓展的关键。

同样地，我们希望设计类Rectangle，那么Rectangle是一个元表，用于记录obj应当有哪些元素和行为；其new出来的对象obj，将其记录为一个table。

```lua
-- bad code 3
Rectangle = {}
function Rectangle:print()
    print(self.fieldA .. self.fieldB)
end
function Rectangle:new(o, A, B)
    o = o or {}
    setmetatable(o, self)
    return o
end

local r = Rectangle:new()
r:print()
```
这段代码依然无法执行，r:print()找不到对应的行为。就目前代码而言，`Rectangle.print`会被正确识别为函数，但`r.print`是nil。为了通过对象访问类的方法，我们需要加上一句关键代码：
```lua
-- bad code 4
Rectangle = {}
function Rectangle:print()
    print(self.fieldA_ .. self.fieldB_)
end
function Rectangle:new(o, A, B)
    o = o or {}
    setmetatable(o, self)
    self.fieldA_ = A or 0
    self.fieldB_ = B or 0
    self.__index = self
    return o
end

r = Rectangle:new()
r.print()
```
之前讲到过，`__index`能够做到逐级向深处查询。若表有key，直接访问；若表没有key，查询__index所载内容，若载有一张表，再对那张表查询key。`self.__index = self`就表达了这样的逻辑：Rectangle对象有任何查询不到的键，来查询元表Rectangle。这完美地复现了面向对象的精神。

但是注意到，这还是一段bad code。r.print()报错。说到这里，就不得不提一下Lua当中的.和:的差别了。

### Lua. vs Lua:
首先，两者都表达一种希望对表t查询k键的愿景：
```lua
r.print --type是函数
r:print() --不能取type
```
然而，r.print虽然能正确拿到一个返回值，在左值接住可以拿到目标函数，但是与那个函数绑定的self却不能传递：
```lua
local a = r.print
print(type(a)) -- function
a() --a执行栈内，self无法识别
Rectangle:print() --如果此前逻辑中有执行过Rectangle:new，则能够执行，否则self不识别
```

那么我们不得不进一步谈一谈self这个关键字了。

第一，冒号语法是一种语法糖，以下两种写法是完全等效的：
```lua
function C:foo(a) 
end
tbl:foo(a)

------
function C.foo(self, a)
end
tbl.foo(tbl, a)
```
第二，在函数体内使用self关键字，这里的self指代运行时传递给该函数的第一个参数。也正因此，Lua面向对象代码的构造器第一个参数永远是self。

因此，我们之前报错的代码中，运行时调用r.print没有向print传递任何参数；而print内部使用了self，也就需要至少一个参数的传入。只要改为r:print就能正确输出了。

我们只要记住：尽量用冒号来访问所有的成员函数。如果试图用点号来访问，则必须特别注意目标函数是否使用了self，如果使用了，其隐含地拥有第一个参数self，必须将自己再传给参数表。
```lua
-- good code 4
Rectangle = {}
function Rectangle:print()
    print(self.fieldA_ .. self.fieldB_)
end
function Rectangle:new(o, A, B)
    o = o or {}
    setmetatable(o, self)
    self.fieldA_ = A or 0
    self.fieldB_ = B or 0
    self.__index = self
    return o
end

r = Rectangle:new()
r:print()
```

### 较好的Lua类代码
在之前的代码中我们不难看出，总是要在Rectangle的new函数之前进行一些Rectangle表的设置。作为类的使用者，我们希望直接定义类后就可以使用类，而不是手动去新建一个表格。为此定义class函数：

```lua
function class(classname)
    --默认构造函数
    local cls = {}
    cls.ctor = function() end
    
    cls.__cname = classname
    cls.__index = cls --表查询转发给元表
    function cls.new(...)
        local instance = setmetatable({}, cls)
        instance.class = cls
        instance:ctor(...)
        return instance
    end
    function cls.print(self)
        print(self.__cname .. " print")
    end

    return cls
end

Myclass = class("Myclass")
local a = Myclass:new()
a:print()
```
这段代码是一个基本的类规约，其中function cls.new(...)这种内部函数的写法于向cls表的key=new项写入一个函数无异：
```lua
    cls["new"] = function(...)
        local instance = setmetatable({}, cls)
        instance.class = cls
        instance:ctor(...)
        return instance
    end
    --cls.new = function(...) end 也可
```
其中,在new的执行期中，`Myclass:new()`=>`Myclass.new(Myclass)`
执行到`instance:ctor(...)`等同于`instance.ctor(instance,...)`,而...此时传入了Myclass，即为
`instance.ctor(instance, Myclass)`。instance并没有ctor键，转向元表查询，查到Myclass有名为new的键，从而执行。在new函数体内，cls就是Myclass。

### 浅谈Lua GC 和 类的规范
计算机语言之间流行的内存回收有两套不同的策略，引用数监测与广义GC。引用数检测即失去最后一个引用的内存被回收；Lua没有采用这种策略，这是因为动态类型语言的引用监测开销不小，即便没有分配内存也需要监测引用。

Lua中的一切对象都是GC的对象，除了注册表registry和共享元表shared metaTable。注册表也是一张表，存放着全局表_G,主线程main thread 和 package.loaded

LuaGC在5.0和更早版本中，遵循mark and sweep过程。在Lua中所有对象都被放在一张列表(object gragh，对象图，实际上是顺序表)中，Mark过程就是遍历整个表，标记活对象(live object)。Sweep就是再次遍历全表，然后删除一切没有被标记为活对象的内存。执行GC的推荐时机是当内存使用量达到上次清扫结束后内存的两倍的时候。老GC方案的弊端被称为stop the world：程序暂停来等待GC完成，影响了性能。

5.1起的Lua GC变得复杂了许多，一般称为三色GC或者incremental collection方案。简单来说，垃圾回收器和主程序变成了交替执行的两套程序；GC中mark阶段不再简单把物体分为live和dead，而是分为三种颜色：近期没有访问的object标记为白色；访问过但是本次遍历没有走到的对象标记为灰色；被遍历到的对象标记为黑色。也就是说，除了GC的mark步骤，就连普通的赋值语句也能改写对象的GC颜色，从而影响对象是否在接下来的一次GC中被删除。

在传统面向对象语言，比如C++中，成员是在类声明期确定的。C++类一旦被声明，就不能再对其中增添更多成员。Lua类本身是一个Table，对象化后，**并没有一种机制限制Lua类的成员列表更改**。也就是说，任何类的成员函数可以在执行期往Lua类中添加新的成员。同时，Lua类代码也并没有一套生命周期机制，类里面并没有C++的析构函数。目前Lua开发者对于LuaGC过程的探究是普遍不足的，基本采取一种完全信任Lua GC的态势。

在UnityLua开发中，所有的Lua脚本最终都被转换成了C#脚本，并由C#进行内存管理。

## Lua GC
在这里我们系统性地讨论LuaGC。总体而言，即便是5.1后最新的Lua，垃圾回收器Garbage Collector
//似乎没有必要讨论GC，因为ToLua会全部将Lua转换成C#
# Learn ToLua
