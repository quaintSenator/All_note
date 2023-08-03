[TOC]


# Unity JobSystem
对于UnityJobSystem的了解最先开始于一次关于CPU模拟软渲器的探讨。大多数的软渲器遍历屏幕来绘制图形，这样做非常慢。
尽管事先知道JobSystem依赖多线程，如果运用起来软渲的过程会被优化的非常快，但是想来我的计算机核心数目也并不多，就算能核心数目倍地优化，我想渲染这个过程依然会很卡。

但实际上，JobSystem能让一遍渲染在1ms左右完成。是的，CPU渲染，1ms左右。

JobSystem的历史可以追溯到18年左右，现在的JobSystem已经是一个全盛姿态。

Unity DOS，Data Oriented Stack面向数据的技术栈，主要包括三个重要的内容:
- JobSystem 作业系统
- Entity Component System 实体组件系统
- Burst Compiler 爆发式编译器


## 多线程
多线程是一个非常基础的计算机知识。在多核CPU电脑占压倒性主导地位的今天，线程切换频繁地发生。

在大多数操作系统中，程序对应的进程开始时候只有一条线程，这被称为主线程。大量的程序可以在主线程下良好地运行而不需要引入多线程。然而有一些场景我们不得不考虑使用更多的线程：
- 网络游戏客户端不被阻塞地与服务端通讯，即便收包乃至处理，也不必停止游戏逻辑的运行
- 读写磁盘，尤其是需要多次运行的读写磁盘

这样看起来就仿佛有两台CPU在同时做两件事情的做法，无疑提升了效率。
然而多线程引入显然也会造成一些问题。作业系统主张以一种比直接使用线程更高效的方法进行多线程编程。

### 多线程风险
多线程带来的最大的成本就是解决数据竞争风险。一个更糟糕的问题是，多线程任务在调试下的表现和运行时的表现是不一样的，因为断点和输出日志可能会打乱线程原有的时序从而无法复现问题。

好在Unity作业系统已经努力地在编译时尽可能检测到潜在的竞争风险。

Unity Job System消除竞争的努力：
- 作业只能得到数据副本
- 向作业传递引用的写法被标记为不安全

这样做的内在逻辑是，如果作业能够直接访问数据引用，也就是修改数据本体，在作业A修改本体的同时，如果作业B又在读取，会引发灾难性后果。

.NET规定了一些类型为Blittable（块传递安全的），而由于上面所说的必须拷贝副本给作业的原因，作业**只能访问 blittable 数据类型**：
- System.Byte
- System.SByte
- System.Int16
- System.UInt16
- System.Int32
- System.UInt32
- System.Int64
- System.UInt64
- System.IntPtr
- System.UIntPtr
- System.Single
- System.Double


## 作业系统设计的初衷

作业这个概念原本是在批处理系统当中出现的，一台计算机被编排作业A B C，一旦运行计算机就一项一项不停止地执行这些作业。这个概念在Unity作业系统当中也保留了，Unity作业用依赖关系描述：
为了绘制一个三角形到屏幕上：
- A作业负责决定当前像素的屏幕坐标
- B作业负责计算像素是否在三角形内
- C作业负责为三角形着色
这三个作业逻辑上是有依赖关系的，也就是一项作业的结果正要用于第二项作业的执行，这种情况下作业之间的关系就是依赖。

作业系统的设计初衷是针对逻辑核心一一对应地派发工作线程，这种允许一个实际物理核心被分拆为多个逻辑核心，并且每个逻辑核心与一个工作线程从而让物理核心同时做到多线程的设计叫做Hyper-Threading超线程。现在的IntelCPU架构定义8核心处理器有16个逻辑核心，因此可以对应16个工作线程。

作业系统把作业丢入Job Queue，随后按照依赖关系决定执行顺序。


## NativeContainer & NativeArray
人们总是贪婪的。人们即想要**实时渲染**，又想要**全局光照**。人们即想要**作业的数据隔离**，又想要**无需拷贝的更高性能**。

NativeContainer是一种托管值类型managed value type。
C#数据类型 = 值类型 + 对象类型
值类型 = 托管值类型 + 非托管值类型

所谓托管值类型，也就是其生命周期几乎全部托管给CLR处理，CLR负责正确初始化和GC这些托管值类型，由于我们最熟悉的值类型都是托管的，很多时候我们都没有注意到，其实并不是所有的值类型都能得到CLR自动GC。

有这样一些值类型不受托管：
- 指针类型（Pointer types）：包括IntPtr和UIntPtr等用于表示指针的类型。

- 值类型封装器（Value type wrappers）：这些类型封装了底层数据结构，例如Decimal、DateTime等。

- 固定大小的数组（Fixed-size arrays）：对于固定大小的数组，CLR无法直接监视其生命周期和释放。

- 位域（Bitfields）：位域是使用特定位数的二进制位来表示一个或多个标志的数据结构。

- 不可变类型（Immutable types）：不可变类型是指其实例在创建后不能被修改的类型，例如**string和DateTime**等。



NativeArray顾名思义为原生数组/本地数组，被设计用于解决多线程场景下的同步问题。

## 最小示例
### 1. 一个最简单的例子
```
using Unity.Collections;
using Unity.Jobs;
using UnityEngine;

public struct MyJob : IJob
{
    public void Execute()
    {
        Debug.Log("Running job...");
    }
}

public class JobExample : MonoBehaviour
{
    private void Start()
    {
        // 创建一个job
        MyJob job = new MyJob();

        // 执行job
        JobHandle handle = job.Schedule();

        // 等待job完成
        handle.Complete();

        Debug.Log("Job completed.");
    }
}
```
在这个例子当中，展现了一个使用Job的基本过程：
- 写一个具体的结构 实现IJob接口
- 在控制线程/主线程中创建Job
- 用Schedule将Job排入Job Queue并返回句柄
- 对句柄调用Complete() 等待任务对应的句柄，即等待job完成

但是这样的job

```
using UnityEngine;
using Unity.Collections;
using Unity.Jobs;

class ApplyVelocityParallelForSample : MonoBehaviour
{
    struct VelocityJob : IJobParallelFor
    {
        // Jobs declare all data that will be accessed in the job
        // By declaring it as read only, multiple jobs are allowed to access the data in parallel
        [ReadOnly]
        public NativeArray<Vector3> velocity;

        // By default containers are assumed to be read & write
        public NativeArray<Vector3> position;

        // Delta time must be copied to the job since jobs generally don't have concept of a frame.
        // The main thread waits for the job same frame or next frame, but the job should do work deterministically
        // independent on when the job happens to run on the worker threads.
        public float deltaTime;

        // The code actually running on the job
        public void Execute(int i)
        {
            // Move the positions based on delta time and velocity
            position[i] = position[i] + velocity[i] * deltaTime;
        }
    }

    public void Update()
    {
        var position = new NativeArray<Vector3>(500, Allocator.Persistent);

        var velocity = new NativeArray<Vector3>(500, Allocator.Persistent);
        for (var i = 0; i < velocity.Length; i++)
            velocity[i] = new Vector3(0, 10, 0);

        // Initialize the job data
        var job = new VelocityJob()
        {
            deltaTime = Time.deltaTime,
            position = position,
            velocity = velocity
        };

        // Schedule a parallel-for job. First parameter is how many for-each iterations to perform.
        // The second parameter is the batch size,
        // essentially the no-overhead innerloop that just invokes Execute(i) in a loop.
        // When there is a lot of work in each iteration then a value of 1 can be sensible.
        // When there is very little work values of 32 or 64 can make sense.
        JobHandle jobHandle = job.Schedule(position.Length, 64);

        // Ensure the job has completed.
        // It is not recommended to Complete a job immediately,
        // since that reduces the chance of having other jobs run in parallel with this one.
        // You optimally want to schedule a job early in a frame and then wait for it later in the frame.
        jobHandle.Complete();

        Debug.Log(job.position[0]);

        // Native arrays must be disposed manually.
        position.Dispose();
        velocity.Dispose();
    }
}
```
这个示例是Unity文档少有的写的很不错的例子。代码注释当中提到一些要点，整理如下：
1. Job要明智地声明自己要用到的量，如果声明一个量是`[ReadOnly]`，那么就可以同时准许大量的Job来读取这个量从而提高了效率
2. Job不适合访问Time.deltaTime，这是因为Time.deltaTime是运行在主线程上的逻辑维护的一个信息，即便让Job去读取这个量也不能保证准确
3. Schedule的参数batchSize限制了把作业绑成Batch的规模，大作业每batch作业少，小作业每batch作业多，因此应该给大作业赋1，小作业赋32或64
4. 可以把一个作业的JobHandle传递给另一个作业，以此形成一个依赖关系
5. NativeArray必须手动Dispose